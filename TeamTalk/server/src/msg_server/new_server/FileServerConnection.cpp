#include "FileServerConnection.h"

#include "../net/TcpClient.h"

bool FileServerConnection::initConnect(EventLoop* loop, const char* ip, short port, int64_t p_nTimerInterval)
{
	if (NULL == ip || 0 == port || NULL == loop)
	{
		LOGE("ip or loop is NULL !!! ip:%x, loop: %x", ip, loop);
		return false;
	}

	//TODO:  增加多连接
	// for (;;)
	// {
		// InetAddress addr(ip, port);
		// LoginServerClientPtr pLoginServerClient(new LoginServerClient(loop, addr, "FileServerConnection"));
		// pLoginServerClient->setConnectionCallback(std::bind(&LoginServerClient::onConnection, this, std::placeholders::_1));
		// pLoginServerClient->setMessageCallback(std::bind(&LoginServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		// pLoginServerClient->setWriteCompleteCallback();
		// pLoginServerClient->enableRetry();    //开启连接断开后重试

		// {
			// std::lock_guard<std::mutex> guard(m_ClientMutex);
			// pLoginServerClient->connect();             //此处开始连接操作必须放到锁内,保证连接回调在加入list之后触发
			// m_FileServerConnectionList.push_back(pLoginServerClient);
		// }

		// loop->runEvery(p_nTimerInterval, std::bind(&LoginServerClient::onHeartBeat, LoginServerClient.get()));
	// }

	return true;
}