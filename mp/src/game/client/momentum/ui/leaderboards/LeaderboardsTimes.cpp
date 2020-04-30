#include "cbase.h"

#include "LeaderboardsTimes.h"

#include <vgui_controls/Label.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/SectionedListPanel.h>
#include "vgui_controls/ImageList.h"
#include "vgui_avatarimage.h"
#include "vgui/ISurface.h"

#include "ClientTimesDisplay.h"
#include "MessageboxPanel.h"
#include "LeaderboardsContextMenu.h"

#include "hud_comparisons.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "run/mom_replay_base.h"
#include "mom_map_cache.h"
#include "mom_api_requests.h"
#include "run/mom_replay_factory.h"
#include "filesystem.h"
#include "fmtstr.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define RANKSTRING "00000"               // A max of 99999 ranks (too generous)
#define DATESTRING "59 minutes ago" // Entire date string
#define TIMESTRING "00:00:00.000"        // Entire time string

#define UPDATE_INTERVAL 15.0f  // The amount of seconds minimum between online checks

class CUtlSortVectorTimeValue
{
public:
    bool Less(CMomReplayBase *lhs, CMomReplayBase *rhs, void *) const
    {
        return lhs->GetRunTime() < rhs->GetRunTime();
    }
};

CLeaderboardsTimes::CLeaderboardsTimes(CClientTimesDisplay* pParent) : BaseClass(pParent, "CLeaderboardsTimes")
{
    m_iSectionId = 0;
    m_pParentPanel = pParent;
    SetSize(10, 10);

    m_pFilterPanel = new EditablePanel(pParent, "FilterPanel");
    m_pFilterPanel->SetSize(10, 10);
    m_pFilterPanel->AddActionSignalTarget(this);
    m_pFilterPanel->LoadControlSettings("resource/ui/leaderboards/filter_panel.res");

    m_pLeaderboardReplayCMenu = new CLeaderboardsContextMenu(this);

    m_pOnlineTimesStatus = new Label(this, "OnlineTimesStatus", "#MOM_API_WaitingForResponse");
    m_pTop10Leaderboards = new SectionedListPanel(this, "Top10Leaderboards");
    m_pAroundLeaderboards = new SectionedListPanel(this, "AroundLeaderboards");
    m_pFriendsLeaderboards = new SectionedListPanel(this, "FriendsLeaderboards");
    m_pLocalLeaderboards = new SectionedListPanel(this, "LocalLeaderboards");

    m_pGlobalLeaderboardsButton = new Button(this, "GlobalLeaderboardsButton", "#MOM_Leaderboards_Global", this, "ShowGlobal");
    m_pGlobalTop10Button = new Button(this, "GlobalTop10Button", "#MOM_Leaderboards_Top10", this, "GlobalTypeTop10");
    m_pGlobalAroundButton = new Button(this, "GlobalAroundButton", "#MOM_Leaderboards_Around", this, "GlobalTypeAround");
    m_pFriendsLeaderboardsButton = new Button(this, "FriendsLeaderboardsButton", "#MOM_Leaderboards_Friends", this, "ShowFriends");
    m_pLocalLeaderboardsButton = new Button(this, "LocalLeaderboardsButton", "#MOM_Leaderboards_Local", this, "ShowLocal");
    m_pRunFilterButton = new ToggleButton(this, "FilterButton", "#MOM_Leaderboards_Filter");
    m_pRunFilterButton->SetCommand("ShowFilter");
    m_pRunFilterButton->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/leaderboards/times.res");

    // Get rid of the scrollbars for the panels
    m_pTop10Leaderboards->SetVerticalScrollbar(false);
    m_pAroundLeaderboards->SetVerticalScrollbar(false);
    m_pLocalLeaderboards->SetVerticalScrollbar(false);
    m_pFriendsLeaderboards->SetVerticalScrollbar(false);

    m_flTimesLastUpdate[TIMES_TOP10] = m_flTimesLastUpdate[TIMES_AROUND] = m_flTimesLastUpdate[TIMES_FRIENDS] = 0.0f;
    for (int i = 1; i < 4; i++)
        m_eTimesStatus[i] = STATUS_TIMES_LOADING;

    m_iFlaggedRuns = RUNFLAG_NONE;

    m_pCurrentLeaderboards = m_pLocalLeaderboards;

    SetDefLessFunc(m_mapAvatarsToImageList);
    SetDefLessFunc(m_mapReplayDownloads);

    pPlayerBorder = nullptr;
    m_pImageList = nullptr;
    LevelInit();
}

CLeaderboardsTimes::~CLeaderboardsTimes()
{
    if (m_pImageList)
        delete m_pImageList;

    m_pCurrentLeaderboards = nullptr;
    m_mapAvatarsToImageList.RemoveAll();
}

void CLeaderboardsTimes::LevelInit()
{
    m_bTimesLoading[TIMES_TOP10] = m_bTimesLoading[TIMES_AROUND] = m_bTimesLoading[TIMES_FRIENDS] = false;
    m_bTimesNeedUpdate[TIMES_LOCAL] = m_bTimesNeedUpdate[TIMES_TOP10] = m_bTimesNeedUpdate[TIMES_AROUND] = m_bTimesNeedUpdate[TIMES_FRIENDS] = true;

    if (m_pImageList)
        delete m_pImageList;

    // HACKHACK: delete is set to false because deleting a scheme image is a no-no.
    // While we do know that the image list will only hold scheme and avatar images, we cannot delete
    // either one. We do not have to delete the scheme images, as they are cached, and the avatar images
    // are also cached, but indefinitely. There's a memory leak with avatar images, since every image just
    // keeps creating Texture IDs and never destroying them, so if you download a lot of *different* avatars 
    // (play a lot of maps and look at the leaderboards for them), you could start to see the perf impact of it.
    // I'll leave it as a HACKHACK for now because this is ugly and that memory needs freed after a while, but may
    // be unnoticeable for most people... we'll see how big the memory leak impact really is.
    m_pImageList = new ImageList(false, false);
    m_mapAvatarsToImageList.RemoveAll();

    SetupDefaultIcons();
    Reset(false);
    m_vLocalTimes.PurgeAndDeleteElements();
    m_vOnlineTimes.PurgeAndDeleteElements();
    m_vAroundTimes.PurgeAndDeleteElements();
    m_vFriendsTimes.PurgeAndDeleteElements();
}

