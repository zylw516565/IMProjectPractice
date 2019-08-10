#include "FileServerConnection.h"

#include "../net/TcpClient.h"

bool FileServerConnection::initConnect(EventLoop* loop, const std::vector<InetAddress>& p_NetAddrList, int64_t p_nTimerInterval)
{
	if (p_NetAddrList.empty() || NULL == loop)
	{
		LOGE("p_NetAddrList is empty !!! loop: %x", loop);
		return false;
	}

	//TODO:  增加多连接
	for (int nIndex = 0; nIndex <= p_NetAddrList.size(); ++nIndex)
	{
		FileServerClientPtr pFileServerClient(new FileServerClient(loop, p_NetAddrList[nIndex], "FileServerConnection"));
		pFileServerClient->setConnectionCallback(std::bind(&FileServerClient::onConnection, this, std::placeholders::_1));
		pFileServerClient->setMessageCallback(std::bind(&FileServerClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		pFileServerClient->setWriteCompleteCallback();
		pFileServerClient->enableRetry();    //开启连接断开后重试

		{
			std::lock_guard<std::mutex> guard(m_ClientMutex);
			pFileServerClient->connect();             //此处开始连接操作必须放到锁内,保证连接回调在加入list之后触发
			m_FileServerConnectionList.push_back(pFileServerClient);
		}

		loop->runEvery(p_nTimerInterval, std::bind(&FileServerClient::onHeartBeat, FileServerClient.get()));
	}

	return true;
}