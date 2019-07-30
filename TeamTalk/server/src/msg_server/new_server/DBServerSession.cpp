#include "DBServerSession.h"
#include "../net/TcpClient.h"



bool DBServerSession::init(const char* ip, short port, EventLoop* loop)
{
	InetAddress addr(ip, port);
	m_pTcpClient.reset(new TcpClient(loop, addr, "DBServerConn"));
	m_pTcpClient->setConnectionCallback();

	m_pTcpClient->connect();
	return true;
}

void DBServerSession::onConnection(std::shared_ptr<TcpConnection> conn)
{
	if (conn->connected())
	{
		LOGD("connect server: %s success", conn->peerAddress().c_str());

	}
	else
	{

	}

}