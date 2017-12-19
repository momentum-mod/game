#pragma once

#include "vgui_controls/Panel.h"
#include "nui/NuiBrowserListener.h"

namespace vgui
{
    class ScrollBar;
}

class CMomNUIPanel : public vgui::Panel, public NuiBrowserListener
{
public:
    DECLARE_CLASS_SIMPLE(CMomNUIPanel, vgui::Panel);

    CMomNUIPanel();
    ~CMomNUIPanel();

    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;
    void OnCursorEntered() OVERRIDE;
    void OnCursorExited() OVERRIDE;
    void OnCursorMoved(int x, int y) OVERRIDE;
    void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    void OnMouseDoublePressed(vgui::MouseCode code) OVERRIDE;
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    void OnMouseWheeled(int delta) OVERRIDE;
    void OnKeyCodePressed(vgui::KeyCode code) OVERRIDE;
    void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    void OnKeyTyped(wchar_t unichar) OVERRIDE;
    void OnKeyCodeReleased(vgui::KeyCode code) OVERRIDE;

    void OnBrowserCreated(HNUIBrowser handle) OVERRIDE;
    void OnBrowserClosed() OVERRIDE;
    void OnBrowserSize(int& wide, int& tall) OVERRIDE;
    void OnBrowserFailedToCreate() OVERRIDE;
    void OnBrowserPaint(uint8* pBGRA, uint32 texWide, uint32 texTall, uint32 unUpdateX, uint32 unUpdateY, uint32 unUpdateWide, uint32 unUpdateTall, uint32 unScrollX, uint32 unScrollY) OVERRIDE;
    void OnBrowserPageLoaded(const char* pURL) OVERRIDE;
    void OnBrowserJSAlertDialog(const char* pString) OVERRIDE;

    void Refresh();

    void LoadURL(const char *pURL);

    MESSAGE_FUNC(OnSliderMoved, "ScrollBarSliderMoved");

private:
    vgui::ScrollBar *_hbar, *_vbar;

    HNUIBrowser m_hBrowser;
    int m_iTextureID;
    int m_iLastWidth;
    int m_iLastHeight;

    // Track the texture width and height requested so we can tell
    // when the size has changed and reallocate the texture.
    int m_allocedTextureWidth;
    int m_allocedTextureHeight;
    bool m_bNeedsFullTextureUpload;
};

extern CMomNUIPanel *g_pMomNUIPanel;