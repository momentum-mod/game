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
#include "IMessageboxPanel.h"
#include "LeaderboardsContextMenu.h"

#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "run/mom_replay_base.h"
#include "mom_map_cache.h"
#include "mom_api_requests.h"
#include "run/mom_replay_factory.h"
#include "filesystem.h"
#include "fmtstr.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define RANKSTRING "00000"               // A max of 99999 ranks (too generous)
#define DATESTRING "59 minutes ago" // Entire date string
#define TIMESTRING "00:00:00.000"        // Entire time string

#define UPDATE_INTERVAL 15.0f  // The amount of seconds minimum between online checks

static const char* const g_pszTimesStatusStrings[] = {
    "", // STATUS_TIMES_LOADED
    "#MOM_API_WaitingForResponse", // STATUS_TIMES_LOADING
    "#MOM_API_NoTimesReturned", // STATUS_NO_TIMES_RETURNED
    "#MOM_API_ServerError", // STATUS_SERVER_ERROR
    "#MOM_API_NoPBSet", // STATUS_NO_PB_SET
    "#MOM_API_NoFriends", // STATUS_NO_FRIENDS
    "#MOM_API_UnauthFriendsList", // STATUS_UNAUTHORIZED_FRIENDS_LIST
};

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

    m_pFilterPanel = new EditablePanel(pParent, "FilterPanel");
    m_pFilterPanel->AddActionSignalTarget(this);
    m_pFilterPanel->LoadControlSettings("resource/ui/leaderboards/filter_panel.res");

    m_pLeaderboardReplayCMenu = new CLeaderboardsContextMenu(this);

    m_pOnlineTimesStatus = new Label(this, "OnlineTimesStatus", "#MOM_API_WaitingForResponse");
    m_pTop10Leaderboards = new SectionedListPanel(this, "Top10Leaderboards");
    m_pAroundLeaderboards = new SectionedListPanel(this, "AroundLeaderboards");
    m_pFriendsLeaderboards = new SectionedListPanel(this, "FriendsLeaderboards");
    m_pLocalLeaderboards = new SectionedListPanel(this, "LocalLeaderboards");

    LoadControlSettings("resource/ui/leaderboards/times.res");

    m_pGlobalLeaderboardsButton = FindControl<Button>("GlobalLeaderboardsButton", true);
    m_pGlobalTop10Button = FindControl<Button>("GlobalTop10Button", true);
    m_pGlobalAroundButton = FindControl<Button>("GlobalAroundButton", true);
    m_pFriendsLeaderboardsButton = FindControl<Button>("FriendsLeaderboardsButton", true);
    m_pLocalLeaderboardsButton = FindControl<Button>("LocalLeaderboardsButton", true);
    m_pRunFilterButton = FindControl<ToggleButton>("FilterButton", true);

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

    // MOM_TODO: HACKHACK: this is changed to false because deleting a scheme image is a no-no.
    // While we do know that the image list will only hold scheme and avatar images, we cannot delete
    // either one. We do not have to delete the scheme images, as they are cached, and the avatar images
    // are also cached, but indefinitely. There's a memory leak with avatar images, since every image just
    // keeps creating Texture IDs and never destroying them, so if you download a lot of *different* avatars 
    // (play a lot of maps and look at the leaderboards for them), you could start to see the perf impact of it.
    // I'll leave it as a HACKHACK for now because this is ugly and that memory needs freed after a while, but may
    // be unnoticeable for most people... we'll see how big the memory leak impact really is.
    m_pImageList = new ImageList(false);
    SetupIcons();
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

    m_pImageList = new ImageList(false);
    m_mapAvatarsToImageList.RemoveAll();

    SetupIcons();
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
                                  0, NAME_WIDTH);
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

