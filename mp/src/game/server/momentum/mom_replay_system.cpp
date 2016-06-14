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
        Log("Recording began!\n");
        m_nCurrentTick = 1; // recoring begins at 1 ;)
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
        m_fRecEndTime = gpGlobals->curtime;
        return;
    }

    char newRecordingName[MAX_PATH], newRecordingPath[MAX_PATH], runTime[BUFSIZETIME];

    m_bShouldStopRec = false;
    CMomentumPlayer *pMOMPlayer = ToCMOMPlayer(pPlayer);
    mom_UTIL->FormatTime(g_Timer->GetLastRunTime(), runTime, 3, true);
    Q_snprintf(newRecordingName, MAX_PATH, "%s_%s_%s.momrec",
                (pMOMPlayer ? pMOMPlayer->GetPlayerName() : "Unnamed"), gpGlobals->mapname.ToCStr(), runTime);
    V_ComposeFileName(RECORDING_PATH, newRecordingName, newRecordingPath,
                        MAX_PATH); // V_ComposeFileName calls all relevant filename functions for us! THANKS GABEN

    V_FixSlashes(RECORDING_PATH);

    // We have to create the directory here just in case it doesn't exist yet
    filesystem->CreateDirHierarchy(RECORDING_PATH,
                                    "MOD");

    // Store the replay in a file and stop recording.
    SetReplayInfo();
    SetRunStats();
    m_pReplayManager->StoreReplay(newRecordingPath, "MOD");
    m_pReplayManager->StopRecording();

    Log("Recording Stopped! Ticks: %i\n", m_nCurrentTick);

    // Load the last run that we did in case we want to watch it
    // TODO (OrfeasZ): Does this need to be re-enabled?
    //LoadRun(newRecordingName);
}

void CMomentumReplaySystem::UpdateRecordingParams()
{
    ++m_nCurrentTick; // increment recording tick

    if (m_pReplayManager->Recording())
        m_pReplayManager->GetRecordingReplay()->AddFrame(CReplayFrame(m_player->EyeAngles(), m_player->GetAbsOrigin(), m_player->m_nButtons));

    if (m_bShouldStopRec && gpGlobals->curtime - m_fRecEndTime >= END_RECORDING_PAUSE)
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
}

void CMomentumReplaySystem::SetRunStats()
{
    if (!m_pReplayManager->Recording())
        return;

    auto stats = m_pReplayManager->GetRecordingReplay()->CreateRunStats(m_player->m_RunStats.GetTotalZones());
    *stats = static_cast<CMomRunStats>(m_player->m_RunStats);
}

void CMomentumReplaySystem::StartReplay(bool firstperson)
{
    CMomentumReplayGhostEntity* pGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));

    if (pGhost != nullptr && m_pReplayManager->GetPlaybackReplay())
    {
        //MOM_TODO: Make sure this is all the data we need to set
        pGhost->SetRunStats(m_pReplayManager->GetPlaybackReplay()->GetRunStats());
        pGhost->m_RunData.m_flRunTime = m_pReplayManager->GetPlaybackReplay()->GetRunTime();
        pGhost->m_RunData.m_iRunFlags = m_pReplayManager->GetPlaybackReplay()->GetRunFlags();
        pGhost->m_flTickRate = m_pReplayManager->GetPlaybackReplay()->GetTickInterval();

        if (firstperson)
            g_Timer->Stop(false); // stop the timer just in case we started a replay while it was running...

        pGhost->StartRun(firstperson);

        AddGhost(pGhost);
    }
}

void CMomentumReplaySystem::AddGhost(CMomentumReplayGhostEntity *pGhost)
{
    if (pGhost)
        m_rgGhosts.AddToTail(pGhost);
}

void CMomentumReplaySystem::RemoveGhost(CMomentumReplayGhostEntity *pGhost)
{
    if (pGhost)
        m_rgGhosts.FindAndRemove(pGhost);
}

void CMomentumReplaySystem::EndReplay(CMomentumReplayGhostEntity *pGhost)
{
    if (pGhost)
    {
        pGhost->EndRun();
    }
    else
    {
        //Backwards since this will be removing ghosts as it goes along
        FOR_EACH_VEC_BACK(m_rgGhosts, i)
        {
            CMomentumReplayGhostEntity *pGhost2 = m_rgGhosts[i];
            if (pGhost2)
                pGhost2->EndRun();
        }
        //Theoretically, m_rcGhosts will be empty here.
    }
}


void CMomentumReplaySystem::OnGhostEntityRemoved(CMomentumReplayGhostEntity* pGhost)
{
    // NOTE: Calling delete here seems to have some adverse side-effects.
    // Are entities deleted by the engine itself when "Remove()" is called?
    // @Gocnak: Yep. Calling Remove() on an ent marks it for deletion next frame.
    RemoveGhost(pGhost);
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

            if (g_ReplaySystem->GetReplayManager()->LoadReplay(recordingName, "MOD"))
            {
                if (!Q_strcmp(STRING(gpGlobals->mapname), g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetMapName()))
                {
                    g_ReplaySystem->StartReplay(firstperson);
                }
                else
                {
                    Warning("Error: Tried to start replay on incorrect map! Please load map %s",
                        g_ReplaySystem->GetReplayManager()->GetPlaybackReplay()->GetMapName());
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
                             "Begins a playback of a replay in first-person mode.", "recordings", momrec);

CON_COMMAND(mom_replay_stop, "Stops playing the current replay.")
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    CMomentumReplayGhostEntity *pGhost = nullptr;
    if (pPlayer)
    {
        if (pPlayer->IsWatchingReplay())
        {
            pGhost = pPlayer->GetReplayEnt();
        }
        else
        {
            //MOM_TODO: Should this just pick the first ghost from the replay system and stop that?
            // Or should we print "Use "mom_replay_stop_all" to stop all ghosts"?
        }

        if (pGhost)
            g_ReplaySystem->EndReplay(pGhost);
    }
}

CON_COMMAND(mom_replay_stop_all, "Stops all currently playing replays.")
{
    g_ReplaySystem->EndReplay(nullptr);
}

CON_COMMAND(mom_spectate, "Start spectating if there are ghosts currently being played.")
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && !pPlayer->IsObserver())
    {
        CBaseEntity *pNext = pPlayer->FindNextObserverTarget(false);
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
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {  
        pPlayer->StopSpectating();
        g_Timer->DispatchTimerStateMessage(pPlayer, false);
    }
}

static CMomentumReplaySystem s_ReplaySystem("MOMReplaySystem");
CMomentumReplaySystem *g_ReplaySystem = &s_ReplaySystem;