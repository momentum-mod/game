#include "cbase.h"

#include "mom_timer.h"
#include "mom_player_shared.h"
#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "util/baseautocompletefilelist.h"
#include "fmtstr.h"
#include "steam/steam_api.h"
#include "run/mom_replay_factory.h"

#include "tier0/memdbgon.h"

static MAKE_CONVAR(mom_replay_timescale, "1.0", FCVAR_NONE,
                   "The timescale of a replay. > 1 is faster, < 1 is slower. \n", 0.01f, 10.0f);
static MAKE_CONVAR(mom_replay_selection, "0", FCVAR_NONE, "Going forward or backward in the replayui \n", 0, 2);

CMomentumReplaySystem::CMomentumReplaySystem(const char* pName):
    CAutoGameSystemPerFrame(pName), m_bRecording(false), m_bPlayingBack(false), m_pPlaybackReplay(nullptr),
    m_player(nullptr),
    m_bShouldStopRec(false),
    m_iTickCount(0),
    m_iStartRecordingTick(-1),
    m_iStartTimerTick(-1),
    m_fRecEndTime(-1.0f)
{
    m_pRecordingReplay = g_ReplayFactory.CreateEmptyReplay(0);
}

CMomentumReplaySystem::~CMomentumReplaySystem()
{
    if (m_pRecordingReplay)
        delete m_pRecordingReplay;

    if (m_pPlaybackReplay)
        delete m_pPlaybackReplay;
}

void CMomentumReplaySystem::FrameUpdatePostEntityThink()
{
    if (m_bRecording)
        UpdateRecordingParams();
}

void CMomentumReplaySystem::LevelShutdownPostEntity()
{
    //Stop a recording if there is one while the level shuts down
    if (m_bRecording)
        StopRecording(true, false);

    if (m_pPlaybackReplay)
        UnloadPlayback(true);
}

void CMomentumReplaySystem::PostInit()
{
    // We have to create the directory here just in case it doesn't exist yet
    filesystem->CreateDirHierarchy(RECORDING_PATH, "MOD");
    CFmtStr path("%s/%s/", RECORDING_PATH, RECORDING_ONLINE_PATH);
    filesystem->CreateDirHierarchy(path.Get(), "MOD");
}

