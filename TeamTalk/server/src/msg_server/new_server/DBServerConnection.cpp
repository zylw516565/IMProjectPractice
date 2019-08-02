#include "DBServerConnection.h"

#include "../net/TcpClient.h"



bool DBServerConnection::initConnect(EventLoop* loop, const char* ip, short port, int64_t p_nTimerInterval)
{
	if (NULL == ip || 0 == port || NULL == loop)
	{
		LOGE("ip or loop is NULL !!! ip:%x, loop: %x", ip, loop);
		return false;
	}

	//TODO:  ���Ӷ�����
	for (;;)
	{
		InetAddress addr(ip, port);
		DBServerClientPtr pDBServerClient(new DBServerClient(loop, addr, "DBServerConn"));
		pDBServerClient->setConnectionCallback(std::bind(&DBServerClient::onConnection, this, std::placeholders::_1));
		pDBServerClient->setMessageCallback(std::bind(&DBServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		pDBServerClient->setWriteCompleteCallback();
		pDBServerClient->enableRetry();    //�������ӶϿ�������

		{
			std::lock_guard<std::mutex> guard(m_ClientMutex);
			pDBServerClient->connect();             //�˴���ʼ���Ӳ�������ŵ�����,��֤���ӻص��ڼ���list֮�󴥷�
			m_DBServerClientList.push_back(pDBServerClient);
			//m_ClientConnMap.insert(TcpClientConnMap::value_type(pDBServerClient->connection(), pDBServerClient));
		}

		loop->runEvery(p_nTimerInterval, std::bind(&DBServerClient::onHeartBeat, pDBServerClient.get()));
	}

	return true;
}

void DBServerConnection::stop()
{
	{
		std::lock_guard<std::mutex> guard(m_ClientMutex);
		auto Iter = m_DBServerClientList.begin();
		for (; Iter != m_DBServerClientList.end(); ++Iter)
		{
			(*Iter)->disconnect();
			(*Iter)->stop();
		}
	}
}
