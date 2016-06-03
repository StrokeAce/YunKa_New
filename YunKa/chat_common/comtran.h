#if !defined _COMMTRAN_EM
#define _COMMTRAN_EM

//����ʹ�õ�����
#define CMD_COM_UNKNOWN						0x0000	// 0 Ԥ��
//ϵͳ�������һЩ����
#define CMD_COM_SENDFAIL					0x0001	// 1 ����ʧ��
#define	CMD_COM_TIMEOUT						0x0002	// 2 ͨѶ��ʱ
#define	CMD_COM_PACKERROR					0x0003	// 3 ����������
#define	CMD_COM_SOCKINITERROR				0x0004	// 4 sock��ʼ������
#define	CMD_COM_IPERROR						0x0005	// 5 �Ƿ���ip


/*************************************************************************/
/*******              		�����		******************/
/*************************************************************************/

//һЩͨ�õ�����
#define CMD_COM_ACK							0x000A	// 10 �յ������Ӧ
#define CMD_COM_ACKEX						0x000B	// 11 ����Ļ�Ӧ���������յ��󣬱��뷢��һ��ack������һ���û���

//������û�а��ṹ
#define CMD_COM_PING						0x042E  // 1070 �û�PING������
#define CMD_COM_MULTIPING					0x0442  // 1090 �������û���ͣ�̼�

//�������ӵ�����
#define CMD_COM_P2PCONNECTNAT				0x1388  // 5000	�������ӵĲ�������

//������Ϊ��������
#define CMD_SRV_ERROR_CLTSEND				0x0000	// 0  �ͻ��˶˴���һ�����������
#define CMD_SRV_RESP_FAIL					0x0014	// 20 ��������Ӧ����ĳһ����ʧ��
#define CMD_SRV_SERVER_COPY					0x001E	// 30 ���������б�־�Ϳͻ��˲�һ��
#define CMD_SRV_CONF_LOGOFF					0x0028	// 40 ȷ��ĳ������
#define CMD_SRV_SERVER_BUSY					0x003C	// 60 ������æ

#define CMD_SRV_CONF_REG					0x0046	// 70 ȷ��ע�����û�
#define CMD_SRV_DENY_REG					0x0050	// 80 �ܾ�ע�����û�
#define CMD_SRV_CONF_LOGON					0x005A	// 90 ȷ���û���¼
#define CMD_SRV_DENY_LOGON					0x0064	// 100 �ܾ��û���¼

//old
#define CMD_SRV_STATUS_FRDONLINE			0x006E	// 110 ֪ͨ���ҵ���������
#define CMD_SRV_STATUS_FRDOFFLINE			0x0078	// 120 ֪ͨ���ҵ���������
#define CMD_SRV_STATUS_USER					0x01A4	// 130 �û�����״̬

#define CMD_SRV_SEARCH_SUCC					0x008C	// 140 ��ѯ���ѽ��
#define CMD_SRV_SEARCH_FAIL					0x0096	// 150 ��ѯ����ʧ��

#define CMD_SRV_UPDATE_SUCC					0x00C8	// 200 ���³ɹ�
#define CMD_SRV_UPDATE_FAIL					0x00D2	// 210 ����ʧ��
#define CMD_SRV_SEND_MSG_OFFLINE			0x00DC	// 220	������Ϣ

#define CMD_SRV_REP_USERINFO				0x0118	// 280 �����û���Ϣ
#define CMD_SRV_REP_SMS						0x0122	// 290 ���ض����յ�����Ϣ
#define CMD_SRV_REP_IDNAME					0x0230	// 300 ����ĳһЩ�û��ļ���Ϣ

//old
#define CMD_SRV_STATUS_USER_FORM			0x01A4	// 420 �û�����״̬

#define CMD_SRV_REP_TRANSFERCLIENT			0x01B8  // 440 ת����ʱ�û��ɹ�ʧ��
#define CMD_SRV_TRANSFERCLIENT				0x01C2  // 450 ֪ͨ�����ߣ�ת����ʱ�û�

#define CMD_SRV_LIST_URL					0x01FE	// 510 WebOA����
#define CMD_SRV_LIST_DEPARTMENT				0x0208	// 520 ���ŵ��б�
#define CMD_SRV_LIST_FRD					0x0212	// 530 ���������б�
#define CMD_SRV_LIST_ONLINEFRD				0x0226  // 550 ���������б�

