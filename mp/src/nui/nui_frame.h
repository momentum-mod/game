#pragma once

#include "nui_client.h"

#include <include/cef_v8.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/wrapper/cef_message_router.h>

class CMomNUIFrame
{
public:
    CMomNUIFrame(uint32_t width, uint32_t height);
    ~CMomNUIFrame();

public:
    bool Init(const std::string& url);
    void OnResized(uint32_t width, uint32_t height);
    void ExecuteJavascript(const std::string& code);
    void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height);

public:
    inline uint32_t FrameWidth() const { return m_iWidth; }
    inline uint32_t FrameHeight() const { return m_iHeight; }

    inline void ShouldRender(bool render) { m_bShouldRender = render; }
    inline bool ShouldRender() const { return m_bShouldRender; }

    inline CefRefPtr<CMomNUIClient> Client() const { return m_pClient; }

    inline void SetTexture(int texture) { m_bNeedsRepaint = true; m_iTextureID = texture; }

    inline bool Dirty() { return m_bDirty; }

protected:
    bool m_bInitialized;
    bool m_bShouldRender;
    bool m_bDirty;

    uint32_t m_iWidth;
    uint32_t m_iHeight;

    CefRefPtr<CMomNUIClient> m_pClient;

    int m_iTextureID;
    bool m_bNeedsRepaint;
};