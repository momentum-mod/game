//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#pragma once

#include "GameEventListener.h"
#include <game/client/iviewport.h>
#include "vgui_controls/EditablePanel.h"
#include "mom_shareddefs.h"

class CLeaderboardsTimes;
class CLeaderboardsStats;
class CLeaderboardsHeader;

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientTimesDisplay : public vgui::EditablePanel, public CGameEventListener, public CAutoGameSystem
{
    DECLARE_CLASS_SIMPLE(CClientTimesDisplay, vgui::EditablePanel);

  public:
    CClientTimesDisplay();
    ~CClientTimesDisplay();

    // CBaseGameSystem defines Init already
    static void InitPanel();

    void OnThink() override;

    void Update(bool bFullUpdate = false);
    void TryUpdate(bool bFullUpdate = false);
    void Reset(bool bFullReset = false);

    void ShowPanel(bool bShow);

    void SetMouseInputEnabled(bool bState) override;

    void SetVisible(bool bState) override;

    void Close();

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel() override { return BaseClass::GetVPanel(); }
    bool IsVisible() override { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) override { BaseClass::SetParent(parent); }

  protected:
    // functions to override
    void OnKeyCodeReleased(vgui::KeyCode code) override;

    void ApplySchemeSettings(vgui::IScheme *pScheme) override;
    void PerformLayout() override;

    // IGameEventListener interface:
    void FireGameEvent(IGameEvent *event) override;
    // CAutoGameSystem:
    void LevelInitPostEntity() override;
    void LevelShutdownPreEntity() override;

    void OnReloadControls() override;

    void MoveToCenterOfScreen();

    CPanelAnimationVar(int, m_iAvatarWidth, "avatar_width", "34"); // Avatar width doesn't scale with resolution
    CPanelAnimationVarAliasType(int, m_iNameWidth, "name_width", "136", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iClassWidth, "class_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iScoreWidth, "score_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iDeathWidth, "death_width", "35", "proportional_int");
    CPanelAnimationVarAliasType(int, m_iPingWidth, "ping_width", "23", "proportional_int");

  private:
    CLeaderboardsHeader *m_pHeader;
    CLeaderboardsStats *m_pStats;
    CLeaderboardsTimes *m_pTimes;

    float m_flNextUpdateTime;

    bool m_bToggledOpen;
    bool m_bNeedsControlUpdate;

    // methods
    void FillScoreBoard();
    void FillScoreBoard(bool pFullUpdate);

    void SetLeaderboardsHideHud(bool bState);
};

extern CClientTimesDisplay *g_pClientTimes;