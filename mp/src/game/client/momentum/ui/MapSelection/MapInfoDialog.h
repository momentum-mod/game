#pragma once

#include "vgui_controls/Frame.h"
#include "mom_shareddefs.h"

struct MapData;
class ImageGallery;

namespace vgui 
{
    class ListPanelItem;
}

//-----------------------------------------------------------------------------
// Purpose: Dialog for displaying information about a game server
//-----------------------------------------------------------------------------
class CDialogMapInfo : public vgui::Frame
{
    DECLARE_CLASS_SIMPLE(CDialogMapInfo, vgui::Frame);

    CDialogMapInfo(Panel *parent, MapData *pMapData);
    ~CDialogMapInfo();

    void Run();

    // forces the dialog to attempt to connect to the server
    void Connect();

    void OnMapDataUpdate(KeyValues *pKv);

    // player list received
    virtual void ClearPlayerList();

    void UpdateMapDownloadState();

protected:
    // vgui overrides
    void PerformLayout() override;
    void OnCommand(const char* command) override;
    void OnClose() override;

    // API
    void GetMapInfo();
    void FillMapInfo();

    bool GetMapTimes(TimeType_t type);
    void OnTop10TimesCallback(KeyValues *pKvResponse);
    void OnAroundTimesCallback(KeyValues *pKvResponse);
    void OnFriendsTimesCallback(KeyValues *pKvResponse);
    void ParseAPITimes(KeyValues *pKvResponse, TimeType_t type);

private:
    // methods
    void RequestInfo();

    float m_fRequestDelays[TIMES_COUNT];
    bool m_bTimesLoading[TIMES_COUNT];

    vgui::Button *m_pMapActionButton, *m_pTop10Button, *m_pAroundButton, *m_pFriendsButton;
    vgui::ListPanel *m_pTimesList;
    ImageGallery *m_pImageGallery;
    EditablePanel *m_pMapInfoPanel;
    vgui::RichText *m_pMapDescription;

    bool m_bUnauthorizedFriendsList;

    MapData *m_pMapData;
};
