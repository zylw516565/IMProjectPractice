#ifndef __FILE_SERVER_CLIENT_H__
#define __FILE_SERVER_CLIENT_H__

#include "ServerBaseClient.h"

#include <memory>

//FileServerClient

class FileServerClient :public ServerBaseClient
{
public:
	FileServerClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg)
		:ServerBaseClient(loop, serverAddr, nameArg)
	{};
	~FileServerClient();

public:

	void onConnection(const TcpConnectionPtr& conn);
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);
	void onWriteComplete(const TcpConnectionPtr& conn);	
	void onHeartBeat();    //每条TCP连接保持自己的心跳
  
private:

	void onClose(const TcpConnectionPtr& conn);
	void handleCommand(const IMPduPtr& pPdu);

	void _HandleFileMsgTransRsp(const IMPduPtr& pPdu);
    void _HandleFileServerIPRsp(const IMPduPtr& pPdu);

};

typedef std::shared_ptr<FileServerClient> FileServerClientPtr;

#endif // __FILE_SERVER_CLIENT_H__