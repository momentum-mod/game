#ifndef ULTRALIGHT_OVERLAY_H
#define ULTRALIGHT_OVERLAY_H
#ifdef _WIN32
#pragma once
#endif

#include <Ultralight/Renderer.h>
#include <Ultralight/View.h>
#include <Ultralight/platform/GPUDriver.h>
#include <hudelement.h>
#include <utlvector.h>
#include <vgui_controls/Panel.h>

class UltralightOverlay
{
  public:
    UltralightOverlay(ultralight::Ref<ultralight::Renderer> renderer, ultralight::GPUDriver *driver, int x, int y,
                      int width, int height);
    virtual ~UltralightOverlay();

    ultralight::Ref<ultralight::View> view() { return m_pView; }

    int width() const { return m_iWidth; }
    int height() const { return m_iHeight; }
    int x() const { return m_iXPos; }
    int y() const { return m_iYPos; }

    void MoveTo(int x, int y)
    {
        m_iXPos = x;
        m_iYPos = y;
        m_bDirty = true;
    }
    void MoveBy(int dx, int dy)
    {
        m_iXPos += dx;
        m_iYPos += dy;
        m_bDirty = true;
    }

    bool Contains(int x, int y) const
    {
        return x >= m_iXPos && y >= m_iYPos && x < m_iXPos + m_iWidth && y < m_iYPos + m_iHeight;
    }

    virtual void Draw();

    virtual void FireKeyEvent(const ultralight::KeyEvent &evt);

    virtual void FireMouseEvent(const ultralight::MouseEvent &evt);

    virtual void FireScrollEvent(const ultralight::ScrollEvent &evt);

    virtual void Resize(int width, int height);

  protected:
    void UpdateGeometry();

    int m_iWidth;
    int m_iHeight;
    int m_iXPos;
    int m_iYPos;
    bool m_bHasFocus = false;
    bool m_bHasHover = false;
    ultralight::Ref<ultralight::View> m_pView;
    ultralight::GPUDriver *m_pDriver;
    CUtlVectorFixed<ultralight::Vertex_2f_4ub_2f_2f_28f, 4> m_vVertices;
    CUtlVectorFixed<ultralight::IndexType, 6> m_vIndices;
    bool m_bDirty;
    uint32_t m_iGeometryId;
    ultralight::GPUState m_GPUState;

    int m_iTextureId;
};

#endif
