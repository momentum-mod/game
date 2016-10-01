#include "cbase.h"

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ProgressBar.h>

#include <vgui_controls/TextEntry.h>

#include "PFrameButton.h"
#include "hud_mapfinished.h"
#include "hud_replayui.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

// Having an interface do this would be better, but for testing it's ok enought
// MOM_TODO: Interface this
CHudReplay *HudReplay = nullptr;

CHudReplay::CHudReplay(const char *pElementName) : Frame(nullptr, pElementName)
{
    SetTitle("Reply Playback", true);

    // MOM_TODO: use FindControl from the .res instead of creating new

    m_pPlayPauseResume = new vgui::ToggleButton(this, "DemoPlayPauseResume", "PlayPauseResume");

    m_pGoStart = new vgui::Button(this, "DemoGoStart", "Go Start");
    m_pGoEnd = new vgui::Button(this, "DemoGoEnd", "Go End");
    m_pPrevFrame = new vgui::Button(this, "DemoPrevFrame", "Prev Frame");
    m_pNextFrame = new vgui::Button(this, "DemoNextFrame", "Next Frame");
    m_pFastForward = new vgui::PFrameButton(this, "DemoFastForward", "Fast Fwd");
    m_pFastBackward = new vgui::PFrameButton(this, "DemoFastBackward", "Fast Bwd");
    m_pGo = new vgui::Button(this, "DemoGo", "Go");

    m_pGotoTick2 = new vgui::TextEntry(this, "DemoGoToTick2");

    if (shared)
    {
        shared->TickRate = 1.0f;
        char buf[0xF]; // This is way too much
        sprintf(buf, "%.1f", shared->TickRate);
        m_pGotoTick2->SetText(buf);
    }

    m_pGo2 = new vgui::Button(this, "DemoGo2", "Go2");

    m_pProgress = new vgui::ProgressBar(this, "DemoProgress");
    m_pProgress->SetSegmentInfo(2, 2);

    m_pProgressLabelFrame = new vgui::Label(this, "DemoProgressLabelFrame", "");
    m_pProgressLabelTime = new vgui::Label(this, "DemoProgressLabelTime", "");

    m_pGotoTick = new vgui::TextEntry(this, "DemoGoToTick");

    LoadControlSettings("resource\\ui\\HudReplay.res"); // Should be always loaded at the end...

    SetMoveable(true);
    SetSizeable(false);
    SetVisible(false);
    SetSize(310, 210);
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
        shared->HasSelected = RUI_MOVEBW;
        shared->m_bIsPlaying = false;
        m_pPlayPauseResume->ForceDepressed(false);
    }
    else if (m_pFastForward->IsSelected())
    {
        shared->HasSelected = RUI_MOVEFW;
        shared->m_bIsPlaying = false;
        m_pPlayPauseResume->ForceDepressed(false);
    }
    else
    {
        if (shared->m_bIsPlaying)
        {
            m_pPlayPauseResume->SetText("Playing");
            m_pPlayPauseResume->SetSelected(true);
        }
        else
        {
            shared->HasSelected = RUI_NOTHING;
            m_pPlayPauseResume->SetText("Paused");
            m_pPlayPauseResume->SetSelected(false);
        }
    }

    // cvar->FindVar("sv_cheats")->SetValue(1);
    // cvar->FindVar("host_timescale")->SetValue(shared->TickRate);

    fProgress = static_cast<float>(shared->m_iCurrentTick) / static_cast<float>(shared->m_iTotalTicks);
    fProgress = clamp(fProgress, 0.0f, 1.0f);

    m_pProgress->SetProgress(fProgress);
    m_pProgressLabelFrame->SetText(mom_UTIL->vaprintf("Tick: %i / %i", shared->m_iCurrentTick, shared->m_iTotalTicks));
    mom_UTIL->FormatTime(TICK_INTERVAL * shared->m_iCurrentTick, curtime);
    mom_UTIL->FormatTime(TICK_INTERVAL * shared->m_iTotalTicks, totaltime);

    m_pProgressLabelTime->SetText(mom_UTIL->vaprintf("Time: %s -> %s", curtime, totaltime));
    // Let's add a check if we entered into end zone without the trigger spot it (since we teleport directly), then we
    // will disable the replayui

    C_MomentumReplayGhostEntity *pGhost = ToCMOMPlayer(CBasePlayer::GetLocalPlayer())->GetReplayEnt();
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

// Command issued
void CHudReplay::OnCommand(const char *command)
{
    if (!shared)
        return BaseClass::OnCommand(command);
    if (!Q_strcasecmp(command, "play"))
    {
        shared->m_bIsPlaying = !shared->m_bIsPlaying;
    }
    else if (!Q_strcasecmp(command, "reload"))
    {
        shared->m_iCurrentTick = 0;
        shared->m_iTotalTicks_Client_Timer = 0;
    }
    else if (!Q_strcasecmp(command, "gotoend"))
    {
        shared->m_iCurrentTick = shared->m_iTotalTicks;
        shared->m_iTotalTicks_Client_Timer = shared->m_iTotalTicks;
    }
    else if (!Q_strcasecmp(command, "prevframe"))
    {
        if (shared->m_iTotalTicks_Client_Timer > 0 && shared->m_iCurrentTick > 0)
        {
            shared->m_iTotalTicks_Client_Timer--;
            shared->m_iCurrentTick--;
        }
    }
    else if (!Q_strcasecmp(command, "nextframe"))
    {
        shared->m_iTotalTicks_Client_Timer++;
        shared->m_iCurrentTick++;
    }
    else if (!Q_strcasecmp(command, "gototick2"))
    {
        char tick[32];
        m_pGotoTick2->GetText(tick, sizeof(tick));
        shared->TickRate = atof(tick);
    }
    else if (!Q_strcasecmp(command, "gototick"))
    {
        // TODO: Teleport at the position we want with timer included
        char tick[32];
        m_pGotoTick->GetText(tick, sizeof(tick));
        shared->m_iCurrentTick = atoi(tick);

        C_MomentumReplayGhostEntity *pGhost = ToCMOMPlayer(CBasePlayer::GetLocalPlayer())->GetReplayEnt();
        if (pGhost)
        {
            shared->m_iTotalTicks_Client_Timer = shared->m_iCurrentTick - pGhost->m_RunData.m_iStartTickD;
        }
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void replayui_f()
{
    if (!HudReplay)
        HudReplay = new CHudReplay("HudReplay");

    if (!HudReplay || !shared)
        return;

    if (HudReplay->IsVisible())
    {
        HudReplay->Close();
    }
    else
    {
        HudReplay->Activate();
    }

    shared->HudReplay = HudReplay;
}

static ConCommand replayui("replayui", replayui_f, "Replay Ghost GUI.");
