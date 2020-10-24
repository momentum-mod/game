#include "cbase.h"

#include "LoadingScreen.h"
#include "gameui/BaseMenuPanel.h"

#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui_controls/ImagePanel.h"
#include "controls/FileImage.h"
#include "FileImageCache.h"

#include "vgui/ISurface.h"
#include "vgui/IInput.h"

#include "filesystem.h"
#include "mom_map_cache.h"
#include "fmtstr.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static CLoadingScreen *g_pLoadingScreen = nullptr;

CLoadingScreen::CLoadingScreen(CBaseMenuPanel *pParent) : BaseClass(pParent, "LoadingScreen")
{
    g_pLoadingScreen = this;

    m_pMapImage = nullptr;
    m_pMapImageStreaming = nullptr;
    m_fMapImageOpacityMax = 0.5;

    SetProportional(true);
    SetScheme("SourceScheme");
    MakePopup();
    SetVisible(false);
    SetMouseInputEnabled(false);

    m_pMapNameLabel = new Label(this, "MapNameLabel", "");
    m_pAuthorsLabel = new Label(this, "AuthorsLabel", "");
    m_pDifficultyLayoutLabel = new Label(this, "DifficultyLayoutLabel", "");
    m_pTrackZonesLabel = new Label(this, "TrackZonesLabel", "");

    m_pLoadingPercentLabel = new Label(this, "LoadingPercentLabel", "");
    m_pTipLabel = new Label(this, "TipLabel", "");
    m_pProgressBar = new ContinuousProgressBar(this, "ProgressBar");

    m_pMapImagePanel = new ImagePanel(this, "MapImagePanel");

    LoadControlSettings("resource/ui/LoadingScreen.res");

    ListenForGameEvent("mapcache_map_load");
}

CLoadingScreen::~CLoadingScreen()
{
    g_pLoadingScreen = nullptr;
}

void CLoadingScreen::FireGameEvent(IGameEvent *event)
{
    LoadMapData(event->GetString("map"));
}

void CLoadingScreen::Activate()
{
    GetBuildGroup()->ReloadControlSettings();

    InvalidateLayout(true, true);
    SetVisible(true);

    m_pTipLabel->SetText("");
    m_pMapNameLabel->SetText("");
    m_pAuthorsLabel->SetText("");
    m_pDifficultyLayoutLabel->SetText("");
    m_pTrackZonesLabel->SetText("");
    ProgressUpdate(0.0f);

    ipanel()->MoveToFront(GetVPanel());
    input()->SetAppModalSurface(GetVPanel());
    surface()->RestrictPaintToSinglePanel(GetVPanel());
}

void CLoadingScreen::Deactivate()
{
    SetVisible(false);
    CleanupMapImage();

    ipanel()->MoveToBack(GetVPanel());
    input()->SetAppModalSurface(NULL);
    surface()->RestrictPaintToSinglePanel(NULL);
}

void CLoadingScreen::ProgressUpdate(float percent)
{
    m_pProgressBar->SetProgress(percent);
    m_pLoadingPercentLabel->SetText(CFmtStr("Loading... %i%%", static_cast<int>(percent * 100)));

    if (m_pMapImagePanel->IsVisible())
    {
        const auto iRemappedAlpha = static_cast<int>(255.0f * RemapValClamped(percent, 0.0f, 1.0f, 0.0f, m_fMapImageOpacityMax));
        m_pMapImagePanel->SetDrawColor(Color(255, 255, 255, iRemappedAlpha));
    }
}

