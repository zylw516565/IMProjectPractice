#include "RouteServerConnection.h"

#include "../net/TcpClient.h"

bool RouteServerConnection::initConnect(EventLoop* loop, const std::vector<InetAddress>& p_NetAddrList, int64_t p_nTimerInterval)
{
	if (p_NetAddrList.empty() || NULL == loop)
	{
		LOGE("p_NetAddrList is empty !!! loop: %x", loop);
		return false;
	}

	//TODO:  增加多连接
	for (int nIndex = 0; nIndex <= p_NetAddrList.size(); ++nIndex)
	{
		RouteServerClientPtr pRouteServerClient(new RouteServerClient(loop, p_NetAddrList[nIndex], "RouteServerConnection"));
		pRouteServerClient->setConnectionCallback(std::bind(&RouteServerClient::onConnection, this, std::placeholders::_1));
		pRouteServerClient->setMessageCallback(std::bind(&RouteServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		pRouteServerClient->setWriteCompleteCallback();
		pRouteServerClient->enableRetry();    //开启连接断开后重试

		{
			std::lock_guard<std::mutex> guard(m_ClientMutex);
			pRouteServerClient->connect();             //此处开始连接操作必须放到锁内,保证连接回调在加入list之后触发
			m_RouteServerConnectionList.push_back(pRouteServerClient);
		}

		loop->runEvery(p_nTimerInterval, std::bind(&RouteServerClient::onHeartBeat, RouteServerClient.get()));
	}

	return true;
}