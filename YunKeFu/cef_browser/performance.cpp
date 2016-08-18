// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#include "cef_browser/performance.h"
#include <sstream>
#include <algorithm>
#include <string>

#include <include/wrapper/cef_stream_resource_handler.h>
#include "cef_browser/performance_setup.h"

using namespace std;


namespace performance {

// Use more interations for a Release build.
#ifdef NDEBUG
const int kDefaultIterations = 1;
#else
const int kDefaultIterations = 1;
#endif

namespace {

const char kGetPerfTests[] = "GetPerfTests";
const char kRunPerfTest[] = "RunPerfTest";
const char kPerfTestReturnValue[] = "PerfTestReturnValue";

//typedef void(RestartSession(HWND hwnd, const CefV8ValueList& arguments));

void RestartSession(const CefV8ValueList& arguments)
{
	string value1(arguments[1]->GetStringValue());
	string value2(arguments[2]->GetStringValue());
	string value3(arguments[3]->GetStringValue());
	string value4(arguments[4]->GetStringValue());
	string value5(arguments[5]->GetStringValue());
	string msg = "rand=" + value1 + "&clientid=" + value2 + "&clientuin=" + value3 + "&chatfrom=" + value4 + "&wxappid=" + value5;

}

class V8Handler : public CefV8Handler {
 public:
  V8Handler() {
  }

  virtual bool Execute(const CefString& name,
                       CefRefPtr<CefV8Value> object,
                       const CefV8ValueList& arguments,
                       CefRefPtr<CefV8Value>& retval,
                       CefString& exception) 
  {
	//MessageBoxA(NULL, "performance Execute����ִ��", 0, 0);
	  if (name == kRunPerfTest)
	  {
		  if (arguments.size() == 1 && arguments[0]->IsString())
		  {
			  // Run the specified perf test.
			  bool found = false;

			  std::string test = arguments[0]->GetStringValue();
			  for (int i = 0; i < kPerfTestsCount; ++i)
			  {
				  if (test == kPerfTests[i].name)
				  {
					  // Execute the test.
					  int64 delta = kPerfTests[i].test(kPerfTests[i].iterations);

					  retval = CefV8Value::CreateInt(delta);
					  found = true;
					  break;
				  }
			  }

			  if (!found)
			  {
				  std::string msg = "Unknown test: ";
				  msg.append(test);
				  exception = msg;
			  }
		  }
		  else
		  {
			  exception = "Invalid function parameters";
		  }
	  }
	  else if (name == kGetPerfTests)
	  {
		  // Retrieve the list of perf tests.
		  retval = CefV8Value::CreateArray(kPerfTestsCount);
		  for (int i = 0; i < kPerfTestsCount; ++i)
		  {
			  CefRefPtr<CefV8Value> val = CefV8Value::CreateArray(2);
			  val->SetValue(0, CefV8Value::CreateString(kPerfTests[i].name));
			  val->SetValue(1, CefV8Value::CreateUInt(kPerfTests[i].iterations));
			  retval->SetValue(i, val);
		  }
	  }
	  else if (name == kPerfTestReturnValue)
	  {
		  if (arguments.size() == 0)
		  {
			  retval = CefV8Value::CreateInt(1);
		  }
		  else if (arguments.size() == 1 && arguments[0]->IsInt())
		  {
			  int32 type = arguments[0]->GetIntValue();
			  CefTime date;
			  switch (type) {
			  case 0:
				  retval = CefV8Value::CreateUndefined();
				  break;
			  case 1:
				  retval = CefV8Value::CreateNull();
				  break;
			  case 2:
				  retval = CefV8Value::CreateBool(true);
				  break;
			  case 3:
				  retval = CefV8Value::CreateInt(1);
				  break;
			  case 4:
				  retval = CefV8Value::CreateUInt(1);
				  break;
			  case 5:
				  retval = CefV8Value::CreateDouble(1.234);
				  break;
			  case 6:
				  date.Now();
				  retval = CefV8Value::CreateDate(date);
				  break;
			  case 7:
				  retval = CefV8Value::CreateString("Hello, world!");
				  break;
			  case 8:
				  retval = CefV8Value::CreateObject(NULL);
				  break;
			  case 9:
				  retval = CefV8Value::CreateArray(8);
				  break;
			  case 10:
				  // retval = CefV8Value::CreateFunction(...);
				  exception = "Not implemented";
				  break;
			  default:
				  exception = "Not supported";
			  }
		  }
	  }

	  return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(V8Handler);
};

// Handle bindings in the render process.
class RenderDelegate : public ClientApp::RenderDelegate {
 public:
	 RenderDelegate() {}

  virtual void OnContextCreated(CefRefPtr<ClientApp> app,
                                  CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) OVERRIDE 
  {
	  CefRefPtr<CefV8Value> object = context->GetGlobal();

	  CefRefPtr<CefV8Handler> handler = new V8Handler();
	  // Bind test functions.
	  object->SetValue(kGetPerfTests,
		  CefV8Value::CreateFunction(kGetPerfTests, handler),
		  V8_PROPERTY_ATTRIBUTE_READONLY);
	  object->SetValue(kRunPerfTest,
		  CefV8Value::CreateFunction(kRunPerfTest, handler),
		  V8_PROPERTY_ATTRIBUTE_READONLY);
	  object->SetValue(kPerfTestReturnValue,
		  CefV8Value::CreateFunction(kPerfTestReturnValue, handler),
		  V8_PROPERTY_ATTRIBUTE_READONLY);
  }

 private:
  IMPLEMENT_REFCOUNTING(RenderDelegate);
};

}  // namespace

void CreateRenderDelegates(ClientApp::RenderDelegateSet& delegates) {
	//delegates.insert(new RenderDelegate());
}

}  // namespace performance
