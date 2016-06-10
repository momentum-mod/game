#include "nui_predef.h"
#include "nui_frame.h"
#include "nui_client.h"

#include "vgui/ISurface.h"
#include "tier3/tier3.h"

CMomNUIFrame::CMomNUIFrame(uint32_t width, uint32_t height) :
    m_iWidth(width),
    m_iHeight(height),
    m_bInitialized(false),
    m_pClient(nullptr),
    m_iTextureID(0),
    m_bNeedsRepaint(false),
    m_bDirty(false)
{
}

CMomNUIFrame::~CMomNUIFrame()
{
    m_bInitialized = false;

    if (m_pClient.get())
    {
        delete m_pClient.get();
        m_pClient = nullptr;
    }
}

bool CMomNUIFrame::Init(const std::string& url)
{
    if (m_bInitialized)
        return true;

    m_pClient = new CMomNUIClient(this);

    CefWindowInfo info;
    info.SetAsWindowless(NULL, true);

    CefBrowserSettings browserSettings;

    browserSettings.windowless_frame_rate = 60;
    browserSettings.webgl = STATE_ENABLED;
    browserSettings.local_storage = STATE_DISABLED;
    browserSettings.databases = STATE_ENABLED;
    browserSettings.application_cache = STATE_DISABLED;
    browserSettings.file_access_from_file_urls = STATE_DISABLED;
    browserSettings.javascript_close_windows = STATE_DISABLED;
    browserSettings.javascript_open_windows = STATE_DISABLED;
    browserSettings.javascript_access_clipboard = STATE_DISABLED;
    browserSettings.universal_access_from_file_urls = STATE_DISABLED;

    CefRefPtr<CefRequestContext> s_RequestContext = CefRequestContext::GetGlobalContext();

    if (!CefBrowserHost::CreateBrowser(info, m_pClient, url.c_str(), browserSettings, s_RequestContext))
        return false;

    m_bInitialized = true;

    return true;
}

void CMomNUIFrame::OnResized(uint32_t width, uint32_t height)
{
    // Set the new dimensions.
    m_iWidth = width;
    m_iHeight = height;

    m_pClient->Browser()->GetHost()->WasResized();
}

void CMomNUIFrame::ExecuteJavascript(const std::string& code)
{
    if (m_pClient.get())
        m_pClient->ExecuteJavascript(code);
}

void CMomNUIFrame::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height)
{
    if (m_iTextureID == 0)
        return;

    if (m_bNeedsRepaint)
    {
        m_bNeedsRepaint = false;
        g_pVGuiSurface->DrawSetTextureRGBAEx(m_iTextureID, (const unsigned char*) buffer, width, height, IMAGE_FORMAT_BGRA8888);
    }
    else
    {
        for (auto rect : dirtyRects)
            g_pVGuiSurface->DrawUpdateRegionTextureRGBA(m_iTextureID, rect.x, rect.y, (const unsigned char*) buffer, rect.width, rect.height, IMAGE_FORMAT_BGRA8888);
    }

    m_bDirty = true;
}