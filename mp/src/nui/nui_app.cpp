#include "nui_app.h"

#include <include/cef_process_message.h>

CMomNUIApp::CMomNUIApp()
{
}

CMomNUIApp::~CMomNUIApp()
{
}

void CMomNUIApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
{
    registrar->AddCustomScheme("mom", true, false, false);
}

void CMomNUIApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    m_pRendererSideRouter->OnContextCreated(browser, frame, context);
}

void CMomNUIApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    m_pRendererSideRouter->OnContextReleased(browser, frame, context);
}

void CMomNUIApp::OnWebKitInitialized()
{
    CefMessageRouterConfig config;
    config.js_query_function = "momQuery";
    config.js_cancel_function = "momQueryCancel";
    m_pRendererSideRouter = CefMessageRouterRendererSide::Create(config);
}

bool CMomNUIApp::OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, NavigationType navigationType, bool redirect)
{
    if (navigationType == NavigationType::NAVIGATION_BACK_FORWARD)
        return true;

    return false;
}

void CMomNUIApp::OnBeforeCommandLineProcessing(const CefString& processType, CefRefPtr<CefCommandLine> commandLine)
{
    commandLine->AppendSwitch("enable-experimental-web-platform-features");
    commandLine->AppendSwitch("disable-gpu");
    commandLine->AppendSwitch("disable-gpu-compositing");
    commandLine->AppendSwitch("enable-begin-frame-scheduling");
}

bool CMomNUIApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message)
{
    return m_pRendererSideRouter->OnProcessMessageReceived(browser, sourceProcess, message);
}

bool CMomNUIApp::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retVal, CefString& exception)
{
    return false;
}