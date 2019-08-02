#pragma once

#include "../base/Singleton.h"

class ChatSession;
typedef std::weak_ptr<ChatSession> ChatSessionPtr;

class ChatFileHandler
{
public:
	ChatFileHandler(){};
	~ChatFileHandler() = default;

	ChatFileHandler(const ChatFileHandler& rhs) = delete;
	ChatFileHandler& operator=(const ChatFileHandler& rhs) = delete;

public:

	static ChatFileHandler& getInstance() { return Singleton<ChatFileHandler>::Instance(); }

	void HandleClientFileRequest(const ChatSessionPtr& pMsgConn, const IMPduPtr& pPdu);
	void HandleClientFileHasOfflineReq(const ChatSessionPtr& pMsgConn, const IMPduPtr& pPdu);
	void HandleClientFileAddOfflineReq(const ChatSessionPtr& pMsgConn, const IMPduPtr& pPdu);
	void HandleClientFileDelOfflineReq(const ChatSessionPtr& pMsgConn, const IMPduPtr& pPdu);
	void HandleFileHasOfflineRes(const IMPduPtr& pPdu);
	void HandleFileNotify(const IMPduPtr& pPdu);

};
