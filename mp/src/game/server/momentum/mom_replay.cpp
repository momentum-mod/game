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
    m_currentFrame.m_nCurrentTick++;
    m_currentFrame.m_nPlayerButtons = m_player->m_nButtons;
    m_currentFrame.m_qEyeAngles = m_player->EyeAngles();
    m_currentFrame.m_vPlayerOrigin = m_player->GetAbsOrigin();
    m_currentFrame.m_vPlayerVelocity = m_player->GetAbsVelocity();

    ByteSwap_replay_frame_t(m_currentFrame); //We need to byteswap all of our data first in order to write each byte in the correct order

    Assert(buf.IsValid());
    buf.Put(&m_currentFrame, sizeof(replay_frame_t)); //stick all the frame info into the buffer
    return &buf;
}
replay_header_t CMomentumReplaySystem::CreateHeader()
{
    replay_header_t header;
    Q_strcpy(header.mapName, gpGlobals->mapname.ToCStr());
    Q_strcpy(header.playerName, m_player->GetPlayerName());
    header.steamID64 = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
    header.interval_per_tick = gpGlobals->interval_per_tick;
    return header;
}
void CMomentumReplaySystem::WriteRecordingToFile(CUtlBuffer &buf)
{
    if (m_fhFileHandle)
    {
        //write header: Mapname, Playername, steam64, interval per tick
        replay_header_t littleEndianHeader = CreateHeader();
        ByteSwap_replay_header_t(littleEndianHeader); //byteswap again

        filesystem->Seek(m_fhFileHandle, 0, FILESYSTEM_SEEK_HEAD);
        filesystem->Write(&littleEndianHeader, sizeof(replay_header_t), m_fhFileHandle);

        Assert(buf.IsValid());
        //write write from the CUtilBuffer to our filehandle:
        filesystem->Write(buf.Base(), buf.TellPut(), m_fhFileHandle);
        buf.Purge();
    }
}
//read a single frame (or tick) of a recording
replay_frame_t CMomentumReplaySystem::ReadSingleFrame(FileHandle_t file)
{
    replay_frame_t frame;
    Assert(file != FILESYSTEM_INVALID_HANDLE);
    filesystem->Read(&frame, sizeof(replay_frame_t), file);
    ByteSwap_replay_frame_t(frame);

    return frame;
}
replay_header_t CMomentumReplaySystem::ReadHeader(FileHandle_t file)
{
    replay_header_t header;
    Q_memset(&header, 0, sizeof(header));

    Assert(file != FILESYSTEM_INVALID_HANDLE);
    filesystem->Seek(file, 0, FILESYSTEM_SEEK_HEAD);
    filesystem->Read(&header, sizeof(replay_header_t), file);

    ByteSwap_replay_header_t(header);

    return header;
}
void CMomentumReplaySystem::LoadRun(const char* filename)
{
    m_vecRunData.RemoveAll();
    char recordingName[BUFSIZELOCL];
    V_ComposeFileName(RECORDING_PATH, filename, recordingName, BUFSIZELOCL);
    m_fhFileHandle = filesystem->Open(recordingName, "r+b", "MOD");

    if (m_fhFileHandle != nullptr && filename != NULL)
    {
        //NNOM_TODO: Do something with the run header data
        replay_header_t header = CMomentumReplaySystem::ReadHeader(m_fhFileHandle);
        DevLog("playername: %s mapname: %s steamid: %llu tickrate: %f", header.playerName, header.mapName, header.steamID64, header.interval_per_tick);
        while (!filesystem->EndOfFile(m_fhFileHandle))
        {
            replay_frame_t frame = CMomentumReplaySystem::ReadSingleFrame(m_fhFileHandle);
            m_vecRunData.AddToTail(frame);
        }
    }
    filesystem->Close(m_fhFileHandle);
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