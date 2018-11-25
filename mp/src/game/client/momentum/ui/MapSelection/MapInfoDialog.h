#ifndef DIALOGGAMEINFO_H
#define DIALOGGAMEINFO_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying information about a game server
//-----------------------------------------------------------------------------
class CDialogMapInfo : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CDialogMapInfo, vgui::Frame);

public:
    CDialogMapInfo(Panel *parent, const char*);
    ~CDialogMapInfo();

    void Run(const char *titleName);

    // forces the dialog to attempt to connect to the server
    void Connect();

    // on individual player added
    virtual void AddPlayerToList(KeyValues *pPlayerInfo);

    // called when the current refresh list is complete
    virtual void RefreshComplete(EMatchMakingServerResponse response);

    // player list received
    virtual void ClearPlayerList();

protected:
    // message handlers
    MESSAGE_FUNC(OnConnect, "Connect");
    // vgui overrides
    void OnTick() OVERRIDE;
    void PerformLayout() OVERRIDE;

    // API
    void GetMapInfo(const char* mapname);
    void GetMapInfoCallback(KeyValues *pKvResponse);

    void Get10MapTimes(const char* mapname);
    void Get10MapTimesCallback(KeyValues *pKvResponse);


private:

    static int PlayerTimeColumnSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &p1, const vgui::ListPanelItem &p2);

    // methods
    void RequestInfo(const char* mapName);
    void ConnectToServer();
    void ApplyConnectCommand(const char *mapName);

    vgui::Button *m_pConnectButton;
    vgui::Button *m_pCloseButton;
    vgui::ListPanel *m_pPlayerList;

    enum { PING_TIMES_MAX = 4 };

    // true if we should try connect to the server when it refreshes
    bool m_bConnecting;
    bool m_bPlayerListUpdatePending;
};

#endif // DIALOGGAMEINFO_H
