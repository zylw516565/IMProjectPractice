#include "LoginServerConnection.h"

#include "../net/TcpClient.h"

bool LoginServerConnection::initConnect(EventLoop* loop, const std::vector<InetAddress>& p_NetAddrList, int64_t p_nTimerInterval)
{
	if (p_NetAddrList.empty() || NULL == loop)
	{
		LOGE("p_NetAddrList is empty !!! loop: %x", loop);
		return false;
	}

	//TODO:  增加多连接
	for (int nIndex = 0; nIndex <= p_NetAddrList.size(); ++nIndex)
	{		
		LoginServerClientPtr pLoginServerClient(new LoginServerClient(loop, p_NetAddrList[nIndex], "LoginServerConnection"));
		pLoginServerClient->setConnectionCallback(std::bind(&LoginServerClient::onConnection, this, std::placeholders::_1));
		pLoginServerClient->setMessageCallback(std::bind(&LoginServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		pLoginServerClient->setWriteCompleteCallback();
		pLoginServerClient->enableRetry();    //开启连接断开后重试

		{
			std::lock_guard<std::mutex> guard(m_ClientMutex);
			pLoginServerClient->connect();             //此处开始连接操作必须放到锁内,保证连接回调在加入list之后触发
			m_LoginServerConnectionList.push_back(pLoginServerClient);
		}

		loop->runEvery(p_nTimerInterval, std::bind(&LoginServerClient::onHeartBeat, LoginServerClient.get()));
	}

	return true;
}