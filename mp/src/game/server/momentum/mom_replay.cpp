#include "cbase.h"
#include "utlbuffer.h"
#include "mom_replay.h"
#include "Timer.h"

void CMomentumReplaySystem::BeginRecording(CBasePlayer *pPlayer)
{
    Reset();
    m_player = pPlayer;
    //delete old temp recording
    V_ComposeFileName(recordingPath, "temprecording.momrec", tempRecordingName, MAX_PATH); //we only need to do this once!
    fh = filesystem->Open(tempRecordingName, "w+b", "MOD");
    Log("Recording began!\n");
    m_bIsRecording = true;

}
void CMomentumReplaySystem::StopRecording(CBasePlayer *pPlayer, bool throwaway)
{
    if (throwaway)
    {
        m_bIsRecording = false;
        return;
    }

    WriteRecordingToFile(*buf);

    filesystem->Close(fh);
    Log("Recording Stopped! Ticks: %i\n", m_nRecordingTicks);
    m_bIsRecording = false;

    CMomentumPlayer *mPlayer = ToCMOMPlayer(pPlayer);
    char newRecordingName[MAX_PATH], newRecordingPath[MAX_PATH];
    Q_snprintf(newRecordingName, MAX_PATH, "%s_%s_%f.momrec", mPlayer->GetPlayerName(), gpGlobals->mapname.ToCStr(), g_Timer.GetLastRunTime());
    V_ComposeFileName(recordingPath, newRecordingName, newRecordingPath, MAX_PATH);
    if (filesystem->FileExists(tempRecordingName, "MOD"))
    {
        filesystem->RenameFile(tempRecordingName, newRecordingPath, "MOD");
    }
    else
        Warning("Recording file doesn't exist, cannot rename!");
}
CUtlBuffer *CMomentumReplaySystem::UpdateRecordingParams()
{
    //TODO: figure out why we have to declare the buffer in this scope, and maybe change it
    static CUtlBuffer buf;

    buf.PutInt(m_player->m_nButtons);
    buf.PutFloat(m_player->EyeAngles().x);
    buf.PutFloat(m_player->EyeAngles().y);
    buf.PutFloat(m_player->EyeAngles().z);
    buf.PutFloat(m_player->GetLocalVelocity().x);
    buf.PutFloat(m_player->GetLocalVelocity().y);
    buf.PutFloat(m_player->GetLocalVelocity().z);
    buf.PutFloat(m_player->GetLocalOrigin().x);
    buf.PutFloat(m_player->GetLocalOrigin().y);
    buf.PutFloat(m_player->GetLocalOrigin().z);
    m_nRecordingTicks++;
    return &buf;
}
void CMomentumReplaySystem::WriteRecordingToFile(CUtlBuffer &buf)
{
    if (fh)
    {
        //buttons, eyeangles XYZ, velocity XYZ, origin XYZ
        filesystem->Write(buf.Base(), buf.TellPut(), fh);
        buf.Purge();
    }
}
class CMOMReplayCommands
{
public:
    static void PlayRecording(const CCommand &args)
    {
        char recordingName[BUFSIZELOCL];
        V_ComposeFileName("recordings", args.ArgS(), recordingName, MAX_PATH);

        if (Q_strlen(args.GetCommandString()) > 1)
        {
            CBasePlayer *pPlayer = UTIL_GetListenServerHost();
            FileHandle_t fh = filesystem->Open(recordingName, "r", "MOD");
            if (fh)
            {
                int file_len = filesystem->Size(fh);
                char recordingLine[1024];
                for (int i = 1; i < file_len; i++)
                {
                    filesystem->ReadLine(recordingLine, sizeof(recordingLine), fh);
                    pPlayer->m_nButtons = Q_atoi(recordingLine);
                }
                //pPlayer->SetLocalOrigin(Vector(, ));
                //pPlayer->SetLocalVelocity(Vector());
                //pPlayer->SetLocalAngles(QAngle());
                filesystem->Close(fh);
            }
        }
    }
};

static ConCommand playrecording("playrecording", CMOMReplayCommands::PlayRecording, "plays a recording", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE);

static CMomentumReplaySystem s_ReplaySystem("MOMReplaySystem");
CMomentumReplaySystem *g_ReplaySystem = &s_ReplaySystem;