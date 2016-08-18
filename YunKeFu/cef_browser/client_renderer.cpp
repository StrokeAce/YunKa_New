// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#include "cef_browser/client_renderer.h"

#include <sstream>
#include <string>
#include "../chat_common/comdef.h"
#include <include/cef_dom.h>
#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_message_router.h>

using namespace std;

namespace client_renderer {

const char kFocusedNodeChangedMessage[] = "ClientRenderer.FocusedNodeChanged";

namespace {

class V8Handler : public CefV8Handler {
private:
	CefRefPtr<CefBrowser> m_browser;
public:
	V8Handler(CefRefPtr<CefBrowser> browser)
	{
		m_browser = browser;
	}

	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception)
	{
		if (name == "RunWebURL")
		{
			if (arguments.size() == 6 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_RestartSession)
			{
				string value1(arguments[1]->GetStringValue());
				string value2(arguments[2]->GetStringValue());
				string value3(arguments[3]->GetStringValue());
				string value4(arguments[4]->GetStringValue());
				string value5(arguments[5]->GetStringValue());
				string msg = "rand=" + value1 + "&clientid=" + value2 + "&clientuin=" + value3 + "&chatfrom=" + value4 + "&wxappid=" + value5;

				CefRefPtr<CefProcessMessage> message =
					CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_RestartSession);
				message->GetArgumentList()->SetString(1, msg);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
		}
		else if (name == "RunMsgList")
		{
			if (arguments.size() == 1 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_StartRecord)
			{
				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_StartRecord);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 1 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_CancelRecord)
			{
				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_CancelRecord);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 3 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_ViewDetails)
			{
				string url(arguments[1]->GetStringValue());
				string msgDataType(arguments[2]->GetStringValue());
				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_ViewDetails);
				message->GetArgumentList()->SetString(1, url);
				message->GetArgumentList()->SetString(2, msgDataType);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 3 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_ChangeChatObject)
			{
				string userId(arguments[1]->GetStringValue());
				string userType(arguments[2]->GetStringValue());

				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_ChangeChatObject);
				message->GetArgumentList()->SetString(1, userId);
				message->GetArgumentList()->SetString(2, userType);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 3 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_SendAudio)
			{
				string userId(arguments[1]->GetStringValue());
				string userType(arguments[2]->GetStringValue());

				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_SendAudio);
				message->GetArgumentList()->SetString(1, userId);
				message->GetArgumentList()->SetString(2, userType);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 6 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_ReSendFile)
			{
				string filePath(arguments[1]->GetStringValue());
				string userType(arguments[2]->GetStringValue());
				string msgId(arguments[3]->GetStringValue());
				string msgDataType(arguments[4]->GetStringValue());
				string userId(arguments[5]->GetStringValue());

				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_ReSendFile);
				message->GetArgumentList()->SetString(1, filePath);
				message->GetArgumentList()->SetString(2, userType);
				message->GetArgumentList()->SetString(3, msgId);
				message->GetArgumentList()->SetString(4, msgDataType);
				message->GetArgumentList()->SetString(5, userId);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 8 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_ReSendMsg)
			{
				string msgId(arguments[1]->GetStringValue());
				string userId(arguments[2]->GetStringValue());
				string userType(arguments[3]->GetStringValue());
				string mediaId(arguments[4]->GetStringValue());
				string msgDataType(arguments[5]->GetStringValue());
				string fileId(arguments[6]->GetStringValue());
				string filePath(arguments[7]->GetStringValue());

				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_ReSendMsg);
				message->GetArgumentList()->SetString(1, msgId);
				message->GetArgumentList()->SetString(2, userId);
				message->GetArgumentList()->SetString(3, userType);
				message->GetArgumentList()->SetString(4, mediaId);
				message->GetArgumentList()->SetString(5, msgDataType);
				message->GetArgumentList()->SetString(6, fileId);
				message->GetArgumentList()->SetString(7, filePath);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
			else if (arguments.size() == 7 && arguments[0]->GetStringValue() == Js_Call_MFC_Func_ReRecvFile)
			{
				string url(arguments[1]->GetStringValue());
				string msgFromUserType(arguments[2]->GetStringValue());
				string msgId(arguments[3]->GetStringValue());
				string msgDataType(arguments[4]->GetStringValue());
				string msgFromUserId(arguments[5]->GetStringValue());
				string assistUserId(arguments[6]->GetStringValue());

				CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
				message->GetArgumentList()->SetString(0, Js_Call_MFC_Func_ReRecvFile);
				message->GetArgumentList()->SetString(1, url);
				message->GetArgumentList()->SetString(2, msgFromUserType);
				message->GetArgumentList()->SetString(3, msgId);
				message->GetArgumentList()->SetString(4, msgDataType);
				message->GetArgumentList()->SetString(5, msgFromUserId);
				message->GetArgumentList()->SetString(6, assistUserId);
				m_browser->SendProcessMessage(PID_BROWSER, message);
			}
		}
		return true;
	}

private:
	IMPLEMENT_REFCOUNTING(V8Handler);
};

class ClientRenderDelegate : public ClientApp::RenderDelegate 
{
 public:
  ClientRenderDelegate()
    : last_node_is_editable_(false) {
  }

  virtual void OnWebKitInitialized(CefRefPtr<ClientApp> app) OVERRIDE {
    // Create the renderer-side router for query handling.
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
  }

  virtual void OnContextCreated(CefRefPtr<ClientApp> app,
                                CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context) OVERRIDE 
  {
    message_router_->OnContextCreated(browser,  frame, context);
	
	if (browser != NULL)
	{
		CefString url = browser->GetMainFrame()->GetURL();
		string strUrl(url);

		CefRefPtr<CefV8Value> object = context->GetGlobal();
		CefRefPtr<V8Handler> handler = new V8Handler(browser);
		bool is = object->SetValue("RunWebURL", CefV8Value::CreateFunction("RunWebURL", handler), V8_PROPERTY_ATTRIBUTE_READONLY);
		is = object->SetValue("RunMsgList", CefV8Value::CreateFunction("RunMsgList", handler), V8_PROPERTY_ATTRIBUTE_READONLY);
	}
  }

  virtual void OnContextReleased(CefRefPtr<ClientApp> app,
                                 CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context) OVERRIDE {
    message_router_->OnContextReleased(browser,  frame, context);
  }

  virtual void OnFocusedNodeChanged(CefRefPtr<ClientApp> app,
                                    CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefDOMNode> node) OVERRIDE {
    bool is_editable = (node.get() && node->IsEditable());
    if (is_editable != last_node_is_editable_) {
      // Notify the browser of the change in focused element type.
      last_node_is_editable_ = is_editable;
      CefRefPtr<CefProcessMessage> message =
          CefProcessMessage::Create(kFocusedNodeChangedMessage);
      message->GetArgumentList()->SetBool(0, is_editable);
      browser->SendProcessMessage(PID_BROWSER, message);
    }
  }

  virtual bool OnProcessMessageReceived(
      CefRefPtr<ClientApp> app,
      CefRefPtr<CefBrowser> browser,
      CefProcessId source_process,
      CefRefPtr<CefProcessMessage> message) OVERRIDE {
    return message_router_->OnProcessMessageReceived(
        browser, source_process, message);
  }

 private:
  bool last_node_is_editable_;

  // Handles the renderer side of query routing.
  CefRefPtr<CefMessageRouterRendererSide> message_router_;

  IMPLEMENT_REFCOUNTING(ClientRenderDelegate);
};

}  // namespace

void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates) {
  delegates.insert(new ClientRenderDelegate);
}

}  // namespace client_renderer
