//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws the normal TF2 or HL2 HUD.
//
//=============================================================================
#include "cbase.h"

#include "leaderboards/ClientTimesDisplay.h"
#include "menus/hud_menu_static.h"
#include "HUD/hud_mapfinished.h"
#include "spectate/mom_spectator_gui.h"
#include "spectate/mom_replayui.h"
#include "tricks/TrickList.h"

#include "IGameUIFuncs.h"
#include "clientmode_mom_normal.h"
#include "hud.h"
#include "ienginevgui.h"
#include "in_buttons.h"
#include "momentum/mom_shareddefs.h"
#include "ZoneMenu/ZoneMenu.h"
#include "c_mom_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
static MAKE_TOGGLE_CONVAR(mom_enable_overlapping_keys, "1", FCVAR_ARCHIVE,
                          "If enabled the game will allow you to press 2 keys at once which will null out the movement gain.\n");

static MAKE_TOGGLE_CONVAR(mom_release_forward_on_jump, "0", FCVAR_ARCHIVE,
                          "When enabled the game will auto release the forward key which is determined by movement, so "
                          "it can be used on all styles except \"half\" styles e.g. HSW.\n");

CON_COMMAND(hud_reloadcontrols, "Reloads the control res files for hud elements.")
{
    Panel *pViewport = g_pClientMode->GetViewport();
    if (pViewport)
    {
        pViewport->PostMessage(pViewport, new KeyValues("ReloadControls"));
    }
}

extern ConVar cl_forwardspeed;
extern ConVar cl_sidespeed;

HScheme g_hVGuiCombineScheme = 0;

// Instance the singleton and expose the interface to it.
IClientMode *GetClientModeNormal()
{
    static ClientModeMOMNormal g_ClientModeNormal;
    return &g_ClientModeNormal;
}

//-----------------------------------------------------------------------------
// Purpose: this is the viewport that contains all the hud elements
//-----------------------------------------------------------------------------
class CHudViewport : public CBaseViewport
{
  private:
    DECLARE_CLASS_SIMPLE(CHudViewport, CBaseViewport);

  protected:
    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
    {
        BaseClass::ApplySchemeSettings(pScheme);

        gHUD.InitColors(pScheme);

        SetPaintBackgroundEnabled(false);
    }

    IViewPortPanel *CreatePanelByName(const char *pzName) OVERRIDE
    {
        if (FStrEq(PANEL_TIMES, pzName))
        {
            return new CClientTimesDisplay(this);
        }
        if (FStrEq(PANEL_TRICK_LIST, pzName))
        {
            return new TrickList(this);
        }
        if (FStrEq(PANEL_REPLAY, pzName))
        {
            return new C_MOMReplayUI(this);
        }
        if (FStrEq(PANEL_SPECGUI, pzName))
        {
            return new CMOMSpectatorGUI(this);
        }

        return BaseClass::CreatePanelByName(pzName);
    }

    void CreateDefaultPanels() OVERRIDE
    {
        AddNewPanel(CreatePanelByName(PANEL_REPLAY), "PANEL_REPLAY");
        AddNewPanel(CreatePanelByName(PANEL_TIMES), "PANEL_TIMES");
        AddNewPanel(CreatePanelByName(PANEL_SPECGUI), "PANEL_SPECGUI");
        AddNewPanel(CreatePanelByName(PANEL_TRICK_LIST), "PANEL_TRICK_LIST");
        // BaseClass::CreateDefaultPanels(); // MOM_TODO: do we want the other panels?
    }

    void OnScreenSizeChanged(int iOldWide, int iOldTall) OVERRIDE
    {
        BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);

        static_cast<ClientModeMOMNormal*>(g_pClientMode)->SetupPointers();
    }
};