//����ģ�´������ϵ�һЩ�Ự��������
#define CMD_FLOAT_CREATECHAT				0x0230	// 560 �����㲥�Ự���� webui->GP->C
#define CMD_FLOAT_LISTCHAT					0x0231	// 561 ���߻Ự�б�    C->GP
#define CMD_FLOAT_CHATMSG					0x0232	// 562 �㲥��Ư����ָ������Աwebui<->GP<->C
#define CMD_FLOAT_CHATMSG_ACK				0x0233	// 563 GP�Է�����CMD_FLOAT_CHATMSG����ack  GP->C
#define CMD_FLOAT_STARTORSTOP_MSG			0x0234	// 564 �ͻ���֪ͨ�������㲻�㲥��Ϣ���ͻ���C->GP
#define CMD_FLOAT_ACCEPTCHAT				0x0235	// 565 �¼�����0���ͷ����ܻỰ 1���Ựת�ƣ�webui->GP<-C
#define CMD_FLOAT_TRANSQUEST				0x0236	// 566 �Ự��������GP<-C
#define CMD_FLOAT_TRANSFAILED				0x0237	// 567 �Ự���볬ʱʧ��GP->C
#define CMD_FLOAT_CLOSECHAT					0x0238	// 568 �Ự��ֹ��     webui->GP<-C
#define CMD_FLOAT_GETCHATMSG				0x0239	// 569 ��ȡ�Ự��Ϣ   C->GP
#define CMD_FLOAT_INVITE_REQUEST			0x023A	// 570 ����ĳЩ�˽���Ự��ֻ������ͬһ����Ա����Ŀͷ�
#define CMD_FLOAT_INVITE_RESULT				0x023B	// 571 �����뷽����������
#define CMD_FLOAT_RELEASE				0x023C	// 572 �ͷ�Ӧ����Ϣ��ʱ        GP->C
#define CMD_FLOAT_CMDERROR					0x023F	// 575 ��ȡ�Ự�е���Ϣ      GP->C
#define CMD_FLOAT_CHATINFO					0x023D	// 573 �Ự����ϸ��Ϣ
#define CMD_FLOAT_SHARELIST					0x0250	// 592 ��ȡ�ɷ�����Ϣ�����б��Ѻö����б�

#define CMD_SRV_GROUP_LIST					0x02C6	// 710 �������б� ����CMD_SRV_LIST_GROUP,(0��1��2)��
#define CMD_SRV_GROUP						0x02D0	// 720 ����������,(0��1��2)��
#define CMD_SRV_GROUP_NOTICE				0x02DA  // 730 ����������ˡ���Ƶ����ȵ�֪ͨ,(0��2)��

//�ͻ���Ϊ��������
#define CMD_CLT_LOGON						0x03E8  // 1000 �û���¼
//�ͻ��˵İ��Ĵ���
//old
#define CMD_CLT_SEND_ALLMSG					0x03F2  // 1010 ����Ⱥ����Ϣ

#define CMD_CLT_REG							0x03FC  // 1020 �û�ע��
#define CMD_CLT_LOGOFF						0x0438  // 1080 �û�����

#define CMD_CLT_SEARCH_FRD					0x041A  // 1050 ��UIN��ѯ
#define CMD_CLT_SEARCH_MUTIFRD				0x0424  // 1060 ����Ϣ��ѯ

//new
#define CMD_CLT_GET_INFO					0x044C  // 1100 ��÷�������ĳһЩ��Ϣ����(�����б���Ϣ)

//old
#define CMD_CLT_GET_CLT_INFO				0x0460  // 1120 ��ø�������

#define CMD_CLT_CHECKPROXY					0x0474  // 1140 �����������
#define CMD_CLT_TRANSFERCLIENT				0x047E  // 1150 ת����ʱ�û�

//old
#define CMD_CLT_UPDATE_STATUS				0x04D8  // 1240 ����״̬
#define CMD_CLT_UPDATE_INFO					0x0500  // 1280 �����û�����
#define CMD_CLT_UPDATE_IMAGE				0x050A  // 1290 ���¸���ͼƬ
#define CMD_CLT_UPDATE_PW					0x0514  // 1300 ��������
#define CMD_CLT_UPDATE_LOGINSTRING			0x051E  // 1310 ���µ�¼�������ַ���

