#pragma once
#ifndef GHOST_SERVER //we can't include tier0 header files in the ghost server
#include "cbase.h"
#include "const.h"
#include "shareddefs.h"
#endif

enum PacketTypes
{
    PT_SIGNON,
    PT_SIGNOFF,
    PT_NET_FRAME,
    PT_APPEARANCE,
    PT_NEWMAP,
    PT_ACK,

    PT_COUNT
};

#define DEFAULT_PORT 9000

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
    char PlayerName[32];
#ifdef GHOST_SERVER //Can't use Vector/QAngle in ghost server
    float EyeAngle[3];
    float Position[3];
    float ViewOffset[3];
#else   
    QAngle EyeAngle;
    Vector Position;
    Vector ViewOffset;
    ghostNetFrame_t(const QAngle eyeAngle, const Vector position, const Vector viewOffset,
        const int buttons, const char* playerName)
    {
        for (int i = 0; i < 3; i++)
        {
            EyeAngle[i] = eyeAngle[i];
            Position[i] = position[i];
            ViewOffset[i] = viewOffset[i];

        }
        Buttons = buttons;
        Q_strncpy(PlayerName, playerName, sizeof(PlayerName));
    }
    ghostNetFrame_t() {}

    bool operator==(const ghostNetFrame_t &other) const
    {
        return EyeAngle == other.EyeAngle &&
            Position == other.Position &&
            ViewOffset == other.ViewOffset &&
            Buttons == other.Buttons &&
            Q_strcmp(PlayerName, other.PlayerName) == 0;
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
struct ghostAckPacket_t
{
    char AckMessage[16];
    bool AckSuccess;
};
#ifndef GHOST_SERVER
extern ConVar mm_updaterate;
extern ConVar mm_timeOutDuration;
extern ConVar mm_lerpRatio;
extern ConVar mm_ghostTesting;
#endif
