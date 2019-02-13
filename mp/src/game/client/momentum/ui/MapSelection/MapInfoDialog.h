#pragma once

#include "vgui_controls/Frame.h"

struct MapData;

namespace vgui 
{
    class ListPanelItem;
}

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying information about a game server
//-----------------------------------------------------------------------------
class CDialogMapInfo : public vgui::Frame, public CGameEventListener
{
    DECLARE_CLASS_SIMPLE(CDialogMapInfo, vgui::Frame);

public:
    CDialogMapInfo(Panel *parent, MapData *pMapData);
    ~CDialogMapInfo();

    void Run();

    // forces the dialog to attempt to connect to the server
    void Connect();

    void FireGameEvent(IGameEvent* event) OVERRIDE;

    // player list received
    virtual void ClearPlayerList();

protected:
    // message handlers
    MESSAGE_FUNC(OnConnect, "Connect");
    // vgui overrides
    void PerformLayout() OVERRIDE;

    // API
    void GetMapInfo();
    void FillMapInfo();

    void GetTop10MapTimes();
    void Get10MapTimesCallback(KeyValues *pKvResponse);

private:

    static int PlayerTimeColumnSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &p1, const vgui::ListPanelItem &p2);

    // methods
    void RequestInfo();
    void ConnectToServer();
    void ApplyConnectCommand();

    vgui::Button *m_pConnectButton;
    vgui::Button *m_pCloseButton;
    vgui::ListPanel *m_pPlayerList;

    // true if we should try connect to the server when it refreshes
    bool m_bConnecting;
    bool m_bPlayerListUpdatePending;
    MapData *m_pMapData;
};
