#ifndef __LOGIN_SERVER_CONNECTION_H__
#define __LOGIN_SERVER_CONNECTION_H__


#include "LoginServerClient.h"
#include "../base/Singleton.h"

#include <atomic>
#include <list>


class LoginServerConnection
{
public:
	LoginServerConnection() = default;
	~LoginServerConnection() = default;

	LoginServerConnection(const LoginServerConnection& rhs) = delete;
	LoginServerConnection& operator=(const LoginServerConnection& rhs) = delete;

public:

	static LoginServerConnection& getInstance() { return Singleton<LoginServerConnection>::Instance(); }

	int initConnect(EventLoop* loop, const char* ip, short port, int64_t p_nTimerInterval);

	void stop();

private:

	std::mutex                                                m_ClientMutex;
	std::list<LoginServerConnectionPtr> m_LoginServerConnectionList;				//TCP连接列表

	std::atomic_int               m_nSessionID{};
};

#endif // __LOGIN_SERVER_CONNECTION_H__