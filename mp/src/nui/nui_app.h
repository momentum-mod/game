#pragma once

#include <include/cef_app.h>
#include <include/wrapper/cef_message_router.h>

class CMomNUIApp :
    public CefApp,
    public CefRenderProcessHandler,
    public CefV8Handler
{
public:
    CMomNUIApp();
    virtual ~CMomNUIApp();

public:
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

public:
    virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
    virtual void OnWebKitInitialized() OVERRIDE;
    virtual bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, NavigationType navigationType, bool redirect) OVERRIDE;

protected:
    virtual void OnBeforeCommandLineProcessing(const CefString& processType, CefRefPtr<CefCommandLine> commandLine) OVERRIDE;
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) OVERRIDE;
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retVal, CefString& exception) OVERRIDE;

protected:
    CefRefPtr<CefMessageRouterRendererSide> m_pRendererSideRouter;

protected:
    IMPLEMENT_REFCOUNTING(CMomNUIApp);
};