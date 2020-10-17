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

#define UPDATE_INTERVAL 5.0f

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
    m_pTop10Button = new Button(this, "Top10Toggle", "#MOM_Leaderboards_Top10", this, "Top10");
    m_pAroundButton = new Button(this, "AroundToggle", "#MOM_Leaderboards_Around", this, "Around");
    m_pFriendsButton = new Button(this, "FriendsToggle", "#MOM_Leaderboards_Friends", this, "Friends");

    LoadControlSettings("resource/ui/mapselector/DialogMapInfo.res");
    m_pMapInfoPanel->LoadControlSettings("resource/ui/mapselector/MapInformationPanel.res");

    m_pTop10Button->SetStaySelectedOnClick(true);
    m_pAroundButton->SetStaySelectedOnClick(true);
    m_pFriendsButton->SetStaySelectedOnClick(true);

    SetTitleBarVisible(false);
    SetCloseButtonVisible(true);

    m_pTimesList->AddColumnHeader(0, "rank", "#MOM_Rank", GetScaledVal(40));
    m_pTimesList->AddColumnHeader(1, "name", "#MOM_Name", GetScaledVal(100));
    m_pTimesList->AddColumnHeader(2, "time", "#MOM_Time", GetScaledVal(50));
    m_pTimesList->AddColumnHeader(3, "date", "#MOM_Achieved", GetScaledVal(50));
    m_pTimesList->SetAutoTallHeaderToFont(true);

    m_pTimesList->SetSortFunc(2, &PlayerTimeColumnSortFunc);
    m_pTimesList->SetSortColumn(2);

    m_pTimesList->SetIgnoreDoubleClick(true);

    m_pTimesList->SetEmptyListText("#MOM_API_NoTimesReturned");
    m_pTimesList->SetShouldCenterEmptyListText(true);
    m_pTimesList->MakeReadyForUse();

    MoveToCenterOfScreen();

    m_bUnauthorizedFriendsList = false;
    V_memset(m_fRequestDelays, 0, sizeof(m_fRequestDelays));
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogMapInfo::~CDialogMapInfo()
{
    g_pMapSelector->RemoveMapInfoDialog(m_pMapData->m_uID);
    m_pMapData = nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the dialog
//-----------------------------------------------------------------------------
void CDialogMapInfo::Run()
{
    // get the info for the map
    RequestInfo();
    m_pTop10Button->DoClick(); // Start with top 10
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

void CDialogMapInfo::OnCommand(const char* command)
{
    if (FStrEq(command, "Top10"))
    {
        GetMapTimes(TIMES_TOP10);
        m_pAroundButton->SetSelected(false);
        m_pFriendsButton->SetSelected(false);
    }
    else if (FStrEq(command, "Around"))
    {
        GetMapTimes(TIMES_AROUND);
        m_pTop10Button->SetSelected(false);
        m_pFriendsButton->SetSelected(false);
    }
    else if (FStrEq(command, "Friends"))
    {
        GetMapTimes(TIMES_FRIENDS);
        m_pAroundButton->SetSelected(false);
        m_pTop10Button->SetSelected(false);
    }
    else BaseClass::OnCommand(command);
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
    SetControlString("AuthorLabel", CConstructLocalizedString(g_pVGuiLocalize->Find("#MOM_Map_Authors"), pLoc));

    // Info stuff
    m_pMapData->m_Info.ToKV(pLoc);
    const auto iTracks = pLoc->GetInt("numTracks");
    pLoc->SetInt("numBonus", iTracks > 1 ? iTracks - 1 : 0);
    m_pMapData->m_MainTrack.ToKV(pLoc);

    // Game mode
    m_pMapInfoPanel->SetControlString("GamemodeText", g_szGameModes[m_pMapData->m_eType]);

    // Difficulty
    m_pMapInfoPanel->SetControlString("DifficultyText", pLoc->GetWString("difficulty"));
    
    // Layout
    m_pMapInfoPanel->SetControlString("LayoutText", m_pMapData->m_MainTrack.m_bIsLinear ?
                     g_pVGuiLocalize->Find("#MOM_Linear") :
                     g_pVGuiLocalize->Find("#MOM_Staged"));

    // Zones
    m_pMapInfoPanel->SetControlString("NumZones", pLoc->GetWString("numZones"));

    // Bonuses
    m_pMapInfoPanel->SetControlString("NumBonusText", pLoc->GetWString("numBonus"));

    // Creation Date
    time_t creationDateTime;
    if (MomUtil::ISODateToTimeT(m_pMapData->m_Info.m_szCreationDate, &creationDateTime))
    {
        wchar_t date[32];
        wcsftime(date, 32, L"%b %d, %Y", localtime(&creationDateTime));
        m_pMapInfoPanel->SetControlString("CreationDateText", date);
    }

    // Release date
    time_t releaseDateTime;
    if (MomUtil::ISODateToTimeT(m_pMapData->m_szCreatedAt, &releaseDateTime))
    {
        wchar_t date[32];
        wcsftime(date, 32, L"%b %d, %Y", localtime(&releaseDateTime));
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

bool CDialogMapInfo::GetMapTimes(TimeType_t type)
{
    if (gpGlobals->curtime - UPDATE_INTERVAL < m_fRequestDelays[type])
        return false;

    bool bSent = false;

    if (type == TIMES_TOP10)
    {
        bSent = g_pAPIRequests->GetTop10MapTimes(m_pMapData->m_uID, UtlMakeDelegate(this, &CDialogMapInfo::OnTop10TimesCallback));
    }
    else if (type == TIMES_AROUND)
    {
        bSent = g_pAPIRequests->GetAroundTimes(m_pMapData->m_uID, UtlMakeDelegate(this, &CDialogMapInfo::OnAroundTimesCallback));
    }
    else if (type == TIMES_FRIENDS)
    {
        if (m_bUnauthorizedFriendsList)
        {
            m_pTimesList->SetEmptyListText(g_szTimesStatusStrings[STATUS_UNAUTHORIZED_FRIENDS_LIST]);
        }
        else
        {
            bSent = g_pAPIRequests->GetFriendsTimes(m_pMapData->m_uID, UtlMakeDelegate(this, &CDialogMapInfo::OnFriendsTimesCallback));
        }
    }

    if (bSent)
        ClearPlayerList();

    m_fRequestDelays[type] = gpGlobals->curtime + UPDATE_INTERVAL;
    return bSent;
}

void CDialogMapInfo::OnTop10TimesCallback(KeyValues *pKvResponse)
{
    ParseAPITimes(pKvResponse, TIMES_TOP10);
}

void CDialogMapInfo::OnAroundTimesCallback(KeyValues* pKvResponse)
{
    ParseAPITimes(pKvResponse, TIMES_AROUND);
}

void CDialogMapInfo::OnFriendsTimesCallback(KeyValues* pKvResponse)
{
    ParseAPITimes(pKvResponse, TIMES_FRIENDS);
}

void CDialogMapInfo::ParseAPITimes(KeyValues *pKvResponse, TimeType_t type)
{
    KeyValues *pData = pKvResponse->FindKey("data");
    KeyValues *pErr = pKvResponse->FindKey("error");
    if (pData)
    {
        KeyValues *pRanks = pData->FindKey("ranks");

        ClearPlayerList();

        if (pRanks && pData->GetInt("count") > 0)
        {
            // Iterate through each loaded run
            FOR_EACH_SUBKEY(pRanks, pRank)
            {
                KeyValuesAD kvEntry("Entry");

                KeyValues *pRun = pRank->FindKey("run");
                if (!pRun) // Should never happen but you never know...
                    continue;

                const float fTime = pRun->GetFloat("time");
                kvEntry->SetFloat("time_f", fTime);
                char buf[BUFSIZETIME];
                MomUtil::FormatTime(fTime, buf);
                kvEntry->SetString("time", buf);

                // Date
                char timeAgoStr[64];

                if (MomUtil::GetTimeAgoString(pRun->GetString("createdAt"), timeAgoStr, sizeof(timeAgoStr)))
                    kvEntry->SetString("date", timeAgoStr);
                else
                    kvEntry->SetString("date", pRun->GetString("createdAt"));

                KeyValues *kvUserObj = pRank->FindKey("user");
                if (kvUserObj)
                {
                    /*uint64 steamID = kvUserObj->GetUint64("steamID");
                    kvEntry->SetUint64("steamid", steamID);

                    int permissions = kvUserObj->GetInt("permissions");*/

                    // MOM_TODO: check if alias banned
                    kvEntry->SetString("name", kvUserObj->GetString("alias"));
                }

                // Rank
                kvEntry->SetInt("rank", pRank->GetInt("rank"));

                m_pTimesList->AddItem(kvEntry, 0, false, true);
            }

            //Update the control with this new info
            InvalidateLayout();
            m_pTimesList->InvalidateLayout();
            Repaint();
        }
        else
        {
            m_pTimesList->SetEmptyListText(g_szTimesStatusStrings[STATUS_NO_TIMES_RETURNED]);
        }
    }
    else if (pErr)
    {
        const int code = pKvResponse->GetInt("code");

        // Handle general errors
        m_pTimesList->SetEmptyListText(g_szTimesStatusStrings[STATUS_SERVER_ERROR]);

        // Handle specific error cases
        if (type == TIMES_AROUND)
        {
            if (code == 403) // User has not done a run yet
            {
                m_pTimesList->SetEmptyListText(g_szTimesStatusStrings[STATUS_NO_PB_SET]);
            }
        }
        else if (type == TIMES_FRIENDS)
        {
            if (code == 409) // The profile is private, we cannot read their friends
            {
                m_pTimesList->SetEmptyListText(g_szTimesStatusStrings[STATUS_UNAUTHORIZED_FRIENDS_LIST]);
                m_bUnauthorizedFriendsList = true;
            }
            else if (code == 418) // Short and stout~
            {
                m_pTimesList->SetEmptyListText(g_szTimesStatusStrings[STATUS_NO_FRIENDS]);
            }
        }
    }
}
