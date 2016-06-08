#ifndef _COM_ENUM_H_
#define _COM_ENUM_H_

// 会话关闭的原因
enum CLOSE_CHAT_REASON
{
	REASON_TIME_OUT,			// 客服长时间没有应答超时了
	REASON_CLIENT_OFFLINE,		// 客服下线了
	REASON_CLIENT_RELEASE		// 客服主动释放了客户
};

// 访客的对话状态
enum TALKSTATUS
{
	TALKSTATUS_NO,			// 没有对话
	TALKSTATUS_AUTOINVITE,	// 自动邀请中
	TALKSTATUS_REQUEST,		// 请求中
	TALKSTATUS_REQUESTRESP,	// 请求回复中，表示主动
	TALKSTATUS_INVITE,		// 邀请中
	TALKSTATUS_INVITERESP,	// 回复中，表示被动
	TALKSTATUS_TRANSFER,	// 转接中
	TALKSTATUS_TRANSFERRESP,// 转接中, 表示被动
	TALKSTATUS_WELLCOME,	// 当代发送wellcome对话消息
	TALKSTATUS_TALK,		// 对话中
};

enum USER_STATUS
{
	STATUS_UNDEFINE=-1,
	STATUS_USERDEFINE,
	STATUS_OFFLINE,
	STATUS_ONLINE,
	STATUS_HIDE,
	STATUS_WORK,
	STATUS_EATING,
	STATUS_LEAVE,
	STATUS_ROBOT,
	STATUS_SEND_SMS,
	STATUS_SEND_MAIL,
	STATUS_BUSY,
	STATUS_SEND_MSG=12,
	STATUS_REFUSE_NEWWEBMSG,
	STATUS_UNKNOWN   //用户状态未知
};

/** 聊天消息来源用户区分 */
enum MSG_FROM_TYPE
{
	MSG_FROM_CLIENT=1,	// 坐席用户
	MSG_FROM_WEBUSER,	// 微信或web用户
	MSG_FROM_ASSIST,	// 协助对象
	MSG_FROM_SYS,		// 系统提示消息
	MSG_FROM_SELF		// 本人
};

/** 发送消息接收用户区分 */
enum USER_TYPE
{
	USER_TYPE_CLIENT=1,	// 坐席用户
	USER_TYPE_WX,		// 微信用户
	USER_TYPE_WEB		// 网页用户
};

/** 消息发送类型 */
enum MSG_TYPE
{
	MSG_TYPE_NORMAL=2,		/**< 普通消息 */
	MSG_TYPE_PREV			/**< 预知消息 */
};

/** 消息数据类型 */
enum MSG_DATA_TYPE
{
	MSG_DATA_TYPE_TEXT=1,	/**< 文字 */
	MSG_DATA_TYPE_IMAGE,	/**< 图片 */
	MSG_DATA_TYPE_VOICE,	/**< 语音 */
	MSG_DATA_TYPE_VIDEO,	/**< 视频 */
	MSG_DATA_TYPE_LOCATION,	/**< 位置 */
	MSG_DATA_TYPE_FILE,		/**< 文件 */
	MSG_DATA_TYPE_LINK,		/**< 链接 */
	MSG_DATA_TYPE_EVENT		/**< 事件 */	
};

enum CODE_CAPTURE
{
	CAPTURE_SAVE_TYPE_FILE = 0,		// 截图保存类型 0 文件形式保存 需要路径
	CAPTURE_SAVE_TYPE_CLIPBOARD,	// 1 剪切板保存 无视路径
	CAPTURE_SAVE_TYPE_BOTH			// 2 两种方式都保存 需要路径
};

// 录音操作的返回码
enum CODE_RECORD_AUDIO
{
	CODE_AUDIO_SUCCESS=0,	// 成功
	CODE_AUDIO_NO_DEVICE,	// 没有录音设备
	CODE_AUDIO_FAIL			// 失败
};

#endif