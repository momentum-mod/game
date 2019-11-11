#include "cbase.h"

#include "time.h"
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

MAKE_CONVAR(mom_replay_timescale, "1.0", FCVAR_NONE, "The timescale of a replay. > 1 is faster, < 1 is slower. \n", 0.01f, 10.0f);
MAKE_CONVAR(mom_replay_selection, "0", FCVAR_NONE, "Going forward or backward in the replayui \n", 0, 2);

CMomentumReplaySystem::CMomentumReplaySystem(const char* pName):
    CAutoGameSystemPerFrame(pName), m_bRecording(false), m_bPlayingBack(false), m_pPlaybackReplay(nullptr),
    m_bShouldStopRec(false),
    m_iTickCount(0),
    m_iStartRecordingTick(0),
    m_iStartTimerTick(0),
    m_iStopTimerTick(0),
    m_bTeleportedThisFrame(false),
    m_fRecEndTime(-1.0f)
{
    m_szMapHash[0] = '\0';
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
    if (!MomUtil::GetFileHash(m_szMapHash, sizeof(m_szMapHash), CFmtStr("maps/%s.bsp", gpGlobals->mapname.ToCStr())))
        Warning("Could not generate a hash for the current map!!!\n");
}

void CMomentumReplaySystem::LevelShutdownPostEntity()
{
    //Stop a recording if there is one while the level shuts down
    if (m_bRecording)
        CancelRecording();

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
    if (m_bRecording || m_pRecordingReplay)
        return;

    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer || pPlayer->GetObserverMode() != OBS_MODE_NONE)
        return;

    m_bRecording = true;
    m_iTickCount = 1; // recording begins at 1 ;)
    m_iStartRecordingTick = gpGlobals->tickcount;
    m_pRecordingReplay = g_ReplayFactory.CreateEmptyReplay(0);
}

void CMomentumReplaySystem::CancelRecording()
{
    m_bRecording = false;

    if (m_pRecordingReplay)
        delete m_pRecordingReplay;

    m_pRecordingReplay = nullptr;

    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        pPlayer->SetAllowUserTeleports(true);
    }

    const auto pReplaySavedEvent = gameeventmanager->CreateEvent("replay_save");
    if (pReplaySavedEvent)
    {
        pReplaySavedEvent->SetBool("save", false);
        gameeventmanager->FireEvent(pReplaySavedEvent);
    }
}

void CMomentumReplaySystem::StopRecording()
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        pPlayer->SetAllowUserTeleports(false);

        SetReplayHeaderAndStats();

        m_bShouldStopRec = true;
        m_fRecEndTime = gpGlobals->curtime + END_RECORDING_DELAY;
    }
}

