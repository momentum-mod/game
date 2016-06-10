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
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

public:
    virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) override;
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual void OnWebKitInitialized() override;
    virtual bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, NavigationType navigationType, bool redirect) override;

protected:
    virtual void OnBeforeCommandLineProcessing(const CefString& processType, CefRefPtr<CefCommandLine> commandLine) override;
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) override;
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retVal, CefString& exception) override;

protected:
    CefRefPtr<CefMessageRouterRendererSide> m_pRendererSideRouter;

protected:
    IMPLEMENT_REFCOUNTING(CMomNUIApp);
};