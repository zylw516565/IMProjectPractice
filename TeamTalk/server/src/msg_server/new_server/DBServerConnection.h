#ifndef __DBSERVER_CONNECTION_H__
#define __DBSERVER_CONNECTION_H__

#include "DBServerClient.h"
#include "../base/Singleton.h"

#include <atomic>
#include <list>


class DBServerConnection
{
public:
	DBServerConnection() = default;
	~DBServerConnection() = default;

	DBServerConnection(const DBServerConnection& rhs) = delete;
	DBServerConnection& operator=(const DBServerConnection& rhs) = delete;

public:

	static DBServerConnection& getInstance() { return Singleton<DBServerConnection>::Instance(); }

	int initConnect(EventLoop* loop, const char* ip, short port, int64_t p_nTimerInterval);

	void stop();

private:

	std::mutex                                 m_ClientMutex;
	std::list<DBServerClientPtr> m_DBServerClientList;				//TCP连接列表

	std::atomic_int               m_nSessionID{};
};

#endif // __DBSERVER_CONNECTION_H__