//-----------------------------------------------------------------------------
// ClientModeHLNormal implementation
//-----------------------------------------------------------------------------
ClientModeMOMNormal::ClientModeMOMNormal()
{
    m_pHudMenuStatic = nullptr;
    m_pHudMapFinished = nullptr;
    m_pLeaderboards = nullptr;
    m_pSpectatorGUI = nullptr;
    m_pLobbyMembers = nullptr;
    m_pTrickList = nullptr;
    m_pViewport = new CHudViewport();
    m_pViewport->Start(gameuifuncs, gameeventmanager);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
ClientModeMOMNormal::~ClientModeMOMNormal()
{
    // MOM_TODO: delete pointers (m_pViewport) here?
}

void ClientModeMOMNormal::Init()
{
    BaseClass::Init();
    SetupPointers();
    // Load up the combine control panel scheme
    g_hVGuiCombineScheme = scheme()->LoadSchemeFromFileEx(
        enginevgui->GetPanel(PANEL_CLIENTDLL),
        "resource/CombinePanelScheme.res", "CombineScheme");

    if (!g_hVGuiCombineScheme)
    {
        Warning("Couldn't load combine panel scheme!\n");
    }
}

int ClientModeMOMNormal::HudElementKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding)
{
    // Swallow the key input if a hud menu is open
    if (m_pHudMenuStatic && m_pHudMenuStatic->IsMenuDisplayed())
    {
        if (down >= 1 && keynum >= KEY_0 && keynum <= KEY_9)
        {
            m_pHudMenuStatic->SelectMenuItem(keynum - KEY_0);
            return 0; // The hud menu static swallowed the key input
        }
    }

    // Detach the mouse if the user right-clicked while the leaderboards are open
    if (m_pLeaderboards && m_pLeaderboards->IsVisible())
    {
        if (keynum == MOUSE_RIGHT)
        {
            m_pLeaderboards->SetMouseInputEnabled(true);
            return 0;
        }
    }

    if (m_pTrickList && m_pTrickList->IsVisible())
    {
        if (keynum == MOUSE_RIGHT)
        {
            m_pTrickList->SetMouseInputEnabled(true);
            return 0;
        }
    }

    // Detach the mouse if the user right-clicked while the map finished dialog is open
    if (m_pHudMapFinished && m_pHudMapFinished->IsVisible())
    {
        if (keynum == MOUSE_RIGHT)
        {
            m_pHudMapFinished->SetMouseInputEnabled(true);
            if (m_pSpectatorGUI)
                m_pSpectatorGUI->SetMouseInputEnabled(true);
            return 0;
        }
    }

    if (g_pZoneMenu && g_pZoneMenu->IsVisible())
    {
        if (g_pZoneMenu->HandleKeyInput(down, keynum))
        {
            return 0;
        }
    }

    return BaseClass::HudElementKeyInput(down, keynum, pszCurrentBinding);
}

int ClientModeMOMNormal::HandleSpectatorKeyInput(int down, ButtonCode_t keynum, const char *pszCurrentBinding)
{
    if (!m_pSpectatorGUI)
        return 1;

    if (!down || !pszCurrentBinding)
        return 1;

    // we are in spectator mode, open spectator menu
    if (FStrEq(pszCurrentBinding, "+duck") || FStrEq(pszCurrentBinding, "toggle_duck"))
    {
        bool bMouseState = m_pSpectatorGUI->IsMouseInputEnabled();
        m_pSpectatorGUI->SetMouseInputEnabled(!bMouseState);
        if (m_pHudMapFinished && m_pHudMapFinished->IsVisible())
            m_pHudMapFinished->SetMouseInputEnabled(!bMouseState);

        return 0; // we handled it, don't handle twice or send to server
    }

    bool bShouldEatSpecInput = (m_pHudMapFinished && m_pHudMapFinished->IsVisible()) || m_pSpectatorGUI->IsMouseInputEnabled();
    if (bShouldEatSpecInput)
        return 1;

    if (FStrEq(pszCurrentBinding, "+attack"))
    {
        engine->ClientCmd("spec_next");
        return 0;
    }
    if (FStrEq(pszCurrentBinding, "+attack2"))
    {
        engine->ClientCmd("spec_prev");
        return 0;
    }
    if (FStrEq(pszCurrentBinding, "+jump") || FStrEq(pszCurrentBinding, "toggle_jump"))
    {
        engine->ClientCmd("spec_mode");
        return 0;
    }

    return 1;
}

void ClientModeMOMNormal::SetupPointers()
{
    m_pHudMenuStatic = GET_HUDELEMENT(CHudMenuStatic);
    m_pHudMapFinished = GET_HUDELEMENT(CHudMapFinishedDialog);
    m_pLeaderboards = dynamic_cast<CClientTimesDisplay *>(m_pViewport->FindPanelByName(PANEL_TIMES));
    m_pSpectatorGUI = dynamic_cast<CMOMSpectatorGUI *>(m_pViewport->FindPanelByName(PANEL_SPECGUI));
    m_pTrickList = dynamic_cast<TrickList*>(m_pViewport->FindPanelByName(PANEL_TRICK_LIST));
}

