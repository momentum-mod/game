#include "cbase.h"
#include "mom_replay.h"
#include "Timer.h"

void CMomentumReplaySystem::BeginRecording(CBasePlayer *pPlayer)
{
    Reset();
    m_player = pPlayer;
    //delete old temp recording
    V_ComposeFileName(recordingPath, "temprecording.momrec", tempRecordingName, MAX_PATH); //we only need to do this once!
    fh = filesystem->Open(tempRecordingName, "w+", "MOD");
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
    filesystem->Close(fh);
    Log("Recording Stopped! Ticks: %i\n", m_nRecordingTicks);
    m_bIsRecording = false;

    CMomentumPlayer *mPlayer = ToCMOMPlayer(pPlayer);
    char newRecordingName[MAX_PATH], newRecordingPath[MAX_PATH];
    Q_snprintf(newRecordingName, MAX_PATH, "%s_%s_%f.momrec", mPlayer->GetPlayerName(), gpGlobals->mapname.ToCStr(), g_Timer.GetLastRunTime());
    V_ComposeFileName(recordingPath, newRecordingName, newRecordingPath, MAX_PATH);
    V_FixSlashes(newRecordingName);
    if (filesystem->FileExists(tempRecordingName, "MOD"))
    {
        filesystem->RenameFile(tempRecordingName, newRecordingPath, "MOD");
    }
    else
        Warning("Recording file doesn't exist, cannot rename!");
}
void CMomentumReplaySystem::UpdateRecordingParams()
{
    m_currentFrame.m_nPlayerButtons = m_player->m_nButtons;
    m_currentFrame.m_qEyeAngles = m_player->EyeAngles();
    m_currentFrame.m_vPlayerVelocity = m_player->GetLocalVelocity();
    m_currentFrame.m_vPlayerOrigin = m_player->GetLocalOrigin();
    m_nRecordingTicks++;
}
void CMomentumReplaySystem::WriteRecordingToFile()
{
    
    if (fh)
    {
        //buttons, eyeangles XYZ, velocity XYZ, origin XYZ
        filesystem->FPrintf(fh, "%i %f, %f, %f %f, %f, %f %f, %f, %f\n",
            m_currentFrame.m_nPlayerButtons,
            m_currentFrame.m_qEyeAngles.x, m_currentFrame.m_qEyeAngles.y, m_currentFrame.m_qEyeAngles.z,
            m_currentFrame.m_vPlayerVelocity.x, m_currentFrame.m_vPlayerVelocity.y, m_currentFrame.m_vPlayerVelocity.z,
            m_currentFrame.m_vPlayerOrigin.x, m_currentFrame.m_vPlayerOrigin.y, m_currentFrame.m_vPlayerOrigin.z
            );
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