void CLeaderboardsTimes::Reset(bool bFullReset)
{
    m_pLocalLeaderboards->DeleteAllItems();
    m_pTop10Leaderboards->DeleteAllItems();
    m_pAroundLeaderboards->DeleteAllItems();
    m_pFriendsLeaderboards->DeleteAllItems();

    if (bFullReset)
    {
        m_pLocalLeaderboards->RemoveAllSections();
        m_pTop10Leaderboards->RemoveAllSections();
        m_pAroundLeaderboards->RemoveAllSections();
        m_pFriendsLeaderboards->RemoveAllSections();
        InitLeaderboardSections();
    }
}

void CLeaderboardsTimes::InitLeaderboardSections()
{
    if (m_pLocalLeaderboards)
    {
        m_pLocalLeaderboards->AddSection(m_iSectionId, "", StaticLocalTimeSortFunc);
        m_pLocalLeaderboards->SetSectionAlwaysVisible(m_iSectionId);
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "time", "#MOM_Time", 0, GetScaledVal(m_aiColumnWidths[2]));
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "date", "#MOM_Achieved", 0, GetScaledVal(m_aiColumnWidths[0]));
        //m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "flags_input", "", SectionedListPanel::COLUMN_IMAGE, 16);
        //m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "flags_movement", "", SectionedListPanel::COLUMN_IMAGE, 16);
        //m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "flags_bonus", "", SectionedListPanel::COLUMN_IMAGE, 16);
    }

    SectionedListPanel *panels[] = { m_pTop10Leaderboards, m_pAroundLeaderboards, m_pFriendsLeaderboards };
    for (SectionedListPanel *panel : panels)
    {
        if (!panel)
            continue;

        // We use online timer sort func as it's the same type of data
        panel->AddSection(m_iSectionId, "", StaticOnlineTimeSortFunc);
        panel->SetSectionAlwaysVisible(m_iSectionId);
        panel->SetImageList(m_pImageList, false);
        panel->AddColumnToSection(m_iSectionId, "rank", "#MOM_Rank", SectionedListPanel::COLUMN_CENTER,
                                  GetScaledVal(m_aiColumnWidths[1]));
        panel->AddColumnToSection(m_iSectionId, "avatar", "",
                                  SectionedListPanel::COLUMN_IMAGE,
                                  DEFAULT_AVATAR_SIZE + 4);
        panel->AddColumnToSection(m_iSectionId, "icon_tm", "", SectionedListPanel::COLUMN_IMAGE, 16);
        panel->AddColumnToSection(m_iSectionId, "icon_vip", "", SectionedListPanel::COLUMN_IMAGE, 16);
        panel->AddColumnToSection(m_iSectionId, "icon_friend", "", SectionedListPanel::COLUMN_IMAGE, 16);
        panel->AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name",
                                  0, 160);
        panel->AddColumnToSection(m_iSectionId, "time_f", "#MOM_Time",
                                  0, GetScaledVal(m_aiColumnWidths[2]));
        panel->AddColumnToSection(m_iSectionId, "date", "#MOM_Achieved", 0, GetScaledVal(m_aiColumnWidths[0]));
        // Scroll only icon
        panel->AddColumnToSection(m_iSectionId, "flags_input", "", SectionedListPanel::COLUMN_IMAGE,
                                  16);
        // HSW/SW/BW/WOnly Icons
        panel->AddColumnToSection(m_iSectionId, "flags_movement", "", SectionedListPanel::COLUMN_IMAGE,
                                  16);
        // Bonus Icon
        panel->AddColumnToSection(m_iSectionId, "flags_bonus", "", SectionedListPanel::COLUMN_IMAGE,
                                  16);
    }
}

void CLeaderboardsTimes::OnRunPosted(bool bPosted)
{
    m_bTimesNeedUpdate[TIMES_TOP10] = m_bTimesNeedUpdate[TIMES_AROUND] = m_bTimesNeedUpdate[TIMES_FRIENDS] = bPosted;
}

void CLeaderboardsTimes::OnRunSaved()
{
    // this updates the local times file, needing a reload of it
    m_bTimesNeedUpdate[TIMES_LOCAL] = true;
}

void CLeaderboardsTimes::OnPanelShow(bool bShow)
{
    // Catch the case where we call ShowPanel before ApplySchemeSettings, eg when
    // going from windowed <-> fullscreen
    if (!m_pImageList && bShow)
    {
        InvalidateLayout(true, true);
    }

    if (m_pLeaderboardReplayCMenu)
    {
        // Close the menu
        ResetLeaderboardContextMenu();
    }
}

inline IImage *GetDefaultImage(const char *pName, int sizeSq = 16)
{
    const auto pImg = scheme()->GetImage(pName, false);
    pImg->SetSize(sizeSq, sizeSq);
    return pImg;
}

void CLeaderboardsTimes::SetupDefaultIcons()
{
    m_pImageList->SetImageAtIndex(ICON_VIP, GetDefaultImage("leaderboards_icon_vip"));
    m_pImageList->SetImageAtIndex(ICON_TEAMMEMBER, GetDefaultImage("leaderboards_icon_mom"));
    m_pImageList->SetImageAtIndex(ICON_FRIEND, GetDefaultImage("leaderboards_icon_friends"));
    m_pImageList->SetImageAtIndex(ICON_DEFAULT_AVATAR, GetDefaultImage("default_steam", 32));
}

