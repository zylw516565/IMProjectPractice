#include "LoginServerConnection.h"

#include "../net/TcpClient.h"

bool LoginServerConnection::initConnect(EventLoop* loop, const char* ip, short port, int64_t p_nTimerInterval)
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
		LoginServerClientPtr pLoginServerClient(new LoginServerClient(loop, addr, "LoginServerConnection"));
		pLoginServerClient->setConnectionCallback(std::bind(&LoginServerClient::onConnection, this, std::placeholders::_1));
		pLoginServerClient->setMessageCallback(std::bind(&LoginServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		pLoginServerClient->setWriteCompleteCallback();
		pLoginServerClient->enableRetry();    //�������ӶϿ�������

		{
			std::lock_guard<std::mutex> guard(m_ClientMutex);
			pLoginServerClient->connect();             //�˴���ʼ���Ӳ�������ŵ�����,��֤���ӻص��ڼ���list֮�󴥷�
			m_LoginServerConnectionList.push_back(pLoginServerClient);
		}

		loop->runEvery(p_nTimerInterval, std::bind(&LoginServerClient::onHeartBeat, LoginServerClient.get()));
	}

	return true;
}