#pragma once

#include "vgui_controls/Frame.h"

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
    void PerformLayout() OVERRIDE;

    // API
    void GetMapInfo();
    void FillMapInfo();

    void GetTop10MapTimes();
    void Get10MapTimesCallback(KeyValues *pKvResponse);

private:
    // methods
    void RequestInfo();

    vgui::Button *m_pMapActionButton;
    vgui::ListPanel *m_pTimesList;
    ImageGallery *m_pImageGallery;
    EditablePanel *m_pMapInfoPanel;
    vgui::RichText *m_pMapDescription;

    MapData *m_pMapData;
};
