#pragma once

#include "vgui_controls/EditablePanel.h"

class MapDownloadProgress : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(MapDownloadProgress, vgui::EditablePanel);

    MapDownloadProgress(const char *pMapName);

    void SetDownloadProgress(float prog);

    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

private:
    char m_szMapName[MAX_MAP_NAME];
    vgui::ProgressBar *m_pProgress;
    vgui::Label *m_pMapLabel;
    Color m_cDownloadStart, m_cDownloadEnd;
};
