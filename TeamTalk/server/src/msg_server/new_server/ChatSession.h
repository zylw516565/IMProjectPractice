/** 
 * ChatSession.h
 * zhangyl, 2017.03.10
 **/

#pragma once
#include "../net/Buffer.h"
#include "TcpSession.h"
using namespace net;

struct OnlineUserInfo
{
    int32_t     userid;
    std::string username;
    std::string nickname;
    std::string password;
    int32_t     clienttype;     //客户端类型, 0未知, pc=1, android/ios=2
    int32_t     status;         //在线状态 0离线 1在线 2忙碌 3离开 4隐身
};


/**
 * 聊天会话类
 */
class ChatSession : public TcpSession
{
public:
    ChatSession(const std::shared_ptr<TcpConnection>& conn, int sessionid);
    virtual ~ChatSession();

    ChatSession(const ChatSession& rhs) = delete;
    ChatSession& operator =(const ChatSession& rhs) = delete;

    //有数据可读, 会被多个工作loop调用
    void OnRead(const std::shared_ptr<TcpConnection>& conn, Buffer* pBuffer, Timestamp receivTime);

	void SetOpen() { m_bOpen = true; }
	bool IsOpen() { return m_bOpen; }

	void SetKickOff() { m_bKickOff = true; }
	bool IsKickOff() { return m_bKickOff; }

	uint32_t GetUserId() { return m_user_id; }
	void SetUserId(uint32_t user_id) { m_user_id = user_id; }

	uint32_t GetHandle() { return m_nSessionId; }

    
private:
	void handleCommand(const IMPduPtr& pIMPdu);

private:
	void _HandleHeartBeat(const IMPduPtr& pPdu);
	void _HandleLoginRequest(const IMPduPtr& pPdu);
	void _HandleLoginOutRequest(const IMPduPtr& pPdu);
	void _HandleClientRecentContactSessionRequest(const IMPduPtr& pPdu);
	void _HandleClientMsgData(const IMPduPtr& pPdu);
	void _HandleClientMsgDataAck(const IMPduPtr& pPdu);
	void _HandleClientTimeRequest(const IMPduPtr& pPdu);
	void _HandleClientGetMsgListRequest(const IMPduPtr& pPdu);
	void _HandleClientGetMsgByMsgIdRequest(const IMPduPtr& pPdu);
	void _HandleClientUnreadMsgCntRequest(const IMPduPtr& pPdu);
	void _HandleClientMsgReadAck(const IMPduPtr& pPdu);
	void _HandleClientGetLatestMsgIDReq(const IMPduPtr& pPdu);
	void _HandleClientP2PCmdMsg(const IMPduPtr& pPdu);
	void _HandleClientUserInfoRequest(const IMPduPtr& pPdu);
	void _HandleClientUsersStatusRequest(const IMPduPtr& pPdu);
	void _HandleClientRemoveSessionRequest(const IMPduPtr& pPdu);
	void _HandleClientAllUserRequest(const IMPduPtr& pPdu);
	void _HandleChangeAvatarRequest(const IMPduPtr& pPdu);
	void _HandleChangeSignInfoRequest(const IMPduPtr& pPdu);

	void _HandleClientDeviceToken(const IMPduPtr& pPdu);
	void _HandleKickPCClient(const IMPduPtr& pPdu);
	void _HandleClientDepartmentRequest(const IMPduPtr& pPdu);
	void _SendFriendStatusNotify(uint32_t status);
	void _HandlePushShieldRequest(const IMPduPtr& pPdu);
	void _HandleQueryPushShieldRequest(const IMPduPtr& pPdu);

private:
	int32_t       m_nSessionId;                 //session id
	std::string          m_login_name;        //登录名拼音
	uint32_t        m_user_id;

	bool			m_bOpen;	// only DB validate passed will be set to true;
	bool            m_bKickOff;
};

//TODO:放开注释?
//typedef std::weak_ptr<ChatSession> ChatSessionPtr;