#ifndef REPLAYFORMAT_H
#define REPLAYFORMAT_H

#include "cbase.h"
#include "mom_shareddefs.h"

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
#endif //REPLAYFORMAT_H