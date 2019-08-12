#include "RouteServerClient.h"

#include "IM.Server.pb.h"
#include "IM.Other.pb.h"
#include "IM.File.pb.h"




void RouteServerClient::onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		LOGD("connect server: %s success", conn->peerAddress().c_str());
		
		//连接route_server成功以后，给route_server发包告诉当前登录在本msg_server上有哪些
		//用户（用户id、用户状态、用户客户端类型）
		list<user_stat_t> online_user_list;
		CImUserManager::GetInstance()->GetOnlineUserInfo(&online_user_list);
		IM::Server::IMOnlineUserInfo msg;
		for (list<user_stat_t>::iterator it = online_user_list.begin(); it != online_user_list.end(); it++) {
			user_stat_t user_stat = *it;
			IM::BaseDefine::ServerUserStat* server_user_stat = msg.add_user_stat_list();
			server_user_stat->set_user_id(user_stat.user_id);
			server_user_stat->set_status((::IM::BaseDefine::UserStatType)user_stat.status);
			server_user_stat->set_client_type((::IM::BaseDefine::ClientType)user_stat.client_type);
		
		}
		CImPdu pdu;
		pdu.SetPBMsg(&msg);
		pdu.SetServiceId(SID_OTHER);
		pdu.SetCommandId(CID_OTHER_ONLINE_USER_INFO);
		conn->send(pdu.GetBuffer(), pdu.GetLength());    
	}
	else
	{
		onClose(conn);
	}
}

void RouteServerClient::onClose(const TcpConnectionPtr& conn)
{
	LOGD("connection: %s is down", conn->peerAddress().c_str());
}

void RouteServerClient::onWriteComplete(const TcpConnectionPtr& conn)
{
	LOGD("connection: %s is writecomplete", conn->peerAddress().c_str());
}

void RouteServerClient::onHeartBeat()
{
	sendHeartBeat();
}

void RouteServerClient::onMessage(const TcpConnectionPtr& conn, Buffer* pBuffer, Timestamp receiveTime)
{
	IMPduPtr pIMPdu;
	pIMPdu.reset(new CImPdu());
	parseMessageHeader(conn, pBuffer, receiveTime, pIMPdu);

	handleCommand(pIMPdu);
}

void RouteServerClient::handleCommand(const IMPduPtr& pPdu)
{
	if (nullptr == pPdu.get())
		return;

	switch (pPdu->GetCommandId()) {
	case CID_OTHER_HEARTBEAT:
		break;
    case CID_OTHER_SERVER_KICK_USER:
        _HandleKickUser( pPdu );
        break;
    case CID_BUDDY_LIST_STATUS_NOTIFY:
        _HandleStatusNotify( pPdu );
        break;
    case CID_BUDDY_LIST_USERS_STATUS_RESPONSE:
        _HandleUsersStatusResponse( pPdu );
        break;
    case CID_MSG_READ_NOTIFY:
        _HandleMsgReadNotify(pPdu);
        break;
    case CID_MSG_DATA:
        _HandleMsgData(pPdu);
        break;
    case CID_SWITCH_P2P_CMD:
        _HandleP2PMsg(pPdu );
        break;
    case CID_OTHER_LOGIN_STATUS_NOTIFY:
        _HandlePCLoginStatusNotify(pPdu);
        break;
    case CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY:
        _HandleRemoveSessionNotify(pPdu);
        break;
    case CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY:
        _HandleSignInfoChangedNotify(pPdu);
    case CID_GROUP_CHANGE_MEMBER_NOTIFY:
        UserGroupChat::getInstance().HandleGroupChangeMemberBroadcast(pPdu);
        break;
    case CID_FILE_NOTIFY:
        ChatFileHandler::getInstance().HandleFileNotify(pPdu);
        break;
	default:
		log("route server, unknown cmd id=%d ", pPdu->GetCommandId());
		break;
	}
}

void RouteServerClient::_HandleKickUser(const IMPduPtr& pPdu)
{
    IM::Server::IMServerKickUser msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_id = msg.user_id();
    uint32_t client_type = msg.client_type();
    uint32_t reason = msg.reason();
	log("HandleKickUser, user_id=%u, client_type=%u, reason=%u. ", user_id, client_type, reason);

    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
	if (pUser) {
		pUser->KickOutSameClientType(client_type, reason);
	}
}

