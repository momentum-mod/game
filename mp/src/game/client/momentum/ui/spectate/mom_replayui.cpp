#include "cbase.h"

#include "vgui_controls/ToggleButton.h"
#include "controls/PFrameButton.h"
#include "vgui_controls/CvarTextEntry.h"
#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/ScrubbableProgressBar.h"

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IInput.h>

#include "hud_mapfinished.h"
#include "mom_spectator_gui.h"
#include "mom_player_shared.h"
#include "mom_replayui.h"
#include "momentum/util/mom_util.h"
#include "c_mom_replay_entity.h"
#include "fmtstr.h"

#include <tier0/memdbgon.h>

using namespace vgui;

C_MOMReplayUI::C_MOMReplayUI(IViewPort *pViewport) : Frame(nullptr, PANEL_REPLAY, false, false)
{
    m_pViewport = pViewport;

    SetProportional(true);
    SetMoveable(true);
    SetSizeable(false);
    SetMaximizeButtonVisible(false);
    SetMinimizeButtonVisible(false);
    SetMenuButtonVisible(false);
    SetMenuButtonResponsive(false);
    SetClipToParent(true); // Needed so we won't go out of bounds

    ListenForGameEvent("mapfinished_panel_closed");

    m_iTotalDuration = 0;
    m_iPlayButtonSelected = RUI_NOTHING;
    m_bWasVisible = false;
    m_bWasClosed = false;

    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    SetScheme("ClientScheme");

    m_pPlayPauseResume = new ToggleButton(this, "ReplayPlayPauseResume", "#MOM_ReplayStatusPlaying");
    m_pPlayPauseResume->AddActionSignalTarget(this);

    m_pGoStart = new Button(this, "ReplayGoStart", "|<", this, "reload");
    m_pGoEnd = new Button(this, "ReplayGoEnd", ">|", this, "gotoend");
    m_pPrevFrame = new Button(this, "ReplayPrevFrame", "<", this, "prevframe");
    m_pNextFrame = new Button(this, "ReplayNextFrame", ">", this, "nextframe");
    m_pFastForward = new PFrameButton(this, "ReplayFastForward", ">>");
    m_pFastForward->AddActionSignalTarget(this);
    m_pFastBackward = new PFrameButton(this, "ReplayFastBackward", "<<");
    m_pFastBackward->AddActionSignalTarget(this);
    m_pGo = new Button(this, "ReplayGo", "#MOM_ReplayGoto", this, "gototick");

    m_pGotoTick = new TextEntry(this, "ReplayGoToTick");
    m_pGotoTick->SetAllowNumericInputOnly(true);
    m_pGotoTick->AddActionSignalTarget(this);

    m_pTimescaleSlider = new CvarSlider(this, "TimescaleSlider", nullptr, 0.1f, 10.0f, "mom_replay_timescale", false, true);
    m_pTimescaleSlider->AddActionSignalTarget(this);
    m_pTimescaleLabel = new Label(this, "TimescaleLabel", "#MOM_ReplayTimescale");
    m_pTimescaleEntry = new CvarTextEntry(this, "TimescaleEntry", "mom_replay_timescale", "%.1f");
    m_pTimescaleEntry->SetAllowNumericInputOnly(true);
    m_pTimescaleEntry->AddActionSignalTarget(this);

    m_pProgress = new ScrubbableProgressBar(this, "ReplayProgress");
    m_pProgress->AddActionSignalTarget(this);

    m_pProgressLabelFrame = new Label(this, "ReplayProgressLabelFrame", "");
    m_pProgressLabelTime = new Label(this, "ReplayProgressLabelTime", "");

    LoadControlSettings("resource/ui/ReplayUI.res");

    SetVisible(false);
    SetBounds(20, 100, GetScaledVal(280), GetScaledVal(150));
    SetTitle("#MOM_ReplayControls", true);

    m_pSpecGUI = nullptr;
}

