//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>

#include "momSpectatorGUI.h"

#include <game/client/iviewport.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/TextImage.h>

#include "mom_replayui.h"
#include <imapoverview.h>
#include <shareddefs.h>
#include <vgui/IInput.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/TextEntry.h>

#include "mom_player_shared.h"
#include "util/mom_util.h"
#include "c_mom_replay_entity.h"
#include "c_mom_online_ghost.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// main spectator panel

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMOMSpectatorGUI::CMOMSpectatorGUI(IViewPort *pViewPort) : EditablePanel(nullptr, PANEL_SPECGUI)
{
    SetSize(10, 10); // Quiet "parent not sized yet" spew
    m_bSpecScoreboard = false;
    m_pViewPort = pViewPort;
    ListenForGameEvent("spec_target_updated");

    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    m_flNextUpdateTime = -1.0f;

    // initialize dialog
    SetVisible(false);
    SetProportional(true);
    SetPaintBorderEnabled(false);
    SetPaintBackgroundEnabled(false);
    SetKeyBoardInputEnabled(false);

    // load the new scheme early!!
    SetScheme("ClientScheme");

    m_pTopBar = new Panel(this, "TopBar");
    m_pPlayerLabel = new Label(this, "PlayerLabel", "#MOM_ReplayPlayer");
    m_pReplayLabel = new Label(this, "ReplayLabel", "#MOM_WatchingReplay");
    m_pMapLabel = new Label(this, "MapLabel", "#Spec_Map");
    m_pTimeLabel = new Label(this, "TimeLabel", "#MOM_MF_RunTime");
    m_pGainControlLabel = new Label(this, "DetachInfo", "#MOM_SpecGUI_GainControl");
    m_pCloseButton = new ImagePanel(this, "ClosePanel");
    m_pShowControls = new ImagePanel(this, "ShowControls");
    m_pPrevPlayerButton = new ImagePanel(this, "PrevPlayerButton");
    m_pNextPlayerButton = new ImagePanel(this, "NextPlayerButton");

    m_pReplayControls = dynamic_cast<C_MOMReplayUI *>(m_pViewPort->FindPanelByName(PANEL_REPLAY));

    LoadControlSettings("resource/ui/Spectator.res");

    m_pCloseButton->InstallMouseHandler(this);

    m_pShowControls->InstallMouseHandler(this);
    
    m_pPrevPlayerButton->InstallMouseHandler(this);
    m_pNextPlayerButton->InstallMouseHandler(this);

    SetMouseInputEnabled(false);
    InvalidateLayout();

    FIND_LOCALIZATION(m_pwReplayPlayer, "#MOM_ReplayPlayer");
    FIND_LOCALIZATION(m_pwGainControl, "#MOM_SpecGUI_GainControl");
    FIND_LOCALIZATION(m_pwRunTime, "#MOM_MF_RunTime");
    FIND_LOCALIZATION(m_pwSpecMap, "#Spec_Map");
    FIND_LOCALIZATION(m_pwWatchingGhost, "#MOM_WatchingGhost");
    FIND_LOCALIZATION(m_pwWatchingReplay, "#MOM_WatchingReplay");
}

//-----------------------------------------------------------------------------
// Purpose: Sets the colour of the top and bottom bars
//-----------------------------------------------------------------------------
void CMOMSpectatorGUI::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_pTopBar->SetVisible(true);
    SetBgColor(Color(0, 0, 0, 0)); // make the background transparent
    m_cBarColor = pScheme->GetColor("SpecUI.TopBarColor", Color(0, 0, 0, 196));
    m_pTopBar->SetBgColor(m_cBarColor);
    SetPaintBorderEnabled(false);

    SetBorder(nullptr);

    TextImage *image = m_pPlayerLabel->GetTextImage();
    if (image)
    {
        HFont hFallbackFont = pScheme->GetFont("DefaultVerySmallFallBack", IsProportional());
        if (INVALID_FONT != hFallbackFont)
        {
            image->SetUseFallbackFont(true, hFallbackFont);
        }
    }

    Update();
}

void CMOMSpectatorGUI::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        VPANEL over = input()->GetMouseOver();
        if (over == m_pCloseButton->GetVPanel())
        {
            SetMouseInputEnabled(false);
            engine->ClientCmd("mom_spectate_stop"); //in case the entitiy recieving this event does not exist..
        }
        else if (over == m_pShowControls->GetVPanel() && m_pReplayControls)
        {
            m_pReplayControls->ShowPanel(!m_pReplayControls->IsVisible());
        }
        else if (over == m_pNextPlayerButton->GetVPanel())
        {
            engine->ClientCmd("spec_next");
        }
        else if (over == m_pPrevPlayerButton->GetVPanel())
        {
            engine->ClientCmd("spec_prev");
        }
    }
}

void CMOMSpectatorGUI::FireGameEvent(IGameEvent* pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "spec_target_updated"))
    {
        // So apparently calling Update from here doesn't work, due to some weird
        // thing that happens upon the player's m_hObserverTarget getting updated.
        // Pushing this back three ticks is more than long enough to delay the Update()
        // to fill the panel with the replay's info.
        m_flNextUpdateTime = gpGlobals->curtime + gpGlobals->interval_per_tick * 3.0f;
    }
}
//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CMOMSpectatorGUI::PerformLayout()
{
    int w, dummy;
    GetHudSize(w, dummy);

    // fill the screen
    SetBounds(0, 0, w, m_pTopBar->GetTall());
}

