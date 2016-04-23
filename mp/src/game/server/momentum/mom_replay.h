#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H
#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "mom_player_shared.h"
#include "mom_shareddefs.h"

#define RECORDING_PATH "recordings"
//describes a single frame of a replay
struct replay_frame_t
{
    QAngle m_qEyeAngles;
    Vector m_vPlayerOrigin;
    Vector m_vPlayerVelocity;
    int m_nPlayerButtons;
    int m_nCurrentTick;
};
inline void ByteSwap_replay_frame_t(replay_frame_t &swap)
{
    for (int i = 0; i < 2; i++) {
        LittleFloat(&swap.m_qEyeAngles[i], &swap.m_qEyeAngles[i]);
        LittleFloat(&swap.m_vPlayerOrigin[i], &swap.m_vPlayerOrigin[i]);
        LittleFloat(&swap.m_vPlayerVelocity[i], &swap.m_vPlayerVelocity[i]);
    }
    swap.m_nPlayerButtons = LittleDWord(swap.m_nPlayerButtons);
    swap.m_nCurrentTick = LittleDWord(swap.m_nCurrentTick);
}
//the replay header
struct replay_header_t
{
    char mapName[MAX_PATH];
    char playerName[MAX_PATH];
    uint64 steamID64;
    float interval_per_tick;
};
//byteswap for int and float members of header
inline void ByteSwap_replay_header_t(replay_header_t &swap) 
{
    LittleFloat(&swap.interval_per_tick, &swap.interval_per_tick);
    swap.steamID64 = LittleLong(swap.steamID64);
}

class CMomentumReplaySystem : CAutoGameSystemPerFrame
{
public:
    CMomentumReplaySystem(const char *pName) : CAutoGameSystemPerFrame(pName) {}
    virtual void FrameUpdatePostEntityThink() //inherited member from CAutoGameSystemPerFrame
    {
        if (m_bIsRecording)
        {
            m_buf = UpdateRecordingParams();
        }
    }
    void BeginRecording(CBasePlayer *pPlayer);
    void StopRecording(CBasePlayer *pPlayer, bool throwaway);
    bool IsRecording(CBasePlayer *pPlayer) { return m_bIsRecording; }
    void WriteRecordingToFile(CUtlBuffer &buf);
    replay_header_t CreateHeader();
    void WriteRecordingToFile();

    replay_frame_t ReadSingleFrame(FileHandle_t file);
    replay_header_t ReadHeader(FileHandle_t file);

    void StartRun();
    void EndRun();
    void LoadRun(const char* fileName);
    CUtlVector<replay_frame_t> m_vecRunData;


private:
    CUtlBuffer *UpdateRecordingParams(); //called every game frame after entities think and update
    bool m_bIsRecording;
    CBasePlayer *m_player;

    replay_frame_t m_currentFrame;

    FileHandle_t m_fhFileHandle;
    CUtlBuffer *m_buf;
    const static char CONTROL_CHAR = '\u001F'; //CONTROL CHAR = UNICODE 001F (37) [00011111]
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif //MOM_REPLAY_H