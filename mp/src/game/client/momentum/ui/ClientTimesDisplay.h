//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once

#include "steam/steam_api.h"
#include "GameEventListener.h"
#include <game/client/iviewport.h>
#include "vgui_controls/EditablePanel.h"
#include "mom_shareddefs.h"

#define ENABLE_HTTP_LEADERBOARDS 1
#define ENABLE_STEAM_LEADERBOARDS 0

class SavelocReqFrame;
class LobbyMembersPanel;
class CLeaderboardsContextMenu;
class CUtlSortVectorTimeValue;
class CMomReplayBase;

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientTimesDisplay : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener, public CAutoGameSystem
{
  private:
    DECLARE_CLASS_SIMPLE(CClientTimesDisplay, vgui::EditablePanel);

  protected:
    // column widths at 640
    enum
    {
        NAME_WIDTH = 160,
        SCORE_WIDTH = 60,
        DEATH_WIDTH = 60,
        PING_WIDTH = 80,
        VOICE_WIDTH = 0,
        FRIENDS_WIDTH = 0
    };
    // total = 340

    enum TIME_TYPE
    {
        TIMES_LOCAL = 0,
        TIMES_TOP10,
        TIMES_FRIENDS,
        TIMES_AROUND,

        // Should always be the last one
        TIMES_COUNT,
    };

  public:
    CClientTimesDisplay(IViewPort *pViewPort);
    ~CClientTimesDisplay();

    const char* GetName(void) OVERRIDE { return PANEL_TIMES; }

    void SetData(KeyValues *data) OVERRIDE{};

    void Reset() OVERRIDE;
    void Update() OVERRIDE;
    void Update(bool pFullUpdate);
    void Reset(bool pFullReset);
    bool NeedsUpdate(void) OVERRIDE;
    
    bool HasInputElements(void) OVERRIDE { return true; }

    void ShowPanel(bool bShow) OVERRIDE;

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel(void) OVERRIDE { return BaseClass::GetVPanel(); }

    bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }

    void SetParent(vgui::VPANEL parent) OVERRIDE { BaseClass::SetParent(parent); }

    // IGameEventListener interface:
    void FireGameEvent(IGameEvent *event) OVERRIDE;
    
    //void UpdatePlayerAvatar(int playerIndex, KeyValues *kv);
    // Updates the local player's avatar image
    void UpdatePlayerAvatarStandalone();
    // Updates an online player's avatar image
    void UpdateLeaderboardPlayerAvatar(uint64, KeyValues *kv);

    CLeaderboardsContextMenu *GetLeaderboardContextMenu(vgui::Panel *pParent);

    // Sets up the icons used in the leaderboard
    void SetupIcons();

    void LevelInitPostEntity() OVERRIDE;

  protected:
    MESSAGE_FUNC_INT(OnPollHideCode, "PollHideCode", code);
    MESSAGE_FUNC_PARAMS(OnItemContextMenu, "ItemContextMenu", data); // Catching from SectionedListPanel
    MESSAGE_FUNC_CHARPTR(OnContextWatchReplay, "ContextWatchReplay", runName);
    MESSAGE_FUNC_INT_CHARPTR(OnContextDeleteReplay, "ContextDeleteReplay", itemID, runName);
    MESSAGE_FUNC_CHARPTR(OnContextGoToMap, "ContextGoToMap", map);
    MESSAGE_FUNC_UINT64(OnContextVisitProfile, "ContextVisitProfile", profile);
    MESSAGE_FUNC_PARAMS(OnContextWatchOnlineReplay, "ContextWatchOnlineReplay", data);
    MESSAGE_FUNC_UINT64(OnSpectateLobbyMember, "ContextSpectate", target);
    MESSAGE_FUNC_UINT64(OnContextReqSavelocs, "ContextReqSavelocs", target);
    MESSAGE_FUNC_INT_CHARPTR(OnConfirmDeleteReplay, "ConfirmDeleteReplay", itemID, file);

    STEAM_CALLBACK(CClientTimesDisplay, OnPersonaStateChange, PersonaStateChange_t);


#if ENABLE_STEAM_LEADERBOARDS
    // Leaderboards API
    SteamLeaderboard_t m_hCurrentLeaderboard;
    CCallResult<CClientTimesDisplay, LeaderboardFindResult_t> m_cLeaderboardFindResult;
    void OnLeaderboardFindResult(LeaderboardFindResult_t *pResult, bool bIOFailure);
    CCallResult<CClientTimesDisplay, LeaderboardScoresDownloaded_t> m_cLeaderboardGlobalScoresDownloaded;
    void OnLeaderboardGlobalScoresDownloaded(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure);
    CCallResult<CClientTimesDisplay, LeaderboardScoresDownloaded_t> m_cLeaderboardFriendsScoresDownloaded;
    void OnLeaderboardFriendScoresDownloaded(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure);

    CCallResult<CClientTimesDisplay, RemoteStorageDownloadUGCResult_t> m_cOnlineReplayDownloaded;
    void OnOnlineReplayDownloaded(RemoteStorageDownloadUGCResult_t *pResult, bool bIOFailure);
