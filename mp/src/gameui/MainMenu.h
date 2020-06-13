#pragma once

#include "vgui_controls/EditablePanel.h"
#include "igameevents.h"

class MainMenuButton;

enum SortFlags_t
{
    FL_SORT_SHARED = 1 << 0,
    FL_SORT_INGAME = 1 << 1,
    FL_SORT_MENU = 1 << 2
};

class MainMenu : public vgui::EditablePanel, public IGameEventListener2
{
    DECLARE_CLASS_SIMPLE(MainMenu, vgui::EditablePanel);

    MainMenu(Panel *parent);
    ~MainMenu();

    void OnThink() OVERRIDE;
    bool IsVisible() OVERRIDE;
    void OnCommand(char const *cmd) OVERRIDE;

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void CreateMenu();
    void DrawMainMenu();
    void DrawLogo();
    void CheckVersion();
    void Paint() OVERRIDE;
    void PerformLayout() override;

    void Activate();

    MESSAGE_FUNC_PARAMS(OnMenuButtonCommand, "MenuButtonCommand", pKv);

private:
    CUtlVector<MainMenuButton *> m_pButtons;

    // Our own buttons...
    MainMenuButton *m_pButtonLobby;
    MainMenuButton *m_pButtonInviteFriends;
    MainMenuButton *m_pButtonSpectate;

    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    bool m_bFocused;
    wchar_t *m_logoLeft;
    wchar_t *m_logoRight;

    int m_iButtonsSpace;

    int m_iButtonsOffsetX, m_iButtonsOffsetY;

    int m_iLogoOffsetX, m_iLogoOffsetY;

    int m_iLogoWidth, m_iLogoHeight;

    bool m_bLogoText;
    vgui::ImagePanel *m_pLogoImage;
    bool m_bLogoAttachToMenu;

    Color m_cLogoLeft;
    Color m_cLogoRight;

    vgui::HFont m_fLogoFont;
    int m_nSortFlags;
    bool m_bNeedSort;

    vgui::Label *m_pVersionLabel;
};
