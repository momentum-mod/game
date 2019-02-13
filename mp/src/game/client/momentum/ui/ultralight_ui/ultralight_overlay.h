#ifndef ULTRALIGHT_OVERLAY_H
#define ULTRALIGHT_OVERLAY_H
#ifdef _WIN32
#pragma once
#endif

#include <Ultralight/Renderer.h>
#include <Ultralight/View.h>
#include <Ultralight/platform/GPUDriver.h>
#include <utlvector.h>
#include <vgui_controls/Panel.h>

class UltralightOverlay : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(UltralightOverlay, vgui::Panel);
  public:
    UltralightOverlay(ultralight::Ref<ultralight::Renderer> renderer,
                      Panel *pParentPanel,
                      bool bTransparent);
    virtual ~UltralightOverlay();

    ultralight::Ref<ultralight::View> view() { return m_pView; }

    bool Contains(int x, int y)
    {
        int posX, posY;
        int width, height;
        GetBounds(posX, posY, width, height);

        return x >= posX && y >= posY && x < posX + width && y < posY + height;
    }

    virtual void Paint() OVERRIDE;

    virtual void OnSizeChanged(int newWide, int newTall) OVERRIDE;

    virtual void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseWheeled(int delta) OVERRIDE;
    virtual void OnCursorMoved(int x, int y) OVERRIDE;
    virtual void OnCursorExited() OVERRIDE;
    virtual void OnKeyCodePressed(vgui::KeyCode code) OVERRIDE;
    virtual void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    virtual void OnKeyCodeReleased(vgui::KeyCode code) OVERRIDE;

    virtual void FireMouseEvent(const ultralight::MouseEvent &evt);
    virtual void FireScrollEvent(const ultralight::ScrollEvent &evt);
    virtual void FireKeyEvent(const ultralight::KeyEvent &evt);

    virtual void OnBeginLoading();
    virtual void OnFinishLoading();
    virtual void OnUpdateHistory();
    virtual void OnDOMReady();

    virtual void OnChangeTitle(const char *title);
    virtual void OnChangeURL(const char *url);
    virtual void OnChangeTooltip(const char *tooltip);
    virtual void OnChangeCursor(ultralight::Cursor cursor);
    virtual void OnAddConsoleMessage(ultralight::MessageSource source, ultralight::MessageLevel level,
                                     const char *message, uint32_t line_number, uint32_t column_number,
                                     const char *source_id);

    bool IsLinkedToConsole() const { return m_bLinkToConsole; }
    void SetLinkedToConsole(bool linked) { m_bLinkToConsole = linked; }
  protected:
    bool m_bHasFocus;
    bool m_bHasHover;
    bool m_bLinkToConsole;
    ultralight::Ref<ultralight::View> m_pView;
    ultralight::ViewListener *m_pViewListener;
    ultralight::LoadListener *m_pLoadListener;

    int m_iTextureId;
};

#endif