int CLeaderboardsTimes::TryAddAvatar(const uint64 &steamid, CUtlMap<uint64, int> *pIDtoIndxMap, ImageList *pImageList)
{
    // Update their avatar
    if (pIDtoIndxMap && pImageList)
    {
        // See if we already have that avatar in our list
        const unsigned short mapIndex = pIDtoIndxMap->Find(steamid);
        int iImageIndex;
        if (!pIDtoIndxMap->IsValidIndex(mapIndex))
        {
            CAvatarImage *pImage = new CAvatarImage();
            // 64 is enough up to full HD resolutions.
            pImage->SetAvatarSteamID(CSteamID(steamid), k_EAvatarSize64x64);

            pImage->SetDrawFriend(false);
            pImage->SetAvatarSize(32, 32);
            pImage->SetDefaultImage(scheme()->GetImage("default_steam", false));
            iImageIndex = pImageList->AddImage(pImage);
            pIDtoIndxMap->Insert(steamid, iImageIndex);
        }
        else
        {
            iImageIndex = pIDtoIndxMap->Element(mapIndex);
        }
        return iImageIndex;
    }
    return -1;
}

void CLeaderboardsTimes::FillLeaderboards(bool bFullUpdate)
{
    m_iSectionId = 0;

    // Times
    KeyValuesAD kvPlayerData("playdata");
    GetPlayerTimes(kvPlayerData, bFullUpdate);

    if (m_pCurrentLeaderboards && m_pTop10Leaderboards && m_pAroundLeaderboards
        && m_pLocalLeaderboards && m_pFriendsLeaderboards)
    {
        SetVisible(false);

        if (m_pCurrentLeaderboards == m_pLocalLeaderboards)
        {
            KeyValues *kvLeaderboards = kvPlayerData->FindKey("leaderboards");
            if (kvLeaderboards)
            {
                KeyValues *kvLocalTimes = kvLeaderboards->FindKey("local");
                if (kvLocalTimes && !kvLocalTimes->IsEmpty())
                {
                    FOR_EACH_SUBKEY(kvLocalTimes, kvLocalTime)
                    {
                        int itemID = FindItemIDForLocalTime(kvLocalTime);
                        if (itemID == -1)
                            m_pLocalLeaderboards->AddItem(m_iSectionId, kvLocalTime);
                        else
                            m_pLocalLeaderboards->ModifyItem(itemID, m_iSectionId, kvLocalTime);
                    }

                    SetPlaceColors(m_pLocalLeaderboards, TIMES_LOCAL);
                }
            }
        }
        // Online works slightly different, we use the vector content, not the ones from m_kvPlayerData
        else if (m_pCurrentLeaderboards == m_pTop10Leaderboards)
        {
            OnlineTimesVectorToLeaderboards(TIMES_TOP10);
        }
        else if (m_pCurrentLeaderboards == m_pAroundLeaderboards)
        {
            OnlineTimesVectorToLeaderboards(TIMES_AROUND);
        }
        else if (m_pCurrentLeaderboards == m_pFriendsLeaderboards)
        {
            OnlineTimesVectorToLeaderboards(TIMES_FRIENDS);
        }

        SetVisible(true);
    }
}

void CLeaderboardsTimes::SetPlaceColors(SectionedListPanel* panel, TimeType_t type) const
{
    int itemCount = panel->GetItemCount();
    if (itemCount == 0)
        return;

    if (type == TIMES_LOCAL || type == TIMES_TOP10)
    {
        panel->SetItemBgColor(panel->GetItemIDFromRow(0), m_cFirstPlace);
        if (itemCount > 1)
        {
            panel->SetItemBgColor(panel->GetItemIDFromRow(1), m_cSecondPlace);

            if (itemCount > 2)
                panel->SetItemBgColor(panel->GetItemIDFromRow(2), m_cThirdPlace);
        }
    }
    else
    {
        Color colors[3] = { m_cFirstPlace, m_cSecondPlace, m_cThirdPlace };
        for (int row = 0; row < 3 && row < itemCount; row++)
        {
            int itemID = panel->GetItemIDFromRow(row);
            KeyValues *pItem = panel->GetItemData(itemID);
            int rank = pItem->GetInt("rank");
            if (rank < 4)
                panel->SetItemBgColor(itemID, colors[rank - 1]);
            if (rank == 3)
                break;
        }
    }
}

void CLeaderboardsTimes::LoadLocalTimes(KeyValues* kv)
{
    if (m_bTimesNeedUpdate[TIMES_LOCAL])
    {
        // Clear the local times for a refresh
        m_vLocalTimes.PurgeAndDeleteElements();

        char path[MAX_PATH];
        Q_snprintf(path, MAX_PATH, "%s/%s-*%s", RECORDING_PATH, g_pGameRules->MapName(), EXT_RECORDING_FILE);
        V_FixSlashes(path);

        FileFindHandle_t found;
        const char *pFoundFile = filesystem->FindFirstEx(path, "MOD", &found);
        while (pFoundFile)
        {
            // NOTE: THIS NEEDS TO BE MANUALLY CLEANED UP!
            char pReplayPath[MAX_PATH];
            V_ComposeFileName(RECORDING_PATH, pFoundFile, pReplayPath, MAX_PATH);

            CMomReplayBase *pBase = g_ReplayFactory.LoadReplayFile(pReplayPath, false);
            Assert(pBase != nullptr);

            if (pBase)
                m_vLocalTimes.InsertNoSort(pBase);

            pFoundFile = filesystem->FindNext(found);
        }

        filesystem->FindClose(found);

        if (!m_vLocalTimes.IsEmpty())
        {
            m_vLocalTimes.RedoSort();
            m_bTimesNeedUpdate[TIMES_LOCAL] = false;
        }
    }

    // Convert
    if (!m_vLocalTimes.IsEmpty())
        ConvertLocalTimes(kv);
}

