function gww() {//��ȡ���ڿ��
	var w = 0;
	if (window.innerWidth) {
		w = window.innerWidth;
	}else{
		w = document.documentElement.offsetWidth || document.body.clientWidth || 0;
	};
	return parseInt(w);
};
function gwh() {//��ȡ���ڸ߶�
	var h = 0;
	if (window.innerHeight) {
		h = window.innerHeight;
	}else{
		h = document.documentElement.offsetHeight || document.body.clientHeight || 0;
	};
	return parseInt(h);
}


// ��Ϣ��Դ����
var MSG_FROM_CLIENT = '1'; // ��ϯ����
var MSG_FROM_WEBUSER = '2'; // �ÿͷ���
var MSG_FROM_ASSIST = '3'; // Э��������
var MSG_FROM_SYS = '4'; // ϵͳ��Ϣ
var MSG_FROM_SELF = '5'; // �Լ����͵�
var curscrolltop = 0;
var msgindex = 0;
var notifyindex = 0;
var timer_prompt = 0;
var timer_vscroll = 0;
var timeforrecord=0;	// ʱ���ʱ
var bar=0;	// ʱ��������
var scrollToBottomFlag = true;
var g_userId = -1;	// ������Ϣ�Ķ����id
var g_userType = -1; // ��������

var $preview = document.getElementById("previewmsg");
var $listbox = document.getElementById("listbox");
var $prompt = document.getElementById("promptmsg");

window.onresize = function(){
	layOutRepos();
	$listbox.scrollTop = curscrolltop;
};

document.onkeydown = function(e){
	var ev = e||window.event;
	if(ev.keyCode==116){
		ev.keyCode=0;
		ev.cancelBubble=true;
		return false;
	}
};

function listBoxReposWnd(){
	var oft = 0;
	if ( $prompt.style.display != "none" )
		oft += $prompt.clientHeight;
	if ( $preview.style.display != "none" )
		oft += $preview.clientHeight;

	$listbox.style.height = (gwh() - oft > 0 ? gwh() - oft : 1 ) + "px";
	$listbox.style.width = gww() - 12>0?gww() - 12 + "px":"95%";
}

function layOutRepos() {
	listBoxReposWnd();
	if ( scrollToBottomFlag ){
		$listbox.scrollTop = $listbox.scrollHeight - $listbox.clientHeight;
		curscrolltop = $listbox.scrollTop;
	}
}

function getMsgIndex(){
	return msgindex++;
}

function getNotifyIndex(){
	return notifyindex++;
}

function scrollViewToBottom(){
	clearTimeout(timer_vscroll);
	timer_vscroll = setTimeout("layOutRepos()",30);
}

function HideElement(eid){
	var obj = eval(eid);
	if(typeof(obj) == "object"){
		obj.style.display = "none";
	}
	listBoxReposWnd();
}

$listbox.onscroll = function(e){
	var scrollTop = 0, clientHeight = 0, scrollHeight = 0;
	scrollTop = $listbox.scrollTop;
	clientHeight = $listbox.offsetHeight;
	scrollHeight = $listbox.scrollHeight;
	scrollToBottomFlag = ((scrollTop+clientHeight) >= scrollHeight);
	if ( scrollToBottomFlag ){
		$prompt.onclick = null;
		HideElement($prompt.id);
	}
};

function OnClickUnreadMessage(e){
	$prompt.onclick = null;
	HideElement($prompt.id);
	scrollToBottomFlag = true;
	scrollViewToBottom();
}

function AppendPromptUnreadMessage(content){
	var strprompt = TQGF.GetReplaceText(content,TQRES_PROMPT_MSG);
	$prompt.style.display = "block";
	$prompt.innerHTML = strprompt;
	$prompt.onclick = OnClickUnreadMessage;
}

