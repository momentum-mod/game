#include "cbase.h"

#include "MapDownloadProgress.h"
#include "util/mom_util.h"

#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"

#include "tier0/memdbgon.h"

using namespace vgui;

MapDownloadProgress::MapDownloadProgress(const char *pMapName) : BaseClass(nullptr, "MapDownloadProgress")
{
    Q_strncpy(m_szMapName, pMapName, sizeof(m_szMapName));
    m_pMapLabel = nullptr;
    m_pProgress = nullptr;

    SetProportional(true);
    SetSize(10, 10);
    SetScheme("MapSelectorScheme");

    m_pMapLabel = new Label(this, "MapName", m_szMapName);
    m_pProgress = new ContinuousProgressBar(this, "ProgressBar");

    LoadControlSettings("resource/ui/MapSelector/MapDownloadProgress.res");

    m_pMapLabel->DisableMouseInputForThisPanel(true);
    m_pProgress->DisableMouseInputForThisPanel(true);
    DisableMouseInputForThisPanel(true);

    MakeReadyForUse();
}

void MapDownloadProgress::SetDownloadProgress(float prog)
{
    m_pMapLabel->SetText(m_szMapName);
    // Interpolate the color
    Color interp = g_pMomentumUtil->ColorLerp(prog, m_cDownloadStart, m_cDownloadEnd);
    m_pMapLabel->SetFgColor(interp);
    m_pProgress->SetProgress(prog);
    m_pProgress->SetFgColor(interp);
}

void MapDownloadProgress::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cDownloadStart = pScheme->GetColor("MapDownloadProgress.DownloadStartColor", COLOR_RED);
    m_cDownloadEnd = pScheme->GetColor("MapDownloadProgress.DownloadEndColor", COLOR_GREEN);
}
