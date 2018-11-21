#include "TASPanel.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/CVarSlider.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include "ColorPicker.h"
#include "clientmode_shared.h"
#include "materialsystem/imaterialvar.h"
#include "mom_gamemovement.h"
#include "mom_inpu"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "util_shared.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "weapon/weapon_csbase.h"

#include "tier0/memdbgon.h"

using namespace vgui;

CTASPanel *g_pTASPanel = nullptr;

static MAKE_CONVAR(mom_tas_max_vispredmove_ticks, "0", FCVAR_NONE, "Ticks to visualize predicting movements.", 0,
                   INT_MAX);

static MAKE_CONVAR(mom_tas_vispredmove, "0", FCVAR_NONE,
                   "0: Disabled, 1: Visualize predicted movements: stop when landing,"
                   "2: Visualize predicted movements by chosen ticks (mom_tas_max_vispredmove_ticks)\n",
                   0, 2);

CON_COMMAND(mom_tas_panel, "Toggle TAS panel")
{
    if (g_pClientMode != nullptr && engine->IsInGame())
    {
        if (g_pTASPanel == nullptr)
            g_pTASPanel = new CTASPanel();

        g_pTASPanel->ToggleVisible();
    }
    else
    {
        if (g_pTASPanel != nullptr)
            delete g_pTASPanel;
    }
}

CTASPanel::CTASPanel() : BaseClass(g_pClientMode->GetViewport(), "TASPanel")
{
    SetSize(2, 2);
    SetProportional(false);
    SetScheme("ClientScheme");
    SetMouseInputEnabled(true);
    SetSizeable(false);
    SetClipToParent(true); // Needed so we won't go out of bounds

    surface()->CreatePopup(GetVPanel(), false, false, false, true, false);

    LoadControlSettings("resource/ui/TASPanel.res");

    m_pEnableTASMode = FindControl<ToggleButton>("EnableTASMode");

    SetVisible(false);
    SetTitle(L"TAS Panel", true);
}

CTASPanel::~CTASPanel() {}

void CTASPanel::RunVPM(CBasePlayer *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    static int iCurrentTickCount = 0;

    float flMaxTime = mom_tas_max_vispredmove_ticks.GetFloat() * gpGlobals->interval_per_tick;

    // Saves fps for simulation.
    if (iCurrentTickCount != gpGlobals->tick_count)
    {
        m_flOldCurtime = gpGlobals->curtime;
        m_flOldFrametime = gpGlobals->frametime;

        gpGlobals->curtime = pPlayer->GetTickBase() * gpGlobals->interval_per_tick;
        gpGlobals->frametime = gpGlobals->interval_per_tick;

        m_flVPMTime = 0.0f;

        while (m_flVPMTime <= flMaxTime)
        {
            gpGlobals->curtime += m_flVPMTime;

            pPlayer->m_bSimulatingMovements = true;

            // Process last movement data we know.
            // This might be changed because it can more accurate during jumps.
            // Imagine having a circle around you wich says you where is the best speed gain and where you will land at
            // the same time.. That would be my feature project I guess.
            g_pGameMovement->ProcessMovement(pPlayer, g_pMoveData->GetMoveData());

            pPlayer->m_bSimulatingMovements = false;

            m_flVPMTime += gpGlobals->frametime;
        }

        gpGlobals->curtime = m_flOldCurtime;
        gpGlobals->frametime;
    }

    iCurrentTickCount = gpGlobals->tick_count;
}

void CTASPanel::VisPredMovements()
{

    if (mom_tas_vispredmove.GetInt() > 0)
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());

        if (mom_tas_vispredmove.GetInt() == 1)
        {
            m_bStopVPM_OnGround = false;
        }
        else
        {
            m_bStopVPM_OnGround = true;
        }
        RunVPM(pPlayer);
    }
}

void CTASPanel::OnThink()
{
    int x, y;
    input()->GetCursorPosition(x, y);
    SetKeyBoardInputEnabled(IsWithin(x, y));

    CMomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        m_pEnableTASMode->SetText((pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS) ? "#MOM_EnabledTASMode"
                                                                                           : "#MOM_DisabledTASMode");
        m_pEnableTASMode->SetSelected(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
    }

    BaseClass::OnThink();
}

void CTASPanel::OnCommand(const char *pcCommand)
{
    BaseClass::OnCommand(pcCommand);

    if (!Q_strcasecmp(pcCommand, "enabletasmode"))
    {
        engine->ServerCmd("mom_tas");
    }
}

void CTASPanel::ToggleVisible()
{
    m_bToggleVisible = !IsVisible();

    if (m_bToggleVisible)
    {
        // Center the mouse in the panel
        int x, y, w, h;
        GetBounds(x, y, w, h);
        input()->SetCursorPos(x + (w / 2), y + (h / 2));
    }

    SetVisible(m_bToggleVisible);
}
