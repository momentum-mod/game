#include "cbase.h"

#include "mom_timer.h"
#include "mom_player_shared.h"
#include "mom_replay_entity.h"
#include "mom_replay_system.h"
#include "run/mom_replay_base.h"
#include "util/baseautocompletefilelist.h"
#include "fmtstr.h"
#include "steam/steam_api.h"
#include "run/mom_replay_factory.h"
#include "util/mom_util.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

static MAKE_CONVAR(mom_replay_timescale, "1.0", FCVAR_NONE,
                   "The timescale of a replay. > 1 is faster, < 1 is slower. \n", 0.01f, 10.0f);
static MAKE_CONVAR(mom_replay_selection, "0", FCVAR_NONE, "Going forward or backward in the replayui \n", 0, 2);

CMomentumReplaySystem::CMomentumReplaySystem(const char* pName):
    CAutoGameSystemPerFrame(pName), m_bRecording(false), m_bPlayingBack(false), m_pPlaybackReplay(nullptr),
    m_pPlayer(nullptr),
    m_bShouldStopRec(false),
    m_iTickCount(0),
    m_iStartRecordingTick(-1),
    m_iStartTimerTick(-1),
    m_iStopTimerTick(-1),
    m_bTeleportedThisFrame(false),
    m_fRecEndTime(-1.0f)
{
    m_szMapHash[0] = '\0';
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

void CMomentumReplaySystem::LevelInitPostEntity()
{
    // Get the map file's hash
    if (!g_pMomentumUtil->GetFileHash(m_szMapHash, sizeof(m_szMapHash), CFmtStr("maps/%s.bsp", gpGlobals->mapname.ToCStr())))
        Warning("Could not generate a hash for the current map!!!\n");
}

void CMomentumReplaySystem::LevelShutdownPostEntity()
{
    //Stop a recording if there is one while the level shuts down
    if (m_bRecording)
        StopRecording(true, false);

    if (m_pPlaybackReplay)
        UnloadPlayback(true);

    // Reset the map hash
    m_szMapHash[0] = '\0';
}

void CMomentumReplaySystem::PostInit()
{
    // We have to create the directory here just in case it doesn't exist yet
    filesystem->CreateDirHierarchy(RECORDING_PATH, "MOD");
    CFmtStr path("%s/%s/", RECORDING_PATH, RECORDING_ONLINE_PATH);
    filesystem->CreateDirHierarchy(path.Get(), "MOD");
}

void CMomentumReplaySystem::BeginRecording()
{
    // don't record if we're watching a preexisting replay or in practice mode
    if (!m_pPlayer->IsSpectatingGhost() && !m_pPlayer->m_bHasPracticeMode)
    {
        m_bRecording = true;
        m_iTickCount = 1; // recording begins at 1 ;)
        m_iStartRecordingTick = gpGlobals->tickcount;
        m_pRecordingReplay = g_ReplayFactory.CreateEmptyReplay(0);
    }
}

void CMomentumReplaySystem::StopRecording(bool throwaway, bool delay)
{
    IGameEvent *replaySavedEvent = gameeventmanager->CreateEvent("replay_save");

    if (throwaway && replaySavedEvent)
    {
        replaySavedEvent->SetBool("save", false);
        gameeventmanager->FireEvent(replaySavedEvent);

        m_bRecording = false;

        // Re-allow the player to teleport
        if (m_pPlayer)
            m_pPlayer->SetAllowUserTeleports(true);

        return;
    }

    if (delay)
    {
        // Prevent the user from teleporting, potentially breaking this delay
        if (m_pPlayer)
            m_pPlayer->SetAllowUserTeleports(false);

        m_bShouldStopRec = true;
        m_fRecEndTime = gpGlobals->curtime + END_RECORDING_DELAY;
        return;
    }

    m_bShouldStopRec = false;
    m_bRecording = false;

    DevLog("Before trimming: %i\n", m_iTickCount);
    TrimReplay();

    // Store the replay in a file and stop recording. Let's Trim before doing this, for our start recorded tick.
    SetReplayInfo();
    SetRunStats();

    int postTrimTickCount = m_pRecordingReplay->GetFrameCount();
    DevLog("After trimming: %i\n", postTrimTickCount);
    char newRecordingPath[MAX_PATH];
    if (StoreReplay(newRecordingPath, MAX_PATH))
    {
        // Note: m_iTickCount updates in TrimReplay(). Passing it here shows the new ticks.
        Log("Recording Stopped! Ticks: %i\n", m_iTickCount);

        if (replaySavedEvent)
        {
            replaySavedEvent->SetBool("save", true);
            // replaySavedEvent->SetString("filename", newRecordingName);
            replaySavedEvent->SetString("filepath", newRecordingPath);
            replaySavedEvent->SetInt("time", static_cast<int>(m_pRecordingReplay->GetRunTime() * 1000.0f));
            gameeventmanager->FireEvent(replaySavedEvent);
        }

        // Load the last run that we did in case we want to watch it
        UnloadPlayback();
        m_pPlaybackReplay = m_pRecordingReplay;
        LoadReplayGhost();
    }
    else
    {
        Warning("Unable to store replay file!\n");
        if (m_pRecordingReplay)
            delete m_pRecordingReplay;
    }

    // Re-allow the player to teleport
    if (m_pPlayer)
        m_pPlayer->SetAllowUserTeleports(true);

    // Reset the m_i*Tick s
    m_iStartRecordingTick = -1;
    m_iStartTimerTick = -1;
    m_iStopTimerTick = -1;
    m_pRecordingReplay = nullptr;
}

bool CMomentumReplaySystem::StoreReplay(char *pOut, size_t outSize)
{
    if (!m_pRecordingReplay)
        return false;

    // Serialize the replay
    CUtlBuffer buf;
    buf.PutUnsignedInt(REPLAY_MAGIC_LE);
    buf.PutUnsignedChar(m_pRecordingReplay->GetVersion());
    m_pRecordingReplay->Serialize(buf);

    // Generate the SHA1 hash for this replay
    char hash[41];
    if (g_pMomentumUtil->GetSHA1Hash(buf, hash, 41))
    {
        DevLog("Replay Hash: %s\n", hash);

        // For later
        m_pRecordingReplay->SetRunHash(hash);

        // Store the file
        CFmtStr newRecordingName("%s-%s%s", gpGlobals->mapname.ToCStr(), hash, EXT_RECORDING_FILE);
        V_ComposeFileName(RECORDING_PATH, newRecordingName.Get(), pOut, outSize);
        Log("Storing replay of version '%d' to %s ...\n", m_pRecordingReplay->GetVersion(), pOut);
        return g_pFullFileSystem->WriteFile(pOut, "MOD", buf);
    }

    return false;
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
    if (m_bRecording && !engine->IsPaused() && !m_bPaused)
    {
        if (!m_pPlayer->m_bHasPracticeMode) // MOM_TODO: && !m_player->IsSpectating
        {
            m_pRecordingReplay->AddFrame(CReplayFrame(m_pPlayer->EyeAngles(), m_pPlayer->GetAbsOrigin(), m_pPlayer->GetViewOffset(),
                                             m_pPlayer->m_nButtons, m_bTeleportedThisFrame));
            m_bTeleportedThisFrame = false;
        }
        else
        {
            // MOM_TODO: The last frame should be repeated until the player returns to their run
        }

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

    // MOM_TODO: Verify the map hash of the replay here with m_szMapHash

    if (bFullLoad && m_pPlaybackReplay)
    {
        // Create the run entity here
        LoadReplayGhost();
    }

    return m_pPlaybackReplay;
}

void CMomentumReplaySystem::SetReplayInfo()
{
    m_pRecordingReplay->SetMapName(gpGlobals->mapname.ToCStr());
    m_pRecordingReplay->SetMapHash(m_szMapHash);
    m_pRecordingReplay->SetPlayerName(m_pPlayer->GetPlayerName());
    ISteamUser *pUser = SteamUser();
    m_pRecordingReplay->SetPlayerSteamID(pUser ? pUser->GetSteamID().ConvertToUint64() : 0);
    m_pRecordingReplay->SetTickInterval(gpGlobals->interval_per_tick);
    m_pRecordingReplay->SetRunTime(g_pMomentumTimer->GetLastRunTime());
    m_pRecordingReplay->SetRunFlags(m_pPlayer->m_Data.m_iRunFlags);
    m_pRecordingReplay->SetRunDate(g_pMomentumTimer->GetLastRunDate());
    m_pRecordingReplay->SetStartTick(m_iStartTimerTick - m_iStartRecordingTick);
    m_pRecordingReplay->SetStopTick(m_iStopTimerTick - m_iStartRecordingTick);
    m_pRecordingReplay->SetBonusZone(g_pMomentumTimer->GetBonus());
}

void CMomentumReplaySystem::SetRunStats()
{
    CMomRunStats* stats = m_pRecordingReplay->CreateRunStats(m_pPlayer->m_RunStats.GetTotalZones());
    m_pPlayer->m_RunStats.FullyCopyStats(stats);
    // MOM_TODO uncomment: stats->SetZoneTime(0, m_pRecordingReplay->GetRunTime());
}

void CMomentumReplaySystem::TogglePause() { m_bPaused = !m_bPaused; }

void CMomentumReplaySystem::SetTeleportedThisFrame()
{
    if (m_bRecording)
    {
        m_bTeleportedThisFrame = true;
    }
}

void CMomentumReplaySystem::StartPlayback(bool firstperson)
{
    if (m_pPlayer)
    {
        SetWasInReplay();

        m_pPlayer->m_vecLastPos = m_pPlayer->GetAbsOrigin();
        m_pPlayer->m_angLastAng = m_pPlayer->GetAbsAngles();
        m_pPlayer->m_vecLastVelocity = m_pPlayer->GetAbsVelocity();
        m_pPlayer->m_fLastViewOffset = m_pPlayer->GetViewOffset().z;
        // memcpy(m_SavedRunStats.m_pData, m_player->m_RunStats.m_pData, sizeof(CMomRunStats::data));
        m_nSavedAccelTicks = m_pPlayer->GetAccelTicks();
        m_nSavedPerfectSyncTicks = m_pPlayer->GetPerfectSyncTicks();
        m_nSavedStrafeTicks = m_pPlayer->GetStrafeTicks();
        CMomentumReplayGhostEntity *pGhost = m_pPlaybackReplay->GetRunEntity();

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

void CMomentumReplaySystem::LoadReplayGhost()
{
    if (m_pPlaybackReplay->GetRunEntity())
        return;

    CMomentumReplayGhostEntity *pGhost =
        static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));
    pGhost->SetRunStats(m_pPlaybackReplay->GetRunStats());
    pGhost->m_Data.m_flRunTime = m_pPlaybackReplay->GetRunTime();
    pGhost->m_Data.m_iRunTimeTicks = m_pPlaybackReplay->GetStopTick() - m_pPlaybackReplay->GetStartTick();
    pGhost->m_Data.m_iRunFlags = m_pPlaybackReplay->GetRunFlags();
    pGhost->m_Data.m_flTickRate = m_pPlaybackReplay->GetTickInterval();
    pGhost->SetPlaybackReplay(m_pPlaybackReplay);
    pGhost->m_iStartTickD = m_pPlaybackReplay->GetStartTick();
    m_pPlaybackReplay->SetRunEntity(pGhost);
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
                    g_ReplaySystem.StartPlayback(firstperson);
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
                             "Begins playback of a replay as a ghost.", RECORDING_PATH, EXT_RECORDING_FILE);
CON_COMMAND_AUTOCOMPLETEFILE(mom_replay_play, CMOMReplayCommands::PlayReplayFirstPerson,
                             "Begins a playback of a replay in first-person mode.", RECORDING_PATH, EXT_RECORDING_FILE);

CON_COMMAND(mom_replay_play_loaded, "Begins playing back a loaded replay (in first person), if there is one.")
{
    if (g_ReplaySystem.GetPlaybackReplay() && !g_ReplaySystem.IsPlayingBack())
    {
        g_ReplaySystem.StartPlayback(true);
        mom_replay_timescale.SetValue(1.0f);
    }
}

CON_COMMAND(mom_replay_restart, "Restarts the current spectated replay, if there is one.")
{
    if (g_ReplaySystem.IsPlayingBack())
    {
        auto pGhost = g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_iCurrentTick = 0;
        }
    }
}

CON_COMMAND(mom_replay_stop, "Stops playing the current replay.")
{
    if (g_ReplaySystem.IsPlayingBack())
    {
        g_ReplaySystem.StopPlayback();
    }
}

CON_COMMAND(mom_replay_pause, "Toggle pausing and playing the playback replay.")
{
    if (g_ReplaySystem.IsPlayingBack())
    {
        auto pGhost = g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_bIsPaused = !pGhost->m_bIsPaused;
        }
    }
}

CON_COMMAND(mom_replay_goto, "Go to a specific tick in the replay.")
{
    if (g_ReplaySystem.IsPlayingBack())
    {
        auto pGhost = g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
        if (pGhost && args.ArgC() > 1)
        {
            int tick = Q_atoi(args[1]);
            if (tick >= 0 && tick <= pGhost->m_iTotalTicks)
            {
                pGhost->m_iCurrentTick = tick;
                pGhost->m_Data.m_bMapFinished = false;
            }
        }
    }
}

CON_COMMAND(mom_replay_goto_end, "Go to the end of the replay.")
{
    if (g_ReplaySystem.IsPlayingBack())
    {
        auto pGhost = g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->m_iCurrentTick = pGhost->m_iTotalTicks - pGhost->m_iStartTickD;
        }
    }
}

CMomentumReplaySystem g_ReplaySystem("MOMReplaySystem");