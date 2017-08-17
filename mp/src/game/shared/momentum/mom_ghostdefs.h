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
    bool FlashlightOn;
    char GhostModel[128]; //maximum model filename is 128 chars

#ifndef GHOST_SERVER
    ghostAppearance_t(const char* playerModel, const int bodyGroup, const uint32_t bodyRGBA, const uint32_t trailRGBA, const uint8 trailLen, const bool hasTrail, const bool flashlight)
    {
        Q_strncpy(GhostModel, playerModel, sizeof(GhostModel));
        GhostModelBodygroup = bodyGroup;
        GhostModelRGBAColorAsHex = bodyRGBA;
        GhostTrailRGBAColorAsHex = trailRGBA;
        GhostTrailLength = trailLen;
        GhostTrailEnable = hasTrail;
        FlashlightOn = flashlight;
    }
    ghostAppearance_t(): GhostModelBodygroup(0), GhostModelRGBAColorAsHex(0), GhostTrailRGBAColorAsHex(0), GhostTrailLength(0), GhostTrailEnable(false), FlashlightOn(false)
    {
    }

    ghostAppearance_t &operator=(const ghostAppearance_t &other) 
    {
        GhostModelBodygroup = other.GhostModelBodygroup;
        GhostModelRGBAColorAsHex = other.GhostModelRGBAColorAsHex;
        GhostTrailRGBAColorAsHex = other.GhostTrailRGBAColorAsHex;
        GhostTrailLength = other.GhostTrailLength;
        GhostTrailEnable = other.GhostTrailEnable;
        FlashlightOn = other.FlashlightOn;
        Q_strncpy(GhostModel, other.GhostModel, sizeof(GhostModel));
        return *this;
    }
    bool operator==(const ghostAppearance_t &other) const
    {
        return Q_strcmp(GhostModel, other.GhostModel) == 0 &&
            GhostModelBodygroup == other.GhostModelBodygroup &&
            GhostModelRGBAColorAsHex == other.GhostModelRGBAColorAsHex &&
            GhostTrailRGBAColorAsHex == other.GhostTrailRGBAColorAsHex &&
            GhostTrailLength == other.GhostTrailLength &&
            GhostTrailEnable == other.GhostTrailEnable &&
            FlashlightOn == other.FlashlightOn;
    }
#endif
};

struct LobbyGhostAppearance_t
{
    ghostAppearance_t appearance;
    char base64[1024]; // Used as a quick verify

#ifndef GHOST_SERVER
    LobbyGhostAppearance_t()
    {
        base64[0] = '\0';
        appearance = ghostAppearance_t();
    }

    LobbyGhostAppearance_t &operator=(const LobbyGhostAppearance_t &other) 
    {
        appearance = other.appearance;
        Q_strncpy(base64, other.base64, sizeof(base64));
        return *this;
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
    ghostNetFrame_t(): Buttons(0), ViewOffset(0)
    {
    }

    ghostNetFrame_t& operator=(const ghostNetFrame_t &other)
    {
        Buttons = other.Buttons;
        ViewOffset = other.ViewOffset;
        EyeAngle = other.EyeAngle;
        Position = other.Position;
        Velocity = other.Velocity;
        return *this;
    }

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

struct ReceivedFrame_t
{
    float recvTime;
    ghostNetFrame_t frame;

    ReceivedFrame_t(float recvTime, ghostNetFrame_t recvFrame)
    {
        this->recvTime = recvTime;
        frame = recvFrame;
    }
};


#ifndef GHOST_SERVER

struct ghostSpecUpdate_t
{
    CSteamID specTarget;
    SPECTATE_MSG_TYPE type;
};

extern ConVar mm_updaterate;

#endif
