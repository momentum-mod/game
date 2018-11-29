// clang-format off
#include "cbase.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CVarSlider.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "ColorPicker.h"
#include "c_mom_player.h"
#include "mom_gamemovement.h"
#include "mom_shareddefs.h"
#include "prediction.h"
#include "clientmode_shared.h"
#include "debugoverlay_shared.h"
#include "materialsystem/imaterialvar.h"
#include "util/mom_util.h"
#include "util_shared.h"
#include "weapon/weapon_csbase.h"
#include "TASPanel.h"
#include "tier2/renderutils.h"

#include "tier0/memdbgon.h"

extern bool ScreenTransform(const Vector&, Vector&);

// clang-format on

using namespace vgui;

CTASPanel *vgui::g_pTASPanel = nullptr;


static MAKE_CONVAR(mom_tas_autostrafe, "1", FCVAR_NONE, "1) Only in air 2) On ground + air.", 0,
                   2);

static MAKE_CONVAR(mom_tas_max_vispredmove_ticks, "250", FCVAR_NONE, "Ticks to visualize predicting movements.", 0,
                   INT_MAX);

static MAKE_CONVAR(mom_tas_max_vispredmove_color, "FFFFFFFF", FCVAR_NONE,
                   "Color of lines to visualize predicting movements.", 0, INT_MAX);

static MAKE_CONVAR(mom_tas_vispredmove, "1", FCVAR_NONE,
                   "0: Disabled, 1: Visualize predicted movements: stop when landing,"
                   "2: Visualize predicted movements by chosen ticks (mom_tas_max_vispredmove_ticks)\n",
                   0, 2);

CON_COMMAND(mom_tas_panel, "Toggle TAS panel")
{
    if (engine->IsInGame())
    {
        if (g_pTASPanel == nullptr)
            g_pTASPanel = new CTASPanel();

        g_pTASPanel->ToggleVisible();
    }
}

CTASPanel::CTASPanel() : BaseClass(g_pClientMode->GetViewport(), "TASPanel"), m_pVisualPanel(nullptr)
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

    m_pVisualPanel = new CTASVisPanel();
}

CTASPanel::~CTASPanel() {}

