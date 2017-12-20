#pragma once

#include "vgui_controls/Panel.h"
#include "nui/NuiBrowserListener.h"

namespace vgui
{
    class ScrollBar;

    class NUIPanel : public Panel, public NuiBrowserListener
    {
    public:
        DECLARE_CLASS_SIMPLE(NUIPanel, vgui::Panel);

        NUIPanel(Panel *pParent, const char *pName);
        ~NUIPanel();

        void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;
        void OnThink() OVERRIDE;
        void Paint() OVERRIDE;
        void OnCursorEntered() OVERRIDE;
        void OnCursorExited() OVERRIDE;
        void OnCursorMoved(int x, int y) OVERRIDE;
        void OnMousePressed(MouseCode code) OVERRIDE;
        void OnMouseDoublePressed(MouseCode code) OVERRIDE;
        void OnMouseReleased(MouseCode code) OVERRIDE;
        void OnMouseWheeled(int delta) OVERRIDE;
        void OnKeyCodePressed(KeyCode code) OVERRIDE;
        void OnKeyCodeTyped(KeyCode code) OVERRIDE;
        void OnKeyTyped(wchar_t unichar) OVERRIDE;
        void OnKeyCodeReleased(KeyCode code) OVERRIDE;
        void OnSizeChanged(int newWide, int newTall) OVERRIDE;

        void OnBrowserCreated(HNUIBrowser handle) OVERRIDE;
        void OnBrowserClosed() OVERRIDE;
        void OnBrowserSize(int& wide, int& tall) OVERRIDE;
        void OnBrowserFailedToCreate() OVERRIDE;
        void OnBrowserPaint(const void* pBGRA, uint32 texWide, uint32 texTall, uint32 unUpdateX, uint32 unUpdateY, uint32 unUpdateWide, uint32 unUpdateTall, uint32 unScrollX, uint32 unScrollY) OVERRIDE;
        void OnBrowserPageLoaded(const char* pURL) OVERRIDE {}
        void OnBrowserJSAlertDialog(const char* pString) OVERRIDE {}

        void Refresh();
        void LoadURL(const char *pURL);
        void RunJavascript(const char *pScript);

        MESSAGE_FUNC(OnSliderMoved, "ScrollBarSliderMoved");

    private:
        CThreadFastMutex m_Mutex;

        uint8 *m_pTextureBuffer;
        bool m_bDirtyBuffer;

        ScrollBar *_hbar, *_vbar;

        HNUIBrowser m_hBrowser;
        int m_iTextureID;
        int m_iLastWidth;
        int m_iLastHeight;

        // Track the texture width and height requested so we can tell
        // when the size has changed and reallocate the texture.
        uint32 m_allocedTextureWidth;
        uint32 m_allocedTextureHeight;
        bool m_bNeedsFullTextureUpload;
    };
}