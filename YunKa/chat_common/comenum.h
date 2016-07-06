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
enum TALKSTATUS
{
	TALKSTATUS_NO,			// û�жԻ�
	TALKSTATUS_AUTOINVITE,	// �Զ�������
	TALKSTATUS_REQUEST,		// ������
	TALKSTATUS_REQUESTRESP,	// ����ظ��У���ʾ����
	TALKSTATUS_INVITE,		// ������
	TALKSTATUS_INVITERESP,	// �ظ��У���ʾ����
	TALKSTATUS_TRANSFER,	// ת����
	TALKSTATUS_TRANSFERRESP,// ת����, ��ʾ����
	TALKSTATUS_WELLCOME,	// ��������wellcome�Ի���Ϣ
	TALKSTATUS_TALK,		// �Ի���
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
	STATUS_UNKNOWN   //�û�״̬δ֪
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

#endif