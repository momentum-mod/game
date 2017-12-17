#include "nui_predef.h"
#include "nui_client.h"
#include "nui_frame.h"

CMomNUIClient::CMomNUIClient(CMomNUIFrame* frame)
    : m_pFrame(frame), m_bLoaded(false)
{

}

CMomNUIClient::~CMomNUIClient()
{

}

bool CMomNUIClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId sourceProcess, CefRefPtr<CefProcessMessage> message)
{
    return m_pBrowserSideRouter->OnProcessMessageReceived(browser, sourceProcess, message);
}

void CMomNUIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    m_pBrowser = browser;

    CefMessageRouterConfig config;
    config.js_query_function = "momQuery";
    config.js_cancel_function = "momQueryCancel";
    m_pBrowserSideRouter = CefMessageRouterBrowserSide::Create(config);

    // TODO (OrfeasZ): Setup our handler.
    //m_pBrowserSideRouter->AddHandler(m_pFrame->Handler(), true);

    m_pBrowser->GetHost()->SetFocus(true);
    m_pBrowser->GetHost()->SendFocusEvent(true);
}

void CMomNUIClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
    m_pFrame->ShouldRender(false);
}


void CMomNUIClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int statusCode)
{
}

void CMomNUIClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    if (!isLoading)
    {
        m_bLoaded = true;

        m_pBrowser->GetHost()->SetFocus(true);
        m_pBrowser->GetHost()->SendFocusEvent(true);


        while (!m_QueuedJavascript.empty())
        {
            m_pBrowser->GetMainFrame()->ExecuteJavaScript(m_QueuedJavascript.front(), "internal", 0);
            m_QueuedJavascript.pop();
        }

        m_pBrowser->GetMainFrame()->ExecuteJavaScript("setLocalization('english')", "internal", 0);
    }
}

void CMomNUIClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{

}

bool CMomNUIClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
    return false;
}

bool CMomNUIClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    rect.Set(0, 0, m_pFrame->FrameWidth(), m_pFrame->FrameHeight());
    return true;
}

void CMomNUIClient::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
    m_pFrame->OnPaint(browser, type, dirtyRects, buffer, width, height);
}

void CMomNUIClient::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{

}

void CMomNUIClient::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{

}

void CMomNUIClient::ExecuteJavascript(const std::string& code)
{
    if (m_bLoaded)
    {
        m_pBrowser->GetMainFrame()->ExecuteJavaScript(code, "internal", 0);
        return;
    }

    m_QueuedJavascript.push(code);
}

void CMomNUIClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    m_pBrowserSideRouter->OnBeforeClose(browser);
}

void CMomNUIClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    m_pBrowserSideRouter->OnRenderProcessTerminated(browser);
}

bool CMomNUIClient::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& targetURL, CefRequestHandler::WindowOpenDisposition targetDisposition, bool userGesture)
{
    return false;
}

bool CMomNUIClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool redirect)
{
    m_pBrowserSideRouter->OnBeforeBrowse(browser, frame);

    CefRequest::TransitionType s_Type = request->GetTransitionType();
    if ((unsigned int) s_Type & TT_FORWARD_BACK_FLAG)
        return true;

    return false;
}