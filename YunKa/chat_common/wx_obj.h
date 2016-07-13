#ifndef _WX_OBJ_H_
#define _WX_OBJ_H_

#include "../json/jsonext.h"
#include "comdef.h"


struct WxLocation
{
	string Latitude;	//  ����λ��γ��  
	string Longitude;	//  ����λ�þ���  
	string Precision;	//  ����λ�þ���  
	string label;		//  ����λ�ñ�ע  

	bool empty()
	{
		return Latitude.empty()&&Longitude.empty()&&Precision.empty();
	}

	bool operator !=(const WxLocation& r)
	{
		if (Latitude==r.Latitude&&Longitude==r.Longitude&&Precision==r.Precision)
		{
			return true;
		}
		return false;
	}
};

struct WxObj
{
	WxObj(const string& sztype) :MsgType(sztype), CreateTime(0){ LanguageType = LANGUAGE_NORMAL; }
	virtual ~WxObj(){}
	string MsgType;			// text  
	string ToUserName;		// ������΢�ź�  
	string FromUserName;	// ���ͷ��ʺţ�һ��OpenID��  
	int LanguageType;
	string AutoAnswerMsg;	// �������Զ��ظ�����Ϣ����Ϣ��ʽjson�ַ���

	unsigned int CreateTime;							//��Ϣ����ʱ�� �����ͣ�  
	virtual bool ParseFromJson(const Json::Value &os);  //��json��ʽ�ı��н�����Ϣ
	virtual bool ToSendJson(Json::Value &os) const;     //�����ͷ�������Ϣ

};

struct WxUserInfo:public WxObj
{
	WxUserInfo():WxObj("userinfo"),subscribe(0),sex(0){
		FromUserName="webui";
	}
	
	void Clear()
	{
		subscribe=sex=0;
		subscribe_time=0;
		openid=nickname=city=country=province=language=headimgurl=unionid="";
	}

	short subscribe;		//�û��Ƿ��ĸù��ںű�ʶ��ֵΪ0ʱ��������û�û�й�ע�ù��ںţ���ȡ����������Ϣ��
	short sex;				//�û����Ա�ֵΪ1ʱ�����ԣ�ֵΪ2ʱ��Ů�ԣ�ֵΪ0ʱ��δ֪
	string openid;			//�û��ı�ʶ���Ե�ǰ���ں�Ψһ,���ܺ��΢�źţ�fromusername
	string nickname;		//�û����ǳ�
	string remark;			//�û����ǳ�
	string city;			//�û����ڳ���
	string country;			//�û����ڹ���
	string province;		//�û�����ʡ��
	string language;		//�û������ԣ���������Ϊzh_CN
	string headimgurl;		//�û�ͷ�����һ����ֵ����������ͷ���С����0��46��64��96��132��ֵ��ѡ��0����640*640������ͷ�񣩣��û�û��ͷ��ʱ����Ϊ��
	time_t subscribe_time;  //�û���עʱ�䣬Ϊʱ���������û�����ι�ע����ȡ����עʱ��
	string unionid;			//ֻ�����û������ںŰ󶨵�΢�ſ���ƽ̨�ʺź󣬲Ż���ָ��ֶΡ��������ȡ�û�������Ϣ��UnionID���ƣ�
	string fromwxname;      //��Դ���ںź�̨���õ�����
	unsigned long long ullCometimes;  //���ô������ԻỰ������Ϊ׼



	virtual bool ToSendJson(Json::Value &jv) const;     //����json��ʽ������Ϣ
	virtual bool ParseFromJson(const Json::Value &jv);     //��json��ʽ�ı��н�����Ϣ

};

struct WxAccessTokenInfo :public WxObj
{
	WxAccessTokenInfo()
	:WxObj("wxactoken")
	{
		expires_in = 0;
		FromUserName = "webui";
	}


	int expires_in;			//��ʱʱ��
	string szAcToken;		//token
	string szWxBaseUrl;		//΢�Ź��ںŵ�ַ


	virtual bool ToSendJson(Json::Value &jv) const;			//����json��ʽ������Ϣ
	virtual bool ParseFromJson(const Json::Value &jv);		//��json��ʽ�ı��н�����Ϣ
};

struct WxMsgBase:public WxObj
{
	WxMsgBase(const string& sztype):WxObj(sztype){}

	ULONGLONG MsgId;	//��Ϣid��64λ����  
	virtual bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	virtual bool ToSendJson(Json::Value &os) const;     //�����ͷ�������Ϣ

};

struct WxMsgText:public WxMsgBase
{
	WxMsgText(const string& sztype):WxMsgBase(sztype){
		assert("text"==sztype);
	}

	string Content; //��Ϣ����
	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;	    //�����ϵĿͻ��˵��ı���Ϣ  

};

struct WxMsgImage:public WxMsgBase
{
	WxMsgImage(const string& sztype):WxMsgBase(sztype){
		assert("image"==sztype);
	}

