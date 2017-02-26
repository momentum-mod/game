#include "cbase.h"

#include "mom_timer.h"
#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "util/baseautocompletefilelist.h"
#include "util/mom_util.h"
#include "utlbuffer.h"

#include "tier0/memdbgon.h"

static MAKE_CONVAR(mom_replay_timescale, "1.0", FCVAR_NONE,
                   "The timescale of a replay. > 1 is faster, < 1 is slower. \n", 0.01f, 10.0f);
static MAKE_CONVAR(mom_replay_selection, "0", FCVAR_NONE, "Going forward or backward in the replayui \n", 0, 2);

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

void CMomentumReplaySystem::StopRecording(bool throwaway, bool delay)
{
    IGameEvent *replaySavedEvent = gameeventmanager->CreateEvent("replay_save");

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    if (throwaway && replaySavedEvent)
    {
        replaySavedEvent->SetBool("save", false);
        gameeventmanager->FireEvent(replaySavedEvent);
        m_pReplayManager->StopRecording();

        // Re-allow the player to teleport
        if (pPlayer)
            pPlayer->m_bAllowUserTeleports = true;
        return;
    }

    if (delay)
    {
        // Prevent the user from teleporting, potentially breaking this delay
        if (pPlayer)
            pPlayer->m_bAllowUserTeleports = false;

        m_bShouldStopRec = true;
        m_fRecEndTime = gpGlobals->curtime + END_RECORDING_DELAY;
        return;
    }

    char newRecordingName[MAX_PATH], newRecordingPath[MAX_PATH], runTime[MAX_PATH], runDate[MAX_PATH];

    m_bShouldStopRec = false;

    // Don't ask why, but these need to be formatted in their own strings.
    Q_snprintf(runDate, MAX_PATH, "%li", g_pMomentumTimer->GetLastRunDate());
    Q_snprintf(runTime, MAX_PATH, "%.3f", g_pMomentumTimer->GetLastRunTime());
    // It's weird.

    Q_snprintf(newRecordingName, MAX_PATH, "%s-%s-%s%s", gpGlobals->mapname.ToCStr(), runDate, runTime, EXT_RECORDING_FILE);

    // V_ComposeFileName calls all relevant filename functions for us! THANKS GABEN
    V_ComposeFileName(RECORDING_PATH, newRecordingName, newRecordingPath, MAX_PATH);

    // We have to create the directory here just in case it doesn't exist yet
    filesystem->CreateDirHierarchy(RECORDING_PATH, "MOD");

    DevLog("Before trimming: %i\n", m_iTickCount);
    TrimReplay();

    // Store the replay in a file and stop recording. Let's Trim before doing this, for our start recorded tick.
    SetReplayInfo();
    SetRunStats();

    int postTrimTickCount = m_pReplayManager->GetRecordingReplay()->GetFrameCount();
    DevLog("After trimming: %i\n", postTrimTickCount);
    m_pReplayManager->StoreReplay(newRecordingPath);
    m_pReplayManager->StopRecording();
    // Note: m_iTickCount updates in TrimReplay(). Passing it here shows the new ticks.
    Log("Recording Stopped! Ticks: %i\n", postTrimTickCount);

    // Re-allow the player to teleport
    if (pPlayer)
        pPlayer->m_bAllowUserTeleports = true;
    
    if (replaySavedEvent)
    {
        replaySavedEvent->SetBool("save", true);
        replaySavedEvent->SetString("filename", newRecordingName);
        gameeventmanager->FireEvent(replaySavedEvent);
    }
    // Load the last run that we did in case we want to watch it
    m_pReplayManager->LoadReplay(newRecordingPath);

    // Reset the m_i*Tick s
    m_iStartRecordingTick = -1;
    m_iStartTimerTick = -1;
}

void CMomentumReplaySystem::TrimReplay()
{
    // Our actual start
    if (m_iStartRecordingTick > -1 && m_iStartTimerTick > -1)
    {
        int newStart = m_iStartTimerTick - static_cast<int>(START_TRIGGER_TIME_SEC / gpGlobals->interval_per_tick);
        // We only need to trim if the player was in the start trigger for longer than what we want
        if (newStart > m_iStartRecordingTick)
        {
            int extraFrames = newStart - m_iStartRecordingTick;
            CMomReplayBase *pReplay = m_pReplayManager->GetRecordingReplay();
            if (pReplay)
            {
                // Remove the amount of extra frames from the head
                // MOM_TODO: If the map allows for prespeed in the trigger, we don't want to trim it!
                pReplay->RemoveFrames(extraFrames);
                // Add our extraFrames because we may have stayed in the start zone
                m_iStartRecordingTick += extraFrames;
                m_iTickCount -= extraFrames;
            }
        }
    }
}

