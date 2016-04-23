#ifndef MOM_REPLAY_H
#define MOM_REPLAY_H
#include "cbase.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "mom_player_shared.h"
#include "mom_shareddefs.h"

#define RECORDING_PATH "recordings"
#define DEMO_HEADER_ID "MOMREPLAY"
#define DEMO_PROTOCOL_VERSION 2

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
//the replay header, stores a bunch of information about the replay as well as the run stats for that replay
struct replay_header_t
{
    char demofilestamp[9];      //should be DEMO_HEADER_ID
    int demoProtoVersion;       //should be DEMO_PROTOCOL_VERSION
    time_t unixEpocDate;        //redundant date check
    char mapName[MAX_PATH];
    char playerName[MAX_PATH];
    uint64 steamID64;
    float interval_per_tick;
    int runTimeTicks;           //Total run time in ticks

    float m_flStartSpeed, m_flEndSpeed;
    int m_nStageJumps[MAX_STAGES], m_nStageStrafes[MAX_STAGES];
    float m_flStageVelocityMax[MAX_STAGES], m_flStageVelocityAvg[MAX_STAGES],
        m_flStageStrafeSyncAvg[MAX_STAGES], m_flStageStrafeSync2Avg[MAX_STAGES], m_flStageEnterVelocity[MAX_STAGES];

};
//byteswap for int and float members of header, swaps the endianness (byte order) in order to read correctly
inline void ByteSwap_replay_header_t(replay_header_t &swap) 
{
    swap.demoProtoVersion = LittleDWord(swap.demoProtoVersion);
    swap.runTimeTicks = LittleDWord(swap.runTimeTicks);
    swap.unixEpocDate = LittleLong(swap.unixEpocDate);
    swap.steamID64 = LittleLong(swap.steamID64);
    LittleFloat(&swap.interval_per_tick, &swap.interval_per_tick);

    // --- run stats ---
    LittleFloat(&swap.m_flEndSpeed, &swap.m_flEndSpeed);
    LittleFloat(&swap.m_flStartSpeed, &swap.m_flStartSpeed);
    for (int i = 0; i < MAX_STAGES; i++) {
        LittleFloat(&swap.m_flStageVelocityMax[i], &swap.m_flStageVelocityMax[i]);
        LittleFloat(&swap.m_flStageVelocityAvg[i], &swap.m_flStageVelocityAvg[i]);
        LittleFloat(&swap.m_flStageStrafeSyncAvg[i], &swap.m_flStageStrafeSyncAvg[i]);
        LittleFloat(&swap.m_flStageStrafeSync2Avg[i], &swap.m_flStageStrafeSync2Avg[i]);
        LittleFloat(&swap.m_flStageEnterVelocity[i], &swap.m_flStageEnterVelocity[i]);
        swap.m_nStageJumps[i] = LittleDWord(swap.m_nStageJumps[i]);
        swap.m_nStageStrafes[i] = LittleDWord(swap.m_nStageStrafes[i]);
    }
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

    replay_frame_t* ReadSingleFrame(FileHandle_t file, const char* filename);
    replay_header_t* ReadHeader(FileHandle_t file, const char* filename);

    void StartRun();
    void EndRun();
    void LoadRun(const char* fileName);
    CUtlVector<replay_frame_t*> m_vecRunData;


private:
    CUtlBuffer *UpdateRecordingParams(); //called every game frame after entities think and update
    bool m_bIsRecording;
    CMomentumPlayer *m_player;

    replay_frame_t m_currentFrame;
    replay_header_t m_replayHeader;

    FileHandle_t m_fhFileHandle;
    CUtlBuffer *m_buf;
    const static char CONTROL_CHAR = '\u001F'; //CONTROL CHAR = UNICODE 001F (37) [00011111]
};

extern CMomentumReplaySystem *g_ReplaySystem;

#endif //MOM_REPLAY_H