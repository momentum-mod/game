#include <include/cef_app.h>
#include <include/wrapper/cef_helpers.h>
#include "nui/NuiBrowserListener.h"
#include "filesystem.h"
#include "nui_app.h"
#include "nui_client.h"
#include "nui_interface.h"

#include "tier0/memdbgon.h"

CNuiInterface::CNuiInterface() : m_bInitialized(false)
{
    SetDefLessFunc(m_mapListeners);
    SetDefLessFunc(m_mapBrowsers);
    m_pNuiClient = new CMomNUIClient(this);
}

CNuiInterface::~CNuiInterface()
{
    Shutdown();

    if (m_pNuiClient)
        delete m_pNuiClient;
}

bool CNuiInterface::Init()
{
    if (m_bInitialized)
        return true;

    if (m_pApp.get())
        delete m_pApp.get();

    m_pApp = CefRefPtr<CMomNUIApp>(new CMomNUIApp);

    CefMainArgs args(GetModuleHandle(NULL));

    int error = CefExecuteProcess(args, m_pApp, nullptr);

    if (error >= 0)
        TerminateProcess(GetCurrentProcess(), 0);

    char hostPath[MAX_PATH];

    FileFindHandle_t handle;
    const char *pFileName = g_pFullFileSystem->FindFirstEx("nui_host*", "gamebin", &handle);
    g_pFullFileSystem->FindClose(handle);
    g_pFullFileSystem->RelativePathToFullPath(pFileName, "gamebin", hostPath, MAX_PATH);

    CefSettings settings;
    m_pApp->GetCEFProcessSettings(settings, hostPath);
    
    if (!CefInitialize(args, settings, m_pApp.get(), nullptr))
        return false;

    /*CefRegisterSchemeHandlerFactory("mom", "", m_SchemeHandlerFactory);
    CefAddCrossOriginWhitelistEntry("mom://game", "http", "", true);*/

    m_bInitialized = true;

    return true;
}

void CNuiInterface::Shutdown()
{
    if (!m_bInitialized)
        return;

    // MOM_TODO: Shut down all browsers?

    m_bInitialized = false;

    // Shut down CEF
    CefShutdown();
}

void CNuiInterface::CreateBrowser(NuiBrowserListener *pListener, const char *pURL)
{
    if (!m_bInitialized)
        return;

    CefWindowInfo info;
    info.SetAsWindowless(nullptr);
    info.windowless_rendering_enabled = true;

    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60;
    browserSettings.webgl = STATE_DISABLED;
    browserSettings.local_storage = STATE_DISABLED;
    browserSettings.databases = STATE_ENABLED;
    browserSettings.application_cache = STATE_DISABLED;
    browserSettings.file_access_from_file_urls = STATE_ENABLED;
    browserSettings.javascript_close_windows = STATE_DISABLED;
    browserSettings.javascript_access_clipboard = STATE_DISABLED;
    browserSettings.universal_access_from_file_urls = STATE_ENABLED;
    browserSettings.background_color = 0x00;

    if (CefBrowserHost::CreateBrowser(info, m_pNuiClient, pURL ? pURL : "about:blank", browserSettings, nullptr))
    {
        m_queueWaitingListeners.Insert(pListener);
    }
    else
    {
        pListener->OnBrowserFailedToCreate();
    }
}

void CNuiInterface::ShutdownBrowser(HNUIBrowser &handle)
{
    if (!m_bInitialized)
        return;

    unsigned short browserIndex = m_mapBrowsers.Find(handle);
    if (browserIndex != m_mapBrowsers.InvalidIndex())
    {
        m_mapBrowsers.Element(browserIndex)->GetHost()->CloseBrowser(true);
        m_mapBrowsers.RemoveAt(browserIndex);

        unsigned short listenerIndex = m_mapListeners.Find(handle);
        if (listenerIndex != m_mapListeners.InvalidIndex())
        {
            m_mapListeners.Element(listenerIndex)->OnBrowserClosed();
            m_mapListeners.RemoveAt(listenerIndex);
        }
    }
}

void CNuiInterface::ExecuteJavascript(HNUIBrowser unBrowserHandle, const char *pchScript)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->GetMainFrame()->ExecuteJavaScript(pchScript, "internal", 0);
    }
}

void CNuiInterface::WasResized(HNUIBrowser unBrowserHandle)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->GetHost()->WasResized();
    }
}

inline CefBrowserHost::MouseButtonType DetermineInputFlags(INuiInterface::EHTMLMouseButton b, CefMouseEvent &e)
{
    uint32 eventFlags = 0;
    switch (b)
    {
    case INuiInterface::eHTMLMouseButton_Left:
        eventFlags |= EVENTFLAG_LEFT_MOUSE_BUTTON;
        break;
    case INuiInterface::eHTMLMouseButton_Middle:
        eventFlags |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
        break;
    case INuiInterface::eHTMLMouseButton_Right:
        eventFlags |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
        break;
    default:
        break;
    }

    e.modifiers = eventFlags;
    return static_cast<CefBrowserHost::MouseButtonType>(b);
}

void CNuiInterface::MouseUp(HNUIBrowser &unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        CefBrowserHost::MouseButtonType type = DetermineInputFlags(eMouseButton, mouse_event);
        browser->GetHost()->SendMouseClickEvent(mouse_event, type, true, 1);
    }
}

