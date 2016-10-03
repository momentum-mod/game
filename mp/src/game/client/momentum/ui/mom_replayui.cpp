#include "cbase.h"

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>

#include <vgui_controls/TextEntry.h>

#include "PFrameButton.h"
#include "hud_mapfinished.h"
#include "mom_replayui.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

CHudReplay::CHudReplay(const char *pElementName) : Frame(nullptr, pElementName)
{
    SetTitle("Reply Playback", true);
    SetMoveable(true);
    SetSizeable(false);
    SetVisible(false);
    SetSize(310, 210);

    LoadControlSettings("resource/ui/ReplayUI.res");

    //m_pPlayPauseResume = new vgui::ToggleButton(this, "ReplayPlayPauseResume", "PlayPauseResume");
    m_pPlayPauseResume = FindControl<ToggleButton>("ReplayPlayPauseResume");

    //m_pGoStart = new vgui::Button(this, "ReplayGoStart", "Go Start");
    m_pGoStart = FindControl<Button>("ReplayGoStart");
    //m_pGoEnd = new vgui::Button(this, "ReplayGoEnd", "Go End");
    m_pGoEnd = FindControl<Button>("ReplayGoEnd");
    //m_pPrevFrame = new vgui::Button(this, "ReplayPrevFrame", "Prev Frame");
    m_pPrevFrame = FindControl<Button>("ReplayPrevFrame");
    //m_pNextFrame = new vgui::Button(this, "ReplayNextFrame", "Next Frame");
    m_pNextFrame = FindControl<Button>("ReplayNextFrame");
    //m_pFastForward = new vgui::PFrameButton(this, "ReplayFastForward", "Fast Fwd");
    m_pFastForward = FindControl<PFrameButton>("ReplayFastForward");
    //m_pFastBackward = new vgui::PFrameButton(this, "ReplayFastBackward", "Fast Bwd");
    m_pFastBackward = FindControl<PFrameButton>("ReplayFastBackward");
    //m_pGo = new vgui::Button(this, "ReplayGo", "Go");
    m_pGo = FindControl<Button>("ReplayGo");

    //m_pGotoTick2 = new vgui::TextEntry(this, "ReplayGoToTick2");
    m_pGotoTick = FindControl<TextEntry>("ReplayGoToTick");
    m_pGotoTick2 = FindControl<TextEntry>("ReplayGoToTick2");

    if (shared)
    {
        shared->RGUI_TimeScale = 1.0f;
        char buf[0xF]; // This is way too much
		sprintf(buf, "%.1f", shared->RGUI_TimeScale);
        m_pGotoTick2->SetText(buf);
    }

    //m_pGo2 = new vgui::Button(this, "ReplayGo2", "Go2");
    m_pGo2 = FindControl<Button>("ReplayGo2");

    //m_pProgress = new vgui::ProgressBar(this, "ReplayProgress");
    m_pProgress = FindControl<ProgressBar>("ReplayProgress");
    m_pProgress->SetSegmentInfo(2, 2);

    //m_pProgressLabelFrame = new vgui::Label(this, "ReplayProgressLabelFrame", "");
    m_pProgressLabelFrame = FindControl<Label>("ReplayProgressLabelFrame");
    //m_pProgressLabelTime = new vgui::Label(this, "ReplayProgressLabelTime", "");
    m_pProgressLabelTime = FindControl<Label>("ReplayProgressLabelTime");

    //m_pGotoTick = new vgui::TextEntry(this, "ReplayGoToTick");
}