void C_MOMReplayUI::OnThink()
{
    BaseClass::OnThink();

    // HACKHACK for focus, Blame Valve
    int x, y;
    input()->GetCursorPosition(x, y);
    const bool bWithin = IsWithin(x, y);
    if (bWithin)
    {
        const auto mouseOver = input()->GetMouseOver();
        SetKeyBoardInputEnabled(mouseOver == GetVPanel() ||
                                mouseOver == m_pGotoTick->GetVPanel() ||
                                mouseOver == m_pTimescaleEntry->GetVPanel());
    }
    else
    {
        SetKeyBoardInputEnabled(false);
    }

    if (!IsMouseInputEnabled() && bWithin)
    {
        SetMouseInputEnabled(true); 
        if (!m_pSpecGUI)
            m_pSpecGUI = static_cast<CMOMSpectatorGUI*>(m_pViewport->FindPanelByName(PANEL_SPECGUI));
        if (m_pSpecGUI && !m_pSpecGUI->IsMouseInputEnabled())
            m_pSpecGUI->SetMouseInputEnabled(true);
    }

    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        const auto pEnt = pPlayer->GetCurrentUIEntity();
        if (pEnt->GetEntType() == RUN_ENT_REPLAY)
        {

            const auto pGhost = static_cast<C_MomentumReplayGhostEntity*>(pEnt);
            int iPlayButtonSelected = RUI_NOTHING;

            if (m_pFastBackward->IsSelected() || m_pFastForward->IsSelected())
            {
                iPlayButtonSelected = m_pFastBackward->IsSelected() ? RUI_MOVEBW : RUI_MOVEFW;

                if (!pGhost->m_bIsPaused)
                    engine->ClientCmd("mom_replay_pause");

                m_pPlayPauseResume->ForceDepressed(false);
            }

            // We need to do it only once
            if (m_iPlayButtonSelected != iPlayButtonSelected)
            {
                char format[32];
                Q_snprintf(format, sizeof format, "mom_replay_selection %i", iPlayButtonSelected);
                engine->ClientCmd(format);
                m_iPlayButtonSelected = iPlayButtonSelected;
            }

            if (!pGhost->m_bIsPaused && !m_pPlayPauseResume->IsArmed())
                m_pPlayPauseResume->SetArmed(true);

            m_pPlayPauseResume->SetSelected(!pGhost->m_bIsPaused);
            m_pPlayPauseResume->SetText(pGhost->m_bIsPaused ? "#MOM_ReplayStatusPaused" : "#MOM_ReplayStatusPlaying");

            m_iTotalDuration = pGhost->m_iTotalTicks - (TIME_TO_TICKS(END_RECORDING_DELAY));

            // Set overall progress
            float fProgress = static_cast<float>(pGhost->m_iCurrentTick) / static_cast<float>(m_iTotalDuration);
            fProgress = clamp<float>(fProgress, 0.0f, 1.0f);
            m_pProgress->SetProgress(fProgress);

            bool negativeTime = pGhost->m_iCurrentTick < pGhost->m_Data.m_iStartTick;
            // Print "Tick: %i / %i"
            m_pProgressLabelFrame->SetText(CConstructLocalizedString(m_pwReplayTimeTick, pGhost->m_iCurrentTick.Get(), m_iTotalDuration));

            // Print "Time: X:XX.XX -> X:XX.XX"
            char curtime[BUFSIZETIME], totaltime[BUFSIZETIME];
            wchar_t wCurtime[BUFSIZETIME], wTotaltime[BUFSIZETIME];
            // Get the times
            MomUtil::FormatTime(TICK_INTERVAL * (pGhost->m_iCurrentTick - pGhost->m_Data.m_iStartTick), curtime, 2,
                                 false, negativeTime);
            MomUtil::FormatTime(float(pGhost->m_Data.m_iRunTime) * pGhost->m_Data.m_flTickRate, totaltime, 2);
            // Convert to Unicode
            ANSI_TO_UNICODE(curtime, wCurtime);
            ANSI_TO_UNICODE(totaltime, wTotaltime);
            m_pProgressLabelTime->SetText(CConstructLocalizedString(m_pwReplayTime, wCurtime, wTotaltime));

            if (pGhost->m_Data.m_bMapFinished)
            {
                // Hide the panel on map finish, but show it afterwards
                m_bWasVisible = true; // Note: This OnThink will never happen if this panel is not drawn!
                ShowPanel(false);
            }
        }
        else
        {
            ShowPanel(false);
        }
    }
}

