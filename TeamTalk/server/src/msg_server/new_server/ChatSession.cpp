/**
 * ChatSession.cpp
 * zhangyl, 2017.03.10
 **/
#include "ChatSession.h"
#include <string.h>
#include <sstream>
#include <list>
#include "../net/TcpConnection.h"
#include "../base/AsyncLog.h"
#include "../base/Singleton.h"

using namespace std;
using namespace net;

uint32_t asInt32(uchar_t *buf)
{
	uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return data;
}

ChatSession::ChatSession(const std::shared_ptr<TcpConnection>& conn, int sessionid) :
TcpSession(conn), 
m_nSessionId(sessionid),
{
}

ChatSession::~ChatSession()
{
	std::shared_ptr<TcpConnection> conn = getConnectionPtr();
    if (conn)
    {
        LOGI("remove check online timerId, client address: %s", conn->peerAddress().toIpPort().c_str());        
    }
}

void ChatSession::OnRead(const std::shared_ptr<TcpConnection>& conn, Buffer* pBuffer, Timestamp receivTime)
{
	if (nullptr == conn.get() || NULL == pBuffer)
		return;

	if (pBuffer->readableBytes() < 4)
	{
		LOGE("recv data error, data is incomplete. data len = %d", pBuffer->readableBytes());
		return;
	}

	//收到的数据不够一个完整的包
	int32_t nPkgLen = pBuffer->readInt32();
	if (pBuffer->readableBytes() < nPkgLen)
	{
		LOGE("recv data error, data is incomplete. data len = %d, nPkgLen = %d", pBuffer->readableBytes(), nPkgLen);
		return;
	}

	IMPduPtr pIMPdu;
	pIMPdu.reset(new CImPdu());
	pIMPdu->Write(pBuffer->peek(), nPkgLen);
	pIMPdu->ReadPduHeader(pBuffer->peek(), IM_PDU_HEADER_LEN);

	handleCommand(pIMPdu);
}