	string PicUrl;		//  ͼƬ����  
	string MediaId;		//  ͼƬ��Ϣý��id�����Ե��ö�ý���ļ����ؽӿ���ȡ���ݡ�
	string MediaUrl;	//	MediaIdý���Ӧ��url
	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;
	    //�����ϵĿͻ��˵��ı���Ϣ  

};

struct WxMsgVoice:public WxMsgBase
{
	WxMsgVoice(const string& sztype)
		:WxMsgBase(sztype){
		assert("voice"==sztype);
	}

	string Format;//   ������ʽ����amr��speex��    
	string MediaId;//  ������Ϣý��id�����Ե��ö�ý���ļ����ؽӿ���ȡ���ݡ� 
	string MediaUrl;        //ý���Ӧ��url

	string Recognition;//��ͨ����ʶ���ܣ��û�ÿ�η������������ں�ʱ��΢�Ż������͵�������ϢXML���ݰ��У�����һ��Recongnition�ֶΡ�UTF8����

	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;
	

};

struct WxMsgVideo :public WxMsgBase
{
	WxMsgVideo(const string& sztype)
		:WxMsgBase(sztype){
			assert("video"==sztype||"shortvideo"==sztype);
		}

	string ThumbMediaId;		//   ��Ƶ��Ϣ����ͼ��ý��id�����Ե��ö�ý���ļ����ؽӿ���ȡ���ݡ� 
	string ThumbMediaUrl;       //ý���Ӧ��url
	string MediaId;				// ��Ƶ��Ϣý��id�����Ե��ö�ý���ļ����ؽӿ���ȡ���ݡ� 
	string MediaUrl;			//ý���Ӧ��url

	//ֻ������΢�ŷ�����������Ϣ
	string Title;				// ��Ƶ��Ϣ�ı���  
	string Description;			//��Ƶ��Ϣ������ 

	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;
	

};

struct WxMsgLocation:public WxMsgBase
{
	WxMsgLocation(const string& sztype)
		:WxMsgBase(sztype){
			assert("location"==sztype);
		}


	string Location_X;	// ����λ��ά��  
	string 	Location_Y;	//  ����λ�þ���  
	string 	Scale;		//  ��ͼ���Ŵ�С  
	string 	Label;		//  ����λ����Ϣ  

	bool ToSendJson(Json::Value &os) const;
	
	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ
};


/*
Ŀǰ΢�Ų������͸�����Ϣ����
��ϯ���Է��͸�����Ϣ
*/
struct WxMsgLink:public WxMsgBase
{
	WxMsgLink(const string& sztype)
		:WxMsgBase(sztype){
			assert("link"==sztype);
		}

	string Title;		//  ��Ϣ����  
	string Description;	//  ��Ϣ����  
	string Url;			//  ��Ϣ����  
	bool ToSendJson(Json::Value &os) const;

	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ
};

/*
Ŀǰ΢�Ų������͸�����Ϣ����
��ϯ���Է��͸�����Ϣ
*/
struct WxMsgMusic:public WxObj
{
	WxMsgMusic(const string& sztype)
		:WxObj(sztype){
			assert("music"==sztype);
		}

	string ThumbMediaId;//  ��  ����ͼ��ý��id��ͨ���ϴ���ý���ļ����õ���id  

	string Title;  //��  ���ֱ���  
	string Description; // ��  ��������  
	string MusicURL; // ��  ��������  
	string HQMusicUrl;//  ��  �������������ӣ�WIFI��������ʹ�ø����Ӳ�������  

	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToResponseXml(ostringstream &os) const;
	bool ToSendJson(Json::Value &os) const;
	

};

struct WxNewsItem
{
	string Title;  //  ��  ͼ����Ϣ����  
	string Description;  //  ��  ͼ����Ϣ����  
	string PicUrl;  //  ��  ͼƬ���ӣ�֧��JPG��PNG��ʽ���Ϻõ�Ч��Ϊ��ͼ360*200��Сͼ200*200  
	string Url;  //  ��  ���ͼ����Ϣ��ת����  
	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ
	bool ToSendJson(Json::Value &os) const;
	


};
struct WxMsgNews:public WxMsgBase
{
	WxMsgNews(const string& sztype)
		:WxMsgBase(sztype){
			assert("news"==sztype);
		}

	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;
	

	vector<WxNewsItem> News;
	bool AddArticle(const WxNewsItem &a);


};

////////////////////////////wxevent�ӿ�//////////////////////////////////////////////

/*
��ע/ȡ����ע�¼�
<xml>
<ToUserName><![CDATA[toUser]]></ToUserName>
<FromUserName><![CDATA[FromUser]]></FromUserName>
<CreateTime>123456789</CreateTime>
<MsgType><![CDATA[event]]></MsgType>
<Event><![CDATA[subscribe]]></Event>
</xml>

MsgType:event
*/
struct WxEventBase:public WxObj
{
	WxEventBase(const string& sztype):WxObj(sztype){}

