#pragma once

#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/wrapper/cef_message_router.h>

#include <queue>

class CNuiInterface;

class CMomNUIClient :
    public CefClient,
    public CefLifeSpanHandler,
    public CefDisplayHandler,
    public CefContextMenuHandler,
    public CefLoadHandler,
    public CefRenderHandler,
    public CefRequestHandler,
    public CefJSDialogHandler
{
public:
    CMomNUIClient(CNuiInterface* pInterface);
    virtual ~CMomNUIClient();

protected:
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
    CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
    CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }
    CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() OVERRIDE { return this; }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) OVERRIDE;
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) OVERRIDE;
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int statusCode) OVERRIDE;
    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE;
    void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) OVERRIDE;
    bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) OVERRIDE;
    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) OVERRIDE;
    void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
    void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) OVERRIDE;
    bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& targetURL, CefRequestHandler::WindowOpenDisposition targetDisposition, bool userGesture) OVERRIDE;
    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool redirect) OVERRIDE;
    bool OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, JSDialogType dialog_type, const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message) OVERRIDE;
    void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y) OVERRIDE;

    IMPLEMENT_REFCOUNTING(CMomNUIClient);

private:
    CefRefPtr<CefMessageRouterBrowserSide> m_pBrowserSideRouter;
    CNuiInterface* m_pNuiInterface;
};
