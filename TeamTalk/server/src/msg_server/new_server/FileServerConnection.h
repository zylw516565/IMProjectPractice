#ifndef __FILE_SERVER_CONNECTION_H__
#define __FILE_SERVER_CONNECTION_H__


#include "FileServerClient.h"
#include "../base/Singleton.h"

#include <atomic>
#include <list>

//FileServerConnection

class FileServerConnection
{
public:
	FileServerConnection() = default;
	~FileServerConnection() = default;

	FileServerConnection(const FileServerConnection& rhs) = delete;
	FileServerConnection& operator=(const FileServerConnection& rhs) = delete;

public:

	static FileServerConnection& getInstance() { return Singleton<FileServerConnection>::Instance(); }

	int initConnect(EventLoop* loop, const char* ip, short port, int64_t p_nTimerInterval);

	void stop();

private:

	std::mutex                             m_ClientMutex;
	std::list<FileServerConnectionPtr>     m_FileServerConnectionList;				//TCP连接列表

	std::atomic_int                        m_nSessionID{};
};

#endif // __FILE_SERVER_CONNECTION_H__