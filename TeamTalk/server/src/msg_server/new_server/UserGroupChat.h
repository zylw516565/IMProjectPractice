#pragma once

#include "ChatSession.h"
#include "../base/Singleton.h"

class ChatSession;

typedef std::weak_ptr<ChatSession> ChatSessionPtr;

class UserGroupChat
{
public:
	UserGroupChat(){};
	~UserGroupChat() = default;

	UserGroupChat(const UserGroupChat& rhs) = delete;
	UserGroupChat& operator=(const UserGroupChat& rhs) = delete;

public:

	static UserGroupChat& getInstance() { return Singleton<UserGroupChat>::Instance(); }

	void HandleClientGroupNormalRequest(const IMPduPtr& pPdu, const ChatSessionPtr& pFromConn);
	void HandleGroupNormalResponse(const IMPduPtr& pPdu);

	void HandleClientGroupInfoRequest(const IMPduPtr& pPdu, const ChatSessionPtr& pFromConn);
	void HandleGroupInfoResponse(const IMPduPtr& pPdu);

	void HandleGroupMessage(const IMPduPtr& pPdu);
	void HandleGroupMessageBroadcast(const IMPduPtr& pPdu);

	void HandleClientGroupCreateRequest(const IMPduPtr& pPdu, const ChatSessionPtr& pFromConn);
	void HandleGroupCreateResponse(const IMPduPtr& pPdu);

	void HandleClientGroupChangeMemberRequest(const IMPduPtr& pPdu, const ChatSessionPtr& pFromConn);
	void HandleGroupChangeMemberResponse(const IMPduPtr& pPdu);
	void HandleGroupChangeMemberBroadcast(const IMPduPtr& pPdu);

	void HandleClientGroupShieldGroupRequest(const IMPduPtr& pPdu,
		const ChatSessionPtr& pFromConn);

	void HandleGroupShieldGroupResponse(const IMPduPtr& pPdu);
	void HandleGroupGetShieldByGroupResponse(const IMPduPtr& pPdu);

private:
	void _SendPduToUser(const IMPduPtr& pPdu, uint32_t user_id, const ChatSessionPtr& pReqConn = nullptr);

};