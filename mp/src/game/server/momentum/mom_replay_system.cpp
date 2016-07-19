#include "cbase.h"

#include "mom_replay_system.h"
#include "mom_replay_entity.h"
#include "util/baseautocompletefilelist.h"
#include "util/mom_util.h"
#include "utlbuffer.h"
#include "Timer.h"

#include "tier0/memdbgon.h"

void CMomentumReplaySystem::BeginRecording(CBasePlayer *pPlayer)
{
    m_player = ToCMOMPlayer(pPlayer);

    // don't record if we're watching a preexisting replay or in practice mode
    if (!m_player->IsWatchingReplay() && !m_player->m_bHasPracticeMode)
    {
        m_pReplayManager->StartRecording();
        m_iTickCount = 1; // recoring begins at 1 ;)
        m_iStartRecordingTick = gpGlobals->tickcount;
    }
}

void CMomentumReplaySystem::StopRecording(CBasePlayer *pPlayer, bool throwaway, bool delay)
{
    if (throwaway)
    {
        m_pReplayManager->StopRecording();
        return;
    }

    if (delay)
    {
        m_bShouldStopRec = true;
        m_fRecEndTime = gpGlobals->curtime + END_RECORDING_DELAY;
        return;
    }

    char newRecordingName[MAX_PATH], newRecordingPath[MAX_PATH], runTime[MAX_PATH], runDate[MAX_PATH];

    m_bShouldStopRec = false;
    
    //Don't ask why, but these need to be formatted in their own strings.
    Q_snprintf(runDate, MAX_PATH, "%i", g_Timer->GetLastRunDate());
    Q_snprintf(runTime, MAX_PATH, "%.3f", g_Timer->GetLastRunTime());
    //It's weird.

    Q_snprintf(newRecordingName, MAX_PATH, "%s-%s%s", runDate, runTime, EXT_RECORDING_FILE);

    // V_ComposeFileName calls all relevant filename functions for us! THANKS GABEN
    V_ComposeFileName(RECORDING_PATH, newRecordingName, newRecordingPath, MAX_PATH); 

    // We have to create the directory here just in case it doesn't exist yet
    filesystem->CreateDirHierarchy(RECORDING_PATH, "MOD");

    // Store the replay in a file and stop recording.
    SetReplayInfo();
    SetRunStats();
    
    DevLog("Before trimming: %i\n", m_iTickCount);
    TrimReplay();
    int postTrimTickCount = m_pReplayManager->GetRecordingReplay()->GetFrameCount();
    DevLog("After trimming: %i\n", postTrimTickCount);
    m_pReplayManager->StoreReplay(newRecordingPath);
    m_pReplayManager->StopRecording();
    //Note: m_iTickCount updates in TrimReplay(). Passing it here shows the new ticks.
    Log("Recording Stopped! Ticks: %i\n", postTrimTickCount);
    IGameEvent *replaySavedEvent = gameeventmanager->CreateEvent("replay_save");
    if (replaySavedEvent)
    {
        replaySavedEvent->SetString("filename", newRecordingName);
        gameeventmanager->FireEvent(replaySavedEvent);
    }
    // Load the last run that we did in case we want to watch it
    m_pReplayManager->LoadReplay(newRecordingPath);

    //Reset the m_i*Tick s
    m_iStartRecordingTick = -1;
    m_iStartTimerTick = -1;
}

void CMomentumReplaySystem::TrimReplay()
{
    //Our actual start
    if (m_iStartRecordingTick > -1 && m_iStartTimerTick > -1)
    {
        int newStart = m_iStartTimerTick - int(START_TRIGGER_TIME_SEC / gpGlobals->interval_per_tick);
        //We only need to trim if the player was in the start trigger for longer than what we want
        if (newStart > m_iStartRecordingTick)
        {
            int extraFrames = newStart - m_iStartRecordingTick;
            CMomReplayBase *pReplay = m_pReplayManager->GetRecordingReplay();
            if (pReplay)
            {
                //Remove the amount of extra frames from the head
                //MOM_TODO: If the map allows for prespeed in the trigger, we don't want to trim it!
                pReplay->RemoveFrames(extraFrames);
                m_iTickCount -= extraFrames;
            }
        }
    }
}

void CMomentumReplaySystem::UpdateRecordingParams()
{
    //We only record frames that the player isn't pausing on
    if (m_pReplayManager->Recording() && !engine->IsPaused())
    {
        m_pReplayManager->GetRecordingReplay()->AddFrame(CReplayFrame(m_player->EyeAngles(), m_player->GetAbsOrigin(), m_player->m_nButtons));
        ++m_iTickCount; // increment recording tick
    }

    if (m_bShouldStopRec && m_fRecEndTime < gpGlobals->curtime)
        StopRecording(UTIL_GetLocalPlayer(), false, false);
}

