/*
 * MsgServer.cpp
 *
 *  Created on: 2019-7-26
 *      Author: zhangyinglei
 */
#include "MsgServer.h"

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

}