#pragma once

#include "vgui_controls/EditablePanel.h"
#include "steam/steam_api.h"

class CClientTimesDisplay;
class CMomReplayBase;
class CUtlSortVectorTimeValue;
class CLeaderboardsContextMenu;

enum TIME_TYPE
{
    TIMES_LOCAL = 0,
    TIMES_TOP10,
    TIMES_FRIENDS,
    TIMES_AROUND,

    // Should always be the last one
    TIMES_COUNT,
};

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

// column widths at 640
enum SIZES
{
    NAME_WIDTH = 160,
    SCORE_WIDTH = 60,
    DEATH_WIDTH = 60,
    PING_WIDTH = 80,
    VOICE_WIDTH = 0,
    FRIENDS_WIDTH = 0
};
// total = 340

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
    void SetupIcons();

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID, CUtlMap<uint64, int> *pMap, vgui::ImageList *pList);

    // Updates an online player's avatar image
    void UpdateLeaderboardPlayerAvatar(uint64, KeyValues *kv);

    bool m_bUnauthorizedFriendlist;
    // widths[0] == WIDTH FOR DATE
    // widths[1] == WIDTH FOR RANK
    // widths[2] == WIDTH FOR TIME
    int m_aiColumnWidths[3];

    // methods
    void FillLeaderboards(bool bFullUpdate);
    void SetPlaceColors(vgui::SectionedListPanel* panel, TIME_TYPE type) const;
    void LoadLocalTimes(KeyValues *kv);
    void LoadOnlineTimes(TIME_TYPE type);
    void ConvertLocalTimes(KeyValues *pKv);
    void ConvertOnlineTimes(KeyValues *kv, float seconds);
    // Place vector times into leaderboards panel (sectionlist)
    void OnlineTimesVectorToLeaderboards(TIME_TYPE type);

    bool GetPlayerTimes(KeyValues *outPlayerInfo, bool fullUpdate);
    
    void ResetLeaderboardContextMenu();

    // sorts players within a section
    static bool StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);
    static bool StaticOnlineTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

    // finds a local time in the scoreboard
    int FindItemIDForLocalTime(KeyValues *kvRef);
    // finds an online time in the scoreboard
    int FindItemIDForOnlineTime(uint64 runID, TIME_TYPE type);

    void GetTop10TimesCallback(KeyValues *pKv);
    void GetFriendsTimesCallback(KeyValues *pKv);
    void GetAroundTimesCallback(KeyValues *pKv);
    void ParseTimesCallback(KeyValues *pKv, TIME_TYPE type);

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

    enum ONLINE_TIMES_STATUS
    {
        STATUS_TIMES_LOADED = 0,
        STATUS_TIMES_LOADING,
        STATUS_NO_TIMES_RETURNED,
        STATUS_SERVER_ERROR,
        STATUS_NO_PB_SET,
        STATUS_NO_FRIENDS,
        STATUS_UNAUTHORIZED_FRIENDS_LIST,
        // Should be last
        STATUS_COUNT,
    };
    ONLINE_TIMES_STATUS m_eTimesStatus[TIMES_COUNT];

    enum LEADERBOARD_ICONS
    {
        ICON_VIP,
        ICON_TEAMMEMBER,
        ICON_FRIEND,

        ICON_TOTAL // Used to control the amount of icons available
    };
    int m_IconsIndex[ICON_TOTAL];

    Color m_cFirstPlace, m_cSecondPlace, m_cThirdPlace;

    Panel *m_pCurrentLeaderboards;
    CClientTimesDisplay *m_pParentPanel;
    CLeaderboardsContextMenu *m_pLeaderboardReplayCMenu;

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
