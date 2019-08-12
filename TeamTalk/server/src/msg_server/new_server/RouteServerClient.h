#ifndef __ROUTE_SERVER_CLIENT_H__
#define __ROUTE_SERVER_CLIENT_H__

#include "ServerBaseClient.h"

#include <memory>

//RouteServerClient

class RouteServerClient :public ServerBaseClient
{
public:
	RouteServerClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg)
		:ServerBaseClient(loop, serverAddr, nameArg)
	{};
	~RouteServerClient();

public:

	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);
	void onWriteComplete(const TcpConnectionPtr& conn);	
	void onHeartBeat();    //每条TCP连接保持自己的心跳
  
private:

	void onClose(const TcpConnectionPtr& conn);
	void handleCommand(const IMPduPtr& pPdu);

    void _HandleKickUser(const IMPduPtr& pPdu);
	void _HandleStatusNotify(const IMPduPtr& pPdu);
    void _HandleMsgReadNotify(const IMPduPtr& pPdu);
	void _HandleMsgData(const IMPduPtr& pPdu);
	void _HandleP2PMsg(const IMPduPtr& pPdu);
	void _HandleUsersStatusResponse(const IMPduPtr& pPdu);
    void _HandlePCLoginStatusNotify(const IMPduPtr& pPdu);
    void _HandleRemoveSessionNotify(const IMPduPtr& pPdu);
    void _HandleSignInfoChangedNotify(const IMPduPtr& pPdu);

};

typedef std::shared_ptr<RouteServerClient> RouteServerClientPtr;

#endif // __ROUTE_SERVER_CLIENT_H__