#ifndef DIALOGGAMEINFO_H
#define DIALOGGAMEINFO_H
#ifdef _WIN32
#pragma once
#endif


/*struct challenge_s
{
    netadr_t addr;
    int challenge;
};*/

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying information about a game server
//-----------------------------------------------------------------------------
class CDialogMapInfo : public vgui::Frame//, public ISteamMatchmakingPlayersResponse, public ISteamMatchmakingPingResponse
{
    DECLARE_CLASS_SIMPLE(CDialogMapInfo, vgui::Frame);

public:
    CDialogMapInfo(vgui::Panel *parent, const char*);
    ~CDialogMapInfo();

    void Run(const char *titleName);

    // forces the dialog to attempt to connect to the server
    void Connect();

    // on individual player added
    virtual void AddPlayerToList(const char *playerName, int score, float timePlayedSeconds);

    // called when the current refresh list is complete
    virtual void RefreshComplete(EMatchMakingServerResponse response);

    // player list received
    virtual void ClearPlayerList();

protected:
    // message handlers
    MESSAGE_FUNC(OnConnect, "Connect");
    // vgui overrides
    virtual void OnTick();
    virtual void PerformLayout();

    // API
    void GetMapInfo(const char* mapname);
    void GetMapInfoCallback(HTTPRequestCompleted_t*, bool);
    CCallResult<CDialogMapInfo, HTTPRequestCompleted_t> cbGetMapInfoCallback;

private:
#ifndef NO_STEAM
    STEAM_CALLBACK(CDialogMapInfo, OnPersonaStateChange, PersonaStateChange_t, m_CallbackPersonaStateChange);
#endif

    static int PlayerTimeColumnSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &p1, const vgui::ListPanelItem &p2);

    // methods
    void RequestInfo(const char* mapName);
    void ConnectToServer();
    void ApplyConnectCommand(const char *mapName);

    vgui::Button *m_pConnectButton;
    vgui::Button *m_pCloseButton;
    vgui::Label *m_pAuthorLabel;
    vgui::ListPanel *m_pPlayerList;

    enum { PING_TIMES_MAX = 4 };

    // true if we should try connect to the server when it refreshes
    bool m_bConnecting;
    bool m_bPlayerListUpdatePending;
};

#endif // DIALOGGAMEINFO_H