// ���һ����Ϣ����Ϣ��¼
function AppendMsgToHistory(msgFrom, msgType, sname, time, content, userId, head, msgid, userType)
{
	var lstmsg = document.createElement("div");
	lstmsg.id = msgid;

	var msgcontent = TQGF.GetReplaceText(content,TQRES_REPLACE_MSG);
	msgcontent = TQGF.SaveHtmlElement(msgcontent);
	if ( TQHtmlCheck.replace(msgcontent) )
	{
		msgcontent = TQGF.EncodeHtmlTags(msgcontent);
		msgcontent = TQHtmlCheck.get();
	} 
	else 
	{
		msgcontent = TQGF.EncodeHtmlTags(msgcontent);
	}
	msgcontent = TQGF.ReplaceStringToFace(msgcontent);
	msgcontent = TQGF.FormatMsg(msgcontent);
	
	switch (msgFrom)
	{
	    case MSG_FROM_SELF:
		{
			// ������Ϣ
			head = head.replace(/\\/g, "\\\\");
			if(msgType == 1)
			{
				lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>"+sname+"&nbsp;<img class = 'head_image' src='"+ head + "'></div><div class='msg_send_text'>"+msgcontent+"</div></div>";
			}
			else if(msgType == 2)
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;<img class = 'head_image' src='" + head + "'></div><div class='msg_send_image'><img style='border:1px #cdcdcb solid;' src='" + msgcontent + "' width=30%%></div></div>";
			}
			else if( msgType == 3)
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;<img class = 'head_image' src='" + head + "'></div><div class='msg_send_image'><audio controls = 'controls' src = '" + msgcontent + "' type = 'audio/mpeg' /></div></div>";
			}
			else if (msgType == 4)
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;<img class = 'head_image' src='" + head + "'></div><div class='msg_send_image'><video controls = 'controls' src = '" + msgcontent + " type = 'video/mp4'></video></div></div>";
			}
			else if ( msgType == 5)
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;<img class = 'head_image' src='" + head + "'></div><div class='msg_send_image'>" + msgcontent + "</div></div>";
			}
			else if (msgType == 6)
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;<img class = 'head_image' src='" + head + "'></div><div class='msg_send_image'>" + msgcontent + "</div></div>";
			}
			break;
		}
	    case MSG_FROM_WEBUSER:
	    case MSG_FROM_ASSIST:
	    case MSG_FROM_CLIENT:
		{
			// ������Ϣ
			if(msgType == 1)
			{
				lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'><img class='head_image' src='"+ head +"' ondblclick=window.RunMsgList('ChangeChatObject','" + userId + "','" + userType + "')>&nbsp;"+sname+"<font class='time'>"+time+"</font></div><div class='msg_recv_text'>"+msgcontent+"</div></div>";
			}
			else if(msgType == 2)
			{
			    lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'><img class='head_image' src='" + head + "' ondblclick=window.RunMsgList('ChangeChatObject','" + userId + "','" + userType + "')>&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_image'><img style='border:1px #cdcdcb solid;' src='" + msgcontent + "' width=30%%></div></div>";
			}
			else if (msgType == 3)
			{
			    lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'><img class='head_image' src='" + head + "' ondblclick=window.RunMsgList('ChangeChatObject','" + userId + "','" + userType + "')>&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_image'><audio controls = 'controls' src = '" + msgcontent + "' type = 'audio/mpeg' /></div></div>";
			}
			else if (msgType == 4)
			{
			    lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'><img class='head_image' src='" + head + "' ondblclick=window.RunMsgList('ChangeChatObject','" + userId + "','" + userType + "')>&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_image'><video controls = 'controls' src = '" + msgcontent + " type = 'video/mp4'></video></div></div>";
			}
			else if (msgType == 5)
			{
			    lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'><img class='head_image' src='" + head + "' ondblclick=window.RunMsgList('ChangeChatObject','" + userId + "','" + userType + "')>&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_image'>" + msgcontent + "</div></div>";
			}
			else if (msgType == 6)
			{
			    lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'><img class='head_image' src='" + head + "' ondblclick=window.RunMsgList('ChangeChatObject','" + userId + "','" + userType + "')>&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_image'>" + msgcontent + "</div></div>";
			}
		}
			break;
	    case MSG_FROM_SYS:
		{
			// ��ʾ��Ϣ
			lstmsg.innerHTML = "<div class='msg_sys'><span class = 'msg_sys_background'>"+msgcontent+"</span><br></div>";
		}
			break;
		default:
			break;
	}

	$listbox.appendChild(lstmsg);
	if (scrollToBottomFlag || msgFrom == MSG_FROM_SELF || msgFrom == MSG_FROM_SYS)
  {
      scrollToBottomFlag = true;
      scrollViewToBottom();
  } 
  else 
  {
      AppendPromptUnreadMessage(msgcontent);
  }
}