void CMomentumReplaySystem::BeginRecording(CBasePlayer *pPlayer)
{
    // don't record if we're watching a preexisting replay or in practice mode
    if (!m_player->IsSpectatingGhost() && !m_player->m_SrvData.m_bHasPracticeMode)
    {
        m_bRecording = true;
        m_iTickCount = 1; // recoring begins at 1 ;)
        m_iStartRecordingTick = gpGlobals->tickcount;
        m_pRecordingReplay = g_ReplayFactory.CreateEmptyReplay(0);
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

        m_bRecording = false;

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

    

    DevLog("Before trimming: %i\n", m_iTickCount);
    TrimReplay();

    // Store the replay in a file and stop recording. Let's Trim before doing this, for our start recorded tick.
    SetReplayInfo();
    SetRunStats();

    int postTrimTickCount = m_pRecordingReplay->GetFrameCount();
    DevLog("After trimming: %i\n", postTrimTickCount);
    StoreReplay(newRecordingPath);
    m_bRecording = false;
    // Note: m_iTickCount updates in TrimReplay(). Passing it here shows the new ticks.
    Log("Recording Stopped! Ticks: %i\n", postTrimTickCount);

    // Re-allow the player to teleport
    if (pPlayer)
        pPlayer->m_bAllowUserTeleports = true;
    
    if (replaySavedEvent)
    {
        replaySavedEvent->SetBool("save", true);
        replaySavedEvent->SetString("filename", newRecordingName);
        replaySavedEvent->SetString("filepath", newRecordingPath);
        replaySavedEvent->SetInt("time", static_cast<int>(m_pRecordingReplay->GetRunTime() * 1000.0f));
        gameeventmanager->FireEvent(replaySavedEvent);
    }
    // Load the last run that we did in case we want to watch it
    LoadPlayback(newRecordingPath);

    // Reset the m_i*Tick s
    m_iStartRecordingTick = -1;
    m_iStartTimerTick = -1;
    m_pRecordingReplay->~CMomReplayBase();
}

bool CMomentumReplaySystem::StoreReplay(const char* path, const char* pathID)
{
    if (!m_pRecordingReplay)
        return false;

    auto file = filesystem->Open(path, "w+b", pathID);

    if (!file)
    {
        Log("Error saving file to path %s\n", path);
        return false;
    }

    Log("Storing replay of version '%d' to '%s'...\n", g_ReplaySystem.m_pRecordingReplay->GetVersion(), path);

    CBinaryWriter writer(file);

    writer.WriteUInt32(REPLAY_MAGIC_LE);
    writer.WriteUInt8(m_pRecordingReplay->GetVersion());
    m_pRecordingReplay->Serialize(&writer);

    filesystem->Close(file);

    return true;
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
            if (m_pRecordingReplay)
            {
                // Remove the amount of extra frames from the head
                // MOM_TODO: If the map allows for prespeed in the trigger, we don't want to trim it!
                m_pRecordingReplay->RemoveFrames(extraFrames);
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
    if (m_bRecording && !engine->IsPaused())
    {
        m_pRecordingReplay->AddFrame(CReplayFrame(m_player->EyeAngles(), m_player->GetAbsOrigin(),
                                                                      m_player->GetViewOffset(), m_player->m_nButtons));
        ++m_iTickCount; // increment recording tick
    }

    if (m_bShouldStopRec && m_fRecEndTime < gpGlobals->curtime)
        StopRecording(false, false);
}

CMomReplayBase *CMomentumReplaySystem::LoadPlayback(const char *pFileName, bool bFullLoad, const char *pPathID)
{
    if (m_bPlayingBack)
        StopPlayback();

    if (m_pPlaybackReplay)
        UnloadPlayback();

    m_pPlaybackReplay = g_ReplayFactory.LoadReplayFile(pFileName, bFullLoad, pPathID);

    if (bFullLoad && m_pPlaybackReplay)
    {
        // Create the run entity here
        CMomentumReplayGhostEntity *pGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));
        pGhost->SetRunStats(m_pPlaybackReplay->GetRunStats());
        pGhost->m_SrvData.m_RunData.m_flRunTime = m_pPlaybackReplay->GetRunTime();
        pGhost->m_SrvData.m_RunData.m_iRunFlags = m_pPlaybackReplay->GetRunFlags();
        pGhost->m_SrvData.m_flTickRate = m_pPlaybackReplay->GetTickInterval();
        pGhost->SetPlaybackReplay(m_pPlaybackReplay);
        pGhost->m_SrvData.m_RunData.m_iStartTickD = m_pPlaybackReplay->GetStartTick();
        m_pPlaybackReplay->SetRunEntity(pGhost);
    }

    return m_pPlaybackReplay;
}

void CMomentumReplaySystem::SetReplayInfo()
{
    if (!m_bRecording)
        return;

    m_pRecordingReplay->SetMapName(gpGlobals->mapname.ToCStr());
    m_pRecordingReplay->SetPlayerName(m_player->GetPlayerName());
    ISteamUser *pUser = SteamUser();
    m_pRecordingReplay->SetPlayerSteamID(pUser ? pUser->GetSteamID().ConvertToUint64() : 0);
    m_pRecordingReplay->SetTickInterval(gpGlobals->interval_per_tick);
    m_pRecordingReplay->SetRunTime(g_pMomentumTimer->GetLastRunTime());
    m_pRecordingReplay->SetRunFlags(m_player->m_SrvData.m_RunData.m_iRunFlags);
    m_pRecordingReplay->SetRunDate(g_pMomentumTimer->GetLastRunDate());
    m_pRecordingReplay->SetStartTick(m_iStartTimerTick - m_iStartRecordingTick);
}

