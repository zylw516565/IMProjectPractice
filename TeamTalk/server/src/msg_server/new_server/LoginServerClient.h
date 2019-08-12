#ifndef __LOGIN_SERVER_CLIENT_H__
#define __LOGIN_SERVER_CLIENT_H__

#include "ServerBaseClient.h"

#include <memory>

class LoginServerClient :public ServerBaseClient
{
public:
	LoginServerClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg)
		:ServerBaseClient(loop, serverAddr, nameArg)
	{}
	~LoginServerClient(){}

public:

	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);
	void onWriteComplete(const TcpConnectionPtr& conn);	
	void onHeartBeat();    //每条TCP连接保持自己的心跳
  
private:

	void onClose(const TcpConnectionPtr& conn);
	void handleCommand(const IMPduPtr& pPdu);

};

typedef std::shared_ptr<LoginServerClient> LoginServerClientPtr;

#endif // __LOGIN_SERVER_CLIENT_H__