void CLeaderboardsTimes::LoadOnlineTimes(TimeType_t type)
{
    if (type == TIMES_FRIENDS && !m_bUnauthorizedFriendlist)
        return;

    if (!m_bTimesLoading[type] && m_bTimesNeedUpdate[type])
    {
        if (g_pMapCache->GetCurrentMapID())
        {
            bool bCalled = false;
            switch (type)
            {
            case TIMES_TOP10:
                bCalled = g_pAPIRequests->GetTop10MapTimes(g_pMapCache->GetCurrentMapID(), UtlMakeDelegate(this, &CLeaderboardsTimes::GetTop10TimesCallback));
                break;
            case TIMES_FRIENDS:
                bCalled = g_pAPIRequests->GetFriendsTimes(g_pMapCache->GetCurrentMapID(), UtlMakeDelegate(this, &CLeaderboardsTimes::GetFriendsTimesCallback));
                break;
            case TIMES_AROUND:
                bCalled = g_pAPIRequests->GetAroundTimes(g_pMapCache->GetCurrentMapID(), UtlMakeDelegate(this, &CLeaderboardsTimes::GetAroundTimesCallback));
            default:
                break;
                
            }

            if (bCalled)
            {
                m_bTimesLoading[type] = true;
                m_bTimesNeedUpdate[type] = false;
                m_flTimesLastUpdate[type] = gpGlobals->curtime;
                m_eTimesStatus[type] = STATUS_TIMES_LOADING;
            }
        }
    }
}

void CLeaderboardsTimes::ConvertLocalTimes(KeyValues* kvInto)
{
    FOR_EACH_VEC(m_vLocalTimes, i)
    {
        CMomReplayBase *t = m_vLocalTimes[i];

        KeyValues *kvLocalTimeFormatted = new KeyValues("localtime");
        char filename[MAX_PATH];

        Q_snprintf(filename, MAX_PATH, "%s-%s%s", t->GetMapName(), t->GetRunHash(), EXT_RECORDING_FILE);
        kvLocalTimeFormatted->SetString("fileName", filename);

        kvLocalTimeFormatted->SetFloat("time_f", t->GetRunTime()); // Used for static compare
        kvLocalTimeFormatted->SetInt("date_t", t->GetRunDate());   // Used for finding

        char timeString[BUFSIZETIME];
        MomUtil::FormatTime(t->GetRunTime(), timeString);
        kvLocalTimeFormatted->SetString("time", timeString); // Used for display

        char dateString[64];
        time_t date = t->GetRunDate();
        if (MomUtil::GetTimeAgoString(&date, dateString, sizeof(dateString)))
        {
            kvLocalTimeFormatted->SetString("date", dateString);
        }
        else
            kvLocalTimeFormatted->SetInt("date", date);

        // MOM_TODO: Convert the run flags to pictures

        kvInto->AddSubKey(kvLocalTimeFormatted);
    }
}

void CLeaderboardsTimes::OnlineTimesVectorToLeaderboards(TimeType_t type)
{
    CUtlVector<TimeOnline *> *pVector;
    SectionedListPanel *pList;
    switch (type)
    {
    case TIMES_FRIENDS:
        pVector = &m_vFriendsTimes;
        pList = m_pFriendsLeaderboards;
        break;
    case TIMES_TOP10:
        pVector = &m_vOnlineTimes;
        pList = m_pTop10Leaderboards;
        break;
    case TIMES_AROUND:
        pVector = &m_vAroundTimes;
        pList = m_pAroundLeaderboards;
        break;
    default:
        return;
    }
    if (pVector && pVector->Count() > 0 && pList)
    {
        // To clear up any count discrepancies, just remove all items
        if (pList->GetItemCount() != pVector->Count())
            pList->RemoveAll();

        FOR_EACH_VEC(*pVector, entry)
        {
            TimeOnline *runEntry = pVector->Element(entry);

            int itemID = FindItemIDForOnlineTime(runEntry->id, type);

            runEntry->m_kv->SetInt("icon_tm", runEntry->momember ? ICON_TEAMMEMBER : -1);
            runEntry->m_kv->SetInt("icon_vip", runEntry->vip ? ICON_VIP : -1);
            runEntry->m_kv->SetInt("icon_friend", runEntry->is_friend ? ICON_FRIEND : -1);

            if (itemID == -1)
            {
                itemID = pList->AddItem(m_iSectionId, runEntry->m_kv);
            }
            else
            {
                pList->ModifyItem(itemID, m_iSectionId, runEntry->m_kv);
            }

            if (runEntry->steamid == SteamUser()->GetSteamID().ConvertToUint64() && pPlayerBorder)
            {
                pList->SetItemBorder(itemID, pPlayerBorder);
            }
            else
            {
                pList->SetItemBorder(itemID, nullptr);
            }
        }

        SetPlaceColors(pList, type);
    }
    if (m_pOnlineTimesStatus)
    {
        if (m_eTimesStatus[type] == STATUS_TIMES_LOADED || pVector->Count())
            m_pOnlineTimesStatus->SetVisible(false);
        else
        {
            m_pOnlineTimesStatus->SetText(g_szTimesStatusStrings[m_eTimesStatus[type]]);
            m_pOnlineTimesStatus->SetVisible(true);
            m_pOnlineTimesStatus->InvalidateLayout(true);
        }
    }
}