void CMomentumReplaySystem::SetRunStats()
{
    if (!m_bRecording)
        return;

    CMomRunStats* stats = m_pRecordingReplay->CreateRunStats(m_player->m_RunStats.GetTotalZones());
    m_player->m_RunStats.FullyCopyStats(stats);
}

void CMomentumReplaySystem::Start(bool firstperson)
{
    if (m_player)
    {
        CMomentumReplayGhostEntity *pGhost = m_pPlaybackReplay->GetRunEntity();
        
        if (firstperson)
            g_pMomentumTimer->Stop(false); // stop the timer just in case we started a replay while it was running...
            
        if (pGhost)
            pGhost->StartRun(firstperson);

        m_bPlayingBack = true;
    }
}

void CMomentumReplaySystem::UnloadPlayback(bool shutdown)
{
    m_bPlayingBack = false;
    
    if (m_pPlaybackReplay)
    {
        if (m_pPlaybackReplay->GetRunEntity() && !shutdown)
            m_pPlaybackReplay->GetRunEntity()->EndRun();

        delete m_pPlaybackReplay;
    }

    m_pPlaybackReplay = nullptr;
}

void CMomentumReplaySystem::StopPlayback()
{
    if (!g_ReplaySystem.m_bPlayingBack)
        return;

    Log("Stopping replay playback.\n");
    g_ReplaySystem.UnloadPlayback();
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

            auto pLoaded = g_ReplaySystem.LoadPlayback(recordingName);
            if (pLoaded)
            {
                if (!Q_strcmp(STRING(gpGlobals->mapname), pLoaded->GetMapName()))
                {
                    g_ReplaySystem.Start(firstperson);
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
    auto pPlaybackReplay = g_ReplaySystem.m_pPlaybackReplay;
    if (pPlaybackReplay && !g_ReplaySystem.m_bPlayingBack)
    {
        g_ReplaySystem.Start(true);
        mom_replay_timescale.SetValue(1.0f);
    }
}

CON_COMMAND(mom_replay_restart, "Restarts the current spectated replay, if there is one.")
{
    if (g_ReplaySystem.m_bPlayingBack)
    {
        auto pGhost = g_ReplaySystem.m_pPlaybackReplay->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_SrvData.m_iCurrentTick = 0;
        }
    }
}

CON_COMMAND(mom_replay_stop, "Stops playing the current replay.")
{
    if (g_ReplaySystem.m_bPlayingBack)
    {
        g_ReplaySystem.StopPlayback();
    }
}

CON_COMMAND(mom_replay_pause, "Toggle pausing and playing the playback replay.")
{
    if (g_ReplaySystem.m_bPlayingBack)
    {
        auto pGhost = g_ReplaySystem.m_pPlaybackReplay->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_SrvData.m_bIsPaused = !pGhost->m_SrvData.m_bIsPaused;
        }
    }
}

CON_COMMAND(mom_replay_goto, "Go to a specific tick in the replay.")
{
    if (g_ReplaySystem.m_bPlayingBack)
    {
        auto pGhost = g_ReplaySystem.m_pPlaybackReplay->GetRunEntity();
        if (pGhost && args.ArgC() > 1)
        {
            int tick = Q_atoi(args[1]);
            if (tick >= 0 && tick <= pGhost->m_SrvData.m_iTotalTimeTicks)
            {
                pGhost->m_SrvData.m_iCurrentTick = tick;
                pGhost->m_SrvData.m_RunData.m_bMapFinished = false;
            }
        }
    }
}

CON_COMMAND(mom_replay_goto_end, "Go to the end of the replay.")
{
    if (g_ReplaySystem.m_bPlayingBack)
    {
        auto pGhost = g_ReplaySystem.m_pPlaybackReplay->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_SrvData.m_iCurrentTick = pGhost->m_SrvData.m_iTotalTimeTicks - pGhost->m_SrvData.m_RunData.m_iStartTickD;
        }
    }
}

CMomentumReplaySystem g_ReplaySystem("MOMReplaySystem");