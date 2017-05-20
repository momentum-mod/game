#pragma once
#ifndef GHOST_SERVER //we can't include tier0 header files in the ghost server
#include "cbase.h"
#include "const.h"
#include "shareddefs.h"
#endif


//Describes all data for visual apperence of players ingame
struct ghostAppearance_t
{
    char GhostModel[128]; //maximum model filename is 128 chars
    int GhostModelBodygroup;
    uint32_t GhostModelRGBAColorAsHex;
    uint32_t GhostTrailRGBAColorAsHex;
    uint8_t GhostTrailLength;
    bool GhostTrailEnable;

#ifndef GHOST_SERVER
    ghostAppearance_t(const char* playerModel, const int bodyGroup, const uint32_t bodyRGBA, const uint32_t trailRGBA, const uint8 trailLen)
    {
        Q_strncpy(GhostModel, playerModel, sizeof(GhostModel));
        GhostModelBodygroup = bodyGroup;
        GhostModelRGBAColorAsHex = bodyRGBA;
        GhostTrailRGBAColorAsHex = trailRGBA;
        GhostTrailLength = trailLen;
    }
    ghostAppearance_t() {}
    bool ghostAppearance_t::operator==(const ghostAppearance_t &other) const
    {
        return Q_strcmp(GhostModel, other.GhostModel) == 0 &&
            GhostModelBodygroup == other.GhostModelBodygroup && 
            GhostModelRGBAColorAsHex == other.GhostModelRGBAColorAsHex &&
            GhostTrailRGBAColorAsHex == other.GhostTrailRGBAColorAsHex &&
            GhostTrailLength == other.GhostTrailLength;
    }
#endif
};
// Based on CReplayFrame, describes data needed for ghost's physical properties 
struct ghostNetFrame_t
{
    int Buttons;
    uint64_t SteamID64;
    char PlayerName[32];
    ghostAppearance_t GhostAppearance;
#ifdef GHOST_SERVER //Can't use Vector/QAngle in ghost server
    float EyeAngle[3];
    float Position[3];
    float ViewOffset[3];
#else   
    QAngle EyeAngle;
    Vector Position;
    Vector ViewOffset;
    ghostNetFrame_t(const QAngle eyeAngle, const Vector position, const Vector viewOffset,
        const int buttons, const uint64_t steamID64, const char* playerName)
    {
        for (int i = 0; i < 3; i++)
        {
            EyeAngle[i] = eyeAngle[i];
            Position[i] = position[i];
            ViewOffset[i] = viewOffset[i];

        }
        Buttons = buttons;
        SteamID64 = steamID64;
        Q_strncpy(PlayerName, playerName, sizeof(PlayerName));
    }
    ghostNetFrame_t() {}

    bool ghostNetFrame_t::operator==(const ghostNetFrame_t &other) const
    {
        return EyeAngle == other.EyeAngle &&
            Position == other.Position &&
            ViewOffset == other.ViewOffset &&
            Buttons == other.Buttons &&
            SteamID64 == other.SteamID64 &&
            Q_strcmp(PlayerName, other.PlayerName) == 0;
    }
#endif //NOT_GHOST_SERVER
};

#ifndef GHOST_SERVER

static ConVar mm_updaterate("mom_ghost_online_updaterate", "20", 
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE, 
    "Number of updates per second to and from the ghost server.\n", true, 1.0f, true, 1000.0f);

static ConVar mm_timeOutDuration("mom_ghost_online_timeout_duration", "10",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Seconds to wait when timimg out from a ghost server.\n", true, 5.0f, true, 30.0f);

//we have to wait a few ticks to let the interpolation catch up with our ghosts!
static ConVar mm_lerpRatio("mom_ghost_online_lerp_ratio", "2",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of ticks to wait before updating ghosts, to allow client to interpolate.\n", true, 0.0f, true, 10.0f);
#endif