void CMomentumReplaySystem::SetReplayInfo()
{
    if (!m_pReplayManager->Recording())
        return;

    auto replay = m_pReplayManager->GetRecordingReplay();
    
    replay->SetMapName(gpGlobals->mapname.ToCStr());
    replay->SetPlayerName(m_player->GetPlayerName());
    replay->SetPlayerSteamID(steamapicontext->SteamUser() ? steamapicontext->SteamUser()->GetSteamID().ConvertToUint64() : 0);
    replay->SetTickInterval(gpGlobals->interval_per_tick);
    replay->SetRunTime(g_Timer->GetLastRunTime());
    replay->SetRunFlags(m_player->m_RunData.m_iRunFlags);
    replay->SetRunDate(g_Timer->GetLastRunDate());
}

void CMomentumReplaySystem::SetRunStats()
{
    if (!m_pReplayManager->Recording())
        return;

    auto stats = m_pReplayManager->GetRecordingReplay()->CreateRunStats(m_player->m_RunStats.GetTotalZones());
    *stats = static_cast<CMomRunStats>(m_player->m_RunStats);
}

class CMOMReplayCommands
{
  public:
    static void StartReplay(const CCommand &args, bool firstperson)
    {
        if (args.ArgC() > 0) // we passed any argument at all
        {
            char filename[MAX_PATH];
            
            if (Q_strstr(args.ArgS(), ".momrec"))
            {
                Q_snprintf(filename, MAX_PATH, "%s", args.ArgS());
            }
            else
            {
                Q_snprintf(filename, MAX_PATH, "%s.momrec", args.ArgS());
            }

            char recordingName[MAX_PATH];
            V_ComposeFileName(RECORDING_PATH, filename, recordingName, MAX_PATH);

            auto pLoaded = g_ReplaySystem->GetReplayManager()->LoadReplay(recordingName);
            if (pLoaded)
            {
                if (!Q_strcmp(STRING(gpGlobals->mapname), pLoaded->GetMapName()))
                {
                    pLoaded->Start(firstperson);
                }
                else
                {
                    Warning("Error: Tried to start replay on incorrect map! Please load map %s",
                        pLoaded->GetMapName());
                }
            }
        }
    }
    static void PlayReplayGhost(const CCommand &args) { StartReplay(args, false); }
    static void PlayReplayFirstPerson(const CCommand &args) { StartReplay(args, true); }
};

CON_COMMAND_AUTOCOMPLETEFILE(mom_replay_play_ghost, CMOMReplayCommands::PlayReplayGhost, "Begins playback of a replay as a ghost.",
                             "recordings", momrec);
CON_COMMAND_AUTOCOMPLETEFILE(mom_replay_play, CMOMReplayCommands::PlayReplayFirstPerson,
                             "Begins a playback of a replay in first-person mode.", 
                             "recordings", momrec);

CON_COMMAND(mom_replay_play_loaded, "Begins playing back a loaded replay (in first person), if there is one.")
{
    auto pPlaybackReplay = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay();
    if (pPlaybackReplay && !g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        pPlaybackReplay->Start(true);
    }
}

CON_COMMAND(mom_replay_restart, "Restarts the current spectated replay, if there is one.")
{
    if (g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        auto pGhost = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->StartRun(pGhost->m_bReplayFirstPerson);
        }
    }
}

CON_COMMAND(mom_replay_stop, "Stops playing the current replay.")
{
    if (g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        g_ReplaySystem->GetReplayManager()->StopPlayback();
    }
}

CON_COMMAND(mom_spectate, "Start spectating if there are ghosts currently being played.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && !pPlayer->IsObserver())
    {
        auto pNext = pPlayer->FindNextObserverTarget(false);
        if (pNext)
        {
            //Setting ob target first is needed for the specGUI panel to update properly
            pPlayer->SetObserverTarget(pNext);
            pPlayer->StartObserverMode(OBS_MODE_IN_EYE);
        }
    }
}

CON_COMMAND(mom_spectate_stop, "Stop spectating.")
{
    auto pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {  
        pPlayer->StopSpectating();
        g_Timer->DispatchTimerStateMessage(pPlayer, false);
    }
}

static CMomentumReplaySystem s_ReplaySystem("MOMReplaySystem");
CMomentumReplaySystem *g_ReplaySystem = &s_ReplaySystem;