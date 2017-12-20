#include "nui_client.h"
#include "nui_interface.h"
#include "nui/NuiBrowserListener.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

CMomNUIClient::CMomNUIClient(CNuiInterface* pInterface)
    : m_pNuiInterface(pInterface)
{
    CefMessageRouterConfig config;
    config.js_query_function = "momQuery";
    config.js_cancel_function = "momQueryCancel";
    m_pBrowserSideRouter = CefMessageRouterBrowserSide::Create(config);
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
    m_pNuiInterface->OnBrowserCreated(browser);
}

void CMomNUIClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
    
    //m_pFrame->ShouldRender(false);
}


void CMomNUIClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int statusCode)
{
    if (frame->IsMain())
    {
        NuiBrowserListener *pListener = m_pNuiInterface->GetBrowserListener((HNUIBrowser) browser->GetIdentifier());
        if (pListener)
        {
            pListener->OnBrowserPageLoaded(frame->GetURL().ToString().c_str());
        }
    }
}

void CMomNUIClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    /*if (!isLoading)
    {
        NuiBrowserListener *pListener = m_pNuiInterface->GetBrowserListener((HNUIBrowser) browser->GetIdentifier());
        if (pListener)
        {
            pListener->OnBrowserPageLoaded(browser->GetMainFrame()->GetURL().ToString().c_str());
        }
    }*/
}

void CMomNUIClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
    model->Clear();
}

bool CMomNUIClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
    DevLog(2, "NUI Console: %s\n", message.ToString().c_str());
    return true;
}

bool CMomNUIClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    int wide, tall;
    NuiBrowserListener *pListener = m_pNuiInterface->GetBrowserListener((HNUIBrowser) browser->GetIdentifier());
    if (pListener)
    {
        pListener->OnBrowserSize(wide, tall);
        rect.Set(0, 0, wide, tall);
        return true;
    }
    
    return false;
}

void CMomNUIClient::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
    if (type == PET_VIEW)
    {
        NuiBrowserListener *pListener = m_pNuiInterface->GetBrowserListener((HNUIBrowser) browser->GetIdentifier());
        if (pListener)
        {
            for (auto r : dirtyRects)
            {
                pListener->OnBrowserPaint(buffer, width, height, r.x, r.y, r.width, r.height,
                    // MOM_TODO: Figure out how to pass these scroll values... maybe store it somewhere? Not sure
                    0, 0);
            }
        }
    }
}

void CMomNUIClient::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{

}

void CMomNUIClient::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{

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

bool CMomNUIClient::OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, JSDialogType dialog_type,
    const CefString& message_text, const CefString& default_prompt_text, CefRefPtr<CefJSDialogCallback> callback,
    bool& suppress_message)
{
    suppress_message = true;

    NuiBrowserListener *pListener = m_pNuiInterface->GetBrowserListener((HNUIBrowser) browser->GetIdentifier());
    if (pListener)
    {
        if (dialog_type == JSDIALOGTYPE_ALERT)
        {
            pListener->OnBrowserJSAlertDialog(message_text.ToString().c_str());
        }
        // MOM_TODO: Add confirm and prompt dialogs?
    }

    
    return false;
}

void CMomNUIClient::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y)
{
    // MOM_TODO: Update the scroll stuff here for the panels that use this?
}
