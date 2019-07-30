/*
 * MsgServer.cpp
 *
 *  Created on: 2019-7-26
 *      Author: zhangyinglei
 */
#include "MsgServer.h"

#include "../base/AsyncLog.h"

MsgServer::MsgServer()
{
}

bool MsgServer::Init(const char* ip, short port, EventLoop* loop)
{
	InetAddress addr(ip, port);
	m_Server.reset(new TcpServer(loop, addr, "MsgServer", TcpServer::kReusePort));
	m_Server->setConnectionCallback(std::bind(&MsgServer::OnConnection, this, std::placeholders::_1));
	
	//启动侦听
	m_Server->start(6);
	return true;
}

void MsgServer::OnConnection(std::shared_ptr<TcpConnection> conn)
{
	if (conn->connected())
	{
		LOGD("client connected: %s", conn->peerAddress().c_str());
		++m_nSessionID;
		std::shared_ptr<ChatSession> spSession(new ChatSession(conn, m_nSessionID));
		conn->setMessageCallback(std::bind(&ChatSession::OnRead, spSession.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		std::lock_guard<std::mutex> guard(m_SessionMutex);
		m_SessionList.push_back(conn);
	}
	else
	{
		OnClose(conn);
	}
}

void MsgServer::OnClose(std::shared_ptr<TcpConnection> conn)
{
	//判断是否有用户下线
	//TODO: 增加离线会话处理
}