void CHudReplay::OnThink()
{
    BaseClass::OnThink();

    char curtime[BUFSIZETIME];
    char totaltime[BUFSIZETIME];
    float fProgress = 0.0f;

    // enable/disable all playback control buttons
    m_pPlayPauseResume->SetEnabled(true);
    m_pNextFrame->SetEnabled(true);
    m_pGoStart->SetEnabled(true);
    m_pGoEnd->SetEnabled(true);
    m_pPrevFrame->SetEnabled(true);
    m_pFastBackward->SetEnabled(true);
    m_pFastForward->SetEnabled(true);
    m_pGotoTick->SetEnabled(true);
    m_pGo->SetEnabled(true);
    m_pGotoTick2->SetEnabled(true);
    m_pGo2->SetEnabled(true);

    if (m_pFastBackward->IsSelected())
    {
		shared->RGUI_HasSelected = RUI_MOVEBW;
		shared->RGUI_bIsPlaying = false;
        m_pPlayPauseResume->ForceDepressed(false);
    }
    else if (m_pFastForward->IsSelected())
    {
		shared->RGUI_HasSelected = RUI_MOVEFW;
		shared->RGUI_bIsPlaying = false;
        m_pPlayPauseResume->ForceDepressed(false);
    }
    else
    {
		if (shared->RGUI_bIsPlaying)
        {
            m_pPlayPauseResume->SetText("Playing");
            m_pPlayPauseResume->SetSelected(true);
        }
        else
        {
            shared->RGUI_HasSelected = RUI_NOTHING;
            m_pPlayPauseResume->SetText("Paused");
            m_pPlayPauseResume->SetSelected(false);
        }
    }

    // cvar->FindVar("sv_cheats")->SetValue(1);
    // cvar->FindVar("host_timescale")->SetValue(shared->TickRate);
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
        if (pGhost)
        {
            fProgress = static_cast<float>(shared->m_iCurrentTick_Server) / static_cast<float>(pGhost->m_iTotalTimeTicks);
            fProgress = clamp(fProgress, 0.0f, 1.0f);

            m_pProgress->SetProgress(fProgress);
            char labelFrame[512];
            Q_snprintf(labelFrame, 512, "Tick: %i / %i", shared->m_iCurrentTick_Server, pGhost->m_iTotalTimeTicks);
            m_pProgressLabelFrame->SetText(labelFrame);
            mom_UTIL->FormatTime(TICK_INTERVAL * shared->m_iCurrentTick_Server, curtime);
            mom_UTIL->FormatTime(TICK_INTERVAL * pGhost->m_iTotalTimeTicks, totaltime);

            char labelTime[512];
            Q_snprintf(labelTime, 512, "Time: %s -> %s", curtime, totaltime);
            m_pProgressLabelTime->SetText(labelTime);
            // Let's add a check if we entered into end zone without the trigger spot it (since we teleport directly), then we
            // will disable the replayui


            if (pGhost)
            {
                // always disable if map is finished
                if (pGhost->m_RunData.m_bMapFinished)
                {
                    // cvar->FindVar("sv_cheats")->SetValue(0);
                    // cvar->FindVar("host_timescale")->SetValue(1.0f);
                    SetVisible(false);
                }
            }
        }
    }
}

// Command issued
void CHudReplay::OnCommand(const char *command)
{
    if (!shared)
        return BaseClass::OnCommand(command);
    C_MomentumReplayGhostEntity *pGhost = ToCMOMPlayer(CBasePlayer::GetLocalPlayer())->GetReplayEnt();
    if (!Q_strcasecmp(command, "play"))
    {
		shared->RGUI_bIsPlaying = !shared->RGUI_bIsPlaying;
    }
    else if (!Q_strcasecmp(command, "reload"))
    {
        shared->m_iCurrentTick_Server = 0;
        shared->m_iTotalTicks_Client_Timer = 0;
    }
    else if (!Q_strcasecmp(command, "gotoend") && pGhost)
    {
        shared->m_iCurrentTick_Server = pGhost->m_iTotalTimeTicks;
        shared->m_iTotalTicks_Client_Timer = pGhost->m_iTotalTimeTicks;
    }
    else if (!Q_strcasecmp(command, "prevframe"))
    {
        if (shared->m_iTotalTicks_Client_Timer > 0 && shared->m_iCurrentTick_Server > 0)
        {
            shared->m_iTotalTicks_Client_Timer--;
            shared->m_iCurrentTick_Server--;
        }
    }
    else if (!Q_strcasecmp(command, "nextframe"))
    {
        shared->m_iTotalTicks_Client_Timer++;
        shared->m_iCurrentTick_Server++;
    }
    else if (!Q_strcasecmp(command, "gototick2"))
    {
        char tick[32];
        m_pGotoTick2->GetText(tick, sizeof(tick));
        shared->RGUI_TimeScale = atof(tick);
    }
    else if (!Q_strcasecmp(command, "gototick"))
    {
        //Teleport at the position we want with timer included
        char tick[32];
        m_pGotoTick->GetText(tick, sizeof(tick));
        shared->m_iCurrentTick_Server = atoi(tick);

       
        if (pGhost)
        {
            shared->m_iTotalTicks_Client_Timer = shared->m_iCurrentTick_Server - pGhost->m_RunData.m_iStartTickD;
        }
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void replayui_f()
{
	if (!shared->HudReplay)
		shared->HudReplay = new CHudReplay("HudReplay");

	if (!shared->HudReplay || !shared)
        return;

	if (shared->HudReplay->IsVisible())
    {
		shared->HudReplay->Close();
    }
    else
    {
		shared->HudReplay->Activate();
    }
}

static ConCommand replayui("replayui", replayui_f, "Replay Ghost GUI.");
