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
class CClientTimesDisplay : public vgui::EditablePanel, public IViewPortPanel, public CGameEventListener, public CAutoGameSystem
{
    DECLARE_CLASS_SIMPLE(CClientTimesDisplay, vgui::EditablePanel);

  public:
    CClientTimesDisplay(IViewPort *pViewPort);
    ~CClientTimesDisplay();

    const char* GetName() OVERRIDE { return PANEL_TIMES; }
    void SetData(KeyValues *data) OVERRIDE{};
    void Reset() OVERRIDE;
    bool NeedsUpdate() OVERRIDE;
    void Update() OVERRIDE;
    void Update(bool bFullUpdate);
    void Reset(bool bFullReset);
    
    bool HasInputElements() OVERRIDE { return true; }

    void ShowPanel(bool bShow) OVERRIDE;

    void SetMouseInputEnabled(bool bState) OVERRIDE;

    void SetVisible(bool bState) OVERRIDE;

    void Close();

    // both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
    vgui::VPANEL GetVPanel() OVERRIDE { return BaseClass::GetVPanel(); }
    bool IsVisible() OVERRIDE { return BaseClass::IsVisible(); }
    void SetParent(vgui::VPANEL parent) OVERRIDE { BaseClass::SetParent(parent); }

  protected:
    MESSAGE_FUNC_INT(OnPollHideCode, "PollHideCode", code);

    // functions to override
    void OnThink() OVERRIDE;

    void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    void PerformLayout() OVERRIDE;

    // IGameEventListener interface:
    void FireGameEvent(IGameEvent *event) OVERRIDE;
    // CAutoGameSystem:
    void LevelInitPostEntity() OVERRIDE;

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

    IViewPort *m_pViewPort;
    ButtonCode_t m_nCloseKey;

    float m_flNextUpdateTime;

    bool m_bToggledOpen;

    // methods
    void FillScoreBoard();
    void FillScoreBoard(bool pFullUpdate);

    void SetLeaderboardsHideHud(bool bState);
};