#ifndef __SERVER_BASE_CLIENT_H__
#define __SERVER_BASE_CLIENT_H__

#include "../base/AsyncLog.h"
#include "../net/TcpClient.h"
#include "../net/EventLoop.h"
#include "../net/InetAddress.h"

#include "IM.BaseDefine.pb.h"
#include "IM.Other.pb.h"

#include "ImPduBase.h"

using namespace IM::BaseDefine;
using namespace net;

class ServerBaseClient : public TcpClient
{
public:
    
	ServerBaseClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg)
        :TcpClient(loop, serverAddr, nameArg)
	{}
	virtual ~ServerBaseClient(){}

protected:

	 void parseMessageHeader(const TcpConnectionPtr& conn, Buffer* pBuffer, Timestamp receiveTime, IMPduPtr& pIMPdu)
	{
		if (nullptr == conn.get() || NULL == pBuffer)
			return;

		if (pBuffer->readableBytes() < sizeof(int32_t))
		{
			LOGE("recv data error, data is incomplete. data len = %d", pBuffer->readableBytes());
			return;
		}

		//收到的数据不够一个完整的包
		int32_t nPkgLen = pBuffer->readInt32();
		if (pBuffer->readableBytes() < nPkgLen)
		{
			LOGE("recv data error, data is incomplete. data len = %d, nPkgLen = %d", pBuffer->readableBytes(), nPkgLen);
			return;
		}

		pIMPdu->Write((uchar_t*)(pBuffer->peek()), nPkgLen);
		pIMPdu->ReadPduHeader((uchar_t*)pBuffer->peek(), IM_PDU_HEADER_LEN);
	}

	//每条TCP连接保持自己的心跳
	 void sendHeartBeat()
	 {
		 IM::Other::IMHeartBeat msg;
		 CImPdu pdu;
		 pdu.SetPBMsg(&msg);
		 pdu.SetServiceId(SID_OTHER);
		 pdu.SetCommandId(CID_OTHER_HEARTBEAT);

		 connection()->send(pdu.GetBuffer(), pdu.GetLength());
		 LOGT("heartbeat: length:%d, content:%s", pdu.GetLength(), pdu.GetBuffer());
	 }

};

#endif // __SERVER_BASE_CLIENT_H__
