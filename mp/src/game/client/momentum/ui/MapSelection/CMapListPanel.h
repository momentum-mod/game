#pragma once

#include "vgui_controls/ListPanel.h"

struct MapDisplay_t;
class CBaseMapsPage;
class MapDownloadProgress;

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

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;

private:
    CBaseMapsPage *m_pOuter;
};