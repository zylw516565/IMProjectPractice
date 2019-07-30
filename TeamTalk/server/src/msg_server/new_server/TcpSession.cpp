/**
 * TcpSession.cpp
 * zhangyl 2017.03.09
 **/
#include "TcpSession.h"
#include "../base/AsyncLog.h"
#include "../base/Singleton.h"
#include "../net/ProtocolStream.h"

TcpSession::TcpSession(const std::weak_ptr<TcpConnection>& tmpconn) 
: tmpConn_(tmpconn)
{   
}

TcpSession::~TcpSession()
{
    
}

void TcpSession::Send(int32_t cmd, int32_t seq, const std::string& data)
{
	Send(cmd, seq, data.c_str(), data.length());
}

void TcpSession::Send(int32_t cmd, int32_t seq, const char* data, int32_t dataLength)
{
	if (NULL == data || dataLength <= 0)
		return;

	std::string outbuf;
	net::BinaryWriteStream writeStream(&outbuf);
	writeStream.WriteInt32(cmd);
	writeStream.WriteInt32(seq);
	writeStream.WriteCString(data, dataLength);

	SendPackage(outbuf.c_str(), outbuf.length());
}

void TcpSession::Send(const std::string& p_strSrcData)
{
	SendPackage(p_strSrcData.c_str(), p_strSrcData.length());
}

void TcpSession::Send(const char* p_pSrcBuf, int32_t p_nLength)
{
	SendPackage(p_pSrcBuf, p_nLength);
}

void TcpSession::SendPackage(const char* p_pSrcBuf, int32_t p_nLength)
{
	if (NULL == p_pSrcBuf || p_nLength <= 0)
	{
		LOGE("p_pSrcBuf null pointer or data length error");
		return;
	}

	std::string strPackageData(p_pSrcBuf, p_nLength);
	if (tmpConn_.expired())
	{
		//FIXME: 出现这种问题需要排查
		LOGE("Tcp connection is destroyed , but why TcpSession is still alive ?");
		return;
	}

	std::shared_ptr<TcpConnection> conn = tmpConn_.lock();
	if (conn)
	{
// 		if (Singleton<MsgServer>::Instance().IsLogPackageBinaryEnabled())
// 		{
// 			size_t length = strPackageData.length();
// 			LOGI("Send data, package length: %d", length);
// 			//LOG_DEBUG_BIN((unsigned char*)strPackageData.c_str(), length);
// 		}

		conn->send(strPackageData);
	}
}