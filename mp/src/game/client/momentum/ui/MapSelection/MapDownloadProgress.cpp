#include "cbase.h"

#include "MapDownloadProgress.h"
#include "util/mom_util.h"

#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"

#include "tier0/memdbgon.h"

using namespace vgui;

MapDownloadProgress::MapDownloadProgress(const char *pMapName) : BaseClass(nullptr, "MapDownloadProgress")
{
    m_pMapLabel = nullptr;
    m_pProgress = nullptr;
    m_uDownloadSize = 0;

    SetProportional(true);
    SetSize(10, 10);
    SetScheme("MapSelectorScheme");

    m_pMapLabel = new Label(this, "MapName", pMapName);
    m_pProgress = new ContinuousProgressBar(this, "ProgressBar");

    LoadControlSettings("resource/ui/MapSelector/MapDownloadProgress.res");

    m_pMapLabel->SetText(pMapName);
    m_pMapLabel->DisableMouseInputForThisPanel(true);
    m_pProgress->DisableMouseInputForThisPanel(true);
    DisableMouseInputForThisPanel(true);

    MakeReadyForUse();
}

void MapDownloadProgress::SetDownloadSize(uint32 size)
{
    m_uDownloadSize = size;
}

void MapDownloadProgress::SetDownloadProgress(uint32 offset)
{
    const float fProgress = float(double(offset) / double(m_uDownloadSize));

    m_pProgress->SetProgress(fProgress);

    // Interpolate the color
    const Color lerpedColor = g_pMomentumUtil->ColorLerp(fProgress, m_cDownloadStart, m_cDownloadEnd);
    m_pMapLabel->SetFgColor(lerpedColor);
    m_pProgress->SetFgColor(lerpedColor);
}

void MapDownloadProgress::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cDownloadStart = pScheme->GetColor("MapDownloadProgress.DownloadStartColor", COLOR_RED);
    m_cDownloadEnd = pScheme->GetColor("MapDownloadProgress.DownloadEndColor", COLOR_GREEN);
}