bool CLeaderboardsTimes::GetPlayerTimes(KeyValues* outPlayerInfo, bool fullUpdate)
{
    if (!outPlayerInfo)
        return false;

    KeyValues *pLeaderboards = new KeyValues("leaderboards");

    // Fill local times:
    KeyValues *pLocal = new KeyValues("local");
    LoadLocalTimes(pLocal);
    pLeaderboards->AddSubKey(pLocal);

    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_UNKNOWN))
    {
        for (int i = 1; i < TIMES_COUNT; i++) // Skip over local
        {
            // Only if we need to calculate it
            if (!m_bTimesNeedUpdate[i])
            {
                float lastUp = gpGlobals->curtime - m_flTimesLastUpdate[i];
                m_bTimesNeedUpdate[i] = fullUpdate && lastUp >= UPDATE_INTERVAL;
            }
        }

        // Fill online times only if needed
        LoadOnlineTimes(TIMES_TOP10);
        LoadOnlineTimes(TIMES_AROUND);
        LoadOnlineTimes(TIMES_FRIENDS);
    }

    outPlayerInfo->AddSubKey(pLeaderboards);
    return true;
}

void CLeaderboardsTimes::ResetLeaderboardContextMenu()
{
    m_pLeaderboardReplayCMenu->SetVisible(false);
    m_pLeaderboardReplayCMenu->DeleteAllItems();
}

bool CLeaderboardsTimes::StaticLocalTimeSortFunc(SectionedListPanel* list, int itemID1, int itemID2)
{
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);

    float t1 = it1->GetFloat("time_f");
    float t2 = it2->GetFloat("time_f");
    // Ascending order
    if (t1 < t2)
        return true; // this time is faster, place it up higher
    if (t1 > t2)
        return false;

    // If the same, use IDs
    return itemID1 < itemID2;
}

bool CLeaderboardsTimes::StaticOnlineTimeSortFunc(SectionedListPanel* list, int itemID1, int itemID2)
{
    // Uses rank insetad of time (Momentum page will handle players with same times)
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);
    int t1 = it1->GetFloat("rank");
    int t2 = it2->GetFloat("rank");
    // Ascending order
    if (t1 < t2)
        return true; // this time is faster, place it up higher
    if (t1 > t2)
        return false;
    // We will *almost* never need this, but just in case...

    float s1 = it1->GetFloat("time");
    float s2 = it2->GetFloat("time");
    // Ascending order
    if (s1 < s2)
        return true; // this time is faster, place it up higher
    if (s1 > s2)
        return false;
    return itemID1 < itemID2;
}

int CLeaderboardsTimes::FindItemIDForLocalTime(KeyValues* kvRef)
{
    for (int i = 0; i <= m_pLocalLeaderboards->GetHighestItemID(); i++)
    {
        if (m_pLocalLeaderboards->IsItemIDValid(i))
        {
            KeyValues *kv = m_pLocalLeaderboards->GetItemData(i);
            if (kv && (kv->GetInt("date_t") == kvRef->GetInt("date_t")))
            {
                return i;
            }
        }
    }
    return -1;
}

int CLeaderboardsTimes::FindItemIDForOnlineTime(uint64 runID, TimeType_t type)
{
    SectionedListPanel *pLeaderboard;
    switch (type)
    {
    case TIMES_FRIENDS:
        pLeaderboard = m_pFriendsLeaderboards;
        break;
    case TIMES_TOP10:
        pLeaderboard = m_pTop10Leaderboards;
        break;
    case TIMES_AROUND:
        pLeaderboard = m_pAroundLeaderboards;
        break;
    default:
        return -1;
    }
    for (int i = 0; i <= pLeaderboard->GetHighestItemID(); i++)
    {
        if (pLeaderboard->IsItemIDValid(i))
        {
            KeyValues *kv = pLeaderboard->GetItemData(i);
            if (kv && (kv->GetUint64("id") == runID))
            {
                return i;
            }
        }
    }
    return -1;
}

void CLeaderboardsTimes::GetTop10TimesCallback(KeyValues* pKv)
{
    ParseTimesCallback(pKv, TIMES_TOP10);
}

void CLeaderboardsTimes::GetFriendsTimesCallback(KeyValues* pKv)
{
    ParseTimesCallback(pKv, TIMES_FRIENDS);
}

void CLeaderboardsTimes::GetAroundTimesCallback(KeyValues* pKv)
{
    ParseTimesCallback(pKv, TIMES_AROUND);
}