// friend online/off-line notify
void RouteServerClient::_HandleStatusNotify(const IMPduPtr& pPdu)
{
    IM::Buddy::IMUserStatNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    IM::BaseDefine::UserStat user_stat = msg.user_stat();

	log("HandleFriendStatusNotify, user_id=%u, status=%u ", user_stat.user_id(), user_stat.status());

	// send friend online message to client
    CImUserManager::GetInstance()->BroadcastPdu(pPdu, CLIENT_TYPE_FLAG_PC);
}

void RouteServerClient::_HandleMsgData(const IMPduPtr& pPdu)
{
    IM::Message::IMMsgData msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    if (CHECK_MSG_TYPE_GROUP(msg.msg_type())) {
        s_group_chat->HandleGroupMessageBroadcast(pPdu);
        return;
    }
	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_user_id = msg.to_session_id();
    uint32_t msg_id = msg.msg_id();
	log("HandleMsgData, %u->%u, msg_id=%u. ", from_user_id, to_user_id, msg_id);
    
    
    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(from_user_id);
    if (pFromImUser)
    {
        pFromImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
    }
    
	CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
	if (pToImUser)
    {
		pToImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
	}
}

void RouteServerClient::_HandleMsgReadNotify(const IMPduPtr& pPdu)
{
    IM::Message::IMMsgDataReadNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t req_id = msg.user_id();
    uint32_t session_id = msg.session_id();
    uint32_t msg_id = msg.msg_id();
    uint32_t session_type = msg.session_type();
    
    log("HandleMsgReadNotify, user_id=%u, session_id=%u, session_type=%u, msg_id=%u. ", req_id, session_id, session_type, msg_id);
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(req_id);
    if (pUser)
    {
        pUser->BroadcastPdu(pPdu);
    }
}

void RouteServerClient::_HandleP2PMsg(const IMPduPtr& pPdu)
{
    IM::SwitchService::IMP2PCmdMsg msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_user_id = msg.to_user_id();

	log("HandleP2PMsg, %u->%u ", from_user_id, to_user_id);
    
    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(from_user_id);
	CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
    
 	if (pFromImUser) {
 		pFromImUser->BroadcastPdu(pPdu);
	}
    
 	if (pToImUser) {
 		pToImUser->BroadcastPdu(pPdu);
	}
}