void ChatSession::handleCommand(const IMPduPtr& pPdu)
{
	if (nullptr == pPdu.get())
		return;
	// request authorization check
	if (pPdu->GetCommandId() != CID_LOGIN_REQ_USERLOGIN && !IsOpen() && IsKickOff()) {
		LOGE("handleCommand, wrong msg. ");
		throw CPduException(pPdu->GetServiceId(), pPdu->GetCommandId(), ERROR_CODE_WRONG_SERVICE_ID, "HandlePdu error, user not login. ");
		return;
	}

	switch (pPdu->GetCommandId()) {
	case CID_OTHER_HEARTBEAT:
		_HandleHeartBeat(pPdu);
		break;
	case CID_LOGIN_REQ_USERLOGIN:
		_HandleLoginRequest(pPdu);
		break;
	case CID_LOGIN_REQ_LOGINOUT:
		_HandleLoginOutRequest(pPdu);
		break;
	case CID_LOGIN_REQ_DEVICETOKEN:
		_HandleClientDeviceToken(pPdu);
		break;
	case CID_LOGIN_REQ_KICKPCCLIENT:
		_HandleKickPCClient(pPdu);
		break;
	case CID_LOGIN_REQ_PUSH_SHIELD:
		_HandlePushShieldRequest(pPdu);
		break;

	case CID_LOGIN_REQ_QUERY_PUSH_SHIELD:
		_HandleQueryPushShieldRequest(pPdu);
		break;
	case CID_MSG_DATA:
		_HandleClientMsgData(pPdu);
		break;
	case CID_MSG_DATA_ACK:
		_HandleClientMsgDataAck(pPdu);
		break;
	case CID_MSG_TIME_REQUEST:
		_HandleClientTimeRequest(pPdu);
		break;
	case CID_MSG_LIST_REQUEST:
		_HandleClientGetMsgListRequest(pPdu);
		break;
	case CID_MSG_GET_BY_MSG_ID_REQ:
		_HandleClientGetMsgByMsgIdRequest(pPdu);
		break;
	case CID_MSG_UNREAD_CNT_REQUEST:
		_HandleClientUnreadMsgCntRequest(pPdu);
		break;
	case CID_MSG_READ_ACK:
		_HandleClientMsgReadAck(pPdu);
		break;
	case CID_MSG_GET_LATEST_MSG_ID_REQ:
		_HandleClientGetLatestMsgIDReq(pPdu);
		break;
	case CID_SWITCH_P2P_CMD:
		_HandleClientP2PCmdMsg(pPdu);
		break;
	case CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST:
		_HandleClientRecentContactSessionRequest(pPdu);
		break;
	case CID_BUDDY_LIST_USER_INFO_REQUEST:
		_HandleClientUserInfoRequest(pPdu);
		break;
	case CID_BUDDY_LIST_REMOVE_SESSION_REQ:
		_HandleClientRemoveSessionRequest(pPdu);
		break;
	case CID_BUDDY_LIST_ALL_USER_REQUEST:
		_HandleClientAllUserRequest(pPdu);
		break;
	case CID_BUDDY_LIST_CHANGE_AVATAR_REQUEST:
		_HandleChangeAvatarRequest(pPdu);
		break;
	case CID_BUDDY_LIST_CHANGE_SIGN_INFO_REQUEST:
		_HandleChangeSignInfoRequest(pPdu);
		break;
	case CID_BUDDY_LIST_USERS_STATUS_REQUEST:
		_HandleClientUsersStatusRequest(pPdu);
		break;
	case CID_BUDDY_LIST_DEPARTMENT_REQUEST:
		_HandleClientDepartmentRequest(pPdu);
		break;

		// for group process
	case CID_GROUP_NORMAL_LIST_REQUEST:
		Singleton<UserGroupChat>::Instance().HandleClientGroupNormalRequest(pPdu, this);
		break;
	case CID_GROUP_INFO_REQUEST:
		Singleton<UserGroupChat>::Instance().HandleClientGroupInfoRequest(pPdu, this);
		break;
	case CID_GROUP_CREATE_REQUEST:
		Singleton<UserGroupChat>::Instance().HandleClientGroupCreateRequest(pPdu, this);
		break;
	case CID_GROUP_CHANGE_MEMBER_REQUEST:
		Singleton<UserGroupChat>::Instance().HandleClientGroupChangeMemberRequest(pPdu, this);
		break;
	case CID_GROUP_SHIELD_GROUP_REQUEST:
		Singleton<UserGroupChat>::Instance().HandleClientGroupShieldGroupRequest(pPdu, this);
		break;

		// for file process
	case CID_FILE_REQUEST:
		Singleton<ChatFileHandler>::Instance().HandleClientFileRequest(this, pPdu);
		break;
	case CID_FILE_HAS_OFFLINE_REQ:
		Singleton<ChatFileHandler>::Instance().HandleClientFileHasOfflineReq(this, pPdu);
		break;
	case CID_FILE_ADD_OFFLINE_REQ:
		Singleton<ChatFileHandler>::Instance().HandleClientFileAddOfflineReq(this, pPdu);
		break;
	case CID_FILE_DEL_OFFLINE_REQ:
		Singleton<ChatFileHandler>::Instance().HandleClientFileDelOfflineReq(this, pPdu);
		break;
	default:
		log("wrong msg, cmd id=%d, user id=%u. ", pPdu->GetCommandId(), GetUserId());
		break;
	}
}

void ChatSession::_HandleHeartBeat(const IMPduPtr& pPdu)
{//响应
	Send(pPdu->GetBuffer(), pPdu->GetLength());
}