int ClientModeMOMNormal::MovementDirection(const QAngle viewangles, const Vector velocity)
{
    if (velocity.Length2D() > 50.f)
    {
        float movement_diff = atanf(velocity.y / velocity.x) * 180.f / M_PI_F;

        if (velocity.x < 0.f)
        {
            if (velocity.y > 0.f)
                movement_diff += 180.f;
            else
                movement_diff -= 180.f;
        }
        if (movement_diff < 0.f)
            movement_diff += 360.f;

        float my_ang = viewangles.y;

        if (my_ang < 0.f)
            my_ang += 360.f;

        movement_diff = movement_diff - my_ang;

        bool flipped = false;

        if (movement_diff < 0.f)
        {
            flipped = true;
            movement_diff = -movement_diff;
        }

        if (movement_diff > 180.f)
        {
            if (flipped)
                flipped = false;
            else
                flipped = true;

            movement_diff = fabs(movement_diff - 360.f);
        }

        if (-0.1f < movement_diff && movement_diff < 67.5f)
        {
            return MD_Forwards; // Forwards
        }
        if (67.5f < movement_diff && movement_diff < 112.5f)
        {
            if (flipped)
            {
                return MD_Sideways; // Sideways
            }
            else
            {
                return MD_Sideways2; // Sideways other way
            }
        }
        if (112.5f < movement_diff && movement_diff < 180.f)
        {
            return MD_Backwards; // Backwards
        }
    }
    return MD_NONE; // Unknown should never happen
}

bool ClientModeMOMNormal::CreateMove(float flInputSampleTime, CUserCmd *cmd)
{
    if (!cmd->command_number)
    {
        return BaseClass::CreateMove(flInputSampleTime, cmd);
    }

    auto local_player = C_MomentumPlayer::GetLocalMomPlayer();
    static int dominant_buttons = 0;
    static int prev_flags = 0;

    if (!local_player)
    {
        return false;
    }

    int mdir = MovementDirection(cmd->viewangles, local_player->GetAbsVelocity());

    if (mom_release_forward_on_jump.GetBool() && prev_flags & FL_ONGROUND && FL_ONGROUND & local_player->GetFlags() &&
        local_player->GetGroundEntity() &&
        cmd->buttons & IN_JUMP)
    {
        switch (mdir)
        {
        case MD_NONE:
        {
            break;
        }
        case MD_Backwards:
        case MD_Forwards:
        {
            // Call these cmds to trigger the release key event
            engine->ClientCmd("-back");
            engine->ClientCmd("-forward");

            // The key release probably triggers next frame, so lets speed it up with this
            cmd->buttons &= ~(IN_FORWARD | IN_BACK);
            cmd->forwardmove = 0.f;
            break;
        }
        case MD_Sideways:
        case MD_Sideways2:
        {
            // Call these cmds to trigger the release key event
            engine->ClientCmd("-moveleft");
            engine->ClientCmd("-moveright");

            // The key release probably triggers next frame, so lets speed it up with this
            cmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT);
            cmd->sidemove = 0.f;
            break;
        }
        }
    }

    if (mom_enable_overlapping_keys.GetBool())
    {
        cmd->buttons &= ~local_player->m_afButtonDisabled;

        // Holding both forward and backwards, which one was the last pressed of these?
        if ((cmd->buttons & (IN_FORWARD | IN_BACK)) == (IN_FORWARD | IN_BACK))
        {
            // Remove presses and apply the dominant key
            cmd->buttons &= ~(IN_FORWARD | IN_BACK);
            cmd->buttons |= (dominant_buttons & (IN_FORWARD | IN_BACK));
            cmd->forwardmove = cmd->buttons & IN_FORWARD ? -cl_forwardspeed.GetFloat() : cl_forwardspeed.GetFloat();
        }
        else if (cmd->buttons & IN_FORWARD)
        {
            dominant_buttons |= IN_FORWARD;
            dominant_buttons &= ~IN_BACK;
        }
        else if (cmd->buttons & IN_BACK)
        {
            dominant_buttons |= IN_BACK;
            dominant_buttons &= ~IN_FORWARD;
        }
        else
        {
            dominant_buttons &= ~(IN_FORWARD | IN_BACK);
        }

        // Holding both forward and backwards, which one was the last pressed of these?
        if ((cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT)) == (IN_MOVELEFT | IN_MOVERIGHT))
        {
            // Remove presses and apply the dominant key
            cmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT);
            cmd->buttons |= (dominant_buttons & (IN_MOVELEFT | IN_MOVERIGHT));
            cmd->sidemove = cmd->buttons & IN_MOVELEFT ? cl_sidespeed.GetFloat() : -cl_sidespeed.GetFloat();
        }
        else if (cmd->buttons & IN_MOVELEFT)
        {
            dominant_buttons |= IN_MOVELEFT;
            dominant_buttons &= ~IN_MOVERIGHT;
        }
        else if (cmd->buttons & IN_MOVERIGHT)
        {
            dominant_buttons |= IN_MOVERIGHT;
            dominant_buttons &= ~IN_MOVELEFT;
        }
        else
        {
            dominant_buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT);
        }
    }

    prev_flags = local_player->GetFlags();

    return BaseClass::CreateMove(flInputSampleTime, cmd);
}
