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

#include "IGameUIFuncs.h" // for key bindings
#include "mom_replayui.h"
#include <imapoverview.h>
#include <shareddefs.h>
#include <vgui/IInput.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/TextEntry.h>

#include "mom_player_shared.h"
#include "util/mom_util.h"

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
    // 	m_bHelpShown = false;
    //	m_bInsetVisible = false;
    //	m_iDuckKey = KEY_NONE;
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

    LoadControlSettings(GetResFile());
    m_pTopBar = FindControl<Panel>("TopBar");
    m_pPlayerLabel = FindControl<Label>("PlayerLabel");
    m_pReplayLabel = FindControl<Label>("ReplayLabel");
    m_pMapLabel = FindControl<Label>("MapLabel");
    m_pTimeLabel = FindControl<Label>("TimeLabel");
    m_pGainControlLabel = FindControl<Label>("DetachInfo");
    m_pCloseButton = FindControl<ImagePanel>("ClosePanel");
    m_pCloseButton->InstallMouseHandler(this);

    m_pShowControls = FindControl<ImagePanel>("ShowControls");
    m_pShowControls->InstallMouseHandler(this);

    m_pReplayControls = dynamic_cast<C_MOMReplayUI *>(m_pViewPort->FindPanelByName(PANEL_REPLAY));

    TextImage *image = m_pPlayerLabel->GetTextImage();
    if (image)
    {
        HFont hFallbackFont = scheme()->GetIScheme(GetScheme())->GetFont("DefaultVerySmallFallBack", false);
        if (INVALID_FONT != hFallbackFont)
        {
            image->SetUseFallbackFont(true, hFallbackFont);
        }
    }

    SetMouseInputEnabled(false);
    InvalidateLayout();

    FIND_LOCALIZATION(m_pwReplayPlayer, "#MOM_ReplayPlayer");
    FIND_LOCALIZATION(m_pwGainControl, "#MOM_SpecGUI_GainControl");
    FIND_LOCALIZATION(m_pwWatchingReplay, "#MOM_WatchingReplay");
    FIND_LOCALIZATION(m_pwRunTime, "#MOM_MF_RunTime");
    FIND_LOCALIZATION(m_pwSpecMap, "#Spec_Map");
}

//-----------------------------------------------------------------------------
// Purpose: Sets the colour of the top and bottom bars
//-----------------------------------------------------------------------------
void CMOMSpectatorGUI::ApplySchemeSettings(IScheme *pScheme)
{
    m_pTopBar->SetVisible(true);

    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(Color(0, 0, 0, 0)); // make the background transparent
    m_pTopBar->SetBgColor(GetBlackBarColor());
    SetPaintBorderEnabled(false);

    SetBorder(nullptr);
}

void CMOMSpectatorGUI::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        VPANEL over = input()->GetMouseOver();
        if (over == m_pCloseButton->GetVPanel())
        {
            SetMouseInputEnabled(false);
            // We're piggybacking on this event because it's basically the same as the X on the mapfinished panel
            IGameEvent *pClosePanel = gameeventmanager->CreateEvent("mapfinished_panel_closed");
            if (pClosePanel)
            {
                // Fire this event so other classes can get at this
                gameeventmanager->FireEvent(pClosePanel);
            }
        }
        else if (over == m_pShowControls->GetVPanel() && m_pReplayControls)
        {
            m_pReplayControls->ShowPanel(!m_pReplayControls->IsVisible());
        }
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
        if (m_pReplayControls && !m_pReplayControls->IsVisible())
            m_pReplayControls->ShowPanel(true);
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

    CMomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        C_MomentumReplayGhostEntity *pReplayEnt = pPlayer->GetReplayEnt();
        if (pReplayEnt)
        {
            // Current player name
            wchar_t wPlayerName[BUFSIZELOCL], szPlayerInfo[BUFSIZELOCL];
            ANSI_TO_UNICODE(pReplayEnt->m_pszPlayerName, wPlayerName);
            g_pVGuiLocalize->ConstructString(szPlayerInfo, sizeof(szPlayerInfo), m_pwReplayPlayer, 1, wPlayerName);

            if (m_pPlayerLabel)
                m_pPlayerLabel->SetText(szPlayerInfo);


            // Duck bind to release mouse
            wchar_t tempControl[BUFSIZELOCL];
            UTIL_ReplaceKeyBindings(m_pwGainControl, sizeof m_pwGainControl, tempControl, sizeof tempControl);
            if (m_pGainControlLabel)
                m_pGainControlLabel->SetText(tempControl);

            // Run time label
            char tempRunTime[BUFSIZETIME];
            wchar_t wTimeLabel[BUFSIZELOCL], wTime[BUFSIZETIME];
            g_pMomentumUtil->FormatTime(pReplayEnt->m_SrvData.m_RunData.m_flRunTime, tempRunTime);
            ANSI_TO_UNICODE(tempRunTime, wTime);
            g_pVGuiLocalize->ConstructString(wTimeLabel, sizeof(wTimeLabel), m_pwRunTime, 1, wTime);

            if (m_pTimeLabel)
                m_pTimeLabel->SetText(wTimeLabel);

            // "REPLAY" label
            if (m_pReplayLabel)
                m_pReplayLabel->SetText(m_pwWatchingReplay);
        }
        else
        {
            if (m_pReplayLabel)
                m_pReplayLabel->SetText("");
        }
    }

    // Show the current map
    wchar_t szMapName[1024];
    char tempstr[128];
    wchar_t wMapName[BUFSIZELOCL];
    Q_FileBase(engine->GetLevelName(), tempstr, sizeof(tempstr));
    ANSI_TO_UNICODE(tempstr, wMapName);
    g_pVGuiLocalize->ConstructString(szMapName, sizeof(szMapName), m_pwSpecMap, 1, wMapName);

    if (m_pMapLabel)
        m_pMapLabel->SetText(szMapName);
}
