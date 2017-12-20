#pragma once

#include <include/cef_app.h>
#include <include/wrapper/cef_message_router.h>

class CMomNUIApp : public CefApp, public CefRenderProcessHandler, public CefV8Handler
{
  public:
    CMomNUIApp();
    virtual ~CMomNUIApp();

    void GetCEFProcessSettings(CefSettings &settings, const char *pHostPath);

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

  public:
    void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) OVERRIDE;
    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) OVERRIDE;
    void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) OVERRIDE;
    void OnWebKitInitialized() OVERRIDE;
    bool OnBeforeNavigation(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
                            NavigationType navigationType, bool redirect) OVERRIDE;

  protected:
    void OnBeforeCommandLineProcessing(const CefString &processType, CefRefPtr<CefCommandLine> commandLine) OVERRIDE;
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess,
                                  CefRefPtr<CefProcessMessage> message) OVERRIDE;
    bool Execute(const CefString &name, CefRefPtr<CefV8Value> object, const CefV8ValueList &arguments,
                 CefRefPtr<CefV8Value> &retVal, CefString &exception) OVERRIDE;

  protected:
    CefRefPtr<CefMessageRouterRendererSide> m_pRendererSideRouter;

  protected:
    IMPLEMENT_REFCOUNTING(CMomNUIApp);
};