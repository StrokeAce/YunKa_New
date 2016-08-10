function gww() {//获取窗口宽度
	var w = 0;
	if (window.innerWidth) {
		w = window.innerWidth;
	}else{
		w = document.documentElement.offsetWidth || document.body.clientWidth || 0;
	};
	return parseInt(w);
};
function gwh() {//获取窗口高度
	var h = 0;
	if (window.innerHeight) {
		h = window.innerHeight;
	}else{
		h = document.documentElement.offsetHeight || document.body.clientHeight || 0;
	};
	return parseInt(h);
}


// 消息来源类型
var MSG_FROM_CLIENT = '1'; // 坐席发来
var MSG_FROM_WEBUSER = '2'; // 访客发来
var MSG_FROM_ASSIST = '3'; // 协助对象发来
var MSG_FROM_SYS = '4'; // 系统消息
var MSG_FROM_SELF = '5'; // 自己发送的
var curscrolltop = 0;
var msgindex = 0;
var notifyindex = 0;
var timer_prompt = 0;
var timer_vscroll = 0;
var timeforrecord=0;	// 时间计时
var bar=0;	// 时间条进度
var scrollToBottomFlag = true;
var g_userId = -1;	// 接收消息的对象的id
var g_userType = -1; // 对象类型

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

function AppendPromptUnreadMessage(content) {
	$prompt.style.display = "block";
	$prompt.innerHTML = content;
	$prompt.onclick = OnClickUnreadMessage;
}

// 添加一条消息到消息记录
function AppendMsgToHistory(msgFrom, msgDataType, sname, time, content, userId, head, msgid, image)
{
	var lstmsg = document.createElement("div");
	lstmsg.id = msgid;
	
	switch (msgFrom)
	{
	    case MSG_FROM_SELF:
		{
			// 发送消息
			head = head.replace(/\\/g, "\\\\");
			if(msgDataType == 1)
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;"+ head +"</div><div class='msg_send_text'><div class='msg_text_background'>" + content + "</div></div></div>";
			}
			else
			{
			    lstmsg.innerHTML = "<div class='msg_send clearfix'><div class='send_name'>" + sname + "&nbsp;"+ head +"</div><div class='msg_send_image'>" + content + "</div></div>";
			}
			break;
		}
	    case MSG_FROM_WEBUSER:
	    case MSG_FROM_ASSIST:
	    case MSG_FROM_CLIENT:
		{
			// 接收消息
		    if (msgDataType == 1)
			{
		        lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'>"+ head +"&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_text'><div class='msg_text_background'>" + content + "</div></div></div>";
		    }
		    else if(msgDataType == 7)
		    {
		        lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'>"+ head +"&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_text'><div class='msg_link_background'>" + content + "</div></div></div>";
		    }
			else
			{
			    lstmsg.innerHTML = "<div class='msg_recv clearfix'><div class='recv_name'>"+ head +"&nbsp;" + sname + "<font class='time'>" + time + "</font></div><div class='msg_recv_image'>" + content + "</div></div>";
			}
		}
			break;
	    case MSG_FROM_SYS:
		{
			// 提示消息
		    lstmsg.innerHTML = "<div class='msg_sys'><span class = 'msg_sys_background'>" + content + "</span><br></div>";
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
	    AppendPromptUnreadMessage(content);
    }
}

// 清空消息记录
function ClearHistory()
{
	document.getElementById("listbox").innerHTML = "";
}

// 开始录音
function StartRecordAudio(userId,userType)
{
	g_userId = userId;
	g_userType = userType;
	var lstmsg = document.createElement("div");
	lstmsg.innerHTML = "<div style='position:fixed; bottom:0; width: 100%;height: 26px;background-color: #e2f2f2;'>" + 
								"<div style='float: left;font-size: 14px;line-height: 26px'>正在录音...</div>" + 
								"<div style='float: left;width: 120px;height: 12px;margin-top: 5px;border: 1px solid #d3d3d3;border-radius: 5px'>" + 
								"<div class='linear' id='timebar' style='width: 120px;margin-top: -1px;margin-left: -1px;height: 12px;border: 1px solid #83949d;border-radius: 5px'></div></div>" +
								"<div style='float: left;font-size: 14px; line-height: 26px';><span id='demo'></span>/60秒</div>" + 
								"<div style='float: right;font-size: 14px; margin-right:10px; line-height: 26px; cursor:pointer' onclick='CancelRecord()'>取消</div>" + 
								"<div style='float: right;font-size: 14px; margin-right:10px; line-height: 26px; cursor:pointer' onclick='SendAudio()'>发送</div>" + 
							    "</div>";
	
	$preview.appendChild(lstmsg);
	window.RunMsgList('StartRecord');
	StartTimer();
}

