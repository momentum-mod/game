#include "cbase.h"

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>

#include <vgui_controls/TextEntry.h>

#include "hud_mapfinished.h"
#include "mom_player_shared.h"
#include "mom_replayui.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"
#include "momspectatorgui.h"

C_MOMReplayUI::C_MOMReplayUI(IViewPort *pViewport) : Frame(nullptr, PANEL_REPLAY, false, false)
{
    m_pViewport = pViewport;

    SetProportional(false);
    SetMoveable(true);
    SetSizeable(false);
    SetVisible(false);
    SetMaximizeButtonVisible(false);
    SetMinimizeButtonVisible(false);
    SetMenuButtonResponsive(false);
    SetClipToParent(true); // Needed so we won't go out of bounds

    m_iTotalDuration = 0;

    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    SetScheme("ClientScheme");
    LoadControlSettings("resource/ui/ReplayUI.res");

    m_pPlayPauseResume = FindControl<ToggleButton>("ReplayPlayPauseResume");

    m_pGoStart = FindControl<Button>("ReplayGoStart");
    m_pGoEnd = FindControl<Button>("ReplayGoEnd");
    m_pPrevFrame = FindControl<Button>("ReplayPrevFrame");
    m_pNextFrame = FindControl<Button>("ReplayNextFrame");
    m_pFastForward = FindControl<PFrameButton>("ReplayFastForward");
    m_pFastBackward = FindControl<PFrameButton>("ReplayFastBackward");
    m_pGo = FindControl<Button>("ReplayGo");

    m_pGotoTick = FindControl<TextEntry>("ReplayGoToTick");

    m_pTimescaleSlider = FindControl<CCvarSlider>("TimescaleSlider");
    m_pTimescaleLabel = FindControl<Label>("TimescaleLabel");
    m_pTimescaleEntry = FindControl<TextEntry>("TimescaleEntry");
    SetLabelText();

    m_pProgress = FindControl<ScrubbableProgressBar>("ReplayProgress");

    m_pProgressLabelFrame = FindControl<Label>("ReplayProgressLabelFrame");
    m_pProgressLabelTime = FindControl<Label>("ReplayProgressLabelTime");

    FIND_LOCALIZATION(m_pwReplayTime, "#MOM_ReplayTime");
    FIND_LOCALIZATION(m_pwReplayTimeTick, "#MOM_ReplayTimeTick");
}

void C_MOMReplayUI::OnThink()
{
    BaseClass::OnThink();

    // HACKHACK for focus, Blame Valve
    int x, y;
    input()->GetCursorPosition(x, y);
    SetKeyBoardInputEnabled(IsWithin(x, y));

    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
        if (pGhost)
        {
            int RUI_HasSelected = RUI_NOTHING;
            static int OldRGUI_Selected = RUI_NOTHING;

            if (m_pFastBackward->IsSelected() || m_pFastForward->IsSelected())
            {
                RUI_HasSelected = m_pFastBackward->IsSelected() ? RUI_MOVEBW : RUI_MOVEFW;
                
                if (!pGhost->m_bIsPaused)
                    engine->ClientCmd("mom_replay_pause");

                m_pPlayPauseResume->ForceDepressed(false);
            }

            //We need to do it only once
            if (OldRGUI_Selected != RUI_HasSelected)
            {
                char format[32];
                sprintf(format, "mom_replay_selection %i", RUI_HasSelected);
                engine->ClientCmd(format);
            }
     
            OldRGUI_Selected = RUI_HasSelected;

            if (!pGhost->m_bIsPaused && !m_pPlayPauseResume->IsArmed())
                m_pPlayPauseResume->SetArmed(true);

            m_pPlayPauseResume->SetSelected(!pGhost->m_bIsPaused);
            m_pPlayPauseResume->SetText(pGhost->m_bIsPaused ? "#MOM_ReplayStatusPaused" : "#MOM_ReplayStatusPlaying");

            m_iTotalDuration = pGhost->m_iTotalTimeTicks - (1.0f / TICK_INTERVAL);  //subtract 1 second from total progress bar due to 1s buffer at end of replay

            // Set overall progress
            float fProgress = static_cast<float>(pGhost->m_iCurrentTick) / static_cast<float>(m_iTotalDuration);
            fProgress = clamp(fProgress, 0.0f, 1.0f);
            m_pProgress->SetProgress(fProgress);

            int currentTick = pGhost->m_iCurrentTick - pGhost->m_RunData.m_iStartTickD;
            bool negativeTime = pGhost->m_iCurrentTick < pGhost->m_RunData.m_iStartTickD;
            // Print "Tick: %i / %i"
            wchar_t wLabelFrame[BUFSIZELOCL];
            V_snwprintf(wLabelFrame, BUFSIZELOCL, m_pwReplayTimeTick, currentTick, m_iTotalDuration - pGhost->m_RunData.m_iStartTickD);
            m_pProgressLabelFrame->SetText(wLabelFrame);

            // Print "Time: X:XX.XX -> X:XX.XX"
            char curtime[BUFSIZETIME], totaltime[BUFSIZETIME];
            wchar_t wCurtime[BUFSIZETIME], wTotaltime[BUFSIZETIME];
            // Get the times
            mom_UTIL->FormatTime(TICK_INTERVAL * currentTick, curtime, 2, false, negativeTime);
            mom_UTIL->FormatTime(pGhost->m_RunData.m_flRunTime, totaltime, 2);
            // Conver to Unicode
            ANSI_TO_UNICODE(curtime, wCurtime);
            ANSI_TO_UNICODE(totaltime, wTotaltime);
            wchar_t pwTime[BUFSIZELOCL];
            V_snwprintf(pwTime, BUFSIZELOCL, m_pwReplayTime, wCurtime, wTotaltime);
            m_pProgressLabelTime->SetText(pwTime);

            if (pGhost->m_RunData.m_bMapFinished)
            {
                // Hide the panel on run finish
                ShowPanel(false);
            }
        }
    }
}

