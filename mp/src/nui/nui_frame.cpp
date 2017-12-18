#include "nui_predef.h"
#include "nui_frame.h"
#include "nui_client.h"

#include "tier3/tier3.h"

CMomNUIFrame::CMomNUIFrame(uint32_t width, uint32_t height) :
    m_iWidth(width),
    m_iHeight(height),
    m_bInitialized(false), m_bShouldRender(false),
    m_pClient(nullptr),
    m_bNeedsRepaint(true),
    m_bDirty(false)
{
    m_pTextureBuffer = new uint8[width * height * 4];
    V_memset(m_pTextureBuffer, 0, m_iHeight * m_iWidth * 4);
}

CMomNUIFrame::~CMomNUIFrame()
{
    m_bInitialized = false;

    if (m_pClient.get())
    {
        delete m_pClient.get();
        m_pClient = nullptr;
    }

    delete[] m_pTextureBuffer;
}

bool CMomNUIFrame::Init(const char *url)
{
    if (m_bInitialized)
        return true;

    //m_pClient = new CMomNUIClient(this);

    CefWindowInfo info;
    info.SetAsWindowless(NULL);
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

    CefRefPtr<CefRequestContext> s_RequestContext = CefRequestContext::GetGlobalContext();

    if (!CefBrowserHost::CreateBrowser(info, m_pClient, url, browserSettings, s_RequestContext))
        return false;

    m_bInitialized = true;

    return true;
}

void CMomNUIFrame::OnResized(uint32_t width, uint32_t height)
{
    // Set the new dimensions.
    m_iWidth = width;
    m_iHeight = height;

    //m_pClient->Browser()->GetHost()->WasResized();

    m_Mutex.Lock();

    delete[] m_pTextureBuffer;
    m_pTextureBuffer = new uint8[m_iWidth * m_iHeight * 4];
    V_memset(m_pTextureBuffer, 0, m_iHeight * m_iWidth * 4);
    m_bNeedsRepaint = true;

    m_Mutex.Unlock();
}

/*void CMomNUIFrame::ExecuteJavascript(const std::string& code)
{
    if (m_pClient.get())
        m_pClient->ExecuteJavascript(code);
}*/

void CMomNUIFrame::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height)
{
    m_Mutex.Lock();

    if (m_bNeedsRepaint)
    {
        V_memcpy(m_pTextureBuffer, static_cast<const unsigned char*>(buffer), m_iWidth * m_iHeight * 4);
        m_bNeedsRepaint = false;
    }
    else
    {
        const unsigned char* textureBuffer = static_cast<const unsigned char*>(buffer);

        for (auto rect : dirtyRects)
        {
            for (int y = rect.y; y < rect.y + rect.height; y++)
            {
                V_memcpy(m_pTextureBuffer + (y * m_iWidth * 4) + (rect.x * 4),
                    textureBuffer + (y * m_iWidth * 4) + (rect.x * 4),
                    rect.width * 4
                );
            }

        }
    }

    m_bDirty = true;
    m_bShouldRender = true;

    m_Mutex.Unlock();
}