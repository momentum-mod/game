#pragma once

#include "vgui_controls/EditablePanel.h"

class DrawerPanel_Profile;
class DrawerPanel_Lobby;
class DrawerPanel_Changelog;

enum DrawerTab_t
{
    DRAWER_TAB_USER = 0,
    DRAWER_TAB_LOBBY,
    DRAWER_TAB_CHANGELOG,
};

class MenuDrawerPanel : public vgui::EditablePanel
{
public:
    DECLARE_CLASS_SIMPLE(MenuDrawerPanel, EditablePanel);

    MenuDrawerPanel(Panel* pParent);

    void OnLobbyLeave();
    void OnLobbyEnter();
    void OnSpecStart();
    void OnSpecStop();
    void OnSiteAuth();

    bool IsDrawerOpen() const { return m_bDrawerOpen; }
    int GetCurrentDrawerTab() const { return m_iActivePage; }

    void OpenDrawerTo(DrawerTab_t tab);

    int GetDrawerButtonWidth() const;

protected:
    void OnKeyCodeTyped(vgui::KeyCode code) override;
    void PerformLayout() override;
    void OnCommand(const char* command) override;
    void OnReloadControls() override;
    void OnThink() override;
    void OnScreenSizeChanged(int oldwide, int oldtall) override;

    MESSAGE_FUNC(OnPageChanged, "PageChanged");

    CPanelAnimationVar(float, m_fPanelX, "PanelX", "-1.0");

private:
    int GetDesiredDrawerPosition(bool bOpen);

    void ToggleDrawer(bool bAnimate = true);

    vgui::Button *m_pDrawerHandleButton;
    vgui::PropertySheet *m_pDrawerContent;

    DrawerPanel_Profile *m_pProfileDrawerPanel;
    DrawerPanel_Lobby *m_pLobbyDrawerPanel;
    DrawerPanel_Changelog *m_pChangelogDrawerPanel;

    int m_iActivePage;
    bool m_bDrawerOpen;
};