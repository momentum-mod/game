// clang-format off
#include "cbase.h"

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CvarToggleCheckButton.h>

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IInput.h>

#include "hud_mapfinished.h"
#include "mom_player_shared.h"
#include "mom_replayui.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"
#include "momSpectatorGUI.h"
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

// clang-format on

using namespace vgui;

CTASPanel *vgui::g_pTASPanel = nullptr;

static MAKE_CONVAR(mom_tas_max_vispredmove_ticks, "250", FCVAR_ARCHIVE, "Ticks to visualize predicting movements.", 0,
                   INT_MAX);

static MAKE_CONVAR(mom_tas_max_vispredmove_color, "FFFFFFFF", FCVAR_ARCHIVE,
                   "Color of lines to visualize predicting movements.", 0, INT_MAX);

static MAKE_CONVAR(mom_tas_vispredmove, "1", FCVAR_ARCHIVE,
                   "0: Disabled, 1: Visualize predicted movements: stop when landing,"
                   "2: Visualize predicted movements by chosen ticks (mom_tas_max_vispredmove_ticks)\n",
                   0, 2);

CON_COMMAND(mom_tas_panel, "Toggle TAS panel")
{
    if (engine->IsInGame() && g_pClientMode && g_pClientMode->GetViewport() != nullptr)
    {
        if (g_pTASPanel == nullptr)
            g_pTASPanel = new CTASPanel();

        g_pTASPanel->ToggleVisible();
    }
}

CTASPanel::CTASPanel()
    : BaseClass(g_pClientMode->GetViewport(), "TASPanel"), m_pVisualPanel(nullptr), m_pReplayUI(nullptr),
      mom_tas_autostrafe("mom_tas_autostrafe")
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
    m_pToggleReplayUI = FindControl<ToggleButton>("ToggleReplayUI");
    m_pVisPredMove = FindControl<ToggleButton>("VisPredMove");
    m_pAutostrafe = FindControl<ToggleButton>("Autostrafe");

    SetVisible(false);
    SetTitle(L"TAS Panel", true);

    m_pVisualPanel = new CTASVisPanel();
    m_pReplayUI = new C_TASMOMReplayUI();
}

CTASPanel::~CTASPanel()
{
    // Seems to make segfaults
    /*if (m_pVisualPanel != nullptr)
    {
        delete m_pVisualPanel;
        m_pVisualPanel = nullptr;
    }

    if (m_pReplayUI != nullptr)
    {
        delete m_pReplayUI;
        m_pReplayUI = nullptr;
    }*/
}