void RouteServerClient::_HandleUsersStatusResponse(const IMPduPtr& pPdu)
{
    IM::Buddy::IMUsersStatRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

	uint32_t user_id = msg.user_id();
	uint32_t result_count = msg.user_stat_list_size();
	log("HandleUsersStatusResp, user_id=%u, query_count=%u ", user_id, result_count);
    
    CPduAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    if (attach_data.GetType() == ATTACH_TYPE_HANDLE)
    {
        uint32_t handle = attach_data.GetHandle();
        CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
        if (pConn) {
            msg.clear_attach_data();
            pPdu->SetPBMsg(&msg);
            pConn->SendPdu(pPdu);
        }
    }
    else if (attach_data.GetType() == ATTACH_TYPE_PDU_FOR_PUSH)
    {
        IM::BaseDefine::UserStat user_stat = msg.user_stat_list(0);
        IM::Server::IMPushToUserReq msg2;
        CHECK_PB_PARSE_MSG(msg2.ParseFromArray(attach_data.GetPdu(), attach_data.GetPduLength()));
        IM::BaseDefine::UserTokenInfo* user_token = msg2.mutable_user_token_list(0);

        //pc client登录，则为勿打扰式推送
        if (user_stat.status() == IM::BaseDefine::USER_STATUS_ONLINE)
        {
            user_token->set_push_type(IM_PUSH_TYPE_SILENT);
            log("HandleUsersStatusResponse, user id: %d, push type: normal. ", user_stat.user_id());
        }
        else
        {
            user_token->set_push_type(IM_PUSH_TYPE_NORMAL);
            log("HandleUsersStatusResponse, user id: %d, push type: normal. ", user_stat.user_id());
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_PUSH_TO_USER_REQ);
        
        CPushServConn* PushConn = get_push_serv_conn();
        if (PushConn)
        {
            PushConn->SendPdu(&pdu);
        }
    }
    else if (attach_data.GetType() == ATTACH_TYPE_HANDLE_AND_PDU_FOR_FILE)
    {
        IM::BaseDefine::UserStat user_stat = msg.user_stat_list(0);
        IM::Server::IMFileTransferReq msg3;
        CHECK_PB_PARSE_MSG(msg3.ParseFromArray(attach_data.GetPdu(), attach_data.GetPduLength()));
        uint32_t handle = attach_data.GetHandle();
        
        IM::BaseDefine::TransferFileType trans_mode = IM::BaseDefine::FILE_TYPE_OFFLINE;
        if (user_stat.status() == IM::BaseDefine::USER_STATUS_ONLINE)
        {
            trans_mode = IM::BaseDefine::FILE_TYPE_ONLINE;
        }
        msg3.set_trans_mode(trans_mode);
        CImPdu pdu;
        pdu.SetPBMsg(&msg3);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_FILE_TRANSFER_REQ);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        CFileServConn* pConn = get_random_file_serv_conn();
        if (pConn) {
            pConn->SendPdu(&pdu);
        }
        else
        {
            log("no file server ");
            IM::File::IMFileRsp msg4;
            msg4.set_result_code(1);
            msg4.set_from_user_id(msg3.from_user_id());
            msg4.set_to_user_id(msg3.to_user_id());
            msg4.set_file_name(msg3.file_name());
            msg4.set_task_id("");
            msg4.set_trans_mode(msg3.trans_mode());
            CImPdu pdu2;
            pdu2.SetPBMsg(&msg4);
            pdu2.SetServiceId(SID_FILE);
            pdu2.SetCommandId(CID_FILE_RESPONSE);
            pdu2.SetSeqNum(pPdu->GetSeqNum());
            CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(msg3.from_user_id(),handle);
            if (pMsgConn)
            {
                pMsgConn->SendPdu(&pdu2);
            }
        }
    }
}

void RouteServerClient::_HandleRemoveSessionNotify(const IMPduPtr& pPdu)
{
    IM::Buddy::IMRemoveSessionNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
    uint32_t user_id = msg.user_id();
    uint32_t session_id = msg.session_id();
    log("HandleRemoveSessionNotify, user_id=%u, session_id=%u ", user_id, session_id);
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if (pUser)
    {
        pUser->BroadcastPdu(pPdu);
    }
}

void RouteServerClient::_HandlePCLoginStatusNotify(const IMPduPtr& pPdu)
{
    IM::Server::IMServerPCLoginStatusNotify msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t user_id = msg.user_id();
    uint32_t login_status = msg.login_status();
    log("HandlePCLoginStatusNotify, user_id=%u, login_status=%u ", user_id, login_status);
    
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if (pUser)
    {
        pUser->SetPCLoginStatus(login_status);
        IM::Buddy::IMPCLoginStatusNotify msg2;
        msg2.set_user_id(user_id);
        if (IM_PC_LOGIN_STATUS_ON == login_status)
        {
            msg2.set_login_stat(::IM::BaseDefine::USER_STATUS_ONLINE);
        }
        else
        {
            msg2.set_login_stat(::IM::BaseDefine::USER_STATUS_OFFLINE);
        }
        CImPdu pdu;
        pdu.SetPBMsg(&msg2);
        pdu.SetServiceId(IM::BaseDefine::SID_BUDDY_LIST);
        pdu.SetCommandId(IM::BaseDefine::CID_BUDDY_LIST_PC_LOGIN_STATUS_NOTIFY);
        pUser->BroadcastPduToMobile(&pdu);
    }
}

void RouteServerClient::_HandleSignInfoChangedNotify(const IMPduPtr& pPdu) {
        IM::Buddy::IMSignInfoChangedNotify msg;
        CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    
        log("HandleSignInfoChangedNotify, changed_user_id=%u, sign_info=%s ", msg.changed_user_id(), msg.sign_info().c_str());
    
        // send friend online message to client
        CImUserManager::GetInstance()->BroadcastPdu(pPdu, CLIENT_TYPE_FLAG_BOTH);
}