void CTASPanel::OnThink()
{
    int x, y;
    input()->GetCursorPosition(x, y);
    SetKeyBoardInputEnabled(IsWithin(x, y));

    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        m_pEnableTASMode->SetText((pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS) ? "#MOM_EnabledTASMode"
                                                                                           : "#MOM_DisabledTASMode");
        m_pEnableTASMode->SetSelected(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
        m_pEnableTASMode->SetArmed(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
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
    // Center the mouse in the panel
    int x, y, w, h;
    GetBounds(x, y, w, h);
    input()->SetCursorPos(x + (w / 2), y + (h / 2));
    SetVisible(true);
}

void CTASVisPanel::RunVPM(C_MomentumPlayer *pPlayer)
{
    static int iOldTickCount = 0;

    if (iOldTickCount != gpGlobals->tickcount)
    {
        if (mom_tas_vispredmove.GetInt() <= 0 || pPlayer == nullptr)
            return;

        if (!(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS))
            return;

        bool bWasPredictable = true;
        if (!pPlayer->GetPredictable())
        {
            pPlayer->InitPredictable();
            bWasPredictable = false;
        }

        pPlayer->SaveData("Simulation", C_BaseEntity::SLOT_ORIGINALDATA, PC_EVERYTHING);

        float flMaxTime = mom_tas_max_vispredmove_ticks.GetInt() * gpGlobals->interval_per_tick;

        m_vecOrigins.RemoveAll();

        m_flVPMTime = 0.0f;

        bool bFirstTimePredicted = prediction->m_bFirstTimePredicted, bInPred = prediction->m_bInPrediction;

        // Remove any sounds.
        prediction->m_bFirstTimePredicted = false;
        prediction->m_bInPrediction = true;

        static ConVarRef sv_footsteps("sv_footsteps");

        int backup_sv_footsteps = sv_footsteps.GetInt();
        sv_footsteps.SetValue("0");

        g_pMomentumGameMovement->StartTrackPredictionErrors(pPlayer);
        prediction->StartCommand(pPlayer, &pPlayer->m_LastCreateMoveCmd);

        float flOldCurtime, flOldFrametime;

        flOldCurtime = gpGlobals->curtime;
        flOldFrametime = gpGlobals->frametime;

        gpGlobals->curtime = pPlayer->GetTimeBase();
        gpGlobals->frametime = gpGlobals->interval_per_tick;

        while (m_flVPMTime <= flMaxTime)
        {
            // Process last movement data we know.
            // This might be changed because it can more accurate during jumps.
            // Imagine having a circle around you wich says you where is the best speed gain and where you will land at
            // the same time.. That would be my feature project I guess.

            gpGlobals->curtime = flOldCurtime + m_flVPMTime;

            // If we want this without bugs we must have a proper ProcessMovement wich copies exactly the server's
            // movement!
            // prediction->SetupMove(pPlayer, &pPlayer->m_LastCreateMoveCmd, MoveHelper(), g_pMoveData);
            // g_pMomentumGameMovement->ProcessMovement(pPlayer, g_pMoveData);
            // prediction->FinishMove(pPlayer, &pPlayer->m_LastCreateMoveCmd, g_pMoveData);
            prediction->RunCommand(pPlayer, &pPlayer->m_LastCreateMoveCmd, MoveHelper());
            // Invalidate physics so the player will update its absolute velocity/angles/position/animation on setupmove
            pPlayer->InvalidatePhysicsRecursive(VELOCITY_CHANGED | POSITION_CHANGED | ANGLES_CHANGED |
                                                ANIMATION_CHANGED);

            m_flVPMTime += gpGlobals->frametime;

            // Let's limit the rendering precision so we can eat less fps.
            m_vecOrigins.AddToHead(g_pMoveData->GetAbsOrigin());

            if (pPlayer->m_hGroundEntity != nullptr && mom_tas_vispredmove.GetInt() == 1)
            {
                break;
            }
        }

        sv_footsteps.SetValue(backup_sv_footsteps);

        gpGlobals->curtime = flOldCurtime;
        gpGlobals->frametime = flOldFrametime;

        g_pMomentumGameMovement->FinishTrackPredictionErrors(pPlayer);
        prediction->FinishCommand(pPlayer);
        prediction->m_bFirstTimePredicted = bFirstTimePredicted;
        prediction->m_bInPrediction = bInPred;

        pPlayer->RestoreData("Simulation", C_BaseEntity::SLOT_ORIGINALDATA, PC_EVERYTHING);

        if (!bWasPredictable)
            pPlayer->ShutdownPredictable();
    }

    iOldTickCount = gpGlobals->tickcount;
}

void CTASVisPanel::Paint()
{
    int x, y;
    engine->GetScreenSize(x, y);
    SetSize(x, y);

    Color c;
    g_pMomentumUtil->GetColorFromHex(mom_tas_max_vispredmove_color.GetString(), c);
    surface()->DrawSetColor(c);

    if (m_vecOrigins.Count() > 1)
    {
        for (int i = 1; i < m_vecOrigins.Count(); i++)
        {
            // RenderLine(m_vecOrigins[i - 1], m_vecOrigins[i], Color(255, 255, 255, 255), true);
            Vector vecFirst, vecSecond;

            if (!debugoverlay->ScreenPosition(m_vecOrigins[i - 1], vecFirst) &&
                !debugoverlay->ScreenPosition(m_vecOrigins[i], vecSecond))
            {
                surface()->DrawLine(vecFirst.x, vecFirst.y, vecSecond.x, vecSecond.y);
            }
        }
    }

    BaseClass::Paint();
}

void CTASVisPanel::VisPredMovements() { RunVPM(ToCMOMPlayer(C_BasePlayer::GetLocalPlayer())); }

CTASVisPanel::CTASVisPanel() : BaseClass(g_pClientMode->GetViewport(), "tasvisgui")
{
    SetVisible(true);
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);
}

CTASVisPanel::~CTASVisPanel() {}
