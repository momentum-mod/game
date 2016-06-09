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
		m_pReplayManager->GetCurrentReplay()->AddFrame(CReplayFrame(m_player->EyeAngles(), m_player->GetAbsOrigin(), m_player->m_nButtons));

    if (m_bShouldStopRec && gpGlobals->curtime - m_fRecEndTime >= END_RECORDING_PAUSE)
        StopRecording(UTIL_GetLocalPlayer(), false, false);
}

void CMomentumReplaySystem::SetReplayInfo()
{
	if (!m_pReplayManager->Recording())
		return;

	auto replay = m_pReplayManager->GetCurrentReplay();
    
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

	auto stats = m_pReplayManager->GetCurrentReplay()->CreateRunStats(m_player->m_PlayerRunStats.GetTotalZones());
	*stats = m_player->m_PlayerRunStats;
}

void CMomentumReplaySystem::StartReplay(bool firstperson)
{
    m_CurrentReplayGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));

    if (m_CurrentReplayGhost != nullptr && m_pReplayManager->GetCurrentReplay())
    {
		m_CurrentReplayGhost->SetRunStats(m_pReplayManager->GetCurrentReplay()->GetRunStats());

        if (firstperson)
            g_Timer->Stop(false); // stop the timer just in case we started a replay while it was running...

        m_CurrentReplayGhost->StartRun(firstperson);
    }
}
void CMomentumReplaySystem::EndReplay()
{
    if (m_CurrentReplayGhost != nullptr)
    {
        m_CurrentReplayGhost->EndRun();
    }
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
                if (!Q_strcmp(STRING(gpGlobals->mapname), g_ReplaySystem->GetReplayManager()->GetCurrentReplay()->GetMapName()))
                {
                    g_ReplaySystem->StartReplay(firstperson);
                }
                else
                {
                    Warning("Error: Tried to start replay on incorrect map! Please load map %s",
						g_ReplaySystem->GetReplayManager()->GetCurrentReplay()->GetMapName());
                }
            }
        }
    }
    static void PlayReplayGhost(const CCommand &args) { StartReplay(args, false); }
    static void PlayReplayFirstPerson(const CCommand &args) { StartReplay(args, true); }
};

CON_COMMAND_AUTOCOMPLETEFILE(playreplay_ghost, CMOMReplayCommands::PlayReplayGhost, "begins playback of a replay ghost",
                             "recordings", momrec);
CON_COMMAND_AUTOCOMPLETEFILE(playreplay, CMOMReplayCommands::PlayReplayFirstPerson,
                             "plays back a replay in first-person", "recordings", momrec);

CON_COMMAND(stop_replay, "Stops playing the current replay") { g_ReplaySystem->EndReplay(); }


static CMomentumReplaySystem s_ReplaySystem("MOMReplaySystem");
CMomentumReplaySystem *g_ReplaySystem = &s_ReplaySystem;