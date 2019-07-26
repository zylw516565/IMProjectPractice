
#pragma once
#include <memory>
#include <list>
#include <map>
#include <mutex>
#include <atomic>

#include "./net/TcpServer.h"
#include "./net/EventLoop.h"

class MsgServer final
{
public:
	MsgServer();
	~MsgServer() = default;

	MsgServer(const MsgServer& rhs) = delete;
	MsgServer& operator= (const MsgServer& rhs) = delete;

	bool Init(const char* ip, short port, EventLoop* loop);

private:
	//新连接到来调用或连接断开，所以需要通过conn->connected()来判断，一般只在主loop里面调用
	void OnConnection(std::shared_ptr<TcpConnection> conn);

private:
	std::shared_ptr<TcpServer>                       m_Server;
	std::list<std::shared_ptr<ChatSession>> m_SessionList;      //会话列表
	std::mutex                                                       m_SessionMutex;  //多线程之间保护m_Session
	std::atomic_int                                               m_nSessionID{};
	std::mutex                                                       m_IDMutex;           //多线程之间保护
	std::atomic_bool                                            m_logPackageBinary;  //是否日志打印出包的二进制数据
};

