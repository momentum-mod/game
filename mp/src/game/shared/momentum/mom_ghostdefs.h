#pragma once
#ifndef GHOST_SERVER //we can't include tier0 header files in the ghost server
#include "cbase.h"
#include "const.h"
#include "shareddefs.h"
#endif

//Network data signatures
#define MOM_SIGNON 0xAB53A53F
#define MOM_SIGNOFF 0x101010

#define MOM_C_SENDING_NEWFRAME 0xDEDEDE
#define MOM_C_RECIEVING_NEWFRAME 0xEDEDED

#define MOM_S_SENDING_NEWFRAME 0xABABAB
#define MOM_S_RECIEVING_NEWFRAME 0xB1B1B1

#define MOM_S_SENDING_NEWMAP 0x9ABF23

#define MOM_S_SENDING_NEWPROPS 0x52A43E
#define MOM_S_RECIEVING_NEWPROPS 0x6F1A42

#define MOM_C_SENDING_NEWPROPS 0x13FF27
#define MOM_C_RECIEVING_NEWPROPS 0x62FACE

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
    ghostAppearance_t(const char* playerModel, const int bodyGroup, const uint32_t bodyRGBA, const uint32_t trailRGBA, const uint8 trailLen)
    {
        Q_strncpy(GhostModel, playerModel, sizeof(GhostModel));
        GhostModelBodygroup = bodyGroup;
        GhostModelRGBAColorAsHex = bodyRGBA;
        GhostTrailRGBAColorAsHex = trailRGBA;
        GhostTrailLength = trailLen;
    }
    ghostAppearance_t() {}
    bool operator==(const ghostAppearance_t &other) const
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

    bool operator==(const ghostNetFrame_t &other) const
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
extern ConVar mm_updaterate;
extern ConVar mm_timeOutDuration;
extern ConVar mm_lerpRatio;
#endif