#endif

    // Attempts to add the avatar for a given steam ID to the given image list, if it doesn't exist already
    // exist in the given ID to index map.
    int TryAddAvatar(const uint64 &steamID, CUtlMap<uint64, int> *pMap, vgui::ImageList *pList);

    // functions to override
    bool GetPlayerTimes(KeyValues *outPlayerInfo, bool fullUpdate);
    void InitScoreboardSections();
    void UpdatePlayerInfo(KeyValues *outPlayerInfo, bool fullUpdate);
    void OnThink() OVERRIDE;
    void AddHeader(); // add the start header of the scoreboard

    void OnCommand(const char *pCommand) OVERRIDE;

    // sorts players within a section
    static bool StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);
    static bool StaticOnlineTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);


    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;

    void PostApplySchemeSettings(vgui::IScheme *pScheme);

    // finds a local time in the scoreboard
    int FindItemIDForLocalTime(KeyValues *kvRef);
    // finds an online time in the scoreboard
    int FindItemIDForOnlineTime(uint64 runID, TIME_TYPE type);

    int m_iSectionId; // the current section we are entering into

    float m_fNextUpdateTime;

    void MoveToCenterOfScreen();
    // Sets the text of the MapInfo label. If it's nullptr, it hides it
    void UpdateMapInfoLabel(const char *text = nullptr);
    void UpdateMapInfoLabel(const char *author, const int tier, const char *layout, const int bonus);

    vgui::ImageList *m_pImageList;
    Panel *m_pHeader;
    Panel *m_pPlayerStats;
    Panel *m_pLeaderboards;
    vgui::Label *m_pMapName;
    vgui::Label *m_pMapAuthor;
    vgui::Label *m_pMapDetails;
    vgui::Label *m_pPlayerName;
    vgui::Label *m_pPlayerMapRank;
    vgui::Label *m_pPlayerPersonalBest;
    vgui::Label *m_pPlayerGlobalRank;
    vgui::Label *m_pPlayerExperience;
    vgui::Label *m_pOnlineTimesStatus;
    vgui::SectionedListPanel *m_pOnlineLeaderboards;
    vgui::SectionedListPanel *m_pAroundLeaderboards;
    vgui::SectionedListPanel *m_pLocalLeaderboards;
    vgui::SectionedListPanel *m_pFriendsLeaderboards;
    vgui::ImagePanel *m_pPlayerAvatar;
    vgui::Button *m_pLocalLeaderboardsButton;
    vgui::Button *m_pGlobalLeaderboardsButton;
    vgui::Button *m_pGlobalTop10Button;
    vgui::Button *m_pGlobalAroundButton;
    vgui::Button *m_pFriendsLeaderboardsButton;

    LobbyMembersPanel *m_pLobbyMembersPanel;
    SavelocReqFrame *m_pSavelocReqFrame;

    vgui::ToggleButton *m_pRunFilterButton;
    EditablePanel *m_pFilterPanel;

    Panel *m_pCurrentLeaderboards;

    CUtlMap<uint64, int> m_mapAvatarsToImageList;

    CPanelAnimationVar(int, m_iAvatarWidth, "avatar_width", "34"); // Avatar width doesn't scale with resolution
    CPanelAnimationVarAliasType(int, m_iNameWidth, "name_width", "136", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iClassWidth, "class_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iScoreWidth, "score_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iDeathWidth, "death_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iPingWidth, "ping_width", "23", "proportional_int");

    // Online API Pre-Alpha functions

#if ENABLE_HTTP_LEADERBOARDS
    void GetTop10TimesCallback(KeyValues *pKv);
    void GetFriendsTimesCallback(KeyValues *pKv);
    void GetAroundTimesCallback(KeyValues *pKv);
    void ParseTimesCallback(KeyValues *pKv, TIME_TYPE type);

    // Replay downloading
    void OnReplayDownloadStart(KeyValues *pKv);
    void OnReplayDownloadProgress(KeyValues *pKv);
    void OnReplayDownloadEnd(KeyValues *pKv);

    void GetPlayerDataForMapCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
    void GetMapInfoCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
#endif

  private:
    int m_iPlayerIndexSymbol;
    int m_iDesiredHeight;

    float m_fLastHeaderUpdate;
    bool m_bFirstHeaderUpdate;

    IViewPort *m_pViewPort;
    ButtonCode_t m_nCloseKey;

    CUtlMap<HTTPRequestHandle, uint64> m_mapReplayDownloads;
    
    void ConvertOnlineTimes(KeyValues *kv, float seconds);
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

    bool m_bUnauthorizedFriendlist;
    // widths[0] == WIDTH FOR DATE
    // widths[1] == WIDTH FOR RANK
    // widths[2] == WIDTH FOR TIME
    // widths[3] == WIDTH FOR DATE
    int m_aiColumnWidths[4];

    // methods
    void FillScoreBoard();
    void SetPlaceColors(vgui::SectionedListPanel* panel, TIME_TYPE type) const;
    void FillScoreBoard(bool pFullUpdate);
    void LoadLocalTimes(KeyValues *kv);
    void LoadOnlineTimes();
    void LoadAroundTimes();
    void LoadFriendsTimes();
    void ConvertLocalTimes(KeyValues *);
    // Place vector times into leaderboards panel (sectionlist)
    void OnlineTimesVectorToLeaderboards(TIME_TYPE type);

    CLeaderboardsContextMenu *m_pLeaderboardReplayCMenu;

    CUtlMap<uint64, const char *> m_umMapNames;

    bool m_bGetTop10Scores;

    bool m_bMapInfoLoaded;

    enum LEADERBOARD_ICONS
    {
        ICON_VIP,
        ICON_TEAMMEMBER,
        ICON_FRIEND,

        ICON_TOTAL // Used to control the amount of icons available
    };
    int m_IconsIndex[ICON_TOTAL];

    int m_bLoadedLocalPlayerAvatar;

    Color m_cFirstPlace, m_cSecondPlace, m_cThirdPlace;

    RUN_FLAG flaggedRuns;
};