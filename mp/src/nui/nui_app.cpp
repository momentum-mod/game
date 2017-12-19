#include "nui_app.h"

#include <include/cef_process_message.h>

CMomNUIApp::CMomNUIApp()
{
}

CMomNUIApp::~CMomNUIApp()
{
}

void CMomNUIApp::GetCEFProcessSettings(CefSettings& settings, const char* pHostPath)
{
    CefString(&settings.product_version) = "Momentum";
    CefString(&settings.browser_subprocess_path).FromASCII(pHostPath);
    settings.multi_threaded_message_loop = true;
    settings.no_sandbox = true;
    settings.pack_loading_disabled = false;
    settings.windowless_rendering_enabled = true;
    settings.ignore_certificate_errors = true;
    settings.log_severity = LOGSEVERITY_DISABLE;
    settings.single_process = false;
    settings.background_color = 0x00;

    // if (debug)
    //    settings.remote_debugging_port = 8884;
}

void CMomNUIApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
    registrar->AddCustomScheme("mom", true, false, false, false, true, false);
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
    commandLine->AppendSwitch("disable-gpu-compositing");
    commandLine->AppendSwitch("enable-begin-frame-scheduling");
    commandLine->AppendSwitch("disable-extensions");
}

bool CMomNUIApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message)
{
    return m_pRendererSideRouter->OnProcessMessageReceived(browser, sourceProcess, message);
}

bool CMomNUIApp::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retVal, CefString& exception)
{
    return false;
}