void CLeaderboardsTimes::SetupIcons()
{
    for (int index = 0; index < ICON_TOTAL; index++)
    {
        m_IconsIndex[index] = -1;
        IImage *image = nullptr;
        switch (index)
        {
        case ICON_VIP:
            image = scheme()->GetImage("leaderboards_icon_vip", false);
            break;
        case ICON_TEAMMEMBER:
            image = scheme()->GetImage("leaderboards_icon_mom", false);
            break;
        case ICON_FRIEND:
            image = scheme()->GetImage("leaderboards_icon_friends", false);
            break;
        default:
            break;
        }
        if (image)
        {
            image->SetSize(16, 16);
            m_IconsIndex[index] = m_pImageList->AddImage(image);
        }
    }
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

void CLeaderboardsTimes::UpdateLeaderboardPlayerAvatar(uint64 steamID, KeyValues* kv)
{
    // Update their avatar
    if (SteamFriends())
    {
        kv->SetBool("is_friend", SteamFriends()->HasFriend(CSteamID(steamID), k_EFriendFlagImmediate));
        kv->SetInt("avatar", TryAddAvatar(steamID, &m_mapAvatarsToImageList, m_pImageList));
    }
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

void CLeaderboardsTimes::SetPlaceColors(vgui::SectionedListPanel* panel, TIME_TYPE type) const
{
    int itemCount = panel->GetItemCount();
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

void CLeaderboardsTimes::LoadOnlineTimes(TIME_TYPE type)
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
        g_pMomentumUtil->FormatTime(t->GetRunTime(), timeString);
        kvLocalTimeFormatted->SetString("time", timeString); // Used for display

        char dateString[64];
        time_t date = t->GetRunDate();
        if (g_pMomentumUtil->GetTimeAgoString(&date, dateString, sizeof(dateString)))
        {
            kvLocalTimeFormatted->SetString("date", dateString);
        }
        else
            kvLocalTimeFormatted->SetInt("date", date);

        // MOM_TODO: Convert the run flags to pictures

        kvInto->AddSubKey(kvLocalTimeFormatted);
    }
}

void CLeaderboardsTimes::ConvertOnlineTimes(KeyValues* kv, float seconds)
{
    char timeString[BUFSIZETIME];

    g_pMomentumUtil->FormatTime(seconds, timeString);
    kv->SetString("time_f", timeString);
}

void CLeaderboardsTimes::OnlineTimesVectorToLeaderboards(TIME_TYPE type)
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
            pList->DeleteAllItems();

        FOR_EACH_VEC(*pVector, entry)
        {
            TimeOnline *runEntry = pVector->Element(entry);

            int itemID = FindItemIDForOnlineTime(runEntry->id, type);

            runEntry->m_kv->SetInt("icon_tm", runEntry->momember ? m_IconsIndex[ICON_TEAMMEMBER] : -1);
            runEntry->m_kv->SetInt("icon_vip", runEntry->vip ? m_IconsIndex[ICON_VIP] : -1);
            runEntry->m_kv->SetInt("icon_friend", runEntry->is_friend ? m_IconsIndex[ICON_FRIEND] : -1);

            if (itemID == -1)
            {
                itemID = pList->AddItem(m_iSectionId, runEntry->m_kv);
            }
            else
            {
                pList->ModifyItem(itemID, m_iSectionId, runEntry->m_kv);
            }

            // MOM_TODO: highlight the local player's thing (some outline?), if it's in the list!
            //if (runEntry->steamid == SteamUser()->GetSteamID().ConvertToUint64())
            //    pList->SetBorderForItem(itemID, someBorder);
        }

        SetPlaceColors(pList, type);
    }
    if (m_pOnlineTimesStatus)
    {
        if (m_eTimesStatus[type] == STATUS_TIMES_LOADED)
            m_pOnlineTimesStatus->SetVisible(false);
        else
        {
            m_pOnlineTimesStatus->SetText(g_pszTimesStatusStrings[m_eTimesStatus[type]]);
            m_pOnlineTimesStatus->SetVisible(true);
            m_pOnlineTimesStatus->InvalidateLayout(true);
        }
    }
}

bool CLeaderboardsTimes::GetPlayerTimes(KeyValues* outPlayerInfo, bool fullUpdate)
{
    ConVarRef gm("mom_gamemode");
    if (!outPlayerInfo || gm.GetInt() == GAMEMODE_UNKNOWN)
        return false;

    KeyValues *pLeaderboards = new KeyValues("leaderboards");

    // Fill local times:
    KeyValues *pLocal = new KeyValues("local");
    LoadLocalTimes(pLocal);
    pLeaderboards->AddSubKey(pLocal);

    // Skip over local
    for (int i = 1; i < TIMES_COUNT; i++)
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

    outPlayerInfo->AddSubKey(pLeaderboards);
    return true;
}

void CLeaderboardsTimes::ResetLeaderboardContextMenu()
{
    m_pLeaderboardReplayCMenu->SetVisible(false);
    m_pLeaderboardReplayCMenu->DeleteAllItems();
}

bool CLeaderboardsTimes::StaticLocalTimeSortFunc(vgui::SectionedListPanel* list, int itemID1, int itemID2)
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

bool CLeaderboardsTimes::StaticOnlineTimeSortFunc(vgui::SectionedListPanel* list, int itemID1, int itemID2)
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

int CLeaderboardsTimes::FindItemIDForOnlineTime(uint64 runID, TIME_TYPE type)
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

