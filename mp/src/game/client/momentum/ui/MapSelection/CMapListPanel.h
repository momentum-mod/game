#pragma once

#include "vgui_controls/ListPanel.h"

struct MapDisplay_t;
class CBaseMapsPage;
class MapDownloadProgress;


struct MapDownloadComponent
{
    MapDisplay_t *pMap;
    MapDownloadProgress *m_pOverridePanel;
    uint64 m_ulDownloadSize;
};

//-----------------------------------------------------------------------------
// Purpose: Acts like a regular ListPanel but forwards enter key presses
// to its outer control.
//-----------------------------------------------------------------------------
class CMapListPanel : public vgui::ListPanel
{
public:
    DECLARE_CLASS_SIMPLE(CMapListPanel, vgui::ListPanel);

    CMapListPanel(CBaseMapsPage *pOuter, const char *pName);

    void OnKeyCodeTyped(vgui::KeyCode code) OVERRIDE;
    void OnMouseReleased(vgui::MouseCode code) OVERRIDE;

    Panel* GetCellRenderer(int itemID, int column) OVERRIDE;

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;

    void SetFont(vgui::HFont font) OVERRIDE;

    void OnMapDownloadStart(KeyValues *pKv, MapDisplay_t *pDisplay);
    void OnMapDownloadProgress(KeyValues *pKv, MapDisplay_t *pDisplay);
    void OnMapDownloadEnd(KeyValues *pKv, MapDisplay_t *pDisplay);

private:
    CBaseMapsPage *m_pOuter;
    CUtlMap<int, MapDownloadComponent> m_mapDownloads;
};