// process: send validate request to db server
void ChatSession::_HandleLoginRequest(const IMPduPtr& pPdu)
{
	// refuse second validate request
	if (m_login_name.length() != 0) {
		log("duplicate LoginRequest in the same conn ");
		return;
	}

	// check if all server connection are OK
	uint32_t result = 0;
	string result_string = "";
	CDBServConn* pDbConn = get_db_serv_conn_for_login();
	if (!pDbConn) {
		result = IM::BaseDefine::REFUSE_REASON_NO_DB_SERVER;
		result_string = "服务端异常";
	}
	else if (!is_login_server_available()) {
		result = IM::BaseDefine::REFUSE_REASON_NO_LOGIN_SERVER;
		result_string = "服务端异常";
	}
	else if (!is_route_server_available()) {
		result = IM::BaseDefine::REFUSE_REASON_NO_ROUTE_SERVER;
		result_string = "服务端异常";
	}

	if (result) {
		IM::Login::IMLoginRes msg;
		msg.set_server_time(time(NULL));
		msg.set_result_code((IM::BaseDefine::ResultType)result);
		msg.set_result_string(result_string);
		CImPdu pdu;
		pdu.SetPBMsg(&msg);
		pdu.SetServiceId(SID_LOGIN);
		pdu.SetCommandId(CID_LOGIN_RES_USERLOGIN);
		pdu.SetSeqNum(pPdu->GetSeqNum());
		Send(pdu.GetBuffer(), pdu.GetLength());
		Close();
		return;
	}

	IM::Login::IMLoginReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	//假如是汉字，则转成拼音
	m_login_name = msg.user_name();
	string password = msg.password();
	uint32_t online_status = msg.online_status();
	if (online_status < IM::BaseDefine::USER_STATUS_ONLINE || online_status > IM::BaseDefine::USER_STATUS_LEAVE) {
		log("HandleLoginReq, online status wrong: %u ", online_status);
		online_status = IM::BaseDefine::USER_STATUS_ONLINE;
	}
	m_client_version = msg.client_version();
	m_client_type = msg.client_type();
	m_online_status = online_status;
	log("HandleLoginReq, user_name=%s, status=%u, client_type=%u, client=%s, ",
		m_login_name.c_str(), online_status, m_client_type, m_client_version.c_str());
	CImUser* pImUser = CImUserManager::GetInstance()->GetImUserByLoginName(GetLoginName());
	if (!pImUser) {
		pImUser = new CImUser(GetLoginName());
		CImUserManager::GetInstance()->AddImUserByLoginName(GetLoginName(), pImUser);
	}
	pImUser->AddUnValidateMsgConn(this);

	CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
	// continue to validate if the user is OK

	IM::Server::IMValidateReq msg2;
	msg2.set_user_name(msg.user_name());
	msg2.set_password(password);
	msg2.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
	CImPdu pdu;
	pdu.SetPBMsg(&msg2);
	pdu.SetServiceId(SID_OTHER);
	pdu.SetCommandId(CID_OTHER_VALIDATE_REQ);
	pdu.SetSeqNum(pPdu->GetSeqNum());
	pDbConn->SendPdu(&pdu);
}

void ChatSession::_HandleLoginOutRequest(const IMPduPtr& pPdu)
{
	log("HandleLoginOutRequest, user_id=%d, client_type=%u. ", GetUserId(), GetClientType());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		IM::Login::IMDeviceTokenReq msg;
		msg.set_user_id(GetUserId());
		msg.set_device_token("");
		CImPdu pdu;
		pdu.SetPBMsg(&msg);
		pdu.SetServiceId(SID_LOGIN);
		pdu.SetCommandId(CID_LOGIN_REQ_DEVICETOKEN);
		pdu.SetSeqNum(pPdu->GetSeqNum());
		pDBConn->SendPdu(&pdu);
	}

	IM::Login::IMLogoutRsp msg2;
	msg2.set_result_code(0);
	CImPdu pdu2;
	pdu2.SetPBMsg(&msg2);
	pdu2.SetServiceId(SID_LOGIN);
	pdu2.SetCommandId(CID_LOGIN_RES_LOGINOUT);
	pdu2.SetSeqNum(pPdu->GetSeqNum());
	Send(pdu2.GetBuffer(), pdu2.GetLength());
	Close();
}

void ChatSession::_HandleKickPCClient(const IMPduPtr& pPdu)
{
	IM::Login::IMKickPCClientReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t user_id = GetUserId();
	if (!CHECK_CLIENT_TYPE_MOBILE(GetClientType()))
	{
		log("HandleKickPCClient, user_id = %u, cmd must come from mobile client. ", user_id);
		return;
	}
	log("HandleKickPCClient, user_id = %u. ", user_id);

	CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(user_id);
	if (pImUser)
	{
		pImUser->KickOutSameClientType(CLIENT_TYPE_MAC, IM::BaseDefine::KICK_REASON_MOBILE_KICK, this);
	}

	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		IM::Server::IMServerKickUser msg2;
		msg2.set_user_id(user_id);
		msg2.set_client_type(::IM::BaseDefine::CLIENT_TYPE_MAC);
		msg2.set_reason(IM::BaseDefine::KICK_REASON_MOBILE_KICK);
		CImPdu pdu;
		pdu.SetPBMsg(&msg2);
		pdu.SetServiceId(SID_OTHER);
		pdu.SetCommandId(CID_OTHER_SERVER_KICK_USER);
		pRouteConn->SendPdu(&pdu);
	}

	IM::Login::IMKickPCClientRsp msg2;
	msg2.set_user_id(user_id);
	msg2.set_result_code(0);
	CImPdu pdu;
	pdu.SetPBMsg(&msg2);
	pdu.SetServiceId(SID_LOGIN);
	pdu.SetCommandId(CID_LOGIN_RES_KICKPCCLIENT);
	pdu.SetSeqNum(pPdu->GetSeqNum());
	Send(pdu.GetBuffer(), pdu.GetLength());
}