void C_MOMReplayUI::OnControlModified(Panel *p)
{
    if (p == m_pTimescaleSlider && m_pTimescaleSlider->HasBeenModified())
    {
        SetLabelText();
    }
}

void C_MOMReplayUI::OnTextChanged(Panel *p)
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

void C_MOMReplayUI::OnNewProgress(float scale)
{
    int tickToGo = static_cast<int>(scale * m_iTotalDuration);
    if (tickToGo > -1 && tickToGo <= m_iTotalDuration)
    {
        engine->ServerCmd(VarArgs("mom_replay_goto %i", tickToGo));
    }
}

void C_MOMReplayUI::OnPBMouseWheeled(int delta)
{
    OnCommand(delta > 0 ? "nextframe" : "prevframe");
}

void C_MOMReplayUI::SetLabelText() const
{
    if (m_pTimescaleSlider && m_pTimescaleEntry)
    {
        char buf[64];
        Q_snprintf(buf, sizeof(buf), "%.1f", m_pTimescaleSlider->GetSliderValue());
        m_pTimescaleEntry->SetText(buf);

        m_pTimescaleSlider->ApplyChanges();
    }
}

void C_MOMReplayUI::ShowPanel(bool state)
{
    SetVisible(state);
    SetMouseInputEnabled(state);
}

// Command issued
void C_MOMReplayUI::OnCommand(const char *command)
{
    if (!shared)
        return BaseClass::OnCommand(command);
    C_MomentumReplayGhostEntity *pGhost = ToCMOMPlayer(CBasePlayer::GetLocalPlayer())->GetReplayEnt();
    if (!Q_strcasecmp(command, "play"))
    {
        engine->ClientCmd("mom_replay_pause"); // Handles the toggle state
    }
    else if (!Q_strcasecmp(command, "reload"))
    {
        engine->ServerCmd("mom_replay_restart");
    }
    else if (!Q_strcasecmp(command, "gotoend"))
    {
        engine->ServerCmd("mom_replay_goto_end");
    }
    else if (!Q_strcasecmp(command, "prevframe") && pGhost)
    {
        if (pGhost->m_iCurrentTick > 0)
        {
            engine->ServerCmd(VarArgs("mom_replay_goto %i", pGhost->m_iCurrentTick - 1));
        }
    }
    else if (!Q_strcasecmp(command, "nextframe") && pGhost)
    {
        if (pGhost->m_iCurrentTick < pGhost->m_iTotalTimeTicks)
        {
            engine->ServerCmd(VarArgs("mom_replay_goto %i", pGhost->m_iCurrentTick + 1));
        }
    }
    else if (!Q_strcasecmp(command, "gototick") && pGhost)
    {
        // Teleport at the position we want with timer included
        char tick[32];
        m_pGotoTick->GetText(tick, sizeof(tick));
        engine->ServerCmd(VarArgs("mom_replay_goto %s", tick));
        m_pGotoTick->SetText("");
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}