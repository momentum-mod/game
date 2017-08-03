#pragma once
#ifndef GHOST_SERVER //we can't include tier0 header files in the ghost server
#include "cbase.h"
#include "const.h"
#include "shareddefs.h"
#endif

enum PacketTypes
{
    PT_CONN_REQ,
    PT_CONN_ACK,
    PT_APPR_DATA,
    PT_APPR_ACK,
    PT_POS_DATA,
    PT_POS_ACK,
    PT_CHAT_DATA,
    PT_MAP_CHANGE,
    PT_DISC_REQ,

    PT_COUNT
};

#define DEFAULT_PORT 9000
#define DEFAULT_STEAM_PORT 9001
#define DEFAULT_MASTER_SERVER_PORT 9002

//Describes all data for visual apperence of players ingame
struct ghostAppearance_t
{
    int GhostModelBodygroup;
    uint32_t GhostModelRGBAColorAsHex;
    uint32_t GhostTrailRGBAColorAsHex;
    uint8_t GhostTrailLength;
    bool GhostTrailEnable;
    char GhostModel[128]; //maximum model filename is 128 chars

#ifndef GHOST_SERVER
    ghostAppearance_t(const char* playerModel, const int bodyGroup, const uint32_t bodyRGBA, const uint32_t trailRGBA, const uint8 trailLen, const bool hasTrail)
    {
        Q_strncpy(GhostModel, playerModel, sizeof(GhostModel));
        GhostModelBodygroup = bodyGroup;
        GhostModelRGBAColorAsHex = bodyRGBA;
        GhostTrailRGBAColorAsHex = trailRGBA;
        GhostTrailLength = trailLen;
        GhostTrailEnable = hasTrail;
    }
    ghostAppearance_t() {}
    bool operator==(const ghostAppearance_t &other) const
    {
        return Q_strcmp(GhostModel, other.GhostModel) == 0 &&
            GhostModelBodygroup == other.GhostModelBodygroup &&
            GhostModelRGBAColorAsHex == other.GhostModelRGBAColorAsHex &&
            GhostTrailRGBAColorAsHex == other.GhostTrailRGBAColorAsHex &&
            GhostTrailLength == other.GhostTrailLength &&
            GhostTrailEnable == other.GhostTrailEnable;
    }
#endif
};
// Based on CReplayFrame, describes data needed for ghost's physical properties 
struct ghostNetFrame_t
{
    int Buttons;
    float ViewOffset;
#ifdef GHOST_SERVER //Can't use Vector/QAngle in ghost server
    float EyeAngle[3];
    float Position[3];
    float Velocity[3];
#else   
    QAngle EyeAngle;
    Vector Position;
    Vector Velocity;
    ghostNetFrame_t(const QAngle eyeAngle, const Vector position, const Vector velocity, 
        const float viewOffsetZ, const int buttons)
    {
        EyeAngle = eyeAngle;
        Position = position;
        Velocity = velocity;

        Buttons = buttons;
        ViewOffset = viewOffsetZ;

    }
    ghostNetFrame_t() {}

    bool operator==(const ghostNetFrame_t &other) const
    {
        return EyeAngle == other.EyeAngle &&
            Position == other.Position &&
            ViewOffset == other.ViewOffset &&
            Buttons == other.Buttons &&
            Velocity == other.Velocity;
    }
#endif //NOT_GHOST_SERVER
};
struct ghostSignOnPacket_t
{
    ghostNetFrame_t newFrame;
    ghostAppearance_t newApps;
    uint64_t SteamID;
    char MapName[96];
};
struct ghostSignOffPacket_t
{
    char Message[128];
};
struct ghostNewMapEvent_t
{
    char MapName[96];
};
#ifndef GHOST_SERVER

extern ConVar mm_updaterate;
extern ConVar mm_timeOutDuration;
extern ConVar mm_lerpRatio;
extern ConVar mm_ghostTesting;
#endif