void CLeaderboardsTimes::ParseTimesCallback(KeyValues* pKv, TimeType_t type)
{
    m_bTimesLoading[type] = false;
    CHECK_STEAM_API(SteamFriends());

    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        KeyValues *pRanks = pData->FindKey("ranks");

        if (pRanks && pData->GetInt("count") > 0)
        {
            CUtlVector<TimeOnline*> *vecs[] = { nullptr, &m_vOnlineTimes, &m_vFriendsTimes, &m_vAroundTimes };
            // By now we're pretty sure everything will be ok, so we can do this
            vecs[type]->PurgeAndDeleteElements();

            // Iterate through each loaded run
            FOR_EACH_SUBKEY(pRanks, pRank)
            {
                KeyValues *kvEntry = new KeyValues("Entry");

                KeyValues *pRun = pRank->FindKey("run");
                if (!pRun) // Should never happen but you never know...
                    continue;

                // Time
                kvEntry->SetFloat("time", pRun->GetFloat("time"));
                // Format the time as well
                char timeString[BUFSIZETIME];
                MomUtil::FormatTime(pRun->GetFloat("time"), timeString);
                kvEntry->SetString("time_f", timeString);

                // Tickrate
                kvEntry->SetFloat("rate", pRun->GetFloat("tickRate"));

                // Date
                char timeAgoStr[64];
                if (MomUtil::GetTimeAgoString(pRun->GetString("createdAt"), timeAgoStr, sizeof(timeAgoStr)))
                    kvEntry->SetString("date", timeAgoStr);
                else
                    kvEntry->SetString("date", pRun->GetString("createdAt"));

                // ID
                kvEntry->SetUint64("id", pRun->GetUint64("id"));

                // File
                kvEntry->SetString("file", pRun->GetString("file"));

                // Hash
                kvEntry->SetString("hash", pRun->GetString("hash"));

                KeyValues *kvUserObj = pRank->FindKey("user");
                if (kvUserObj)
                {
                    uint64 steamID = kvUserObj->GetUint64("steamID");
                    kvEntry->SetUint64("steamid", steamID);

                    const auto roles = kvUserObj->GetInt("roles");
                    const auto bans = kvUserObj->GetInt("bans");

                    // Is part of the momentum team?
                    // MOM_TODO: Make this the actual permission
                    kvEntry->SetBool("tm", roles & (USER_ADMIN | USER_MODERATOR));

                    // Is vip?
                    // MOM_TODO: Make this the actual permission
                    kvEntry->SetBool("vip", pRun->GetBool("vip"));

                    // MOM_TODO Set an icon for the map creator?

                    kvEntry->SetString("personaname", kvUserObj->GetString("alias"));

                    const auto bAvatarBanned = bans & USER_BANNED_AVATAR;
                    kvEntry->SetInt("avatar", bAvatarBanned ? ICON_DEFAULT_AVATAR : TryAddAvatar(steamID, &m_mapAvatarsToImageList, m_pImageList));

                    kvEntry->SetBool("is_friend", SteamFriends()->HasFriend(CSteamID(steamID), k_EFriendFlagImmediate));
                }

                // Rank
                kvEntry->SetInt("rank", pRank->GetInt("rank"));

                // Add this baby to the online times vector
                vecs[type]->AddToTail(new TimeOnline(kvEntry));
            }

            m_eTimesStatus[type] = STATUS_TIMES_LOADED;
        }
        else
        {
            m_eTimesStatus[type] = STATUS_NO_TIMES_RETURNED;
        }
    }
    else if (pErr)
    {
        int code = pKv->GetInt("code");

        // Handle general errors
        m_eTimesStatus[type] = STATUS_SERVER_ERROR;

        // Handle specific error cases
        if (type == TIMES_AROUND)
        {
            if (code == 403) // User has not done a run yet
            {
                m_eTimesStatus[type] = STATUS_NO_PB_SET;
            }
        }
        else if (type == TIMES_FRIENDS)
        {
            if (code == 409) // The profile is private, we cannot read their friends
            {
                m_eTimesStatus[type] = STATUS_UNAUTHORIZED_FRIENDS_LIST;
                m_bUnauthorizedFriendlist = true;
            }
            else if (code == 418) // Short and stout~
            {
                m_eTimesStatus[type] = STATUS_NO_FRIENDS;
            }
        }
    }

    if (pData || pErr)
        FillLeaderboards(false);
}

void CLeaderboardsTimes::OnReplayDownloadStart(KeyValues* pKvHeaders)
{
    // MOM_TODO: Make a progress bar here
}

void CLeaderboardsTimes::OnReplayDownloadProgress(KeyValues* pKvProgress)
{
    uint16 fileIndx = m_mapReplayDownloads.Find(pKvProgress->GetUint64("request"));
    if (fileIndx != m_mapReplayDownloads.InvalidIndex())
    {
        // DevLog("Progress: %0.2f!\n", pKvProgress->GetFloat("percent"));

        // MOM_TODO: update the progress bar here, but do not use the percent! Use the offset and size of the chunk!
        // Percent seems to be cached, i.e. sends a lot of "100%" if Steam downloaded the file and is sending the chunks from cache to us
    }
}

void CLeaderboardsTimes::OnReplayDownloadEnd(KeyValues* pKvEnd)
{
    uint16 fileIndx = m_mapReplayDownloads.Find(pKvEnd->GetUint64("request"));
    if (fileIndx != m_mapReplayDownloads.InvalidIndex())
    {
        if (pKvEnd->GetBool("error"))
        {
            // MOM_TODO: Show some sort of error icon on the progress bar
            Warning("Could not download replay! Error code: %i\n", pKvEnd->GetInt("code"));
        }
        else
        {
            // MOM_TODO: show success on the progress bar here
            DevLog("Successfully downloaded the replay with ID: %lld\n", m_mapReplayDownloads[fileIndx]);

            // Play it
            CFmtStr command("mom_replay_play %s/%s-%lld%s\n", RECORDING_ONLINE_PATH, m_pParentPanel->MapName(), m_mapReplayDownloads[fileIndx], EXT_RECORDING_FILE);
            engine->ClientCmd(command.Get());
            m_pParentPanel->Close();
        }

        m_mapReplayDownloads.RemoveAt(fileIndx);
    }
}


