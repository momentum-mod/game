#pragma once

#include "vgui2d/panel2d.h"

#include "button_mainmenu.h"

enum SortFlags_t
{
    FL_SORT_SHARED = 0x01,
    FL_SORT_INGAME = 0x02,
    FL_SORT_MENU = 0x04
};

class MainMenu : public Panel2D
{
    DECLARE_CLASS_SIMPLE(MainMenu, Panel2D);

  public:
    MainMenu(Panel *parent);

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    virtual void CreateMenu(const char *menu);
    void OnThink() OVERRIDE;
    bool IsVisible() OVERRIDE;
    virtual void DrawMainMenu();
    virtual void DrawLogo();
    void Paint() OVERRIDE;
    void OnCommand(char const *cmd) OVERRIDE;
    void OnSetFocus() OVERRIDE;
    void OnKillFocus() OVERRIDE;

    void Activate()
    {
        MoveToFront();
        SetVisible(true);
        SetEnabled(true);
    }

  private:
    CUtlVector<Button_MainMenu *> m_pButtons;

    // MOM_TODO: Remove this when it's no longer needed
    Button_MainMenu *m_pButtonFeedback;

    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    bool m_bFocused;
    wchar_t *m_logoLeft;
    wchar_t *m_logoRight;

    float m_fButtonsSpace;

    float m_fButtonsOffsetX;
    float m_fButtonsOffsetY;

    float m_fLogoOffsetX;
    float m_fLogoOffsetY;

    bool m_bLogoText;
    vgui::ImagePanel *m_pLogoImage;
    bool m_bLogoAttachToMenu;

    Color m_cLogoLeft;
    Color m_cLogoRight;

    vgui::HFont m_fLogoFont;
    int m_nSortFlags;
    bool m_bNeedSort;
};