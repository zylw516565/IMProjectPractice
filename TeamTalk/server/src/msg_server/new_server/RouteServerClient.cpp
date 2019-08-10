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


	default:
		log("route server, unknown cmd id=%d ", pPdu->GetCommandId());
		break;
	}
}

