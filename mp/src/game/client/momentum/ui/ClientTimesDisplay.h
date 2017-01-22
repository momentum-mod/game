//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef CLIENTTIMESDISPLAY_H
#define CLIENTTIMESDISPLAY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#include "steam/steam_api.h"

#include "GameEventListener.h"

#include "ReplayContextMenu.h"
#include "momentum/mom_shareddefs.h"
#include <KeyValues.h>
#include <game/client/iviewport.h>
#include <vgui_controls/pch_vgui_controls.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/SectionedListPanel.h>
#include <vgui_controls/TextImage.h>
#include "run/mom_replay_base.h"

#define TYPE_NOTEAM 0 // NOTEAM must be zero :)
#define TYPE_TEAM 1   // a section for a single team
#define TYPE_PLAYERS 2
#define TYPE_SPECTATORS 3 // a section for a spectator group
#define TYPE_BLANK 4

#define SCALE(num) scheme()->GetProportionalScaledValueEx(GetScheme(), (num))

#define DELAY_NEXT_UPDATE 10.0f           // Delay for the next API update, in seconds
#define MIN_ONLINE_UPDATE_INTERVAL 15.0f  // The amount of seconds minimum between online checks
#define MAX_ONLINE_UPDATE_INTERVAL 45.0f  // The amount of seconds maximum between online checks
#define MIN_FRIENDS_UPDATE_INTERVAL 15.0f // The amount of seconds minimum between online checks
#define MAX_FRIENDS_UPDATE_INTERVAL 45.0f // The amount of seconds maximum between online checks

