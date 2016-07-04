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

#include "GameEventListener.h"
#include "momentum/mom_shareddefs.h"
#include <KeyValues.h>
#include <game/client/iviewport.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/SectionedListPanel.h>

#include "ReplayContextMenu.h"

#define TYPE_NOTEAM 0 // NOTEAM must be zero :)
#define TYPE_TEAM 1   // a section for a single team
#define TYPE_PLAYERS 2
#define TYPE_SPECTATORS 3 // a section for a spectator group
#define TYPE_BLANK 4

#define SCALE(num) scheme()->GetProportionalScaledValueEx(GetScheme(), (num))


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

  public:
    CClientTimesDisplay(IViewPort *pViewPort);
    ~CClientTimesDisplay();

    const char *GetName(void) override { return PANEL_TIMES; }

    void SetData(KeyValues *data) override{};

    void Reset() override;
    void Update() override;
    void Update(bool pFullUpdate);
    void Reset(bool pFullReset);
    bool NeedsUpdate(void) override;

    bool HasInputElements(void) override { return true; }

    void ShowPanel(bool bShow) override;

    virtual bool ShowAvatars() { return IsPC(); }

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel(void) override { return BaseClass::GetVPanel(); }

    bool IsVisible() override { return BaseClass::IsVisible(); }

    void SetParent(vgui::VPANEL parent) override { BaseClass::SetParent(parent); }

    // IGameEventListener interface:
    void FireGameEvent(IGameEvent *event) override;

    virtual void UpdatePlayerAvatar(int playerIndex, KeyValues *kv);

    CReplayContextMenu *GetLeaderboardReplayContextMenu(vgui::Panel *pParent);

  protected:
    MESSAGE_FUNC_INT(OnPollHideCode, "PollHideCode", code);
    MESSAGE_FUNC_ENUM(OnMousePressed, "MousePressed", ButtonCode_t, code);
    MESSAGE_FUNC_PARAMS(OnItemContextMenu, "ItemContextMenu", data);
    virtual void OnCommand(const char *command);

    // functions to override
    virtual bool GetPlayerTimes(KeyValues *outPlayerInfo);
    virtual void InitScoreboardSections();
    virtual void UpdateTeamInfo();
    virtual void UpdatePlayerInfo(KeyValues *outPlayerInfo);
    void OnThink() override;
    virtual void AddHeader(vgui::Label *pMapSummary);      // add the start header of the scoreboard
    virtual void AddSection(int teamType, int teamNumber); // add a new section header for a team
    virtual int GetAdditionalHeight() { return 0; }

    // sorts players within a section
    static bool StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);
    static bool StaticOnlineTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;

    virtual void PostApplySchemeSettings(vgui::IScheme *pScheme);

    // finds the player in the scoreboard
    int FindItemIDForPlayerIndex(int playerIndex);
    // finds a local time in the scoreboard
    int FindItemIDForLocalTime(KeyValues *kvRef);

    int m_iNumTeams;

    vgui::SectionedListPanel *m_pPlayerList;
    int m_iSectionId; // the current section we are entering into

    float m_fNextUpdateTime;

    void MoveLabelToFront(const char *textEntryName);
    void MoveToCenterOfScreen();

    vgui::ImageList *m_pImageList;
    vgui::Panel *m_pHeader;
    vgui::Panel *m_pPlayerStats;
    vgui::Panel *m_pLeaderboards;
    vgui::Label *m_lMapSummary;
    vgui::Label *m_lPlayerName;
    vgui::Label *m_lPlayerMapRank;
    vgui::Label *m_lPlayerGlobalRank;
    vgui::SectionedListPanel *m_pOnlineLeaderboards;
    vgui::SectionedListPanel *m_pLocalLeaderboards;
    vgui::SectionedListPanel *m_pFriendsLeaderboards;
    vgui::ImagePanel *m_pPlayerAvatar;
    vgui::ImagePanel *m_pMomentumLogo;

    CUtlMap<CSteamID, int> m_mapAvatarsToImageList;

    CPanelAnimationVar(int, m_iAvatarWidth, "avatar_width", "34"); // Avatar width doesn't scale with resolution
    CPanelAnimationVarAliasType(int, m_iNameWidth, "name_width", "136", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iClassWidth, "class_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iScoreWidth, "score_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iDeathWidth, "death_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iPingWidth, "ping_width", "23", "proportional_int");
  private:
    int m_iPlayerIndexSymbol;
    int m_iDesiredHeight;
    IViewPort *m_pViewPort;
    ButtonCode_t m_nCloseKey;
    struct Time
    {
        float time_sec, rate;
        time_t date;

        explicit Time(KeyValues *kv)
        {
            time_sec = Q_atof(kv->GetName());
            rate = kv->GetFloat("rate", gpGlobals->interval_per_tick);
            date = static_cast<time_t>(kv->GetInt("date", 0));
        };
    };
    CUtlVector<Time> m_vLocalTimes;
    // MOM_TODO: CUtlVector<SomeStruct> m_vOnlineTimes;

    bool m_bLocalTimesLoaded = false;
    bool m_bLocalTimesNeedUpdate = false;
    bool m_bOnlineNeedUpdate = false;
    // widths[0] == WIDTH FOR DATE
    // widths[1] == WIDTH FOR RANK
    // widths[2] == WIDTH FOR TIME
    int m_aiColumnWidths[3];

    // methods
    void FillScoreBoard();
    void FillScoreBoard(bool pFullUpdate);
    void LoadLocalTimes(KeyValues *kv);
    void ConvertLocalTimes(KeyValues *);

    CReplayContextMenu *m_pLeaderboardReplayCMenu;
};
#endif // CLIENTSCOREBOARDDIALOG_H