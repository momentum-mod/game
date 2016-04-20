#include "cbase.h"
#include "utlbuffer.h"
#include "mom_replay.h"
#include "mom_replay_entity.h"
#include "Timer.h"

void CMomentumReplaySystem::BeginRecording(CBasePlayer *pPlayer)
{
    m_player = pPlayer;
    m_bIsRecording = true;
    Log("Recording began!\n");
}
void CMomentumReplaySystem::StopRecording(CBasePlayer *pPlayer, bool throwaway)
{
    if (throwaway)
    {
        m_bIsRecording = false;
        return;
    }
    CMomentumPlayer *pMOMPlayer = ToCMOMPlayer(pPlayer);
    char newRecordingName[MAX_PATH], newRecordingPath[MAX_PATH];
    Q_snprintf(newRecordingName, MAX_PATH, "%s_%s_%.3f.momrec", pMOMPlayer->GetPlayerName(), gpGlobals->mapname.ToCStr(), g_Timer.GetLastRunTime());
    V_ComposeFileName(RECORDING_PATH, newRecordingName, newRecordingPath, MAX_PATH); //V_ComposeFileName calls all relevent filename functions for us! THANKS GABEN

    V_FixSlashes(RECORDING_PATH);
    filesystem->CreateDirHierarchy(RECORDING_PATH, "MOD"); //we have to create the directory here just in case it doesnt exist yet

    m_fhFileHandle = filesystem->Open(newRecordingPath, "w+b", "MOD");

    WriteRecordingToFile(*m_buf);

    filesystem->Close(m_fhFileHandle);
    Log("Recording Stopped! Ticks: %i\n", m_currentFrame.m_nCurrentTick);
    m_bIsRecording = false;
    LoadRun(newRecordingName); //load the last run that we did in case we want to watch it
}
CUtlBuffer *CMomentumReplaySystem::UpdateRecordingParams()
{
    //TODO: figure out why we have to declare the buffer in this scope, and maybe change it
    static CUtlBuffer buf;

    buf.PutInt(m_player->m_nButtons);
    buf.PutInt(++m_currentFrame.m_nCurrentTick);
    buf.PutFloat(m_player->EyeAngles().x);
    buf.PutFloat(m_player->EyeAngles().y);
    buf.PutFloat(m_player->EyeAngles().z);
    buf.PutFloat(m_player->GetLocalVelocity().x);
    buf.PutFloat(m_player->GetLocalVelocity().y);
    buf.PutFloat(m_player->GetLocalVelocity().z);
    buf.PutFloat(m_player->GetLocalOrigin().x);
    buf.PutFloat(m_player->GetLocalOrigin().y);
    buf.PutFloat(m_player->GetLocalOrigin().z);
    return &buf;
}
replay_header_t CMomentumReplaySystem::CreateHeader()
{
    replay_header_t header;
    header.interval_per_tick = gpGlobals->interval_per_tick;
    header.mapName = gpGlobals->mapname.ToCStr();
    header.playerName = m_player->GetPlayerName();
    header.steamID64 = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
    return header;
}
void CMomentumReplaySystem::WriteRecordingToFile(CUtlBuffer &buf)
{
    if (m_fhFileHandle)
    {
        replay_header_t header = CreateHeader();
        //write header: Mapname, Playername, steam64, interval per tick
        filesystem->FPrintf(m_fhFileHandle, "|| %s %s %llu %f\n",
            header.mapName,
            header.playerName,
            header.steamID64,
            header.interval_per_tick
            );
        //buttons, eyeangles XYZ, velocity XYZ, origin XYZ
        filesystem->Write(buf.Base(), buf.TellPut(), m_fhFileHandle);
        buf.Purge();
    }
}
//read a single frame of a recording
replay_frame_t CMomentumReplaySystem::ReadSingleFrame(FileHandle_t file)
{
    replay_frame_t frame;

    char cmp[256];
    filesystem->ReadLine(cmp, 256, file);

    if (Q_strncmp(cmp, "||", sizeof(cmp)) != 0) //check to see that we're not trying to read the header
    {
        filesystem->Read(&frame.m_nCurrentTick, sizeof(frame.m_nCurrentTick), file);
        filesystem->Read(&frame.m_nPlayerButtons, sizeof(frame.m_nPlayerButtons), file);

        for (int i = 0; i < 2; i++) //loop through XYZ
            filesystem->Read(&frame.m_qEyeAngles[i], sizeof(frame.m_qEyeAngles[i]), file);
        for (int i = 0; i < 2; i++)
            filesystem->Read(&frame.m_vPlayerVelocity[i], sizeof(frame.m_vPlayerVelocity[i]), file);
        for (int i = 0; i < 2; i++)
            filesystem->Read(&frame.m_vPlayerOrigin[i], sizeof(frame.m_vPlayerOrigin[i]), file);
    }
    return frame;
}
replay_header_t CMomentumReplaySystem::ReadHeader(FileHandle_t file)
{
    replay_header_t header;

    char cmp[256];
    filesystem->ReadLine(cmp, 256, file);
    if (Q_strncmp(cmp, "||", sizeof(cmp)) == 0)
    {
        filesystem->Read(&header.interval_per_tick, sizeof(header.interval_per_tick), file);
        filesystem->Read(&header.mapName, sizeof(header.mapName), file);
        filesystem->Read(&header.playerName, sizeof(header.playerName), file);
        filesystem->Read(&header.steamID64, sizeof(header.steamID64), file);
    }
    return header;
}
void CMomentumReplaySystem::LoadRun(const char* filename)
{
    m_vecRunData.RemoveAll();
    char recordingName[BUFSIZELOCL];
    V_ComposeFileName(RECORDING_PATH, filename, recordingName, MAX_PATH);
    FileHandle_t replayFile = filesystem->Open(recordingName, "r+b", "MOD");

    if (replayFile != nullptr && filename != NULL)
    {
        //NNOM_TODO: Do something with the run header data
        //replay_header_t header = CMomentumReplaySystem::ReadHeader(replayFile); 
        while (!filesystem->EndOfFile(replayFile))
        {
            replay_frame_t frame = CMomentumReplaySystem::ReadSingleFrame(replayFile);
            m_vecRunData.AddToTail(frame);
        }
    }
    filesystem->Close(replayFile);
}
void CMomentumReplaySystem::StartRun()
{
    CMomentumReplayGhostEntity *ghost = static_cast<CMomentumReplayGhostEntity*>(CreateEntityByName("mom_replay_ghost"));
    if (ghost != nullptr)
    {
        FOR_EACH_VEC(m_vecRunData, i)
        {
            ghost->m_entRunData[i] = m_vecRunData[i];
        }
        ghost->StartRun();
    }
}
class CMOMReplayCommands
{
public:
    static void PlayRecording(const CCommand &args)
    {
        g_ReplaySystem->LoadRun(args.ArgS());
        g_ReplaySystem->StartRun();
    }
};

static ConCommand playrecording("playrecording", CMOMReplayCommands::PlayRecording, "plays a recording", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE);

static CMomentumReplaySystem s_ReplaySystem("MOMReplaySystem");
CMomentumReplaySystem *g_ReplaySystem = &s_ReplaySystem;