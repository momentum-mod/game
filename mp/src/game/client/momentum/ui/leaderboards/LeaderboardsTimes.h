#pragma once

#include "vgui_controls/EditablePanel.h"
#include "steam/isteamhttp.h"
#include "mom_shareddefs.h"

class CClientTimesDisplay;
class CMomReplayBase;
class CUtlSortVectorTimeValue;
class ContextMenu;

struct TimeOnline
{
    int rank, avatar;
    float time_sec, rate;
    uint64 steamid, id;
    time_t date;
    char name[MAX_PLAYER_NAME_LENGTH];
    bool momember, vip, is_friend;

    KeyValues *m_kv;

    explicit TimeOnline(KeyValues *kv)
    {
        m_kv = kv;
        id = kv->GetUint64("id", 0);
        rank = kv->GetInt("rank", 0);
        time_sec = kv->GetFloat("time", -1);
        Q_strncpy(name, kv->GetString("personaname"), MAX_PLAYER_NAME_LENGTH);
        rate = kv->GetFloat("rate", 100);
        date = static_cast<time_t>(Q_atoi(kv->GetString("date", "0")));
        steamid = kv->GetUint64("steamid", 0);
        avatar = kv->GetInt("avatar", 0);
        momember = kv->GetBool("tm", false);
        vip = kv->GetBool("vip", false);
        is_friend = kv->GetBool("is_friend", false);
    };

    ~TimeOnline()
    {
        if (m_kv)
            m_kv->deleteThis();
        m_kv = nullptr;
    }
};

class CLeaderboardsTimes : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(CLeaderboardsTimes, EditablePanel);
    CLeaderboardsTimes(CClientTimesDisplay *pParent);
    ~CLeaderboardsTimes();

    void LevelInit();
    void Reset(bool bFullReset);
    void InitLeaderboardSections();
    void OnRunPosted(bool bPosted);
    void OnRunSaved();
    void OnPanelShow(bool bShow);

    CUtlMap<HTTPRequestHandle, uint64> m_mapReplayDownloads;

    // Sets up the icons used in the leaderboard
    void SetupDefaultIcons();

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID, CUtlMap<uint64, int> *pMap, vgui::ImageList *pList);

    bool m_bUnauthorizedFriendlist;
    // widths[0] == WIDTH FOR DATE
    // widths[1] == WIDTH FOR RANK
    // widths[2] == WIDTH FOR TIME
    int m_aiColumnWidths[3];

    // methods
    void FillLeaderboards(bool bFullUpdate);
    void SetPlaceColors(vgui::SectionedListPanel* panel, TimeType_t type) const;
    void LoadLocalTimes(KeyValues *kv);
    void LoadOnlineTimes(TimeType_t type);
    void ConvertLocalTimes(KeyValues *pKv);
    // Place vector times into leaderboards panel (sectionlist)
    void OnlineTimesVectorToLeaderboards(TimeType_t type);

    bool GetPlayerTimes(KeyValues *outPlayerInfo, bool fullUpdate);
    
    void ResetLeaderboardContextMenu();

    // sorts players within a section
    static bool StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);
    static bool StaticOnlineTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

    // finds a local time in the scoreboard
    int FindItemIDForLocalTime(KeyValues *kvRef);
    // finds an online time in the scoreboard
    int FindItemIDForOnlineTime(uint64 runID, TimeType_t type);

    void GetTop10TimesCallback(KeyValues *pKv);
    void GetFriendsTimesCallback(KeyValues *pKv);
    void GetAroundTimesCallback(KeyValues *pKv);
    void ParseTimesCallback(KeyValues *pKv, TimeType_t type);

    // Replay downloading
    void OnReplayDownloadStart(KeyValues *pKv);
    void OnReplayDownloadProgress(KeyValues *pKv);
    void OnReplayDownloadEnd(KeyValues *pKv);

protected:
    void OnCommand(const char* command) OVERRIDE;
    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

    MESSAGE_FUNC_PARAMS(OnItemContextMenu, "ItemContextMenu", data); // Catching from SectionedListPanel
    MESSAGE_FUNC_CHARPTR(OnContextWatchReplay, "ContextWatchReplay", runName);
    MESSAGE_FUNC_INT_CHARPTR(OnContextDeleteReplay, "ContextDeleteReplay", itemID, runName);
    MESSAGE_FUNC_PARAMS(OnContextWatchOnlineReplay, "ContextWatchOnlineReplay", data);
    MESSAGE_FUNC_INT_CHARPTR(OnConfirmDeleteReplay, "ConfirmDeleteReplay", itemID, file);
    MESSAGE_FUNC_UINT64(OnContextVisitProfile, "ContextVisitProfile", profile);

private:

    int m_iSectionId; // the current section we are entering into

    uint32 m_iFlaggedRuns;

    CUtlMap<uint64, int> m_mapAvatarsToImageList;

    CUtlSortVector<CMomReplayBase*, CUtlSortVectorTimeValue> m_vLocalTimes;
    CUtlVector<TimeOnline *> m_vOnlineTimes;
    CUtlVector<TimeOnline *> m_vAroundTimes;
    CUtlVector<TimeOnline *> m_vFriendsTimes;

    bool m_bTimesNeedUpdate[TIMES_COUNT];
    bool m_bTimesLoading[TIMES_COUNT];
    float m_flTimesLastUpdate[TIMES_COUNT];

    OnlineTimesStatus_t m_eTimesStatus[TIMES_COUNT];

    enum LeaderboardIconIndex_t
    {
        ICON_VIP = 0,
        ICON_TEAMMEMBER,
        ICON_FRIEND,
        ICON_DEFAULT_AVATAR,

        ICON_TOTAL // Used to control the amount of icons available
    };

    Color m_cFirstPlace, m_cSecondPlace, m_cThirdPlace;
    vgui::IBorder *pPlayerBorder;

    Panel *m_pCurrentLeaderboards;
    CClientTimesDisplay *m_pParentPanel;
    ContextMenu *m_pLeaderboardReplayCMenu;

    vgui::ImageList *m_pImageList;
    vgui::Label *m_pOnlineTimesStatus;
    vgui::SectionedListPanel *m_pTop10Leaderboards;
    vgui::SectionedListPanel *m_pAroundLeaderboards;
    vgui::SectionedListPanel *m_pLocalLeaderboards;
    vgui::SectionedListPanel *m_pFriendsLeaderboards;
    vgui::Button *m_pLocalLeaderboardsButton;
    vgui::Button *m_pGlobalLeaderboardsButton;
    vgui::Button *m_pGlobalTop10Button;
    vgui::Button *m_pGlobalAroundButton;
    vgui::Button *m_pFriendsLeaderboardsButton;
    vgui::ToggleButton *m_pRunFilterButton;
    EditablePanel *m_pFilterPanel;
};