void ChatSession::_HandleClientRecentContactSessionRequest(const IMPduPtr& pPdu)
{
	CDBServConn* pConn = get_db_serv_conn_for_login();
	if (!pConn) {
		return;
	}

	IM::Buddy::IMRecentContactSessionReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	log("HandleClientRecentContactSessionRequest, user_id=%u, latest_update_time=%u. ", GetUserId(), msg.latest_update_time());

	msg.set_user_id(GetUserId());
	// 请求最近联系会话列表
	CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
	msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
	pPdu->SetPBMsg(&msg);
	pConn->SendPdu(pPdu);
}

void ChatSession::_HandleClientMsgData(const IMPduPtr& pPdu)
{
	IM::Message::IMMsgData msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	if (msg.msg_data().length() == 0) {
		log("discard an empty message, uid=%u ", GetUserId());
		return;
	}

	if (m_msg_cnt_per_sec >= MAX_MSG_CNT_PER_SECOND) {
		log("!!!too much msg cnt in one second, uid=%u ", GetUserId());
		return;
	}

	if (msg.from_user_id() == msg.to_session_id() && CHECK_MSG_TYPE_SINGLE(msg.msg_type()))
	{
		log("!!!from_user_id == to_user_id. ");
		return;
	}

	m_msg_cnt_per_sec++;

	uint32_t to_session_id = msg.to_session_id();
	uint32_t msg_id = msg.msg_id();
	uint8_t msg_type = msg.msg_type();
	string msg_data = msg.msg_data();

	if (g_log_msg_toggle) {
		log("HandleClientMsgData, %d->%d, msg_type=%u, msg_id=%u. ", GetUserId(), to_session_id, msg_type, msg_id);
	}

	uint32_t cur_time = time(NULL);
	CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
	msg.set_from_user_id(GetUserId());
	msg.set_create_time(cur_time);
	msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
	pPdu->SetPBMsg(&msg);
	// send to DB storage server
	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		pDbConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientMsgDataAck(const IMPduPtr& pPdu)
{
	IM::Message::IMMsgDataAck msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	IM::BaseDefine::SessionType session_type = msg.session_type();
	if (session_type == IM::BaseDefine::SESSION_TYPE_SINGLE)
	{
		uint32_t msg_id = msg.msg_id();
		uint32_t session_id = msg.session_id();
		DelFromSendList(msg_id, session_id);
	}
}

void ChatSession::_HandleClientTimeRequest(const IMPduPtr& pPdu)
{
	IM::Message::IMClientTimeRsp msg;
	msg.set_server_time((uint32_t)time(NULL));
	CImPdu pdu;
	pdu.SetPBMsg(&msg);
	pdu.SetServiceId(SID_MSG);
	pdu.SetCommandId(CID_MSG_TIME_RESPONSE);
	pdu.SetSeqNum(pPdu->GetSeqNum());
	Send(pdu.GetBuffer(), pdu.GetLength());
}

void ChatSession::_HandleClientGetMsgListRequest(const IMPduPtr& pPdu)
{
	IM::Message::IMGetMsgListReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t session_id = msg.session_id();
	uint32_t msg_id_begin = msg.msg_id_begin();
	uint32_t msg_cnt = msg.msg_cnt();
	uint32_t session_type = msg.session_type();
	log("HandleClientGetMsgListRequest, req_id=%u, session_type=%u, session_id=%u, msg_id_begin=%u, msg_cnt=%u. ",
		GetUserId(), session_type, session_id, msg_id_begin, msg_cnt);
	CDBServConn* pDBConn = get_db_serv_conn_for_login();
	if (pDBConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientGetMsgByMsgIdRequest(const IMPduPtr& pPdu)
{
	IM::Message::IMGetMsgByIdReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t session_id = msg.session_id();
	uint32_t session_type = msg.session_type();
	uint32_t msg_cnt = msg.msg_id_list_size();
	log("_HandleClientGetMsgByMsgIdRequest, req_id=%u, session_type=%u, session_id=%u, msg_cnt=%u.",
		GetUserId(), session_type, session_id, msg_cnt);
	CDBServConn* pDBConn = get_db_serv_conn_for_login();
	if (pDBConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientUnreadMsgCntRequest(const IMPduPtr& pPdu)
{
	log("HandleClientUnreadMsgCntReq, from_id=%u ", GetUserId());
	IM::Message::IMUnreadMsgCntReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	CDBServConn* pDBConn = get_db_serv_conn_for_login();
	if (pDBConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientMsgReadAck(const IMPduPtr& pPdu)
{
	IM::Message::IMMsgDataReadAck msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t session_type = msg.session_type();
	uint32_t session_id = msg.session_id();
	uint32_t msg_id = msg.msg_id();
	log("HandleClientMsgReadAck, user_id=%u, session_id=%u, msg_id=%u, session_type=%u. ", GetUserId(), session_id, msg_id, session_type);

	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		msg.set_user_id(GetUserId());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
	IM::Message::IMMsgDataReadNotify msg2;
	msg2.set_user_id(GetUserId());
	msg2.set_session_id(session_id);
	msg2.set_msg_id(msg_id);
	msg2.set_session_type((IM::BaseDefine::SessionType)session_type);
	CImPdu pdu;
	pdu.SetPBMsg(&msg2);
	pdu.SetServiceId(SID_MSG);
	pdu.SetCommandId(CID_MSG_READ_NOTIFY);
	CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(GetUserId());
	if (pUser)
	{
		pUser->BroadcastPdu(&pdu, this);
	}
	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		pRouteConn->SendPdu(&pdu);
	}

	if (session_type == IM::BaseDefine::SESSION_TYPE_SINGLE)
	{
		DelFromSendList(msg_id, session_id);
	}
}

void ChatSession::_HandleClientGetLatestMsgIDReq(const IMPduPtr& pPdu)
{
	IM::Message::IMGetLatestMsgIdReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t session_type = msg.session_type();
	uint32_t session_id = msg.session_id();
	log("HandleClientGetMsgListRequest, user_id=%u, session_id=%u, session_type=%u. ", GetUserId(), session_id, session_type);

	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientP2PCmdMsg(const IMPduPtr& pPdu)
{
	IM::SwitchService::IMP2PCmdMsg msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	string cmd_msg = msg.cmd_msg_data();
	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_user_id = msg.to_user_id();

	log("HandleClientP2PCmdMsg, %u->%u, cmd_msg: %s ", from_user_id, to_user_id, cmd_msg.c_str());

	CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(GetUserId());
	CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);

	if (pFromImUser) {
		pFromImUser->BroadcastPdu(pPdu, this);
	}

	if (pToImUser) {
		pToImUser->BroadcastPdu(pPdu, NULL);
	}

	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		pRouteConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientUserInfoRequest(const IMPduPtr& pPdu)
{
	IM::Buddy::IMUsersInfoReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t user_cnt = msg.user_id_list_size();
	log("HandleClientUserInfoReq, req_id=%u, user_cnt=%u ", GetUserId(), user_cnt);
	CDBServConn* pDBConn = get_db_serv_conn_for_login();
	if (pDBConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientRemoveSessionRequest(const IMPduPtr& pPdu)
{
	IM::Buddy::IMRemoveSessionReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t session_type = msg.session_type();
	uint32_t session_id = msg.session_id();
	log("HandleClientRemoveSessionReq, user_id=%u, session_id=%u, type=%u ", GetUserId(), session_id, session_type);

	CDBServConn* pConn = get_db_serv_conn();
	if (pConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pConn->SendPdu(pPdu);
	}

	if (session_type == IM::BaseDefine::SESSION_TYPE_SINGLE)
	{
		IM::Buddy::IMRemoveSessionNotify msg2;
		msg2.set_user_id(GetUserId());
		msg2.set_session_id(session_id);
		msg2.set_session_type((IM::BaseDefine::SessionType)session_type);
		CImPdu pdu;
		pdu.SetPBMsg(&msg2);
		pdu.SetServiceId(SID_BUDDY_LIST);
		pdu.SetCommandId(CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY);
		CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(GetUserId());
		if (pImUser) {
			pImUser->BroadcastPdu(&pdu, this);
		}
		CRouteServConn* pRouteConn = get_route_serv_conn();
		if (pRouteConn) {
			pRouteConn->SendPdu(&pdu);
		}
	}
}

void ChatSession::_HandleClientAllUserRequest(const IMPduPtr& pPdu)
{
	IM::Buddy::IMAllUserReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t latest_update_time = msg.latest_update_time();
	log("HandleClientAllUserReq, user_id=%u, latest_update_time=%u. ", GetUserId(), latest_update_time);

	CDBServConn* pConn = get_db_serv_conn();
	if (pConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleChangeAvatarRequest(const IMPduPtr& pPdu)
{
	IM::Buddy::IMChangeAvatarReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	log("HandleChangeAvatarRequest, user_id=%u ", GetUserId());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		msg.set_user_id(GetUserId());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientUsersStatusRequest(const IMPduPtr& pPdu)
{
	IM::Buddy::IMUsersStatReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	uint32_t user_count = msg.user_id_list_size();
	log("HandleClientUsersStatusReq, user_id=%u, query_count=%u.", GetUserId(), user_count);

	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn)
	{
		msg.set_user_id(GetUserId());
		CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pRouteConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientDepartmentRequest(const IMPduPtr& pPdu)
{
	IM::Buddy::IMDepartmentReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	log("HandleClientDepartmentRequest, user_id=%u, latest_update_time=%u.", GetUserId(), msg.latest_update_time());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		msg.set_user_id(GetUserId());
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleClientDeviceToken(const IMPduPtr& pPdu)
{
	if (!CHECK_CLIENT_TYPE_MOBILE(GetClientType()))
	{
		log("HandleClientDeviceToken, user_id=%u, not mobile client.", GetUserId());
		return;
	}
	IM::Login::IMDeviceTokenReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	string device_token = msg.device_token();
	log("HandleClientDeviceToken, user_id=%u, device_token=%s ", GetUserId(), device_token.c_str());

	IM::Login::IMDeviceTokenRsp msg2;
	msg.set_user_id(GetUserId());
	msg.set_client_type((::IM::BaseDefine::ClientType)GetClientType());
	CImPdu pdu;
	pdu.SetPBMsg(&msg2);
	pdu.SetServiceId(SID_LOGIN);
	pdu.SetCommandId(CID_LOGIN_RES_DEVICETOKEN);
	pdu.SetSeqNum(pPdu->GetSeqNum());
	Send(pdu.GetBuffer(), pdu.GetLength());

	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		msg.set_user_id(GetUserId());
		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleChangeSignInfoRequest(const IMPduPtr& pPdu) {
	IM::Buddy::IMChangeSignInfoReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	log("HandleChangeSignInfoRequest, user_id=%u ", GetUserId());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		msg.set_user_id(GetUserId());
		CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());

		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandlePushShieldRequest(const IMPduPtr& pPdu) {
	IM::Login::IMPushShieldReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	log("_HandlePushShieldRequest, user_id=%u, shield_status ", GetUserId(), msg.shield_status());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		msg.set_user_id(GetUserId());
		CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());

		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}

void ChatSession::_HandleQueryPushShieldRequest(const IMPduPtr& pPdu) {
	IM::Login::IMQueryPushShieldReq msg;
	CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	log("HandleChangeSignInfoRequest, user_id=%u ", GetUserId());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		msg.set_user_id(GetUserId());
		CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
		msg.set_attach_data(attach.GetBuffer(), attach.GetLength());

		pPdu->SetPBMsg(&msg);
		pDBConn->SendPdu(pPdu);
	}
}
