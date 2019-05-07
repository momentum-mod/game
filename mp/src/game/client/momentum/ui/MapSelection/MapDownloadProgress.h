#pragma once

#include "vgui_controls/EditablePanel.h"

class MapDownloadProgress : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(MapDownloadProgress, vgui::EditablePanel);

    MapDownloadProgress(const char *pMapName);

    void SetDownloadSize(uint32 size);
    void SetDownloadProgress(uint32 offset);

    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

private:
    vgui::ProgressBar *m_pProgress;
    vgui::Label *m_pMapLabel;
    Color m_cDownloadStart, m_cDownloadEnd;
    uint32 m_uDownloadSize;
};