//-----------------------------------------------------------------------------
// Purpose: checks spec_scoreboard cvar to see if the scoreboard should be displayed
//-----------------------------------------------------------------------------
void CMOMSpectatorGUI::OnThink()
{
    if (m_flNextUpdateTime > 0.0f && gpGlobals->curtime > m_flNextUpdateTime)
    {
        Update();
        m_flNextUpdateTime = -1.0f;
    }

    BaseClass::OnThink();
}

void CMOMSpectatorGUI::SetMouseInputEnabled(bool bState)
{
    BaseClass::SetMouseInputEnabled(bState);

    if (m_pReplayControls && m_pReplayControls->IsVisible())
        m_pReplayControls->SetMouseInputEnabled(bState);
}

//-----------------------------------------------------------------------------
// Purpose: shows/hides the buy menu
//-----------------------------------------------------------------------------
void CMOMSpectatorGUI::ShowPanel(bool bShow)
{
    if (engine->IsPlayingDemo())
        return;

    // If we're becoming visible (for the first time)
    if (bShow && !IsVisible())
    {
        m_bSpecScoreboard = false;
        SetMouseInputEnabled(true);
    }

    SetVisible(bShow);

    if (!bShow)
    {

        if (m_pReplayControls && m_pReplayControls->IsVisible())
            m_pReplayControls->ShowPanel(false);

        if (m_bSpecScoreboard)
        {
            m_pViewPort->ShowPanel(PANEL_TIMES, false);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: Updates the gui, rearranges elements
//-----------------------------------------------------------------------------
void CMOMSpectatorGUI::Update()
{
    int wide, tall;
    int bx, by, bwide, btall;
    
    GetHudSize(wide, tall);
    m_pTopBar->GetBounds(bx, by, bwide, btall);

    IViewPortPanel *overview = m_pViewPort->FindPanelByName(PANEL_OVERVIEW);

    if (overview && overview->IsVisible())
    {
        int mx, my, mwide, mtall;

        VPANEL p = overview->GetVPanel();
        ipanel()->GetPos(p, mx, my);
        ipanel()->GetSize(p, mwide, mtall);

        if (my < btall)
        {
            // reduce to bar
            m_pTopBar->SetSize(wide - (mx + mwide), btall);
            m_pTopBar->SetPos((mx + mwide), 0);
        }
        else
        {
            // full top bar
            m_pTopBar->SetSize(wide, btall);
            m_pTopBar->SetPos(0, 0);
        }
    }
    else
    {
        // full top bar
        m_pTopBar->SetSize(wide, btall); // change width, keep height
        m_pTopBar->SetPos(0, 0);
    }

    // Duck bind to release mouse
    wchar_t tempControl[BUFSIZELOCL];
    UTIL_ReplaceKeyBindings(m_pwGainControl, sizeof m_pwGainControl, tempControl, sizeof tempControl);
    m_pGainControlLabel->SetText(tempControl);

    CMomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        C_MomentumReplayGhostEntity *pReplayEnt = pPlayer->GetReplayEnt();
        C_MomentumOnlineGhostEntity *pOnlineGhostEnt = pPlayer->GetOnlineGhostEnt();

        bool isReplayGhost = (pReplayEnt && pReplayEnt->IsReplayGhost());
        bool isOnlineGhost = (pOnlineGhostEnt && pOnlineGhostEnt->IsOnlineGhost());
        if (isOnlineGhost || isReplayGhost )
        {
            // Current player name
            wchar_t wPlayerName[BUFSIZELOCL];
            ANSI_TO_UNICODE(isReplayGhost ? pReplayEnt->m_szGhostName : pOnlineGhostEnt->m_szGhostName, wPlayerName);
            m_pPlayerLabel->SetText(CConstructLocalizedString(m_pwReplayPlayer, wPlayerName));

            if (isReplayGhost)
            {
                // Run time label
                char tempRunTime[BUFSIZETIME];
                wchar_t wTime[BUFSIZETIME];
                g_pMomentumUtil->FormatTime(pReplayEnt->m_Data.m_flRunTime, tempRunTime);
                ANSI_TO_UNICODE(tempRunTime, wTime);
                m_pTimeLabel->SetText(CConstructLocalizedString(m_pwRunTime, wTime));

                // "REPLAY" label
                m_pReplayLabel->SetText(m_pwWatchingReplay);

                // Show the current map
                wchar_t wMapName[BUFSIZELOCL];
                ANSI_TO_UNICODE(g_pGameRules->MapName(), wMapName);
                m_pMapLabel->SetText(CConstructLocalizedString(m_pwSpecMap, wMapName));

                m_pShowControls->SetVisible(true);

                if (!m_pReplayControls->IsVisible())
                    m_pReplayControls->ShowPanel(true);
                
                //MOM_TODO: check if an online ghost has spawned, and don't hide spec buttons?
                m_pPrevPlayerButton->SetVisible(false);
                m_pNextPlayerButton->SetVisible(false);
            }
            if (isOnlineGhost)
            {
                // "REPLAY" label
                m_pReplayLabel->SetText(m_pwWatchingGhost);
                m_pMapLabel->SetText("");
                m_pTimeLabel->SetText("");

                // We don't need replay controls in online spectating!
                m_pShowControls->SetVisible(false);
                m_pReplayControls->ShowPanel(false);
                m_pPrevPlayerButton->SetVisible(true);
                m_pNextPlayerButton->SetVisible(true);
            }
        }
        else
        {
            m_pReplayLabel->SetText("");
            m_pMapLabel->SetText("");
            m_pTimeLabel->SetText("");
            m_pPlayerLabel->SetText("");
        }
    }
}