void CNuiInterface::MouseDown(HNUIBrowser &unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        CefBrowserHost::MouseButtonType type = DetermineInputFlags(eMouseButton, mouse_event);
        browser->GetHost()->SendMouseClickEvent(mouse_event, type, false, 1);
    }
}

void CNuiInterface::MouseDoubleClick(HNUIBrowser &unBrowserHandle, int x, int y, EHTMLMouseButton eMouseButton)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        CefBrowserHost::MouseButtonType type = DetermineInputFlags(eMouseButton, mouse_event);
        browser->GetHost()->SendMouseClickEvent(mouse_event, type, false, 2);
    }
}

void CNuiInterface::MouseMove(HNUIBrowser &unBrowserHandle, int x, int y, bool bMouseLeft)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefMouseEvent event;
        event.x = x;
        event.y = y;
        browser->GetHost()->SendMouseMoveEvent(event, bMouseLeft);
    }
}

void CNuiInterface::MouseWheel(HNUIBrowser &unBrowserHandle, int32 nDelta)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefMouseEvent e;
        // MOM_TODO: Does this really need location?
        browser->GetHost()->SendMouseWheelEvent(e, 0, nDelta);
    }
}

void CNuiInterface::KeyChar(HNUIBrowser unBrowserHandle, wchar_t cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefKeyEvent ke;
        ke.character = cUnicodeChar;
        ke.modifiers = eHTMLKeyModifiers;

        browser->GetHost()->SendKeyEvent(ke);
    }
}

void CNuiInterface::KeyDown(HNUIBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefKeyEvent ke;
        ke.type = KEYEVENT_KEYDOWN;
        ke.native_key_code = nNativeKeyCode;
        ke.modifiers = eHTMLKeyModifiers;

        browser->GetHost()->SendKeyEvent(ke);
    }
}

void CNuiInterface::KeyUp(HNUIBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefKeyEvent ke;
        ke.type = KEYEVENT_KEYUP;
        ke.native_key_code = nNativeKeyCode;
        ke.modifiers = eHTMLKeyModifiers;

        browser->GetHost()->SendKeyEvent(ke);
    }
}

void CNuiInterface::SetKeyFocus(HNUIBrowser unBrowserHandle, bool bHasKeyFocus)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->GetHost()->SetFocus(bHasKeyFocus);
        browser->GetHost()->SendFocusEvent(bHasKeyFocus);
    }
}

void CNuiInterface::LoadURL(HNUIBrowser unBrowserHandle, const char *pchURL, const char *pchPostData)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        CefRefPtr<CefRequest> request = CefRequest::Create();
        request->SetURL(pchURL);
        if (pchPostData)
        {
            CefRefPtr<CefPostData> pPostData = CefPostData::Create();
            CefRefPtr<CefPostDataElement> pPostDataElem = CefPostDataElement::Create();
            pPostDataElem->SetToBytes(sizeof(pchPostData), pchPostData);
            pPostData->AddElement(pPostDataElem);
            request->SetPostData(pPostData);
        }

        browser->GetMainFrame()->LoadRequest(request);
    }
}

void CNuiInterface::StopLoad(HNUIBrowser unBrowserHandle)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->StopLoad();
    }
}

void CNuiInterface::Reload(HNUIBrowser unBrowserHandle)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->Reload();
    }
}

void CNuiInterface::GoBack(HNUIBrowser unBrowserHandle)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->GoBack();
    }
}

void CNuiInterface::GoForward(HNUIBrowser unBrowserHandle)
{
    if (!m_bInitialized)
        return;

    CefRefPtr<CefBrowser> browser = GetBrowser(unBrowserHandle);
    if (browser.get())
    {
        browser->GoForward();
    }
}

NuiBrowserListener *CNuiInterface::GetBrowserListener(HNUIBrowser handle)
{
    unsigned short listenerIndex = m_mapListeners.Find(handle);
    if (listenerIndex != m_mapListeners.InvalidIndex())
    {
        return m_mapListeners.Element(listenerIndex);
    }

    return nullptr;
}

void CNuiInterface::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    AssertMsg(!m_queueWaitingListeners.IsEmpty(), "nui_interface:: The waiting queue is empty!");

    HNUIBrowser handle = static_cast<HNUIBrowser>(browser->GetIdentifier());

    // Pair the queue's head listener with this browser
    NuiBrowserListener *pListener = m_queueWaitingListeners.RemoveAtHead();
    m_mapListeners.Insert(handle, pListener);

    // Add this browser to our global map
    m_mapBrowsers.Insert(handle, browser);

    // Fire our event
    pListener->OnBrowserCreated(handle);
}

CefRefPtr<CefBrowser> CNuiInterface::GetBrowser(HNUIBrowser handle)
{
    unsigned short bIndex = m_mapBrowsers.Find(handle);
    if (bIndex != m_mapBrowsers.InvalidIndex())
    {
        return m_mapBrowsers.Element(bIndex);
    }
    return nullptr;
}

static CNuiInterface s_CNuiInterface;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CNuiInterface, INuiInterface, NUI_INTERFACE_VERSION, s_CNuiInterface);
INuiInterface *nui = &s_CNuiInterface;