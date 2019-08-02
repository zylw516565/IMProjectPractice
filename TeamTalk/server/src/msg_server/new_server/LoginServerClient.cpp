#include "LoginServerClient.h"

#include "IM.Other.pb.h"
#include "IM.Server.pb.h"


void LoginServerClient::onConnection(const TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		LOGD("connect server: %s success", conn->peerAddress().c_str());

		//����login_server�ɹ��Ժ�,����login_server�Լ���ip��ַ���˿ں�
		//�͵�ǰ��¼���û������Ϳ����ɵ�����û�����
		list<user_conn_t> user_conn_list;
		CImUserManager::GetInstance()->GetUserConnCnt(&user_conn_list, cur_conn_cnt);
		char hostname[256] = { 0 };
		gethostname(hostname, 256);
		IM::Server::IMMsgServInfo msg;
		msg.set_ip1(g_msg_server_ip_addr1);
		msg.set_ip2(g_msg_server_ip_addr2);
		msg.set_port(g_msg_server_port);
		msg.set_max_conn_cnt(g_max_conn_cnt);
		msg.set_cur_conn_cnt(cur_conn_cnt);
		msg.set_host_name(hostname);
		CImPdu pdu;
		pdu.SetPBMsg(&msg);
		pdu.SetServiceId(SID_OTHER);
		pdu.SetCommandId(CID_OTHER_MSG_SERV_INFO);
		conn->send(pdu.GetBuffer(), pdu.GetLength());
	}
	else
	{
		onClose(conn);
	}
}

void LoginServerClient::onClose(const TcpConnectionPtr& conn)
{
	LOGD("connection: %s is down", conn->peerAddress().c_str());
}

void LoginServerClient::onWriteComplete(const TcpConnectionPtr& conn)
{
	LOGD("connection: %s is writecomplete", conn->peerAddress().c_str());
}

void LoginServerClient::onHeartBeat()
{
	sendHeartBeat();
}

void LoginServerClient::onMessage(const TcpConnectionPtr& conn, Buffer* pBuffer, Timestamp receiveTime)
{
	IMPduPtr pIMPdu;
	pIMPdu.reset(new CImPdu());
	parseMessageHeader(conn, pBuffer, receiveTime, pIMPdu);

	handleCommand(pIMPdu);
}

void LoginServerClient::handleCommand(const IMPduPtr& pPdu)
{
	return;
}
