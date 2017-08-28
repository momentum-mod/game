#pragma once

#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/wrapper/cef_message_router.h>

#include <queue>

class CMomNUIFrame;

class CMomNUIClient :
    public CefClient,
    public CefLifeSpanHandler,
    public CefDisplayHandler,
    public CefContextMenuHandler,
    public CefLoadHandler,
    public CefRenderHandler,
    public CefRequestHandler
{
public:
    CMomNUIClient(CMomNUIFrame* frame);
    virtual ~CMomNUIClient();

protected:
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE { return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE { return this; }

protected:
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) OVERRIDE;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) OVERRIDE;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int statusCode) OVERRIDE;
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) OVERRIDE;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) OVERRIDE;
    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) OVERRIDE;
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE;
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) OVERRIDE;
    virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& targetURL, CefRequestHandler::WindowOpenDisposition targetDisposition, bool userGesture);
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool redirect) OVERRIDE;

public:
    inline CefRefPtr<CefBrowser> Browser() const { return m_pBrowser; }
    void ExecuteJavascript(const std::string& code);

protected:
    std::queue<std::string> m_QueuedJavascript;

    CefRefPtr<CefBrowser> m_pBrowser;
    CefRefPtr<CefMessageRouterBrowserSide> m_pBrowserSideRouter;

    CMomNUIFrame* m_pFrame;

    bool m_bLoaded;

protected:
    IMPLEMENT_REFCOUNTING(CMomNUIClient);
};
