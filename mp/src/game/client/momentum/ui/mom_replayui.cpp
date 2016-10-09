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
            if (m_pFastBackward->IsSelected() || m_pFastForward->IsSelected())
            {
                shared->RGUI_HasSelected = m_pFastBackward->IsSelected() ? RUI_MOVEBW : RUI_MOVEFW;
                
                if (!pGhost->m_bIsPaused)
                    engine->ClientCmd("mom_replay_pause");

                m_pPlayPauseResume->ForceDepressed(false);
            }
            else
            {
                shared->RGUI_HasSelected = RUI_NOTHING;
            }

            if (!pGhost->m_bIsPaused && !m_pPlayPauseResume->IsArmed())
                m_pPlayPauseResume->SetArmed(true);

            m_pPlayPauseResume->SetSelected(!pGhost->m_bIsPaused);
            m_pPlayPauseResume->SetText(pGhost->m_bIsPaused ? "Paused" : "Playing"); // MOM_TODO: Localize

            m_iTotalDuration = pGhost->m_iTotalTimeTicks;

            float fProgress = static_cast<float>(pGhost->m_iCurrentTick) / static_cast<float>(m_iTotalDuration);
            fProgress = clamp(fProgress, 0.0f, 1.0f);

            m_pProgress->SetProgress(fProgress);
            char labelFrame[512];
            // MOM_TODO: Localize this
            Q_snprintf(labelFrame, 512, "Tick: %i / %i", pGhost->m_iCurrentTick, m_iTotalDuration);
            m_pProgressLabelFrame->SetText(labelFrame);
            char curtime[BUFSIZETIME];
            char totaltime[BUFSIZETIME];
            mom_UTIL->FormatTime(TICK_INTERVAL * pGhost->m_iCurrentTick, curtime, 2);
            mom_UTIL->FormatTime(TICK_INTERVAL * m_iTotalDuration, totaltime, 2);

            char labelTime[512];
            // MOM_TODO: LOCALIZE
            Q_snprintf(labelTime, 512, "Time: %s -> %s", curtime, totaltime);
            m_pProgressLabelTime->SetText(labelTime);
            // Let's add a check if we entered into end zone without the trigger spot it (since we teleport directly),
            // then we will disable the replayui

            if (pGhost->m_RunData.m_bMapFinished)
            {
                ShowPanel(false);

                // Hide spec input as well
                CMOMSpectatorGUI *pSpec = dynamic_cast<CMOMSpectatorGUI *>(m_pViewport->FindPanelByName(PANEL_SPECGUI));
                if (pSpec)
                    pSpec->SetMouseInputEnabled(false);
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