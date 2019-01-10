#pragma once

#include "vgui_controls/EditablePanel.h"

struct MapData;

class CLeaderboardsHeader : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CLeaderboardsHeader, EditablePanel);
    CLeaderboardsHeader(Panel *pParent);

    void LoadData(const char *pMapName, bool bFullUpdate);
    void Reset();

    void LoadMapData();

    // Sets the text of the MapInfo label
    void UpdateMapInfoLabel(MapData *pData);

private:
    vgui::Label *m_pMapName;
    vgui::Label *m_pMapAuthor;
    vgui::Label *m_pMapDetails;

    bool m_bMapInfoLoaded;
};
