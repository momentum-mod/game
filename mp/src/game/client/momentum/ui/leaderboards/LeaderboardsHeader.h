#pragma once

#include "vgui_controls/EditablePanel.h"

class CLeaderboardsHeader : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CLeaderboardsHeader, EditablePanel);
    CLeaderboardsHeader(Panel *pParent);

    void LoadData(const char *pMapName, bool bFullUpdate);
    void Reset() { m_bMapInfoLoaded = false;}

    void OnGetPlayerDataForMap(KeyValues *pKv);
    void OnGetMapInfo(KeyValues *pKv);

    // Sets the text of the MapInfo label. If it's nullptr, it hides it
    void UpdateMapInfoLabel(const char *text = nullptr);
    void UpdateMapInfoLabel(const char *author, const int tier, const char *layout, const int bonus);

private:
    vgui::Label *m_pMapName;
    vgui::Label *m_pMapAuthor;
    vgui::Label *m_pMapDetails;

    bool m_bMapInfoLoaded;
};
