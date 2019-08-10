#ifndef __ROUTE_SERVER_CONNECTION_H__
#define __ROUTE_SERVER_CONNECTION_H__


#include "RouteServerClient.h"
#include "../base/Singleton.h"

#include <atomic>
#include <list>

//RouteServerConnection

class RouteServerConnection
{
public:
	RouteServerConnection() = default;
	~RouteServerConnection() = default;

	RouteServerConnection(const RouteServerConnection& rhs) = delete;
	RouteServerConnection& operator=(const RouteServerConnection& rhs) = delete;

public:

	static RouteServerConnection& getInstance() { return Singleton<RouteServerConnection>::Instance(); }

	int initConnect(EventLoop* loop, const std::vector<InetAddress>& p_NetAddrList, int64_t p_nTimerInterval);

	void stop();

private:

	std::mutex                             m_ClientMutex;
	std::list<RouteServerConnectionPtr>     m_RouteServerConnectionList;				//TCP连接列表

	std::atomic_int                        m_nSessionID{};
};

#endif // __ROUTE_SERVER_CONNECTION_H__