void CLeaderboardsTimes::ParseTimesCallback(KeyValues* pKv, TIME_TYPE type)
{
    m_bTimesLoading[type] = false;

    KeyValues *pData = pKv->FindKey("data");
    KeyValues *pErr = pKv->FindKey("error");
    if (pData)
    {
        KeyValues *pRuns = pData->FindKey("runs");

        if (pRuns && pData->GetInt("count") > 0)
        {
            CUtlVector<TimeOnline*> *vecs[] = { nullptr, &m_vOnlineTimes, &m_vFriendsTimes, &m_vAroundTimes };
            // By now we're pretty sure everything will be ok, so we can do this
            vecs[type]->PurgeAndDeleteElements();

            // Iterate through each loaded run
            FOR_EACH_SUBKEY(pRuns, pRun)
            {
                KeyValues *kvEntry = new KeyValues("Entry");

                // Around does UserMapRank -> Run instead of the other way around, so we do some funny business here
                KeyValues *pOuter = nullptr;
                if (type == TIMES_AROUND)
                {
                    pOuter = pRun;
                    pRun = pOuter->FindKey("run");
                    AssertMsg(pRun, "Around times didn't work!");
                }

                // Time is handled by the converter
                kvEntry->SetFloat("time", pRun->GetFloat("time"));

                // Tickrate
                kvEntry->SetFloat("rate", pRun->GetFloat("tickRate"));

                // Date
                char timeAgoStr[64];
                if (g_pMomentumUtil->GetTimeAgoString(pRun->GetString("dateAchieved"), timeAgoStr, sizeof(timeAgoStr)))
                    kvEntry->SetString("date", timeAgoStr);
                else
                    kvEntry->SetString("date", pRun->GetString("dateAchieved"));

                // ID
                kvEntry->SetUint64("id", pRun->GetUint64("id"));

                // File
                kvEntry->SetString("file", pRun->GetString("file"));

                // Hash
                kvEntry->SetString("hash", pRun->GetString("hash"));

                KeyValues *kvUserObj = pRun->FindKey("user");
                if (kvUserObj)
                {
                    uint64 steamID = Q_atoui64(kvUserObj->GetString("id"));
                    kvEntry->SetUint64("steamid", steamID);

                    int permissions = kvUserObj->GetInt("permissions");

                    // Is part of the momentum team?
                    // MOM_TODO: Make this the actual permission
                    kvEntry->SetBool("tm", permissions & (USER_ADMIN | USER_MODERATOR));

                    // Is vip?
                    // MOM_TODO: Make this the actual permission
                    kvEntry->SetBool("vip", pRun->GetBool("vip"));

                    // MOM_TODO: check if alias banned
                    kvEntry->SetString("personaname", kvUserObj->GetString("alias"));

                    if (SteamFriends() && SteamUser())
                    {
                        uint64 localSteamID = SteamUser()->GetSteamID().ConvertToUint64();
                        // These handle setting "avatar" for kvEntry
                        if (localSteamID == steamID)
                        {
                            kvEntry->SetInt("avatar", TryAddAvatar(localSteamID, &m_mapAvatarsToImageList, m_pImageList));
                        }
                        else
                        {
                            // MOM_TODO: check if avatar banned
                            UpdateLeaderboardPlayerAvatar(steamID, kvEntry);
                        }
                    }
                }

                // Rank
                if (type != TIMES_AROUND)
                {
                    KeyValues *kvRankObj = pRun->FindKey("rank");
                    if (kvRankObj)
                    {
                        kvEntry->SetInt("rank", kvRankObj->GetInt("rank"));
                    }
                }
                else
                {
                    kvEntry->SetInt("rank", pOuter->GetInt("rank"));
                    pRun = pOuter; // Make sure to reset to outer so we can continue the loop
                }

                // Add this baby to the online times vector
                TimeOnline *ot = new TimeOnline(kvEntry);
                // Convert the time
                ConvertOnlineTimes(ot->m_kv, ot->time_sec);
                vecs[type]->AddToTail(ot);
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
            DevLog("Successfully downloaded the replay with ID: %i\n", m_mapReplayDownloads[fileIndx]);

            // Play it
            CFmtStr command("mom_replay_play %s/%s-%lld%s\n", RECORDING_ONLINE_PATH, m_pParentPanel->MapName(), m_mapReplayDownloads[fileIndx], EXT_RECORDING_FILE);
            engine->ClientCmd(command.Get());
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

void CLeaderboardsTimes::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_cFirstPlace = pScheme->GetColor("FirstPlace", Color(240, 210, 147, 50));
    m_cSecondPlace = pScheme->GetColor("SecondPlace", Color(175, 175, 175, 50));
    m_cThirdPlace = pScheme->GetColor("ThirdPlace", Color(205, 127, 50, 50));

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
        m_pLocalLeaderboards->RemoveItem(itemID);
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
        messageboxpanel->CreateConfirmationBox(this, "#MOM_Leaderboards_DeleteReplay",
                                               "#MOM_MB_DeleteRunConfirmation", pCommand,
                                               nullptr, "#MOM_Leaderboards_DeleteReplay");
    }
}

void CLeaderboardsTimes::OnContextVisitProfile(uint64 profile)
{
    if (profile != 0 && SteamFriends())
    {
        SteamFriends()->ActivateGameOverlayToUser("steamid", CSteamID(profile));
        m_pParentPanel->ShowPanel(false);
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
    }
    else if (g_pMomentumUtil->FileExists(filePathOnline.Get(), pReplayHash, "MOD"))
    {
        DevLog("Already downloaded the replay, no need to download again!\n");
        CFmtStr command("mom_replay_play %s/%s\n", RECORDING_ONLINE_PATH, fileNameOnline.Get());
        engine->ClientCmd(command.Get());
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
                "MOD");
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
        m_pParentPanel->ShowPanel(false);
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