void CLeaderboardsTimes::OnCommand(const char* pCommand)
{
    BaseClass::OnCommand(pCommand);

    // MOM_TODO: Implement run tags
// Leaderboards type
    bool isTop10 = FStrEq(pCommand, "GlobalTypeTop10");
    bool isAround = FStrEq(pCommand, "GlobalTypeAround");
    bool isLocal = FStrEq(pCommand, "ShowLocal");
    bool isFriends = FStrEq(pCommand, "ShowFriends");
    bool isGlobal = FStrEq(pCommand, "ShowGlobal");
    bool isFilter = FStrEq(pCommand, "ShowFilter");
    bool isReset = FStrEq(pCommand, "ResetFlags");
    bool isFlagScrollOnly = FStrEq(pCommand, "ToggleScrollOnly");
    bool isFlagWOnly = FStrEq(pCommand, "ToggleWOnly");
    bool isFlagHSW = FStrEq(pCommand, "ToggleHSW");
    bool isFlagSideways = FStrEq(pCommand, "ToggleSideways");
    bool isFlagBackwards = FStrEq(pCommand, "ToggleBackwards");
    bool isFlagBonus = FStrEq(pCommand, "ToggleBonus");
    if (isTop10 || isAround)
    {
        m_pFriendsLeaderboardsButton->SetEnabled(true);

        m_pGlobalTop10Button->SetEnabled(!isTop10);
        m_pGlobalAroundButton->SetEnabled(isTop10);

        // Show the right type of leaderboards
        m_pCurrentLeaderboards->SetVisible(false);
        m_pCurrentLeaderboards = isTop10 ? m_pTop10Leaderboards : m_pAroundLeaderboards;
        m_pCurrentLeaderboards->SetVisible(true);

        FillLeaderboards(false);
    }
    else if (isLocal || isFriends || isGlobal)
    {
        // Show the right type of leaderboards
        m_pCurrentLeaderboards->SetVisible(false);
        m_pCurrentLeaderboards =
            isGlobal ? m_pTop10Leaderboards : (isLocal ? m_pLocalLeaderboards : m_pFriendsLeaderboards);
        m_pCurrentLeaderboards->SetVisible(true);

        m_pGlobalLeaderboardsButton->SetEnabled(!isGlobal && !isFriends);
        m_pGlobalAroundButton->SetEnabled(true);
        m_pGlobalTop10Button->SetEnabled(!isGlobal);
        m_pFriendsLeaderboardsButton->SetEnabled(!isFriends);
        m_pLocalLeaderboardsButton->SetEnabled(!isLocal);

        m_pGlobalTop10Button->SetVisible(isGlobal || isFriends);
        m_pGlobalAroundButton->SetVisible(isGlobal || isFriends);
        m_pFriendsLeaderboardsButton->SetVisible(isGlobal || isFriends);

        if (isLocal)
            m_pOnlineTimesStatus->SetVisible(false);

        else if (isFriends)
        {
            m_pGlobalAroundButton->SetEnabled(true);
            m_pGlobalTop10Button->SetEnabled(true);
        }
        FillLeaderboards(false);
    }
    else if (isFilter)
    {
        m_pFilterPanel->SetVisible(m_pRunFilterButton->IsSelected());
    }
    else if (isReset)
    {
        m_iFlaggedRuns = RUNFLAG_NONE;
        for (int i = 0; i < m_pFilterPanel->GetChildCount(); i++)
        {
            ToggleButton *pChild = dynamic_cast<ToggleButton*>(m_pFilterPanel->GetChild(i));
            if (pChild)
            {
                pChild->ForceDepressed(false);
                pChild->SetSelected(false);
            }
        }
    }
    else if (isFlagScrollOnly)
    {
        m_iFlaggedRuns ^= RUNFLAG_SCROLL;
    }
    else if (isFlagWOnly)
    {
        m_iFlaggedRuns ^= RUNFLAG_W_ONLY;
    }
    else if (isFlagHSW)
    {
        m_iFlaggedRuns ^= RUNFLAG_HSW;
    }
    else if (isFlagSideways)
    {
        m_iFlaggedRuns ^= RUNFLAG_SW;
    }
    else if (isFlagBackwards)
    {
        m_iFlaggedRuns ^= RUNFLAG_BW;
    }
    else if (isFlagBonus)
    {
        m_iFlaggedRuns ^= RUNFLAG_BONUS;
    }
    else
    {
        DevLog("Caught an unhandled command: %s\n", pCommand);
    }
}

void CLeaderboardsTimes::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cFirstPlace = pScheme->GetColor("FirstPlace", Color(240, 210, 147, 50));
    m_cSecondPlace = pScheme->GetColor("SecondPlace", Color(175, 175, 175, 50));
    m_cThirdPlace = pScheme->GetColor("ThirdPlace", Color(205, 127, 50, 50));

    pPlayerBorder = pScheme->GetBorder("LeaderboardsPlayerBorder");

    const char *columnNames[] = { DATESTRING, RANKSTRING, TIMESTRING };

    HFont font = pScheme->GetFont("Default", true);
    for (int i = 0; i < 3; i++)
    {
        const char *currName = columnNames[i];
        const int len = Q_strlen(currName);
        int pixels = 0;
        for (int currentChar = 0; currentChar < len; currentChar++)
        {
            pixels += surface()->GetCharacterWidth(font, currName[currentChar]);
        }
        m_aiColumnWidths[i] = pixels;
    }
}

void CLeaderboardsTimes::OnConfirmDeleteReplay(int itemID, const char* file)
{
    if (file)
    {
        g_pFullFileSystem->RemoveFile(file, "MOD");
        m_bTimesNeedUpdate[TIMES_LOCAL] = true;
        if (m_pLocalLeaderboards->RemoveItem(itemID))
        {
            g_pMOMRunCompare->LoadComparisons();
            FillLeaderboards(false);
        }
    }
}

void CLeaderboardsTimes::OnContextDeleteReplay(int itemID, const char* runName)
{
    if (runName)
    {
        char file[MAX_PATH];
        V_ComposeFileName(RECORDING_PATH, runName, file, MAX_PATH);

        KeyValues *pCommand = new KeyValues("ConfirmDeleteReplay", "file", file);
        pCommand->SetInt("itemID", itemID);
        g_pMessageBox->CreateConfirmationBox(this, "#MOM_Leaderboards_DeleteReplay",
                                               "#MOM_MB_DeleteRunConfirmation", pCommand,
                                               nullptr, "#MOM_Leaderboards_DeleteReplay");
    }
}

void CLeaderboardsTimes::OnContextVisitProfile(uint64 profile)
{
    if (profile != 0 && SteamFriends())
    {
        SteamFriends()->ActivateGameOverlayToUser("steamid", CSteamID(profile));
        m_pParentPanel->Close();
    }
}

