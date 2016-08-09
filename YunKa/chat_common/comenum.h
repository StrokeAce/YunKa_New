#ifndef _COM_ENUM_H_
#define _COM_ENUM_H_

// �Ự�رյ�ԭ��
enum CLOSE_CHAT_REASON
{
	REASON_TIME_OUT,			// �ͷ���ʱ��û��Ӧ��ʱ��
	REASON_CLIENT_OFFLINE,		// �ͷ�������
	REASON_CLIENT_RELEASE		// �ͷ������ͷ��˿ͻ�
};

// �ÿ͵ĶԻ�״̬
enum TALK_STATUS
{
	TALK_STATUS_NO,				// û�жԻ�
	TALK_STATUS_AUTOINVITE,		// �Զ�������
	TALK_STATUS_REQUEST,		// ������
	TALK_STATUS_REQUESTRESP,	// ����ظ��У���ʾ����
	TALK_STATUS_INVITE,			// ������
	TALK_STATUS_INVITERESP,		// �ظ��У���ʾ����
	TALK_STATUS_TRANSFER,		// ת����
	TALK_STATUS_TRANSFERRESP,	// ת����, ��ʾ����
	TALK_STATUS_WELLCOME,		// ��������wellcome�Ի���Ϣ
	TALK_STATUS_TALK,			// �Ի���
};


enum TREENODEENUM
{
	DEFAULT_TYPE = 0,
	MYSELF_CHILD_ACTIVE_1, //�Լ��Ի��� �Լ����������Э��
	MYSELF_CHILD_1,   //�Լ��Ի���  �Լ�ͬ���Э��
	MYSELF_CHILD_2,   //ת����
    MYSELF_CHILD_3,   //������
	MYSELF_CHILD_4,   //�ڲ��Ի�
	OTHER_CHILD_1,   //���� �Ի���
	OTHER_CHILD_2,   //ת����
	OTHER_CHILD_3,   //������
	NODE_END = 20
};

enum VISITOR_TYPE
{
	DEFAULT_POS = 0,
	HOST_USER_DEFAULT = 0,
	//��ϯ״̬
	HOST_USER_ONLINE,
	HOST_USER_OFFLINE,
	HOST_USER_MYSELF,
	//�ÿ�״̬
	VISITOR_REQ_ING,                       //������
	VISITOR_TALKING_MYSELF,                //�Ի���
	VISITOR_TALKING_OTHER,                 //�Ի���
	VISITOR_TALKING_HELP_OTHER,            //�Ǳ��˵�Э������ �Ի���
    VISITOR_TRANING,                       //ת����
	VISITOR_INVOTING,                      //������
	VISITOR_IN_TALK_ING,                   //�ڲ��Ի���
	VISITOR_ONLINE_AUTO_INVOTING,          //�Զ�������
	VISITOR_ONLINE_AUTO_VISITING,          //������
	VISITOR_ONLINE_AUTO_END,               //�ѽ���  
	VISITOR_END = 100

};

enum USER_STATUS
{
	USER_STATUS_UNDEFINE = -1,
	USER_STATUS_USERDEFINE,
	USER_STATUS_OFFLINE,
	USER_STATUS_ONLINE,
	USER_STATUS_HIDE,
	USER_STATUS_WORK,
	USER_STATUS_EATING,
	USER_STATUS_LEAVE,
	USER_STATUS_ROBOT,
	USER_STATUS_SEND_SMS,
	USER_STATUS_SEND_MAIL,
	USER_STATUS_BUSY,
	USER_STATUS_SEND_MSG = 12,
	USER_STATUS_REFUSE_NEWWEBMSG,
	USER_STATUS_UNKNOWN   //�û�״̬δ֪
};

/** ������Ϣ��Դ�û����� */
enum MSG_FROM_TYPE
{
	MSG_FROM_CLIENT=1,	// ��ϯ�û�
	MSG_FROM_WEBUSER,	// ΢�Ż�web�û�
	MSG_FROM_ASSIST,	// Э������
	MSG_FROM_SYS,		// ϵͳ��ʾ��Ϣ
	MSG_FROM_SELF		// ����
};

/** ������Ϣ�����û����� */
enum MSG_RECV_TYPE
{	
	MSG_RECV_CLIENT = 1,// ��ϯ�û�
	MSG_RECV_WX,		// ΢���û�
	MSG_RECV_WEB,		// ��ҳ�û�
	MSG_RECV_ERROR=10
};

/** ��Ϣ�������� */
enum MSG_TYPE
{
	MSG_TYPE_NORMAL=2,		/**< ��ͨ��Ϣ */
	MSG_TYPE_PREV			/**< Ԥ֪��Ϣ */
};

/** ��Ϣ�������� */
enum MSG_DATA_TYPE
{
	MSG_DATA_TYPE_TEXT=1,	/**< ���� */
	MSG_DATA_TYPE_IMAGE,	/**< ͼƬ */
	MSG_DATA_TYPE_VOICE,	/**< ���� */
	MSG_DATA_TYPE_VIDEO,	/**< ��Ƶ */
	MSG_DATA_TYPE_LOCATION,	/**< λ�� */
	MSG_DATA_TYPE_FILE,		/**< �ļ� */
	MSG_DATA_TYPE_LINK,		/**< ���� */
	MSG_DATA_TYPE_EVENT,	/**< �¼� */
};

enum CODE_CAPTURE
{
	CAPTURE_SAVE_TYPE_FILE = 0,		// ��ͼ�������� 0 �ļ���ʽ���� ��Ҫ·��
	CAPTURE_SAVE_TYPE_CLIPBOARD,	// 1 ���а屣�� ����·��
	CAPTURE_SAVE_TYPE_BOTH			// 2 ���ַ�ʽ������ ��Ҫ·��
};

// ¼�������ķ�����
enum CODE_RECORD_AUDIO
{
	CODE_AUDIO_SUCCESS=0,	// �ɹ�
	CODE_AUDIO_NO_DEVICE,	// û��¼���豸
	CODE_AUDIO_FAIL,		// ʧ��
	CODE_AUDIO_LITTLE_TIME,	// ʱ��̫��
	CODE_AUDIO_IS_RECORDING	// ����¼��
};

enum RESULT_STATUS
{
	INVITE_ACCEPT,		// �ɹ�����
	INVITE_REFUSE,		// �ܾ�����
	INVITE_ING			// ������
};

// ֪ͨ����ÿ���Ϣ�и���
enum WEBUSER_INFO_NOTIFY_TYPE
{
	NOTIFY_NAME,	// ���ֵĸ���
	NOTIFY_IS_WX,	// �û����͵ĸ���
	NOTIFY_ID		// �û�webuserid�ĸ�ϸ
};

enum ALERT_TYPE
{
	ALERT_NEW_VISIT=1,
	ALERT_NEW_CHAT,
	ALERT_NEW_MSG,
	ALERT_NEW_TRANSFER,
	ALERT_NEW_OTHER
};

#endif