// �����Ϣ��¼
function ClearHistory()
{
	document.getElementById("listbox").innerHTML = "";
}

// ��ʼ¼��
function StartRecordAudio(userId,userType)
{
	g_userId = userId;
	g_userType = userType;
	var lstmsg = document.createElement("div");
	lstmsg.innerHTML = "<div style='position:fixed; bottom:0; width: 100%;height: 26px;background-color: #e2f2f2;'>" + 
								"<div style='float: left;font-size: 14px;line-height: 26px'>����¼��...</div>" + 
								"<div style='float: left;width: 120px;height: 12px;margin-top: 5px;border: 1px solid #d3d3d3;border-radius: 5px'>" + 
									"<div class='linear' id='timebar' style='width: 120px;margin-top: -1px;margin-left: -1px;height: 12px;border: 1px solid #83949d;border-radius: 5px'></div>" + 
								"</div>" + 
								"<div style='float: left;font-size: 14px; line-height: 26px';><span id='demo'></span>/60��</div>" + 
								"<div style='float: right;font-size: 14px; margin-right:10px; line-height: 26px; cursor:pointer' onclick='CancelRecord()'>ȡ��</div>" + 
								"<div style='float: right;font-size: 14px; margin-right:10px; line-height: 26px; cursor:pointer' onclick='SendAudio()'>����</div>" + 
							"</div>";
	
	$preview.appendChild(lstmsg);
	window.RunMsgList('StartRecord');
	StartTimer();
}

// ¼����ʼ��ʱ
function StartTimer()
{
            /*setInterval() ���ָ���ĺ�������ͣ��ִ��ָ���Ĵ���*/
            timeforrecord=0;
            bar=0;
            myVar=setInterval(
                    function()
                    {
                        if(timeforrecord<=60)
                        {
                            IncreaseTimer()
                        }
                        else
                        {
                           StopTimer()
                        }
                    }
                    ,1000)
}

// ��ʱÿ���1
function IncreaseTimer()
{
    document.getElementById("demo").innerHTML=timeforrecord;
    document.getElementById("timebar").style.width=bar+"px";
    timeforrecord++;
    bar=bar+2;
}

// ֹͣ��ʱ
function StopTimer()
{
		clearInterval(myVar);
    $preview.innerHTML = "";
    window.RunMsgList('SendAudio',g_userId,g_userType);
}

// ������Ͱ�ť����Ӧ�¼�
function SendAudio()
{
	/* clearInterval() ��������ֹͣ setInterval() ����ִ�еĺ�������*/
    clearInterval(myVar);
    $preview.innerHTML = "";
    window.RunMsgList('SendAudio',g_userId,g_userType);
}

// ���ȡ����ť����Ӧ�¼�
function CancelRecord()
{
    clearInterval(myVar);
    $preview.innerHTML = "";
    window.RunMsgList('CancelRecord');
}

// ���·�������
function ReSendFile(filePath,userType,msgId,msgDataType,userId) 
{
		document.getElementById(msgId).innerHTML = "";
		window.RunMsgList('ReSendFile',filePath,userType,msgId,msgDataType,userId);
}

// ���½������� 
// filePath �����ļ��Ĵ洢·��
// url ���ص�url
// userType ����Ϣ���û�����
// msgId ��Ϣ��id��
// msgDataType ��Ϣ��������
// userId ����Ϣ��Э�������id
// webUserId ����Ϣ�ķÿ͵�id
// groupUserId ��Ϣ������Ⱥ,Ⱥ����id
function ReRecvFile(filePath,url,userType,msgId,msgDataType,userId,webUserId,groupUserId)
{
		document.getElementById(msgId).innerHTML = "";
		window.RunMsgList('ReRecvFile',filePath,url,userType,msgId,msgDataType,userId,webUserId,groupUserId);
}

function ReSendMsg(msgId,userId,userType,mediaID,msgDataType,fileId,filePath)
{
		document.getElementById(msgId).remove();
		window.RunMsgList('ReSendMsg',msgId, userId, userType, mediaID, msgDataType, fileId, filePath);
}