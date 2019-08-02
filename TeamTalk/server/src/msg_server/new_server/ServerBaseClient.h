#ifndef __SERVER_BASE_CLIENT_H__
#define __SERVER_BASE_CLIENT_H__

#include "TcpClient.h"
#include "EventLoop.h"
#include "InetAddress.h"

class ServerBaseClient : public TcpClient
{
public:
	ServerBaseClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg)
		:TcpClient(loop, serverAddr, nameArg)
	{};
	virtual ~ServerBaseClient();

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

		//�յ������ݲ���һ�������İ�
		int32_t nPkgLen = pBuffer->readInt32();
		if (pBuffer->readableBytes() < nPkgLen)
		{
			LOGE("recv data error, data is incomplete. data len = %d, nPkgLen = %d", pBuffer->readableBytes(), nPkgLen);
			return;
		}

		pIMPdu->Write(pBuffer->peek(), nPkgLen);
		pIMPdu->ReadPduHeader(pBuffer->peek(), IM_PDU_HEADER_LEN);
	}

	//ÿ��TCP���ӱ����Լ�������
	 void sendHeartBeat()
	 {
		 IM::Other::IMHeartBeat msg;
		 CImPdu pdu;
		 pdu.SetPBMsg(&msg);
		 pdu.SetServiceId(SID_OTHER);
		 pdu.SetCommandId(CID_OTHER_HEARTBEAT);

		 connection()->send(pdu.GetBuffer(), pdu.GetLength());
		 LOGT("heartbeat: length:%d, content:%s", pdu.GetLength(), pdu.GetBuffer());
	 };

};

#endif // __SERVER_BASE_CLIENT_H__