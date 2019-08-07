#include "DBServerConnection.h"

#include "../net/TcpClient.h"



bool DBServerConnection::initConnect(EventLoop* loop, const std::vector<InetAddress>& p_NetAddrList, int64_t p_nTimerInterval)
{
	if (p_NetAddrList.empty() || NULL == loop)
	{
		LOGE("p_NetAddrList is empty !!! loop: %x", loop);
		return false;
	}

	//TODO:  增加多连接
	for (int nIndex = 0; nIndex <= p_NetAddrList.size(); ++nIndex)
	{		
		DBServerClientPtr pDBServerClient(new DBServerClient(loop, p_NetAddrList[nIndex], "DBServerConn" + std::to_string(nIndex)));
		pDBServerClient->setConnectionCallback(std::bind(&DBServerClient::onConnection, this, std::placeholders::_1));
		pDBServerClient->setMessageCallback(std::bind(&DBServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		pDBServerClient->setWriteCompleteCallback();
		pDBServerClient->enableRetry();    //开启连接断开后重试

		{
			std::lock_guard<std::mutex> guard(m_ClientMutex);
			pDBServerClient->connect();             //此处开始连接操作必须放到锁内,保证连接回调在加入list之后触发
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