void CMomentumReplaySystem::UpdateRecordingParams()
{
    // We only record frames that the player isn't pausing on
    if (m_pReplayManager->Recording() && !engine->IsPaused())
    {
        m_pReplayManager->GetRecordingReplay()->AddFrame(CReplayFrame(m_player->EyeAngles(), m_player->GetAbsOrigin(),
                                                                      m_player->GetViewOffset(), m_player->m_nButtons));
        ++m_iTickCount; // increment recording tick
    }

    if (m_bShouldStopRec && m_fRecEndTime < gpGlobals->curtime)
        StopRecording(false, false);
}

void CMomentumReplaySystem::SetReplayInfo()
{
    if (!m_pReplayManager->Recording())
        return;

    auto replay = m_pReplayManager->GetRecordingReplay();

    replay->SetMapName(gpGlobals->mapname.ToCStr());
    replay->SetPlayerName(m_player->GetPlayerName());
    ISteamUser *pUser = steamapicontext->SteamUser();
    replay->SetPlayerSteamID(pUser ? pUser->GetSteamID().ConvertToUint64() : 0);
    replay->SetTickInterval(gpGlobals->interval_per_tick);
    replay->SetRunTime(g_pMomentumTimer->GetLastRunTime());
    replay->SetRunFlags(m_player->m_RunData.m_iRunFlags);
    replay->SetRunDate(g_pMomentumTimer->GetLastRunDate());
    replay->SetStartTick(m_iStartTimerTick - m_iStartRecordingTick);
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

            if (Q_strstr(args.ArgS(), EXT_RECORDING_FILE))
            {
                Q_snprintf(filename, MAX_PATH, "%s", args.ArgS());
            }
            else
            {
                Q_snprintf(filename, MAX_PATH, "%s%s", args.ArgS(), EXT_RECORDING_FILE);
            }

            char recordingName[MAX_PATH];
            V_ComposeFileName(RECORDING_PATH, filename, recordingName, MAX_PATH);

            auto pLoaded = g_ReplaySystem->GetReplayManager()->LoadReplay(recordingName);
            if (pLoaded)
            {
                if (!Q_strcmp(STRING(gpGlobals->mapname), pLoaded->GetMapName()))
                {
                    pLoaded->Start(firstperson);
                    mom_replay_timescale.SetValue(1.0f);
                    mom_replay_selection.SetValue(0);
                }
                else
                {
                    Warning("Error: Tried to start replay on incorrect map! Please load map %s", pLoaded->GetMapName());
                }
            }
        }
    }
    static void PlayReplayGhost(const CCommand &args) { StartReplay(args, false); }
    static void PlayReplayFirstPerson(const CCommand &args) { StartReplay(args, true); }
};

CON_COMMAND_AUTOCOMPLETEFILE(mom_replay_play_ghost, CMOMReplayCommands::PlayReplayGhost,
                             "Begins playback of a replay as a ghost.", RECORDING_PATH, momrec);
CON_COMMAND_AUTOCOMPLETEFILE(mom_replay_play, CMOMReplayCommands::PlayReplayFirstPerson,
                             "Begins a playback of a replay in first-person mode.", RECORDING_PATH, momrec);

CON_COMMAND(mom_replay_play_loaded, "Begins playing back a loaded replay (in first person), if there is one.")
{
    auto pPlaybackReplay = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay();
    if (pPlaybackReplay && !g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        pPlaybackReplay->Start(true);
        mom_replay_timescale.SetValue(1.0f);
    }
}

CON_COMMAND(mom_replay_restart, "Restarts the current spectated replay, if there is one.")
{
    if (g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        auto pGhost = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_iCurrentTick = 0;
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

CON_COMMAND(mom_replay_pause, "Toggle pausing and playing the playback replay.")
{
    if (g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        auto pGhost = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_bIsPaused = !pGhost->m_bIsPaused;
        }
    }
}

CON_COMMAND(mom_replay_goto, "Go to a specific tick in the replay.")
{
    if (g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        auto pGhost = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetRunEntity();
        if (pGhost && args.ArgC() > 1)
        {
            int tick = Q_atoi(args[1]);
            if (tick >= 0 && tick <= pGhost->m_iTotalTimeTicks)
            {
                pGhost->m_iCurrentTick = tick;
                pGhost->m_RunData.m_bMapFinished = false;
            }
        }
    }
}

CON_COMMAND(mom_replay_goto_end, "Go to the end of the replay.")
{
    if (g_ReplaySystem->GetReplayManager()->PlayingBack())
    {
        auto pGhost = g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_iCurrentTick = pGhost->m_iTotalTimeTicks - pGhost->m_RunData.m_iStartTickD;
        }
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
            // Setting ob target first is needed for the specGUI panel to update properly
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
        g_pMomentumTimer->DispatchTimerStateMessage(pPlayer, false);
    }
}

static CMomentumReplaySystem s_ReplaySystem("MOMReplaySystem");
CMomentumReplaySystem *g_ReplaySystem = &s_ReplaySystem;