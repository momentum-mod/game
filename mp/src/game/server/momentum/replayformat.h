#ifndef REPLAYFORMAT_H
#define REPLAYFORMAT_H

#include "cbase.h"
#include "filesystem.h"
#include "mom_shareddefs.h"
#include "util/run_stats.h"

#define REPLAY_HEADER_ID "MOMREPLAY"
#define REPLAY_PROTOCOL_VERSION 4

// describes a single frame of a replay
struct replay_frame_t
{
    QAngle m_qEyeAngles;
    Vector m_vPlayerOrigin;
    int m_nPlayerButtons;

    replay_frame_t &operator=(const replay_frame_t &src)
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
    for (int i = 0; i < 2; i++)
    {
        LittleFloat(&swap.m_qEyeAngles[i], &swap.m_qEyeAngles[i]);
        LittleFloat(&swap.m_vPlayerOrigin[i], &swap.m_vPlayerOrigin[i]);
    }
    swap.m_nPlayerButtons = LittleDWord(swap.m_nPlayerButtons);
}

// the replay header, stores a bunch of information about the replay
struct replay_header_t
{
    int numZones;              // the number of zones, controls the size of the runstats struct array
    char demofilestamp[9];     // should be REPLAY_HEADER_ID
    int demoProtoVersion;      // should be REPLAY_PROTOCOL_VERSION
    time_t unixEpocDate;       // redundant date check
    char mapName[MAX_PATH];    // The map that this run was done on
    char playerName[MAX_PATH]; // The name of the player who did this run
    uint64 steamID64;          // The steam64 ID of the player who did this run
    float interval_per_tick;   // The tickrate of the run
    float runTime;             // The overall time of the run in seconds
    int runFlags;              // Should be the flags that the player ran this run with

    replay_header_t()
    {
        numZones = -1;
        demofilestamp[0] = '\0';
        demoProtoVersion = -1;
        unixEpocDate = 0;
        mapName[0] = '\0';
        playerName[0] = '\0';
        steamID64 = uint64(-1);
        interval_per_tick = 0.0f;
        runTime = 0.0f;
        runFlags = -1;
    }
};

//File loading
inline void HandleReplayHeader(replay_header_t &header, FileHandle_t file, bool read)
{
    void(*handle)(const void*, int, FileHandle_t) = read ? &RunStats_t::Read : &RunStats_t::Write;

    handle(&header.numZones, sizeof(header.numZones), file);
    handle(&header.demofilestamp, sizeof(header.demofilestamp), file);
    handle(&header.demoProtoVersion, sizeof(header.demoProtoVersion), file);
    handle(&header.unixEpocDate, sizeof(header.unixEpocDate), file);
    //If we're reading, this value will be overridden
    int mapNameSize = Q_strlen(header.mapName) + 1;//+ 1 for null termination char \0
    handle(&mapNameSize, sizeof(mapNameSize), file);
    handle(&header.mapName, mapNameSize, file);
    int playerNameSize = Q_strlen(header.playerName) + 1;
    handle(&playerNameSize, sizeof(playerNameSize), file);
    handle(&header.playerName, playerNameSize, file);
    handle(&header.steamID64, sizeof(header.steamID64), file);
    handle(&header.interval_per_tick, sizeof(header.interval_per_tick), file);
    handle(&header.runTime, sizeof(header.runTime), file);
    handle(&(header.runFlags), sizeof(header.runFlags), file);
}

// byteswap for int and float members of header, swaps the endianness (byte order) in order to read correctly
inline void ByteSwap_replay_header_t(replay_header_t &swap)
{
    swap.numZones = LittleDWord(swap.numZones);
    swap.demoProtoVersion = LittleDWord(swap.demoProtoVersion);
    swap.unixEpocDate = LittleLong(swap.unixEpocDate);
    swap.steamID64 = LittleLong(swap.steamID64);
    LittleFloat(&swap.interval_per_tick, &swap.interval_per_tick);
    LittleFloat(&swap.runTime, &swap.runTime);
    swap.runFlags = LittleDWord(swap.numZones);
}

inline void ByteSwap_replay_stats_t(RunStats_t &swap)
{
    swap.m_iTotalZones = LittleDWord(swap.m_iTotalZones);
    for (int i = 0; i < swap.m_iTotalZones + 1; i++)
    {
        LittleFloat(&swap.m_flZoneEnterTime[i], &swap.m_flZoneEnterTime[i]);
        LittleFloat(&swap.m_flZoneTime[i], &swap.m_flZoneTime[i]);
        LittleFloat(&swap.m_flZoneStrafeSyncAvg[i], &swap.m_flZoneStrafeSyncAvg[i]);
        LittleFloat(&swap.m_flZoneStrafeSync2Avg[i], &swap.m_flZoneStrafeSync2Avg[i]);
        swap.m_iZoneJumps[i] = LittleDWord(swap.m_iZoneJumps[i]);
        swap.m_iZoneStrafes[i] = LittleDWord(swap.m_iZoneStrafes[i]);

        for (int k = 0; k < 2; k++)
        {
            LittleFloat(&swap.m_flZoneEnterSpeed[i][k], &swap.m_flZoneEnterSpeed[i][k]);
            LittleFloat(&swap.m_flZoneExitSpeed[i][k], &swap.m_flZoneExitSpeed[i][k]);
            LittleFloat(&swap.m_flZoneVelocityAvg[i][k], &swap.m_flZoneVelocityAvg[i][k]);
            LittleFloat(&swap.m_flZoneVelocityMax[i][k], &swap.m_flZoneVelocityMax[i][k]);
        }
    }
}
#endif // REPLAYFORMAT_H