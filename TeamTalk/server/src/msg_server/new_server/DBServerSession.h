#pragma 
#include "TcpClient.h"

#include <memory>
#include <atomic>

typedef std::map<uint64_t, TcpClientPtr> TcpClientConnMap;
typedef TcpClientConnMap::iterator TcpClientConnMapIter;

class DBServerSession
{
public:
	DBServerSession();
	~DBServerSession() = default;

	DBServerSession(const DBServerSession& rhs) = delete;
	DBServerSession& operator=(const DBServerSession& rhs) = delete;

public:

		int init();

private:

	void onConnection(std::shared_ptr<TcpConnection> conn);

private:
	TcpClientPtr                   m_pTcpClient;
	std::atomic_int               m_nSessionID{};
};
