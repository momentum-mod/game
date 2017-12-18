#pragma once

#include "vgui_controls/Panel.h"
#include "NuiBrowserListener.h"
#include "INuiInterface.h"

class CNuiInterface;
namespace vgui
{
    class ScrollBar;
}

class CMomNUIPanel : public vgui::Panel, public NuiBrowserListener
{
public:
    DECLARE_CLASS_SIMPLE(CMomNUIPanel, vgui::Panel);

public:
    CMomNUIPanel();
    ~CMomNUIPanel();

public:
    virtual void OnThink() OVERRIDE;
    virtual void Paint() OVERRIDE;
    virtual void OnCursorEntered() OVERRIDE;
    virtual void OnCursorExited() OVERRIDE;
    virtual void OnCursorMoved(int x, int y) OVERRIDE;
    virtual void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseDoublePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseWheeled(int delta) OVERRIDE;
    virtual void OnKeyCodePressed(vgui::KeyCode code) OVERRIDE;
    virtual void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    virtual void OnKeyTyped(wchar_t unichar) OVERRIDE;
    virtual void OnKeyCodeReleased(vgui::KeyCode code) OVERRIDE;

    void OnBrowserCreated(HNUIBrowser handle) OVERRIDE;
    void OnBrowserClosed() OVERRIDE;
    void OnBrowserSize(int& wide, int& tall) OVERRIDE;
    void OnBrowserFailedToCreate() OVERRIDE;
    void OnBrowserPaint(uint8* pBGRA, uint32 texWide, uint32 texTall, uint32 unUpdateX, uint32 unUpdateY, uint32 unUpdateWide, uint32 unUpdateTall, uint32 unScrollX, uint32 unScrollY) OVERRIDE;
    void OnBrowserPageLoaded(const char* pURL) OVERRIDE;


    void Refresh();

    void LoadURL(const char *pURL);

    MESSAGE_FUNC(OnSliderMoved, "ScrollBarSliderMoved");

private:
    vgui::ScrollBar *_hbar, *_vbar;

    HNUIBrowser m_hBrowser;
    CNuiInterface *m_pInterface;
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