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


enum TREENODEENUM
{
	DEFAULT_TYPE = 0,
	MYSELF_CHILD_ACTIVE_1, //自己对话中 自己被动邀请的协助
	MYSELF_CHILD_1,   //自己对话中  自己同意的协助
	MYSELF_CHILD_2,   //转接中
    MYSELF_CHILD_3,   //邀请中
	MYSELF_CHILD_4,   //内部对话
	OTHER_CHILD_1,   //别人 对话中
	OTHER_CHILD_2,   //转接中
	OTHER_CHILD_3,   //邀请中





	NODE_END = 20

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
enum MSG_RECV_TYPE
{	
	MSG_RECV_CLIENT = 1,// 坐席用户
	MSG_RECV_WX,		// 微信用户
	MSG_RECV_WEB,		// 网页用户
	MSG_RECV_ERROR=10
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
	MSG_DATA_TYPE_EVENT,	/**< 事件 */
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
	CODE_AUDIO_FAIL,		// 失败
	CODE_AUDIO_LITTLE_TIME,	// 时间太短
	CODE_AUDIO_IS_RECORDING	// 正在录音
};

enum RESULT_STATUS
{
	INVITE_ACCEPT,		// 成功接受
	INVITE_REFUSE,		// 拒绝接受
	INVITE_ING			// 邀请中
};

#endif