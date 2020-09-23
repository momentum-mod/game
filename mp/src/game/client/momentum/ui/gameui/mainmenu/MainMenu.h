#pragma once

#include "vgui_controls/EditablePanel.h"
#include "igameevents.h"

class MainMenuButton;
class CBaseMenuPanel;
class UserComponent;
class MenuDrawerPanel;
class CRenderPanel;

enum SortFlags_t
{
    FL_SORT_SHARED = 1 << 0,
    FL_SORT_INGAME = 1 << 1,
    FL_SORT_MENU = 1 << 2
};

class MainMenu : public vgui::EditablePanel, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(MainMenu, vgui::EditablePanel);

    MainMenu(CBaseMenuPanel *pParent);
    ~MainMenu();

    void OnThink() override;
    bool IsVisible() override;
    void OnCommand(char const *cmd) override;

    void FireGameEvent(IGameEvent* event) override;

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void CreateMenu();
    void DrawMainMenu();
    void DrawLogo();
    void CheckVersion();
    void Paint() override;
    void PerformLayout() override;
    void SetVisible(bool state) override;

    void SetModelPanelEnabled(bool bEnabled);
    void SetModelPanelModel(const char *pModel);
    void SetModelPanelRotationSpeed(int iSpeed);

    MESSAGE_FUNC(OnUserComponentClicked, "UserComponentClicked");
    MESSAGE_FUNC_PARAMS(OnMenuButtonCommand, "MenuButtonCommand", pKv);

    // Messagebox forwards
    MESSAGE_FUNC(OnConfirmDisconnect, "ConfirmDisconnect");
    MESSAGE_FUNC(OnConfirmMapChange, "ConfirmMapChange");
private:
    CUtlVector<MainMenuButton *> m_pButtons;

    UserComponent *m_pUserComponent;
    MenuDrawerPanel *m_pMenuDrawer;

    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    bool m_bFocused;
    wchar_t m_logoLeft[128];
    wchar_t m_logoRight[128];

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

    CBaseMenuPanel *m_pBasePanel;

    PostProcessParameters_t m_PostProcessParameters;
    CRenderPanel *m_pModelPanel;
};