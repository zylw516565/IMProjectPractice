#include "FileServerClient.h"

#include "IM.Server.pb.h"
#include "IM.Other.pb.h"
#include "IM.File.pb.h"




void FileServerClient::onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		LOGD("connect server: %s success", conn->peerAddress().c_str());
		
		//连上file_server以后，给file_server发送获取ip地址的数据包
	    IM::Server::IMFileServerIPReq msg;
	    CImPdu pdu;
	    pdu.SetPBMsg(&msg);
	    pdu.SetServiceId(SID_OTHER);
	    pdu.SetCommandId(CID_OTHER_FILE_SERVER_IP_REQ);
		conn->send(pdu.GetBuffer(), pdu.GetLength());    
	}
	else
	{
		onClose(conn);
	}
}

void FileServerClient::onClose(const TcpConnectionPtr& conn)
{
	LOGD("connection: %s is down", conn->peerAddress().c_str());
}

void FileServerClient::onWriteComplete(const TcpConnectionPtr& conn)
{
	LOGD("connection: %s is writecomplete", conn->peerAddress().c_str());
}

void FileServerClient::onHeartBeat()
{
	sendHeartBeat();
}

void FileServerClient::onMessage(const TcpConnectionPtr& conn, Buffer* pBuffer, Timestamp receiveTime)
{
	IMPduPtr pIMPdu;
	pIMPdu.reset(new CImPdu());
	parseMessageHeader(conn, pBuffer, receiveTime, pIMPdu);

	handleCommand(pIMPdu);
}

void FileServerClient::handleCommand(const IMPduPtr& pPdu)
{
	if (nullptr == pPdu.get())
		return;

	switch (pPdu->GetCommandId()) {
	case CID_OTHER_HEARTBEAT:
		break;
	case CID_OTHER_FILE_TRANSFER_RSP:
		_HandleFileMsgTransRsp(pPdu);
		break;
    case CID_OTHER_FILE_SERVER_IP_RSP:
		_HandleFileServerIPRsp(pPdu);
		break;
	default:
		log("file server, unknown cmd id=%d ", pPdu->GetCommandId());
		break;
	}
}

void FileServerClient::_HandleFileMsgTransRsp(const IMPduPtr& pPdu)
{
    IM::Server::IMFileTransferRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

    uint32_t result = msg.result_code();
    uint32_t from_id = msg.from_user_id();
    uint32_t to_id = msg.to_user_id();
    string file_name = msg.file_name();
    uint32_t file_size = msg.file_size();
    string task_id = msg.task_id();
    uint32_t trans_mode = msg.trans_mode();
    CDbAttachData attach((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
    log("HandleFileMsgTransRsp, result: %u, from_user_id: %u, to_user_id: %u, file_name: %s, \
        task_id: %s, trans_mode: %u. ", result, from_id, to_id,
        file_name.c_str(), task_id.c_str(), trans_mode);

    const list<IM::BaseDefine::IpAddr>* ip_addr_list = GetFileServerIPList();

    IM::File::IMFileRsp msg2;
    msg2.set_result_code(result);
    msg2.set_from_user_id(from_id);
    msg2.set_to_user_id(to_id);
    msg2.set_file_name(file_name);
    msg2.set_task_id(task_id);
    msg2.set_trans_mode((IM::BaseDefine::TransferFileType)trans_mode);
    for (list<IM::BaseDefine::IpAddr>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
    {
        IM::BaseDefine::IpAddr ip_addr_tmp = *it;
        IM::BaseDefine::IpAddr* ip_addr = msg2.add_ip_addr_list();
        ip_addr->set_ip(ip_addr_tmp.ip());
        ip_addr->set_port(ip_addr_tmp.port());
    }
    CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_FILE);
    pdu.SetCommandId(CID_FILE_RESPONSE);
    pdu.SetSeqNum(pPdu->GetSeqNum());
    uint32_t handle = attach.GetHandle();
    
    CMsgConn* pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_id, handle);
    if (pFromConn)
    {
        pFromConn->SendPdu(&pdu);
    }
    
    if (result == 0)
    {
        IM::File::IMFileNotify msg3;
        msg3.set_from_user_id(from_id);
        msg3.set_to_user_id(to_id);
        msg3.set_file_name(file_name);
        msg3.set_file_size(file_size);
        msg3.set_task_id(task_id);
        msg3.set_trans_mode((IM::BaseDefine::TransferFileType)trans_mode);
        msg3.set_offline_ready(0);
        for (list<IM::BaseDefine::IpAddr>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
        {
            IM::BaseDefine::IpAddr ip_addr_tmp = *it;
            IM::BaseDefine::IpAddr* ip_addr = msg3.add_ip_addr_list();
            ip_addr->set_ip(ip_addr_tmp.ip());
            ip_addr->set_port(ip_addr_tmp.port());
        }
        CImPdu pdu2;
        pdu2.SetPBMsg(&msg3);
        pdu2.SetServiceId(SID_FILE);
        pdu2.SetCommandId(CID_FILE_NOTIFY);
        
        //send notify to target user
        CImUser* pToUser = CImUserManager::GetInstance()->GetImUserById(to_id);
        if (pToUser)
        {
            pToUser->BroadcastPduWithOutMobile(&pdu2);
        }
        
        //send to route server
        CRouteServConn* pRouteConn = get_route_serv_conn();
        if (pRouteConn) {
            pRouteConn->SendPdu(&pdu2);
        }
    }
}

void FileServerClient::_HandleFileServerIPRsp(const IMPduPtr& pPdu)
{
    IM::Server::IMFileServerIPRsp msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
    uint32_t ip_addr_cnt = msg.ip_addr_list_size();
    
    for (uint32_t i = 0; i < ip_addr_cnt ; i++)
    {
        IM::BaseDefine::IpAddr ip_addr = msg.ip_addr_list(i);
        log("_HandleFileServerIPRsp -> %s : %d ", ip_addr.ip().c_str(), ip_addr.port());
        m_ip_list.push_back(ip_addr);
    }
}


