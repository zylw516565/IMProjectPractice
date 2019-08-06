/**
 * TcpSession.h
 * zhangyl 2017.03.09
 **/
#pragma once

#include <memory>
#include "../net/TcpConnection.h"

using namespace net;

//为了让业务与逻辑分开，实际应该新增一个子类继承自TcpSession，让TcpSession中只有逻辑代码，其子类存放业务代码
class TcpSession
{
public:
	TcpSession(const std::weak_ptr<TcpConnection>& tmpconn);
	~TcpSession();

	TcpSession(const TcpSession& rhs) = delete;
	TcpSession& operator=(const TcpSession& rhs) = delete;

	std::weak_ptr<TcpConnection> getConnectionPtr()
	{
		if (tmpConn_->expired())
			return NULL;

		return tmpConn_.lock();
	}

	void Send(int32_t cmd, int32_t seq, const std::string& data);
	void Send(int32_t cmd, int32_t seq, const char* data, int32_t dataLength);
	void Send(const std::string& p_strSrcData);
	void Send(const char* p_pSrcBuf, int32_t p_nLength);

private:
    void SendPackage(const char* p, int32_t length);

protected:
    //TcpSession引用TcpConnection类必须是弱指针，因为TcpConnection可能会因网络出错自己销毁，此时TcpSession应该也要销毁
    std::weak_ptr<TcpConnection>    tmpConn_;
};