void C_MOMReplayUI::OnNewProgress(float scale)
{
    int tickToGo = static_cast<int>(scale * m_iTotalDuration);
    if (tickToGo > -1 && tickToGo <= m_iTotalDuration)
    {
        engine->ServerCmd(VarArgs("mom_replay_goto %i", tickToGo));
    }
}

void C_MOMReplayUI::OnPBMouseWheeled(int delta) { OnCommand(delta > 0 ? "nextframe" : "prevframe"); }

void C_MOMReplayUI::SetWasClosed(bool bWasClosed)
{
    m_bWasClosed = bWasClosed;
}

bool C_MOMReplayUI::WasClosed() const
{
    return m_bWasClosed;
}

void C_MOMReplayUI::ShowPanel(bool state)
{
    SetVisible(state);
    const auto pSpecUI = gViewPortInterface->FindPanelByName(PANEL_SPECGUI);
    if (pSpecUI && pSpecUI->IsVisible() && ipanel()->IsMouseInputEnabled(pSpecUI->GetVPanel()))
        SetMouseInputEnabled(true);
    if (state)
    {
        FIND_LOCALIZATION(m_pwReplayTime, "#MOM_ReplayTime");
        FIND_LOCALIZATION(m_pwReplayTimeTick, "#MOM_ReplayTimeTick");
        MoveToFront();
    }
    m_bWasClosed = false;
}

void C_MOMReplayUI::FireGameEvent(IGameEvent *pEvent)
{
    if (pEvent->GetBool("restart"))
        ShowPanel(m_bWasVisible);
    else
        m_bWasVisible = false;
}

// Command issued
void C_MOMReplayUI::OnCommand(const char *command)
{
    if (FStrEq(command, "play"))
    {
        engine->ClientCmd("mom_replay_pause"); // Handles the toggle state
    }
    else if (FStrEq(command, "reload"))
    {
        engine->ServerCmd("mom_replay_restart");
    }
    else if (FStrEq(command, "gotoend"))
    {
        engine->ServerCmd("mom_replay_goto_end");
    }
    else if (FStrEq(command, "prevframe") || FStrEq(command, "nextframe"))
    {
        const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
        if (pPlayer && pPlayer->GetCurrentUIEntity()->GetEntType() == RUN_ENT_REPLAY)
        {
            const bool bIsPrev = command[0] == 'p'; // Bleh, too lazy to do FStrEq again
            const auto pGhost = static_cast<C_MomentumReplayGhostEntity*>(pPlayer->GetCurrentUIEntity());
            if (bIsPrev)
            {
                if (pGhost->m_iCurrentTick > 0)
                {
                    engine->ServerCmd(CFmtStr("mom_replay_goto %i", pGhost->m_iCurrentTick - 1));
                }
            }
            else
            {
                if (pGhost->m_iCurrentTick < pGhost->m_iTotalTicks)
                {
                    engine->ServerCmd(CFmtStr("mom_replay_goto %i", pGhost->m_iCurrentTick + 1));
                }
            }
        }
    }
    else if (FStrEq(command, "gototick"))
    {
        // Teleport at the position we want with timer included
        char tick[32];
        m_pGotoTick->GetText(tick, sizeof(tick));
        engine->ServerCmd(CFmtStr("mom_replay_goto %s", tick));
        m_pGotoTick->SetText("");
    }
    else if (FStrEq("close", command))
    {
        m_bWasClosed = true;
        m_bWasVisible = false;
        Close();
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}