void CMomentumReplaySystem::FinishRecording()
{
    m_bShouldStopRec = false;
    m_bRecording = false;

    DevLog("Before trimming: %i\n", m_iTickCount);
    TrimReplay();
    DevLog("After trimming: %i\n", m_pRecordingReplay->GetFrameCount());

    char newRecordingPath[MAX_PATH];
    if (StoreReplay(newRecordingPath, MAX_PATH))
    {
        // Note: m_iTickCount updates in TrimReplay(). Passing it here shows the new ticks.
        Log("Recording Stopped! Ticks: %i\n", m_iTickCount);

        const auto pReplaySavedEvent = gameeventmanager->CreateEvent("replay_save");
        if (pReplaySavedEvent)
        {
            pReplaySavedEvent->SetBool("save", true);
            // replaySavedEvent->SetString("filename", newRecordingName);
            pReplaySavedEvent->SetString("filepath", newRecordingPath);
            pReplaySavedEvent->SetInt("time", static_cast<int>(m_pRecordingReplay->GetRunTime() * 1000.0f));
            gameeventmanager->FireEvent(pReplaySavedEvent);
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

    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
        pPlayer->SetAllowUserTeleports(true);

    // Reset the m_i*Tick s
    m_iStartRecordingTick = 0;
    m_iStartTimerTick = 0;
    m_iStopTimerTick = 0;
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
    if (MomUtil::GetSHA1Hash(buf, hash, 41))
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
    if (m_iStartRecordingTick > 0 && m_iStartTimerTick > 0)
    {
        const auto newStart = m_iStartTimerTick - static_cast<int>(START_TRIGGER_TIME_SEC / gpGlobals->interval_per_tick);
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
    if (m_bRecording)
    {
        const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
        if (!pPlayer->m_bHasPracticeMode && pPlayer->GetObserverMode() == OBS_MODE_NONE)
        {
            m_pRecordingReplay->AddFrame(CReplayFrame(pPlayer->EyeAngles(), pPlayer->GetAbsOrigin(), pPlayer->GetViewOffset().z,
                                             pPlayer->m_nButtons, m_bTeleportedThisFrame));
            m_bTeleportedThisFrame = false;
        }
        else
        {
            // MOM_TODO just repeat the last frame created (part of the mega refactor)
            SavedState_t *pSaved = pPlayer->GetSavedRunState();
            m_pRecordingReplay->AddFrame(CReplayFrame(pSaved->m_angLastAng, pSaved->m_vecLastPos, pSaved->m_fLastViewOffset, pSaved->m_nButtons, false));
        }

        ++m_iTickCount; // increment recording tick
    }

    if (m_bShouldStopRec && m_fRecEndTime < gpGlobals->curtime)
        FinishRecording();
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

void CMomentumReplaySystem::SetReplayHeaderAndStats()
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    ISteamUser *pUser = SteamUser();

    // Header
    m_pRecordingReplay->SetMapName(gpGlobals->mapname.ToCStr());
    m_pRecordingReplay->SetMapHash(m_szMapHash);
    m_pRecordingReplay->SetPlayerName(pPlayer->GetPlayerName());
    m_pRecordingReplay->SetPlayerSteamID(pUser ? pUser->GetSteamID().ConvertToUint64() : 0);
    m_pRecordingReplay->SetTickInterval(gpGlobals->interval_per_tick);
    m_pRecordingReplay->SetRunFlags(pPlayer->m_Data.m_iRunFlags);
    m_pRecordingReplay->SetRunDate(time(nullptr));
    m_pRecordingReplay->SetStartTick(m_iStartTimerTick - m_iStartRecordingTick);
    m_pRecordingReplay->SetStopTick(m_iStopTimerTick - m_iStartRecordingTick);
    m_pRecordingReplay->SetTrackNumber(g_pMomentumTimer->GetTrackNumber());
    m_pRecordingReplay->SetZoneNumber(0); // MOM_TODO (0.9.0) allow individual zone runs

    // Stats
    CMomRunStats *stats = m_pRecordingReplay->CreateRunStats(pPlayer->m_RunStats.GetTotalZones());
    stats->FullyCopyFrom(pPlayer->m_RunStats);
    // MOM_TODO uncomment: stats->SetZoneTime(0, m_pRecordingReplay->GetRunTime());
}

void CMomentumReplaySystem::SetTeleportedThisFrame()
{
    if (m_bRecording)
    {
        m_bTeleportedThisFrame = true;
    }
}

void CMomentumReplaySystem::StartPlayback(bool firstperson)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        CMomentumReplayGhostEntity *pGhost = m_pPlaybackReplay->GetRunEntity();

        if (pGhost)
        {
            mom_replay_timescale.SetValue(1.0f);
            mom_replay_selection.SetValue(0);

            pGhost->StartRun(firstperson);
        }

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

    auto pGhost = static_cast<CMomentumReplayGhostEntity *>(CreateEntityByName("mom_replay_ghost"));
    pGhost->LoadFromReplayBase(m_pPlaybackReplay);
}

void CMomentumReplaySystem::StopPlayback()
{
    if (!m_bPlayingBack)
        return;

    Log("Stopping replay playback.\n");
    UnloadPlayback();
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
    }
}

CON_COMMAND(mom_replay_restart, "Restarts the current spectated replay, if there is one.")
{
    if (g_ReplaySystem.IsPlayingBack())
    {
        auto pGhost = g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
        if (pGhost)
        {
            pGhost->GoToTick(0);
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
            pGhost->GoToTick(Q_atoi(args[1]));
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
            pGhost->GoToTick(pGhost->m_iTotalTicks - pGhost->m_Data.m_iStartTick);
        }
    }
}

CMomentumReplaySystem g_ReplaySystem("MOMReplaySystem");