	string Event; 
	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;

};

/*
Event: subscribe(����)��unsubscribe(ȡ������),SCAN(�û��ѹ�עʱ���¼����� )
ֻ�ṩ���ͻ���ʹ�ã�΢�ŷ����������ܸ�����Ϣ
*/
struct WxEventSubscribe:public WxEventBase
{
	WxEventSubscribe(const string& sztype):WxEventBase(sztype){}

	/*
	EventΪsubscribe ʱ��	�¼�KEYֵ��qrscene_Ϊǰ׺������Ϊ��ά��Ĳ���ֵ 
	EventΪSCAN ʱ     ��	�¼�KEYֵ����һ��32λ�޷�����������������ά��ʱ�Ķ�ά��scene_id 
	*/
	string EventKey;

	string Ticket;		//��ά���ticket����������ȡ��ά��ͼƬ 


	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;

	

};

/*
�ϱ�����λ���¼�
ͨ���ϱ�����λ�ýӿڵĹ��ںţ��û��ڹ�ע����빫�ںŻỰʱ���ᵯ�����û�ȷ���Ƿ������ں�ʹ�������λ�á�����ֻ�ڹ�ע�����һ�Σ��û��Ժ�����ڹ��ں�����ҳ����в����� 
�û�ͬ���ϱ�����λ�ú�ÿ�ν��빫�ںŻỰʱ�������ڽ���ʱ�ϱ�����λ�ã��ϱ�����λ��������XML���ݰ�����������д��URL��ʵ�֡� 
Event: LOCATION
*/
struct WxEventLocation:public WxEventBase
{
	WxEventLocation(const string& sztype):WxEventBase(sztype){}


	WxLocation  local;

	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;
	
};

/*
Event: CLICK ����˵���ȡ��Ϣʱ���¼����� 
Event: VIEW ����˵���ת����ʱ���¼����� 
*/
struct WxEventMenu:public WxEventBase
{
	WxEventMenu(const string& sztype):WxEventBase(sztype){}
	/*
	CLICK:�¼�KEYֵ�����Զ���˵��ӿ���KEYֵ��Ӧ 
	VIEW:�¼�KEYֵ�����õ���תURL 
	*/
	string EventKey ;
	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ

	bool ToSendJson(Json::Value &os) const;
};


/*�¼�����Ⱥ�����
<xml>
<ToUserName><![CDATA[gh_3e8adccde292]]>< / ToUserName>
<FromUserName><![CDATA[oR5Gjjl_eiZoUpGozMo7dbBJ362A]]>< / FromUserName>
< CreateTime>1394524295 < / CreateTime >
<MsgType><![CDATA[event]]>< / MsgType>
<Event><![CDATA[MASSSENDJOBFINISH]]>< / Event>
< MsgID>1988 < / MsgID >
<Status><![CDATA[sendsuccess]]>< / Status>
< TotalCount>100 < / TotalCount >
< FilterCount>80 < / FilterCount >
< SentCount>75 < / SentCount >
< ErrorCount>5 < / ErrorCount >
< / xml>

Ⱥ���Ľṹ��Ϊ��send success����send fail����err(num)������send successʱ��Ҳ�п������û����չ��ںŵ���Ϣ��ϵͳ�����ԭ����������û�����ʧ�ܡ�
err(num)�����ʧ�ܵľ���ԭ�򣬿��ܵ�������£�
err(10001),���ӹ��
err(20001), ��������
err(20004), �������
err(20002), ����ɫ��
err(20006), ����Υ������
err(20008),������թ
err(20013),���Ӱ�Ȩ
err(22000), ���ӻ���(��������)
err(21000), //��������
*/
struct WxEventMasssendjobfinish :public WxEventBase
{
	WxEventMasssendjobfinish(const string& sztype) :WxEventBase(sztype) {}


	unsigned long long MsgID;		//Ⱥ������ϢID
	unsigned long long TotalCount;	//group_id�·�˿��������openid_list�еķ�˿��
	unsigned long long FilterCount; // ���ˣ�������ָ����Щ�û���΢�����ò����ոù��ںŵ���Ϣ����׼�����͵ķ�˿����ԭ���ϣ�FilterCount = SentCount + ErrorCount
	unsigned long long SentCount; //���ͳɹ��ķ�˿��
	unsigned long long ErrorCount; //����ʧ�ܵķ�˿��


	bool ParseFromJson(const Json::Value &os);   //��json��ʽ�ı��н�����Ϣ
};
//////////////////////////////////////////////////////////////////////////

struct GroupInfo
{
	int id;			//  ����id����΢�ŷ���  
	string	name;	//  �������֣�UTF8����  
    int count;		//  �������û�����  
};

#endif