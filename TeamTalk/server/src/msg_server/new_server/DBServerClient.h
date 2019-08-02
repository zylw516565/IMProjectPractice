#ifndef __DB_SERVER_CLIENT_H__
#define __DB_SERVER_CLIENT_H__

#include "ServerBaseClient.h"

#include <memory>

class DBServerClient :public ServerBaseClient
{
public:
	DBServerClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg)
		:ServerBaseClient(loop, serverAddr, nameArg)
	{};
	~DBServerClient();

public:

	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);
	void onWriteComplete(const TcpConnectionPtr& conn);	
	void onHeartBeat();    //每条TCP连接保持自己的心跳
  
private:

	void onClose(const TcpConnectionPtr& conn);
	void handleCommand(const IMPduPtr& pPdu);

	void _HandleValidateResponse(const IMPduPtr& pPdu);
	void _HandleRecentSessionResponse(const IMPduPtr& pPdu);
	void _HandleAllUserResponse(const IMPduPtr& pPdu);
	void _HandleGetMsgListResponse(const IMPduPtr& pPdu);
	void _HandleGetMsgByIdResponse(const IMPduPtr& pPdu);
	void _HandleMsgData(const IMPduPtr& pPdu);
	void _HandleUnreadMsgCountResponse(const IMPduPtr& pPdu);
	void _HandleGetLatestMsgIDRsp(const IMPduPtr& pPdu);
	void _HandleDBWriteResponse(const IMPduPtr& pPdu);
	void _HandleUsersInfoResponse(const IMPduPtr& pPdu);
	void _HandleStopReceivePacket(const IMPduPtr& pPdu);
	void _HandleRemoveSessionResponse(const IMPduPtr& pPdu);
	void _HandleChangeAvatarResponse(const IMPduPtr& pPdu);
	void _HandleChangeSignInfoResponse(const IMPduPtr& pPdu);
	void _HandleSetDeviceTokenResponse(const IMPduPtr& pPdu);
	void _HandleGetDeviceTokenResponse(const IMPduPtr& pPdu);
	void _HandleDepartmentResponse(const IMPduPtr& pPdu);

	void _HandlePushShieldResponse(const IMPduPtr& pPdu);
	void _HandleQueryPushShieldResponse(const IMPduPtr& pPdu);

};

typedef std::shared_ptr<DBServerClient> DBServerClientPtr;

#endif // __DB_SERVER_CLIENT_H__