#include "cbase.h"

#include "MapInfoDialog.h"

#include "mom_api_requests.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "mom_map_cache.h"
#include <ctime>

#include "MapSelectorDialog.h"

#include "fmtstr.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/RichText.h"
#include "controls/ImageGallery.h"
#include "controls/FileImage.h"
#include "vgui/ILocalize.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static int __cdecl PlayerTimeColumnSortFunc(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
    float p1time = p1.kv->GetFloat("time_f");
    float p2time = p2.kv->GetFloat("time_f");

    if (p1time > p2time)
        return 1;
    if (p1time < p2time)
        return -1;

    return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogMapInfo::CDialogMapInfo(Panel *parent, MapData *pMapData) : Frame(parent, "DialogMapInfo"), m_pMapData(pMapData)
{
    SetProportional(true);
    SetBounds(0, 0, GetScaledVal(300), GetScaledVal(350));
    SetMinimumSize(GetScaledVal(300), GetScaledVal(350));
    SetDeleteSelfOnClose(true);

    m_pMapActionButton = new Button(this, "MapActionButton", "#MOM_MapSelector_StartMap", parent, "Connect");
    m_pTimesList = new ListPanel(this, "TimesList");
    m_pImageGallery = new ImageGallery(this, "MapGallery", true);
    m_pMapDescription = new RichText(this, "MapDescription");
    m_pMapInfoPanel = new EditablePanel(this, "MapInfoPanel");

    LoadControlSettings("resource/ui/MapSelector/DialogMapInfo.res");
    m_pMapInfoPanel->LoadControlSettings("resource/ui/MapSelector/MapInformationPanel.res");

    SetTitleBarVisible(false);
    SetCloseButtonVisible(true);

    m_pTimesList->AddColumnHeader(0, "rank", "#MOM_Rank", 54);
    m_pTimesList->AddColumnHeader(1, "name", "#MOM_Name", 166);
    m_pTimesList->AddColumnHeader(2, "time", "#MOM_Time", 120);
    m_pTimesList->AddColumnHeader(3, "date", "#MOM_Achieved", 100);

    m_pTimesList->SetSortFunc(2, &PlayerTimeColumnSortFunc);
    m_pTimesList->SetSortColumn(2);

    m_pTimesList->SetIgnoreDoubleClick(true);

    m_pTimesList->SetEmptyListText("#MOM_API_NoTimesReturned");
    m_pTimesList->SetShouldCenterEmptyListText(true);
    m_pTimesList->MakeReadyForUse();

    MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogMapInfo::~CDialogMapInfo()
{
    MapSelectorDialog().RemoveMapInfoDialog(m_pMapData->m_uID);
    m_pMapData = nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the dialog
//-----------------------------------------------------------------------------
void CDialogMapInfo::Run()
{
    // get the info for the map
    RequestInfo();
    Activate();
    UpdateMapDownloadState();
}

//-----------------------------------------------------------------------------
// Purpose: lays out the data
//-----------------------------------------------------------------------------
void CDialogMapInfo::PerformLayout()
{
    BaseClass::PerformLayout();
    
    Repaint();
}

void CDialogMapInfo::OnMapDataUpdate(KeyValues* pKv)
{
    if (pKv->GetBool("info") || pKv->GetBool("main"))
    {
        FillMapInfo();
        UpdateMapDownloadState();
    }
}


//-----------------------------------------------------------------------------
// Purpose: Requests the right info from the server
//-----------------------------------------------------------------------------
void CDialogMapInfo::RequestInfo()
{
    GetMapInfo();
    GetTop10MapTimes();
}

//-----------------------------------------------------------------------------
// Purpose: player list received
//-----------------------------------------------------------------------------
void CDialogMapInfo::ClearPlayerList()
{
    m_pTimesList->DeleteAllItems();
    Repaint();
}

void CDialogMapInfo::UpdateMapDownloadState()
{
    KeyValues *pCmd = new KeyValues("_");
    pCmd->SetInt("id", m_pMapData->m_uID);
    if (g_pMapCache->IsMapDownloading(m_pMapData->m_uID))
    {
        m_pMapActionButton->SetText("#MOM_MapSelector_CancelDownload");
        pCmd->SetName("CancelDownload");
    }
    else if (g_pMapCache->IsMapQueuedToDownload(m_pMapData->m_uID))
    {
        m_pMapActionButton->SetText("#MOM_MapSelector_RemoveFromQueue");
        pCmd->SetName("RemoveFromQueue");
    }
    else if (m_pMapData->m_bInLibrary)
    {
        m_pMapActionButton->SetText(m_pMapData->m_bMapFileNeedsUpdate ? "#MOM_MapSelector_DownloadMap" : "#MOM_MapSelector_StartMap");
        pCmd->SetName(m_pMapData->m_bMapFileNeedsUpdate ? "DownloadMap" : "StartMap");
    }
    else
    {
        m_pMapActionButton->SetText("#MOM_MapSelector_AddToLibrary");
        pCmd->SetName("AddToLibrary");
    }

    m_pMapActionButton->SetCommand(pCmd);
}

void CDialogMapInfo::GetMapInfo()
{
    // Update it, map cache can return false here meaning it doesn't need to
    g_pMapCache->UpdateMapInfo(m_pMapData->m_uID);

    FillMapInfo();
}

void CDialogMapInfo::FillMapInfo()
{
    if (!m_pMapData)
        return;

    KeyValues *pLoc = new KeyValues("Loc");
    KeyValuesAD whatever(pLoc);

    // Name
    SetControlString("MapLabel", m_pMapData->m_szMapName);

    // Author
    CUtlString authors;
    m_pMapData->GetCreditString(&authors, CREDIT_AUTHOR);
    pLoc->SetString("authors", authors.Get());
    SetControlString("AuthorLabel", CConstructLocalizedString(g_pVGuiLocalize->Find("#MOM_Map_Author"), pLoc));

    // Info stuff
    m_pMapData->m_Info.ToKV(pLoc);

    // Game mode
    m_pMapInfoPanel->SetControlString("GamemodeText", g_szGameModes[m_pMapData->m_eType]);

    // Difficulty
    m_pMapInfoPanel->SetControlString("DifficultyText", pLoc->GetWString("difficulty"));
    
    // Layout
    m_pMapInfoPanel->SetControlString("LayoutText", m_pMapData->m_Info.m_bIsLinear ?
                     g_pVGuiLocalize->Find("#MOM_Linear") :
                     g_pVGuiLocalize->Find("#MOM_Staged"));

    // Zones
    m_pMapInfoPanel->SetControlString("NumZones", pLoc->GetWString("numZones"));

    // Bonuses
    m_pMapInfoPanel->SetControlString("NumBonusText", pLoc->GetWString("numBonuses"));

    // Creation Date
    time_t creationDateTime;
    if (g_pMomentumUtil->ISODateToTimeT(m_pMapData->m_Info.m_szCreationDate, &creationDateTime))
    {
        char date[32];
        strftime(date, 32, "%b %d, %Y", localtime(&creationDateTime));
        m_pMapInfoPanel->SetControlString("CreationDateText", date);
    }

    // Release date
    time_t releaseDateTime;
    if (g_pMomentumUtil->ISODateToTimeT(m_pMapData->m_szCreatedAt, &releaseDateTime))
    {
        char date[32];
        strftime(date, 32, "%b %d, %Y", localtime(&releaseDateTime));
        m_pMapInfoPanel->SetControlString("ReleaseDateText", date);
    }

    // Description
    m_pMapDescription->SetText(m_pMapData->m_Info.m_szDescription);
    m_pMapDescription->GotoTextStart();

    // Update images
    if (m_pMapData->m_vecImages.Count() != m_pImageGallery->GetImageCount())
    {
        m_pImageGallery->RemoveAllImages();

        FOR_EACH_VEC(m_pMapData->m_vecImages, i)
        {
            m_pImageGallery->AddImage(new URLImage(m_pMapData->m_vecImages[i].m_szURLLarge, nullptr, true));
        }
    }
}

void CDialogMapInfo::GetTop10MapTimes()
{
    g_pAPIRequests->GetTop10MapTimes(m_pMapData->m_uID, UtlMakeDelegate(this, &CDialogMapInfo::Get10MapTimesCallback));
}

void CDialogMapInfo::Get10MapTimesCallback(KeyValues *pKvResponse)
{
    KeyValues *pData = pKvResponse->FindKey("data");
    KeyValues *pErr = pKvResponse->FindKey("error");
    if (pData)
    {
        KeyValues *pRuns = pData->FindKey("runs");

        if (pRuns && pData->GetInt("count") > 0)
        {
            ClearPlayerList();

            // Iterate through each loaded run
            FOR_EACH_SUBKEY(pRuns, pRun)
            {
                KeyValuesAD kvEntry("Entry");

                float fTime = pRun->GetFloat("time");
                kvEntry->SetFloat("time_f", fTime);
                char buf[BUFSIZETIME];
                g_pMomentumUtil->FormatTime(fTime, buf);
                kvEntry->SetString("time", buf);

                // Date
                char timeAgoStr[64];
                if (g_pMomentumUtil->GetTimeAgoString(pRun->GetString("dateAchieved"), timeAgoStr, sizeof(timeAgoStr)))
                    kvEntry->SetString("date", timeAgoStr);
                else
                    kvEntry->SetString("date", pRun->GetString("dateAchieved"));

                KeyValues *kvUserObj = pRun->FindKey("user");
                if (kvUserObj)
                {
                    /*uint64 steamID = Q_atoui64(kvUserObj->GetString("id"));
                    kvEntry->SetUint64("steamid", steamID);

                    int permissions = kvUserObj->GetInt("permissions");*/

                    // MOM_TODO: check if alias banned
                    kvEntry->SetString("name", kvUserObj->GetString("alias"));
                }

                // Rank
                KeyValues *kvRankObj = pRun->FindKey("rank");
                if (kvRankObj)
                {
                    kvEntry->SetInt("rank", kvRankObj->GetInt("rank"));
                }

                m_pTimesList->AddItem(kvEntry, 0, false, true);
            }

            //Update the control with this new info
            InvalidateLayout();
            m_pTimesList->InvalidateLayout();
            Repaint();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle
    }
}