#define CMD_CLT_GROUP_INVITE				0x05A0	// 1440 �����������,(0��2)������


//�ͻ��˷�������Ҫʹ�õ�����
#define CMD_COM_SEND_MSG					0x0F00	// 3840 �û���������
#define CMD_CLT_SEND_SMS					0x0F28  // 3880 ���Ͷ���

#define CMD_COM_AUTH_NEED					0x0FC3  // 3900 �Է���Ҫ��֤
#define CMD_COM_AUTH_RESP					0x0F46	// 3910 �ظ���֤��Ϣ, �Ƿ�ͬ��
#define CMD_COM_ADD_GROUP                   0x0F4F  // 3920 �ͻ��������飬ɾ���飬������,���������ڵ���fendjz
#define CMD_COM_ADD_FRD						0x0F50  // 3920 ����ɾ��ĳ��

#define CMD_SRV_ALL_FRIEND_OFFLINE			0x1100	// 4352 ����������ϵ�˶�����,����������½,zht

//�ļ����������
#define CMD_COM_FILE_GETLIST				0x0F64  // 3940	�ͻ��������͹����ļ��б�
//old 
#define CMD_COM_FILE_GET					0x0F6E  // 3950 �ͻ��˷��������ļ�,������Ҫ�����֤
#define CMD_COM_FILE_SENDLIST				0x0F78  // 3960 �ͻ��˷��͹����ļ��б�
//old
#define CMD_COM_FILE_SEND					0x0F8C  // 3980 �ͻ���������ĳһ���ļ�
#define CMD_COM_SCREEN						0x0F96	// 3990 ���ͽ�ͼ

//��Ƶ��Ƶ��������
#define CMD_COM_VIDEOVOICE_INVITE			0x0FA0	// 4000	���������Ƶ������
#define CMD_COM_VIDEOVOICE_GET				0x0FAA  // 4010	���ĳ�˵���Ƶ����ȡĳ������

//����Ƶ��ʹ��
#define CMD_COM_VIDEOVOICE					0x0FB4  // 4020	��Ƶ��Ƶ��ͨѶ��

//����Ƶ��ʹ�þ�
#define CMD_COM_VIDEO_FORM					0x109A  // 4250	 
#define CMD_COM_AUDIO_FORM					0x10A4  // 4260	 
#define CMD_COM_VIDEOTEXT_FORM				0x10AE  // 4270	
#define CMD_COM_GETPORT						0x10B8  // 4280	�ھ���������ʱ������˿�
#define CMD_COM_SENDPORT					0x10C2  // 4290	�ھ���������ʱ�����ͱ��ض˿ڸ������û�
#define CMD_COM_HANG						0x10CC  // 4300	�Ҷ�����

//�ļ����������
#define CMD_COM_FILE_START					0x1194  // 4500 �ͻ���������/����ĳһ���ļ�(��һ�Ρ�������ֹͣ)
#define CMD_COM_FILE_SENDATTR				0x119E  // 4510 �����ļ���һЩ���ԣ��������ơ����ڡ���С��
#define CMD_COM_FILE_SENDFCT 				0x11A8  // 4520 �����ļ���У���
#define CMD_COM_FILE_SENDDATA 				0x11B2  // 4530 �����ļ�������
#define CMD_COM_FILE_SENDEND 				0x11BC  // 4540 ����һ���ļ���Ĵ���
#define CMD_COM_FILE_RESP 					0x11C6  // 4550 ���շ��յ�4510,4520,4530,4540�ĸ������Ļظ���


#define CMD_COM_UPDATE_FRDLIST				0x2000	// 8192	�����ĸ���
#define CMD_COM_SENDINVITE					0x2010	// 8208	����
#define CMD_COM_RESPINVITE					0x2020	// 8224	�ظ�

#define CMD_EVENT_ANNOUNCEMENT				0x7fff  // 32767 �¼�ͨ�������� djz 2010-3-24

#define CMD_COM_AP_LIST						0x8100	// 33024 ��ȡ�Լ�����AP�������б�
#define CMD_COM_MANY_USER_INFO				0x8110	// 33040 ������ȡ�û���Ϣ

#endif