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
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

protected:
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message) override;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int statusCode) override;
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) override;
    virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) override;
    virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& targetURL, CefRequestHandler::WindowOpenDisposition targetDisposition, bool userGesture);
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool redirect) override;

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
