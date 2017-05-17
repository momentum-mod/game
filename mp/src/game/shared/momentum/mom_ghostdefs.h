#pragma once
#ifndef GHOST_SERVER //we can't include tier0 header files in the ghost server
#include "cbase.h"
#include "shareddefs.h"
#endif

#include "const.h"


//Describes all data for visual apperence of players ingame
struct ghostAppearance_t
{
    char GhostModel[32];
    int GhostModelBodygroup;
    uint32 GhostModelRGBAColorAsHex;
    uint32 GhostTrailRGBAColorAsHex;

#ifndef GHOST_SERVER
    ghostAppearance_t(const char* playerModel, const int bodyGroup, const uint32 bodyRGBA, const uint32 trailRGBA)
    {
        Q_strncpy(GhostModel, playerModel, sizeof(GhostModel));
        GhostModelBodygroup = bodyGroup;
        GhostModelRGBAColorAsHex = bodyRGBA;
        GhostTrailRGBAColorAsHex = trailRGBA;
    }
    ghostAppearance_t() {}
    bool ghostAppearance_t::operator==(const ghostAppearance_t &other) const
    {
        Q_strcmp(GhostModel, other.GhostModel) == 0 &&
        GhostModelBodygroup == other.GhostModelBodygroup && 
        GhostModelRGBAColorAsHex == other.GhostModelRGBAColorAsHex &&
        GhostTrailRGBAColorAsHex == other.GhostTrailRGBAColorAsHex;
    }
#endif
};
// Based on CReplayFrame, describes data needed for ghost's physical properties 
struct ghostNetFrame_t
{
    int Buttons;
    uint64 SteamID64;
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
        const int buttons, const uint64 steamID64,
        const char* playerName)
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