void CLeaderboardsTimes::OnContextWatchOnlineReplay(KeyValues* data)
{
    DevLog("Attempting to download UGC...\n");

    uint64 replayID = data->GetUint64("id");
    const char *pFileURL = data->GetString("file", nullptr);
    const char *pMapName = m_pParentPanel->MapName();
    const char *pReplayHash = data->GetString("hash");
    CFmtStr fileNameLocal("%s-%s%s", pMapName, pReplayHash, EXT_RECORDING_FILE);
    CFmtStr filePathLocal("%s/%s", RECORDING_PATH, fileNameLocal.Get());
    CFmtStr fileNameOnline("%s-%lld%s", pMapName, replayID, EXT_RECORDING_FILE);
    CFmtStr filePathOnline("%s/%s/%s", RECORDING_PATH, RECORDING_ONLINE_PATH, fileNameOnline.Get());
    DevLog("File URL: %s\n", pFileURL);
    DevLog("File name: %s\n", fileNameOnline.Get());
    DevLog("ID: %lld\n", replayID);

    // Check if we already have it
    if (g_pFullFileSystem->FileExists(filePathLocal.Get(), "MOD"))
    {
        DevLog("Already had the replay locally, no need to download!\n");
        CFmtStr comm("mom_replay_play %s\n", fileNameLocal.Get());
        engine->ClientCmd(comm.Get());
        m_pParentPanel->Close();
    }
    else if (MomUtil::FileExists(filePathOnline.Get(), pReplayHash, "MOD"))
    {
        DevLog("Already downloaded the replay, no need to download again!\n");
        CFmtStr command("mom_replay_play %s/%s\n", RECORDING_ONLINE_PATH, fileNameOnline.Get());
        engine->ClientCmd(command.Get());
        m_pParentPanel->Close();
    }
    else
    {
        // Check if we're already downloading it
        bool bFound = false;
        unsigned short indx = m_mapReplayDownloads.FirstInorder();
        while (indx != m_mapReplayDownloads.InvalidIndex())
        {
            if (m_mapReplayDownloads[indx] == replayID)
            {
                bFound = true;
                break;
            }

            indx = m_mapReplayDownloads.NextInorder(indx);
        }
        if (bFound)
        {
            // Already downloading!
            Log("Already downloading replay %lld!\n", replayID);
        }
        else if (replayID)
        {
            // We either don't have it, or it's outdated, so let's get the latest one!
            auto handle = g_pAPIRequests->DownloadFile(
                pFileURL, UtlMakeDelegate(this, &CLeaderboardsTimes::OnReplayDownloadStart),
                UtlMakeDelegate(this, &CLeaderboardsTimes::OnReplayDownloadProgress),
                UtlMakeDelegate(this, &CLeaderboardsTimes::OnReplayDownloadEnd),
                filePathOnline.Get(),
                "MOD",
                true);
            if (handle != INVALID_HTTPREQUEST_HANDLE)
            {
                m_mapReplayDownloads.Insert(handle, replayID);
            }
            else
            {
                Warning("Failed to try to download the replay %lld!\n", replayID);
            }
        }
    }
}

void CLeaderboardsTimes::OnContextWatchReplay(const char* runName)
{
    if (runName)
    {
        char command[MAX_PATH];
        Q_snprintf(command, MAX_PATH, "mom_replay_play %s", runName);
        engine->ServerCmd(command);
        m_pParentPanel->Close();
    }
}

inline bool CheckParent(Panel *pPanel, SectionedListPanel *pParentToCheck, int itemID)
{
    return pPanel->GetParent() == pParentToCheck && pParentToCheck->IsItemIDValid(itemID);
}

void CLeaderboardsTimes::OnItemContextMenu(KeyValues* pData)
{
    int itemID = pData->GetInt("itemID", -1);
    Panel *pPanel = static_cast<Panel *>(pData->GetPtr("SubPanel", nullptr));
    if (pPanel && pPanel->GetParent())
    {
        if (CheckParent(pPanel, m_pLocalLeaderboards, itemID))
        {
            ResetLeaderboardContextMenu();

            KeyValues *selectedRun = m_pLocalLeaderboards->GetItemData(itemID);

            const char *pFileName = selectedRun->GetString("fileName");

            m_pLeaderboardReplayCMenu->AddMenuItem("StartMap", "#MOM_Leaderboards_WatchReplay", 
                                                   new KeyValues("ContextWatchReplay", "runName", pFileName), 
                                                   this);
            m_pLeaderboardReplayCMenu->AddSeparator();
            KeyValues *pMessage = new KeyValues("ContextDeleteReplay", "runName", pFileName);
            pMessage->SetInt("itemID", itemID);
            m_pLeaderboardReplayCMenu->AddMenuItem("DeleteRun", "#MOM_Leaderboards_DeleteReplay", pMessage, this);
            m_pLeaderboardReplayCMenu->ShowMenu();
        }
        else if (CheckParent(pPanel, m_pFriendsLeaderboards, itemID) || CheckParent(pPanel, m_pTop10Leaderboards, itemID) || CheckParent(pPanel, m_pAroundLeaderboards, itemID))
        {
            ResetLeaderboardContextMenu();

            SectionedListPanel *pLeaderboard = static_cast<SectionedListPanel *>(pPanel->GetParent());
            KeyValues *pKVItemData = pLeaderboard->GetItemData(itemID);

            KeyValues *pKv = new KeyValues("ContextVisitProfile");
            pKv->SetUint64("profile", pKVItemData->GetUint64("steamid"));
            m_pLeaderboardReplayCMenu->AddMenuItem("VisitProfile", "#MOM_Leaderboards_SteamProfile", pKv, this);

            KeyValues *data = pKVItemData->MakeCopy();
            data->SetName("ContextWatchOnlineReplay");
            m_pLeaderboardReplayCMenu->AddMenuItem("WatchOnlineReplay", "#MOM_Leaderboards_WatchReplay", data, this);

            m_pLeaderboardReplayCMenu->ShowMenu();
        }
    }
}