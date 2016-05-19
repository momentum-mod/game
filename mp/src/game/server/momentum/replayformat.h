#ifndef REPLAYFORMAT_H
#define REPLAYFORMAT_H

#include "cbase.h"
#include "mom_shareddefs.h"
#include "util/run_stats.h"

#define DEMO_HEADER_ID "MOMREPLAY"
#define DEMO_PROTOCOL_VERSION 2

//describes a single frame of a replay
struct replay_frame_t
{
    QAngle m_qEyeAngles;
    Vector m_vPlayerOrigin;
    int m_nPlayerButtons;

    replay_frame_t& operator=(const replay_frame_t &src)
    {
        if (this == &src)
            return *this;
        m_qEyeAngles = src.m_qEyeAngles;
        m_nPlayerButtons = src.m_nPlayerButtons;
        m_vPlayerOrigin = src.m_vPlayerOrigin;
        
        return *this;
    }
};
inline void ByteSwap_replay_frame_t(replay_frame_t &swap)
{
    for (int i = 0; i < 2; i++) {
        LittleFloat(&swap.m_qEyeAngles[i], &swap.m_qEyeAngles[i]);
        LittleFloat(&swap.m_vPlayerOrigin[i], &swap.m_vPlayerOrigin[i]);
    }
    swap.m_nPlayerButtons = LittleDWord(swap.m_nPlayerButtons);
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

    RunStats_t stats;
    /*float m_flStartSpeed, m_flEndSpeed;
    int m_nStageJumps[MAX_STAGES], m_nStageStrafes[MAX_STAGES];
    float m_flStageVelocityMax[MAX_STAGES], m_flStageVelocityAvg[MAX_STAGES],
        m_flStageStrafeSyncAvg[MAX_STAGES], m_flStageStrafeSync2Avg[MAX_STAGES], m_flStageEnterVelocity[MAX_STAGES];*/
};
//byteswap for int and float members of header, swaps the endianness (byte order) in order to read correctly
inline void ByteSwap_replay_header_t(replay_header_t &swap)
{
    swap.demoProtoVersion = LittleDWord(swap.demoProtoVersion);
    swap.runTimeTicks = LittleDWord(swap.runTimeTicks);
    swap.unixEpocDate = LittleLong(swap.unixEpocDate);
    swap.steamID64 = LittleLong(swap.steamID64);
    LittleFloat(&swap.interval_per_tick, &swap.interval_per_tick);
    //MOM_TODO: Do we want to also have a float time?
    // --- run stats ---
    for (int i = 0; i < 2; i++)
    {
        LittleFloat(&swap.stats.m_flStageExitSpeed[0][i], &swap.stats.m_flStageExitSpeed[0][i]);
        LittleFloat(&swap.stats.m_flStageEnterSpeed[0][i], &swap.stats.m_flStageEnterSpeed[0][i]);
    }
    
    for (int i = 0; i < MAX_STAGES; i++) {

        for (int k = 0; k < 2; k++)
        {
            LittleFloat(&swap.stats.m_flStageEnterSpeed[i][k], &swap.stats.m_flStageEnterSpeed[i][k]);
            LittleFloat(&swap.stats.m_flStageExitSpeed[i][k], &swap.stats.m_flStageExitSpeed[i][k]);
            LittleFloat(&swap.stats.m_flStageVelocityAvg[i][k], &swap.stats.m_flStageVelocityAvg[i][k]);
            LittleFloat(&swap.stats.m_flStageVelocityMax[i][k], &swap.stats.m_flStageVelocityMax[i][k]);
        }

        LittleFloat(&swap.stats.m_flStageStrafeSyncAvg[i], &swap.stats.m_flStageStrafeSyncAvg[i]);
        LittleFloat(&swap.stats.m_flStageStrafeSync2Avg[i], &swap.stats.m_flStageStrafeSync2Avg[i]);
        swap.stats.m_iStageJumps[i] = LittleDWord(swap.stats.m_iStageJumps[i]);
        swap.stats.m_iStageStrafes[i] = LittleDWord(swap.stats.m_iStageStrafes[i]);
    }
}
#endif //REPLAYFORMAT_H