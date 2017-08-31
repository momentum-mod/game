#pragma once

#include "vgui2d/panel2d.h"

#include "button_mainmenu.h"

enum SortFlags_t
{
    FL_SORT_SHARED = 0x01,
    FL_SORT_INGAME = 0x02,
    FL_SORT_MENU = 0x04
};

class MainMenuHTML;

class MainMenu : public Panel2D
{
    DECLARE_CLASS_SIMPLE(MainMenu, Panel2D);

  public:
    MainMenu(Panel *parent);

    ~MainMenu();

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    virtual void CreateMenu(const char *menu);
    void OnThink() OVERRIDE;
    void OnTick() OVERRIDE;
    bool IsVisible() OVERRIDE;
    virtual void DrawMainMenu();
    virtual void DrawLogo();
    void Paint() OVERRIDE;
    void OnCommand(char const *cmd) OVERRIDE;
    void OnSetFocus() OVERRIDE;
    void OnKillFocus() OVERRIDE;

    void ReloadMenu();

    void Activate()
    {
        MoveToFront();
        SetVisible(true);
        SetEnabled(true);
    }

  private:
    CUtlVector<Button_MainMenu *> m_pButtons;

    // Our own buttons...

    MainMenuHTML *m_pMainMenuHTMLPanel;

    // MOM_TODO: Remove this when it's no longer needed
    Button_MainMenu *m_pButtonFeedback;

    bool m_bInLobby;

    Button_MainMenu *m_pButtonLobby;
    Button_MainMenu *m_pButtonInviteFriends;

    // Pointers to main menu buttons...

    Button_MainMenu *m_pButtonSpectate;
    bool m_bIsSpectating;
 

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

    bool m_bLogoPlayerCount;
    bool m_bLogoText;
    vgui::ImagePanel *m_pLogoImage;
    bool m_bLogoAttachToMenu;

    Color m_cLogoLeft;
    Color m_cLogoRight;
    Color m_cLogoPlayerCount;

    vgui::HFont m_fLogoPlayerCount;
    vgui::HFont m_fLogoFont;
    int m_nSortFlags;
    bool m_bNeedSort;

    bool m_bInGame;
    float m_fGameVolume;
    ConVarRef volumeRef;
};