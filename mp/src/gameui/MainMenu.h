#pragma once

#include "vgui_controls/NUIPanel.h"
#include "igameevents.h"

class MomentumURLResolver;

class MainMenu : public vgui::NUIPanel, public IGameEventListener2
{
    DECLARE_CLASS_SIMPLE(MainMenu, vgui::NUIPanel);

    MainMenu(Panel *parent);
    ~MainMenu();

    void OnThink() OVERRIDE;
    void OnTick() OVERRIDE;
    bool IsVisible() OVERRIDE;
    void OnCommand(char const *cmd) OVERRIDE;

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    void LoadMenu()
    {
        LoadURL(m_pszMainMenuPath);
    }

    MESSAGE_FUNC_CHARPTR(OnURLResolved, "ResolvedURL", url)
    {
        LoadURL(url);
    }

    //void OnFinishRequest(const char* url, const char* pageTitle, const CUtlMap<CUtlString, CUtlString>& headers) OVERRIDE;

    // Commands (this code -> the browser)
    void SendVolumeCommand(); // Send if the game volume changed
    void SendVersionCommand(); // Send for the version of the game
    void SendLocalizationCommand(); // Send for the language of the game
    void SendLobbyUpdateCommand(); // Send if we join/leave a lobby
    void SendGameStatusCommand(); // Send if we're now in game or menu

    // Inputs from the browser
    void OnBrowserJSAlertDialog(const char* pString) OVERRIDE;
    void OnBrowserPageLoaded(const char* pURL) OVERRIDE;

    // MOM_TODO: Remove this in favor of custom URL scheme handling and redirecting
    void GetMainMenuFile(char *pOut, int outSize);

    void OnMousePressed(vgui::MouseCode mc) OVERRIDE;
    void OnScreenSizeChanged(int oldwide, int oldtall) OVERRIDE;

private:
    MomentumURLResolver *m_pURLResolver;
 
    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    char m_pszMainMenuPath[MAX_PATH];

    bool m_bInGame;
    bool m_bInLobby;
    float m_fGameVolume;
    ConVarRef volumeRef;
};