class CUtlSortVectorTimeValue
{
public:
    bool Less(CMomReplayBase *lhs, CMomReplayBase *rhs, void *) const
    {
        return lhs->GetRunTime() < rhs->GetRunTime();
    }
};

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientTimesDisplay : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener
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

    enum LEADERBOARDS
    {
        FRIENDS_LEADERBOARDS = 0,
        ONLINE_LEADERBOARDS,
        LOCAL_LEADERBOARDS
    };

  public:
    CClientTimesDisplay(IViewPort *pViewPort);
    ~CClientTimesDisplay();

    const char *GetName(void) OVERRIDE { return PANEL_TIMES; }

    void SetData(KeyValues *data) OVERRIDE{};

    void Reset() OVERRIDE;
    void Update() OVERRIDE;
    void Update(bool pFullUpdate);
    void Reset(bool pFullReset);
    bool NeedsUpdate(void) OVERRIDE;

    bool HasInputElements(void) OVERRIDE { return true; }

    void ShowPanel(bool bShow) OVERRIDE;

    bool ShowAvatars() { return IsPC(); }

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel(void) OVERRIDE { return BaseClass::GetVPanel(); }

    bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }

    void SetParent(vgui::VPANEL parent) OVERRIDE { BaseClass::SetParent(parent); }

    // IGameEventListener interface:
    void FireGameEvent(IGameEvent *event) OVERRIDE;

    //void UpdatePlayerAvatar(int playerIndex, KeyValues *kv);
    // Updates the local player's avatar image
    void UpdatePlayerAvatarStandalone();
    // This updates the local player's avatar in the online/friends leaderboards
    void UpdatePlayerAvatarStandaloneOnline(KeyValues *);
    // Updates an online player's avatar image
    void UpdateLeaderboardPlayerAvatar(uint64, KeyValues *kv);

    CReplayContextMenu *GetLeaderboardReplayContextMenu(vgui::Panel *pParent);

  protected:
    MESSAGE_FUNC_INT(OnPollHideCode, "PollHideCode", code);
    MESSAGE_FUNC_PARAMS(OnItemContextMenu, "ItemContextMenu", data); // Catching from SectionedListPanel
    MESSAGE_FUNC_CHARPTR(OnContextWatchReplay, "ContextWatchReplay", runName);
    MESSAGE_FUNC_CHARPTR(OnContextDeleteReplay, "ContextDeleteReplay", runName);
    MESSAGE_FUNC_UINT64(OnContextVisitProfile, "ContextVisitProfile", profile);
    MESSAGE_FUNC_PARAMS(OnConfirmDeleteReplay, "ConfirmDeleteReplay", data);

    STEAM_CALLBACK(CClientTimesDisplay, OnPersonaStateChange, PersonaStateChange_t);

    int TryAddAvatar(const CSteamID &);

    // functions to override
    bool GetPlayerTimes(KeyValues *outPlayerInfo, bool fullUpdate);
    void InitScoreboardSections();
    void UpdatePlayerInfo(KeyValues *outPlayerInfo, bool fullUpdate);
    void OnThink() OVERRIDE;
    void AddHeader(); // add the start header of the scoreboard
    static int GetAdditionalHeight() { return 0; }

    void OnCommand(const char *pCommand) OVERRIDE;

    // sorts players within a section
    static bool StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);
    static bool StaticOnlineTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;

    void PostApplySchemeSettings(vgui::IScheme *pScheme);

    // finds a local time in the scoreboard
    int FindItemIDForLocalTime(KeyValues *kvRef);
    // finds an online time in the scoreboard
    int FindItemIDForOnlineTime(int runID, LEADERBOARDS);

    int m_iNumTeams;

    int m_iSectionId; // the current section we are entering into

    float m_fNextUpdateTime;

    void MoveLabelToFront(const char *textEntryName);
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
    vgui::Label *m_pLoadingOnlineTimes;
    vgui::SectionedListPanel *m_pOnlineLeaderboards;
    vgui::SectionedListPanel *m_pLocalLeaderboards;
    vgui::SectionedListPanel *m_pFriendsLeaderboards;
    vgui::ImagePanel *m_pPlayerAvatar;
    vgui::ImagePanel *m_pMomentumLogo;
    vgui::Button *m_pLocalLeaderboardsButton;
    vgui::Button *m_pGlobalLeaderboardsButton;
    vgui::Button *m_pGlobalTop10Button;
    vgui::Button *m_pGlobalAroundButton;
    vgui::Button *m_pFriendsLeaderboardsButton;

    vgui::ToggleButton *m_pRunFilterButton;
    EditablePanel *m_pFilterPanel;

    Panel *m_pCurrentLeaderboards;

    CUtlMap<CSteamID, int> m_mapAvatarsToImageList;

    CPanelAnimationVar(int, m_iAvatarWidth, "avatar_width", "34"); // Avatar width doesn't scale with resolution
    CPanelAnimationVarAliasType(int, m_iNameWidth, "name_width", "136", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iClassWidth, "class_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iScoreWidth, "score_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iDeathWidth, "death_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iPingWidth, "ping_width", "23", "proportional_int");

    // Online API Pre-Alpha functions

    void GetOnlineTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
    CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t> cbGetOnlineTimesCallback;
    void GetPlayerDataForMapCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
    CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t> cbGetPlayerDataForMapCallback;
    void GetFriendsTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
    CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t> cbGetFriendsTimesCallback;
    void GetMapInfoCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
    CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t> cbGetMapInfoCallback;

    void CreateAndSendHTTPReq(const char *, CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t> *,
                              CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t>::func_t);

    void ParseTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure, bool bFriendsTimes);

  private:
    int m_iPlayerIndexSymbol;
    int m_iDesiredHeight;

    float m_fLastHeaderUpdate;
    bool m_bFirstHeaderUpdate;

    float m_flLastOnlineTimeUpdate;
    bool m_bFirstOnlineTimesUpdate;

    float m_flLastFriendsTimeUpdate;
    bool m_bFirstFriendsTimesUpdate;

    IViewPort *m_pViewPort;
    ButtonCode_t m_nCloseKey;
    
    void ConvertOnlineTimes(KeyValues *kv, float seconds);
    struct TimeOnline
    {
        int rank, id, avatar;
        float time_sec, rate;
        uint64 steamid;
        time_t date;
        const char *personaname;
        bool momember, vip, is_friend;

        KeyValues *m_kv;

        explicit TimeOnline(KeyValues *kv)
        {
            m_kv = kv;
            id = kv->GetInt("id", -1);
            rank = kv->GetInt("rank", 0);
            time_sec = kv->GetFloat("time", -1);
            personaname = kv->GetString("personaname", "Unknown");
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
            personaname = nullptr;
        }
    };

    CUtlSortVector<CMomReplayBase*, CUtlSortVectorTimeValue> m_vLocalTimes;
    CUtlVector<TimeOnline *> m_vOnlineTimes;
    CUtlVector<TimeOnline *> m_vFriendsTimes;

    bool m_bLocalTimesLoaded;
    bool m_bLocalTimesNeedUpdate;
    bool m_bOnlineNeedUpdate;
    bool m_bOnlineTimesLoaded;
    bool m_bFriendsNeedUpdate;
    bool m_bFriendsTimesLoaded;
    bool m_bUnauthorizedFriendlist;
    // widths[0] == WIDTH FOR DATE
    // widths[1] == WIDTH FOR RANK
    // widths[2] == WIDTH FOR TIME
    int m_aiColumnWidths[3];

    // methods
    void FillScoreBoard();
    void SetPlaceColors(vgui::SectionedListPanel* panel) const;
    void FillScoreBoard(bool pFullUpdate);
    void LoadLocalTimes(KeyValues *kv);
    void LoadOnlineTimes();
    void LoadFriendsTimes();
    void ConvertLocalTimes(KeyValues *);
    // Place vector times into leaderboards panel (sectionlist)
    void OnlineTimesVectorToLeaderboards(LEADERBOARDS);

    CReplayContextMenu *m_pLeaderboardReplayCMenu;

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
#endif // CLIENTSCOREBOARDDIALOG_H
