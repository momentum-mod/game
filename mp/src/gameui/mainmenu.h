#pragma once

#include "vgui_controls/HTML.h"
#include "igameevents.h"

class MomentumURLResolver;

class MainMenu : public vgui::HTML, public IGameEventListener2
{
    DECLARE_CLASS_SIMPLE(MainMenu, vgui::HTML);

    MainMenu(Panel *parent);
    ~MainMenu();

    void OnThink() OVERRIDE;
    void OnTick() OVERRIDE;
    bool IsVisible() OVERRIDE;
    void OnCommand(char const *cmd) OVERRIDE;

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    void LoadMenu()
    {
        OpenURL("mom://menu");
    }

    void OpenURL(const char *pURL)
    {
        BaseClass::OpenURL(pURL, nullptr);
    }

    MESSAGE_FUNC_CHARPTR(OnURLResolved, "ResolvedURL", url)
    {
        OpenURL(url);
    }

    void OnFinishRequest(const char* url, const char* pageTitle, const CUtlMap<CUtlString, CUtlString>& headers) OVERRIDE;

    // Commands (this code -> the browser)
    void SendVolumeCommand(); // Send if the game volume changed
    void SendVersionCommand(); // Send for the version of the game
    void SendLocalizationCommand(); // Send for the language of the game
    void SendLobbyUpdateCommand(); // Send if we join/leave a lobby
    void SendGameStatusCommand(); // Send if we're now in game or menu

    // Inputs from the browser
    void OnJSAlert(HTML_JSAlert_t* pAlert) OVERRIDE;

    void OnMousePressed(vgui::MouseCode mc) OVERRIDE;
    void OnScreenSizeChanged(int oldwide, int oldtall) OVERRIDE;

private:
    MomentumURLResolver *m_pURLResolver;
 
    char m_pszMenuOpenSound[MAX_PATH];
    char m_pszMenuCloseSound[MAX_PATH];

    bool m_bInGame;
    bool m_bInLobby;
    float m_fGameVolume;
    ConVarRef volumeRef;
};
