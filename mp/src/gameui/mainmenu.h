#pragma once

#include "vgui2d/panel2d.h"

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

    void OnThink() OVERRIDE;
    void OnTick() OVERRIDE;
    bool IsVisible() OVERRIDE;
    void OnCommand(char const *cmd) OVERRIDE;
    void OnSetFocus() OVERRIDE;
    void OnKillFocus() OVERRIDE;

    void SetVisible(bool state) OVERRIDE;

    void ReloadMenu();

    void Activate()
    {
        MoveToFront();
        SetVisible(true);
        SetEnabled(true);
    }

  private:
    // Our own buttons...

    MainMenuHTML *m_pMainMenuHTMLPanel;
 
    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    bool m_bInGame;
    bool m_bInLobby;
    float m_fGameVolume;
    ConVarRef volumeRef;
};