void CTASPanel::OnThink()
{
    int x, y;
    input()->GetCursorPosition(x, y);
    SetKeyBoardInputEnabled(IsWithin(x, y));

    C_MomentumPlayer *pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        static bool wasTASMode;

        if (wasTASMode != (pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS))
        {
            m_pEnableTASMode->SetText((pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS)
                                          ? "#MOM_EnabledTASMode"
                                          : "#MOM_DisabledTASMode");
            m_pEnableTASMode->SetSelected(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
            m_pEnableTASMode->SetArmed(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
            m_pEnableTASMode->ForceDepressed(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
        }

        wasTASMode = pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS;

        static bool bDoOnce = true;
        if (pPlayer->m_SrvData.m_RunData.m_bMapFinished)
        {
            m_pVisualPanel->SetVisible(false);
            bDoOnce = false;
        }
        else
        {
            if (!bDoOnce)
            {
                m_pVisualPanel->SetVisible(true);
                bDoOnce = true;
            }
        }
    }

    if (m_pToggleReplayUI->IsSelected())
    {
        m_pReplayUI->ShowPanel(true);
    }
    else
    {
        m_pReplayUI->ShowPanel(false);
    }

    static bool bAutoStrafe, bVisPredMove;

    if (bAutoStrafe != mom_tas_autostrafe.GetBool())
    {
        m_pAutostrafe->SetSelected(mom_tas_autostrafe.GetBool());
        m_pAutostrafe->SetArmed(mom_tas_autostrafe.GetBool());
        m_pAutostrafe->ForceDepressed(mom_tas_autostrafe.GetBool());
    }

    bAutoStrafe = mom_tas_autostrafe.GetBool();

    if (bVisPredMove != mom_tas_vispredmove.GetBool())
    {
        m_pVisPredMove->SetSelected(mom_tas_vispredmove.GetBool());
        m_pVisPredMove->SetArmed(mom_tas_vispredmove.GetBool());
        m_pVisPredMove->ForceDepressed(mom_tas_vispredmove.GetBool());
    }

    bVisPredMove = mom_tas_vispredmove.GetBool();

    BaseClass::OnThink();
}

void CTASPanel::OnCommand(const char *pcCommand)
{
    BaseClass::OnCommand(pcCommand);

    if (!Q_strcasecmp(pcCommand, "enabletasmode"))
    {
        engine->ServerCmd("mom_tas");
    }
    else if (!Q_strcasecmp(pcCommand, "vispredmove"))
    {
        mom_tas_vispredmove.SetValue(!mom_tas_vispredmove.GetBool());
    }
    else if (!Q_strcasecmp(pcCommand, "autostrafe"))
    {
        mom_tas_autostrafe.SetValue(!mom_tas_autostrafe.GetBool());
    }
}

void CTASPanel::ToggleVisible()
{
    // Center the mouse in the panel
    int x, y, w, h;
    GetBounds(x, y, w, h);
    input()->SetCursorPos(x + (w / 2), y + (h / 2));
    SetVisible(true);
    engine->ServerCmd("mom_tas_pause");
}

void CTASPanel::SetVisible(bool state)
{
    if (!state && m_pReplayUI != nullptr)
        m_pReplayUI->ShowPanel(false);

    if (!state)
        engine->ClientCmd("mom_tas_record");

    BaseClass::SetVisible(state);
}

void CTASVisPanel::RunVPM(C_MomentumPlayer *pPlayer)
{
    static int iOldTickCount = 0;

    if (pPlayer != nullptr)
    {
        SetVisible(pPlayer->m_SrvData.m_RunData.m_iRunFlags & RUNFLAG_TAS);
    }

    if (!IsVisible())
        return;

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

CTASVisPanel::CTASVisPanel() : BaseClass(nullptr, "tasvisgui")
{
    SetParent(g_pClientMode->GetViewport());
    SetVisible(true);
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);
}

CTASVisPanel::~CTASVisPanel() {}

C_TASMOMReplayUI::C_TASMOMReplayUI() : Frame(g_pClientMode->GetViewport(), "TASReplayControls")
{
    SetVisible(false);
    SetProportional(false);
    SetMoveable(true);
    SetSizeable(false);
    SetMaximizeButtonVisible(false);
    SetMinimizeButtonVisible(false);
    SetMenuButtonResponsive(false);
    SetMouseInputEnabled(true);
    SetClipToParent(true); // Needed so we won't go out of bounds
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    SetScheme("ClientScheme");
    LoadControlSettings("resource/ui/TASReplayUI.res");

    m_pPlayPauseResume = FindControl<ToggleButton>("ReplayPlayPauseResume");

    m_pGoStart = FindControl<Button>("ReplayGoStart");
    m_pGoEnd = FindControl<Button>("ReplayGoEnd");
    m_pPrevFrame = FindControl<Button>("ReplayPrevFrame");
    m_pNextFrame = FindControl<Button>("ReplayNextFrame");
    m_pFastForward = FindControl<PFrameButton>("ReplayFastForward");
    m_pFastBackward = FindControl<PFrameButton>("ReplayFastBackward");
    m_pGo = FindControl<Button>("ReplayGo");

    m_pGotoTick = FindControl<TextEntry>("ReplayGoToTick");

    m_pTimescaleSlider = FindControl<CvarSlider>("TimescaleSlider");
    m_pTimescaleLabel = FindControl<Label>("TimescaleLabel");
    m_pTimescaleEntry = FindControl<TextEntry>("TimescaleEntry");
    SetLabelText();

    m_pProgress = FindControl<ScrubbableProgressBar>("ReplayProgress");

    m_pProgressLabelFrame = FindControl<Label>("ReplayProgressLabelFrame");
    m_pProgressLabelTime = FindControl<Label>("ReplayProgressLabelTime");

    FIND_LOCALIZATION(m_pwReplayTime, "#MOM_ReplayTime");
    FIND_LOCALIZATION(m_pwReplayTimeTick, "#MOM_ReplayTimeTick");

    m_pPlayPauseResume->SetText("Go!");
}

void C_TASMOMReplayUI::OnThink()
{
    BaseClass::OnThink();

    // HACKHACK for focus, Blame Valve
    int x, y;
    input()->GetCursorPosition(x, y);
    SetKeyBoardInputEnabled(IsWithin(x, y));

    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        int iPlayButtonSelected = RUI_NOTHING;

        if (m_pFastBackward->IsSelected() || m_pFastForward->IsSelected())
        {
            iPlayButtonSelected = m_pFastBackward->IsSelected() ? RUI_MOVEBW : RUI_MOVEFW;
        }

        if (m_pPlayPauseResume->IsSelected())
        {
            m_pPlayPauseResume->ForceDepressed(false);
            m_pPlayPauseResume->SetSelected(false);
            m_pPlayPauseResume->SetArmed(false);
        }

        // We need to do it only once
        if (m_iPlayButtonSelected != iPlayButtonSelected)
        {
            char format[32];
            Q_snprintf(format, sizeof format, "mom_tas_selection %i", iPlayButtonSelected);
            engine->ClientCmd(format);
            m_iPlayButtonSelected = iPlayButtonSelected;
        }

        m_iTotalDuration = pPlayer->m_SrvData.m_tasData.m_iTotalTimeTicks;
        int iCurrentTick = pPlayer->m_SrvData.m_tasData.m_iCurrentTick - 1;
        // Set overall progress
        float fProgress = static_cast<float>(iCurrentTick) / static_cast<float>(m_iTotalDuration);
        fProgress = clamp<float>(fProgress, 0.0f, 1.0f);
        m_pProgress->SetProgress(fProgress);

        bool negativeTime = iCurrentTick < pPlayer->m_SrvData.m_RunData.m_iStartTickD;
        // Print "Tick: %i / %i"
        wchar_t wLabelFrame[BUFSIZELOCL];
        V_snwprintf(wLabelFrame, BUFSIZELOCL, m_pwReplayTimeTick, iCurrentTick, m_iTotalDuration);
        m_pProgressLabelFrame->SetText(wLabelFrame);

        // Print "Time: X:XX.XX -> X:XX.XX"
        char curtime[BUFSIZETIME], totaltime[BUFSIZETIME];
        wchar_t wCurtime[BUFSIZETIME], wTotaltime[BUFSIZETIME];
        // Get the times
        g_pMomentumUtil->FormatTime(
            TICK_INTERVAL * (pPlayer->m_SrvData.m_tasData.m_iCurrentTick - pPlayer->m_SrvData.m_RunData.m_iStartTickD),
            curtime, 2, false, negativeTime);
        g_pMomentumUtil->FormatTime(TICKS_TO_TIME(m_iTotalDuration), totaltime, 2);
        // Convert to Unicode
        ANSI_TO_UNICODE(curtime, wCurtime);
        ANSI_TO_UNICODE(totaltime, wTotaltime);
        wchar_t pwTime[BUFSIZELOCL];
        g_pVGuiLocalize->ConstructString(pwTime, sizeof(pwTime), m_pwReplayTime, 2, wCurtime, wTotaltime);
        // V_snwprintf(pwTime, BUFSIZELOCL, m_pwReplayTime, wCurtime, wTotaltime);
        m_pProgressLabelTime->SetText(pwTime);
    }
}

void C_TASMOMReplayUI::OnControlModified(Panel *p)
{
    if (p == m_pTimescaleSlider && m_pTimescaleSlider->HasBeenModified())
    {
        SetLabelText();
    }
}

void C_TASMOMReplayUI::OnTextChanged(Panel *p)
{
    if (p == m_pTimescaleEntry)
    {
        char buf[64];
        m_pTimescaleEntry->GetText(buf, 64);

        float fValue = float(atof(buf));
        if (fValue >= 0.01f && fValue <= 10.0f)
        {
            m_pTimescaleSlider->SetSliderValue(fValue);
            m_pTimescaleSlider->ApplyChanges();
        }
    }
}

void C_TASMOMReplayUI::OnNewProgress(float scale)
{
    int tickToGo = static_cast<int>(scale * m_iTotalDuration);
    if (tickToGo > -1 && tickToGo <= m_iTotalDuration)
    {
        engine->ServerCmd(VarArgs("mom_tas_goto %i", tickToGo));
    }
}

void C_TASMOMReplayUI::OnPBMouseWheeled(int delta) { OnCommand(delta > 0 ? "nextframe" : "prevframe"); }

void C_TASMOMReplayUI::SetLabelText() const
{
    if (m_pTimescaleSlider && m_pTimescaleEntry)
    {
        char buf[64];
        Q_snprintf(buf, sizeof(buf), "%.1f", m_pTimescaleSlider->GetSliderValue());
        m_pTimescaleEntry->SetText(buf);

        m_pTimescaleSlider->ApplyChanges();
    }
}

void C_TASMOMReplayUI::ShowPanel(bool state)
{
    SetVisible(state);
    SetMouseInputEnabled(state);
}

// Command issued
void C_TASMOMReplayUI::OnCommand(const char *command)
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (!Q_strcasecmp(command, "play"))
    {
        engine->ClientCmd("mom_tas_record");
        g_pTASPanel->SetVisible(false);
    }
    else if (!Q_strcasecmp(command, "reload"))
    {
        engine->ServerCmd("mom_tas_restart");
    }
    else if (!Q_strcasecmp(command, "gotoend"))
    {
        engine->ServerCmd("mom_tas_goto_end");
    }
    else if (!Q_strcasecmp(command, "prevframe") && pPlayer)
    {
        if (pPlayer->m_SrvData.m_tasData.m_iCurrentTick > 0)
        {
            engine->ServerCmd(VarArgs("mom_tas_goto %i", pPlayer->m_SrvData.m_tasData.m_iCurrentTick - 1));
        }
    }
    else if (!Q_strcasecmp(command, "nextframe") && pPlayer)
    {
        if (pPlayer->m_SrvData.m_tasData.m_iCurrentTick < pPlayer->m_SrvData.m_tasData.m_iTotalTimeTicks)
        {
            engine->ServerCmd(VarArgs("mom_tas_goto %i", pPlayer->m_SrvData.m_tasData.m_iCurrentTick + 1));
        }
    }
    else if (!Q_strcasecmp(command, "gototick") && pPlayer)
    {
        // Teleport at the position we want with timer included
        char tick[32];
        m_pGotoTick->GetText(tick, sizeof(tick));
        engine->ServerCmd(VarArgs("mom_tas_goto %s", tick));
        m_pGotoTick->SetText("");
    }
    else if (FStrEq("close", command))
    {
        Close();
        SetVisible(false);
        g_pTASPanel->m_pToggleReplayUI->ForceDepressed(false);
        g_pTASPanel->m_pToggleReplayUI->SetSelected(false);
        g_pTASPanel->m_pToggleReplayUI->SetArmed(false);
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}