void CLoadingScreen::LoadMapData(const char *pMapName)
{
    CleanupMapImage();

    SetTipPanelText(pMapName);

    const auto pMapData = g_pMapCache->GetCurrentMapData();
    if (pMapData)
    {
        m_pMapNameLabel->SetText(pMapData->m_szMapName);

        CUtlString m_Authors;
        pMapData->GetCreditString(&m_Authors, CREDIT_AUTHOR);
        m_pAuthorsLabel->SetText(CFmtStr("By: %s", m_Authors.Get()));
        m_pAuthorsLabel->SetVisible(true);

        m_pDifficultyLayoutLabel->SetText(CFmtStr("Tier %i - %s", pMapData->m_MainTrack.m_iDifficulty,
                                                                          pMapData->m_MainTrack.m_bIsLinear ? "LINEAR" : "STAGED"));
        m_pDifficultyLayoutLabel->SetVisible(true);

        const auto numBonuses = pMapData->m_Info.m_iNumTracks - 1;
        if (numBonuses > 0)
        {
            m_pTrackZonesLabel->SetText(CFmtStr("%i zones - %i bonus%s", pMapData->m_MainTrack.m_iNumZones,
                                                                                 numBonuses,
                                                                                 numBonuses > 1 ? "es" : ""));
        }
        else
        {
            m_pTrackZonesLabel->SetText(CFmtStr("%i zones", pMapData->m_MainTrack.m_iNumZones));
        }

        m_pTrackZonesLabel->SetVisible(true);

        const auto pEntryLarge = g_pFileImageCache->FindImageByPath(pMapData->m_Thumbnail.m_szURLLarge);
        const auto pEntrySmall = g_pFileImageCache->FindImageByPath(pMapData->m_Thumbnail.m_szURLSmall);
        if (pEntryLarge)
        {
            m_pMapImage = new URLImage;
            m_pMapImage->LoadFromUtlBuffer(pEntryLarge->m_bufOriginalImage);
        }
        else if (pEntrySmall)
        {
            m_pMapImage = new URLImage;
            m_pMapImage->LoadFromUtlBuffer(pEntrySmall->m_bufOriginalImage);

            m_pMapImageStreaming = new URLImage;
            m_pMapImageStreaming->AddImageLoadListener(GetVPanel());
            m_pMapImageStreaming->LoadFromURL(pMapData->m_Thumbnail.m_szURLLarge);
        }
        else
        {
            m_pMapImage = new URLImage(pMapData->m_Thumbnail.m_szURLLarge);
        }

        m_pMapImagePanel->SetImage(m_pMapImage);
        m_pMapImagePanel->SetVisible(true);
    }
    else
    {
        const bool bIsCredits = FStrEq(pMapName, "credits");
        const bool bIsBackground = Q_strnistr(pMapName, "bg_", 3);
        if (!(bIsCredits || bIsBackground))
        {
            m_pMapNameLabel->SetText(pMapName);
        }
        else
        {
            m_pTipLabel->SetText("");
        }

        m_pAuthorsLabel->SetVisible(false);
        m_pAuthorsLabel->SetText("");

        m_pDifficultyLayoutLabel->SetText("");
        m_pDifficultyLayoutLabel->SetVisible(false);

        m_pTrackZonesLabel->SetText("");
        m_pTrackZonesLabel->SetVisible(false);

        m_pMapImagePanel->SetVisible(false);
    }
}

void CLoadingScreen::OnLargeImageLoad()
{
    if (m_pMapImagePanel->IsVisible())
    {
        const auto pOld = m_pMapImage;

        m_pMapImage = m_pMapImageStreaming;
        m_pMapImagePanel->SetImage(m_pMapImage);

        delete pOld;

        m_pMapImage->RemoveImageLoadListener(GetVPanel());
    }
    else if (m_pMapImageStreaming)
    {
        m_pMapImageStreaming->RemoveImageLoadListener(GetVPanel());
        delete m_pMapImageStreaming;
    }

    m_pMapImageStreaming = nullptr;
}

void CLoadingScreen::OnKeyCodeTyped(KeyCode code)
{
    if (code == KEY_ESCAPE)
    {
        Deactivate();
        engine->DisconnectInternal();
    }

    BaseClass::OnKeyCodeTyped(code);
}

void CLoadingScreen::SetTipPanelText(const char *pMapName)
{
    const auto pFound = g_pGameModeSystem->GetGameModeFromMapName(pMapName);
    const auto eMode = pFound ? pFound->GetType() : GAMEMODE_UNKNOWN;
    m_pTipLabel->SetText(m_TipManager.GetTipForGamemode(eMode));
}

void CLoadingScreen::CleanupMapImage()
{
    if (m_pMapImage)
    {
        delete m_pMapImage;
        m_pMapImage = nullptr;
    }

    m_pMapImagePanel->SetVisible(false);
    m_pMapImagePanel->SetImage((IImage *)nullptr);
}