// 录音开始计时
function StartTimer()
{
    /*setInterval() 间隔指定的毫秒数不停地执行指定的代码*/
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

// 计时每秒加1
function IncreaseTimer()
{
    document.getElementById("demo").innerHTML=timeforrecord;
    document.getElementById("timebar").style.width=bar+"px";
    timeforrecord++;
    bar=bar+2;
}

// 停止计时
function StopTimer()
{
	clearInterval(myVar);
    $preview.innerHTML = "";
    window.RunMsgList('SendAudio',g_userId,g_userType);
}

// 点击发送按钮的响应事件
function SendAudio()
{
	/* clearInterval() 方法用于停止 setInterval() 方法执行的函数代码*/
    clearInterval(myVar);
    $preview.innerHTML = "";
    window.RunMsgList('SendAudio',g_userId,g_userType);
}

// 点击取消按钮的响应事件
function CancelRecord()
{
    clearInterval(myVar);
    $preview.innerHTML = "";
    window.RunMsgList('CancelRecord');
}

// 重新接收语音 
// filePath 下载文件的存储路径
// url 下载的url
// userType 发消息的用户类型
// msgId 消息的id号
// msgDataType 消息数据类型
// userId 发消息的协助对象的id
// webUserId 发消息的访客的id
// groupUserId 消息所属的群,群主的id
function ReRecvFile(url, msgFromType, msgId, msgDataType, msgFromUserId, assistUserId, imagePath)
{
    var oImg = document.getElementById(msgId + "_image");
    oImg.src = imagePath + "msg_wait.gif";
    window.RunMsgList('ReRecvFile', url, msgFromType, msgId, msgDataType, msgFromUserId, assistUserId);
}

function ReSendFile(filePath, recvUserType, msgId, msgDataType, userId,imagePath)
{
    var oImg = document.getElementById(msgId + "_image");
    oImg.src = imagePath + "msg_wait.gif";
    window.RunMsgList('ReSendFile', filePath, recvUserType, msgId, msgDataType, userId);
}

function ResultSendMsg(msgId, bSuccess, imagePath, filePath, recvUserType, msgDataType, userId)
{
    if (bSuccess == '0')
    {
        var imgId = msgId + "_image";
        var oImg = document.getElementById(imgId);
        oImg.src = imagePath + "msg_fail.png";
        oImg.onclick = function ()
        {
            window.RunMsgList('ReSendFile', filePath, recvUserType, msgId, msgDataType, userId);
            oImg.src = imagePath + "msg_wait.gif";
        }
        if (msgDataType == 6)
        {
            var spanId = msgId + "_span";
            var oSpan = document.getElementById(spanId);
            oSpan.innerHTML = "<span class='file_text'>文件发送失败</span>";
        }
    }
    else
    {
        var imgId = msgId + "_image";
        var oImg = document.getElementById(imgId);
        oImg.src = "";
        if (msgDataType == 6)
        {
            var spanId = msgId + "_span";
            var oSpan = document.getElementById(spanId);
            oSpan.innerHTML = "<span class='file_text'>发送文件 </span><a href='" + filePath + "'>" + imagePath + "</a>";
        }        
    }
}

function ResultRecvMsg(msgId, bSuccess, url, imagePath, filePath, msgFromType, msgDataType, msgFromUserId, assistUserId)
{
    if (bSuccess == '0')
    {
        var oImg = document.getElementById(msgId + "_image");
        oImg.src = imagePath + "msg_fail.png";
        oImg.onclick = function ()
        {
            window.RunMsgList('ReRecvFile', url, msgFromType, msgId, msgDataType, msgFromUserId, assistUserId);
            oImg.src = imagePath + "msg_wait.gif";
        }
    }
    else
    {
        var oImgMsg = document.getElementById(msgId + "_msg");
        oImgMsg.src = filePath;
        oImgMsg.onclick = function ()
        {
            window.RunMsgList('ViewDetails', filePath, msgDataType);
        }
        var oImgFail = document.getElementById(msgId + "_image");
        oImgFail.src = "";
    }
}