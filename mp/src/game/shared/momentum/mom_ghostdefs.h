#pragma once
#ifndef GHOST_SERVER //we can't include tier0 header files in the ghost server
#include "cbase.h"
#include "utlbuffer.h"
#endif

enum PacketTypes
{
    PT_CONN_REQ = 0,
    PT_CONN_ACK,
    PT_APPR_DATA,
    PT_APPR_ACK,
    PT_POS_DATA,
    PT_POS_ACK,
    PT_CHAT_DATA,
    PT_MAP_CHANGE,
    PT_DISC_REQ,

    PT_DECAL_DATA,
    PT_SPEC_UPDATE,

    PT_COUNT
};

#define DEFAULT_PORT 9000
#define DEFAULT_STEAM_PORT 9001
#define DEFAULT_MASTER_SERVER_PORT 9002

struct MomentumPacket_t
{
    uint8_t type;
    virtual ~MomentumPacket_t() = default;

#ifndef GHOST_SERVER
    virtual void Write(CUtlBuffer &buf) { buf.PutUnsignedChar(type); }
#endif
};

//Describes all data for visual apperence of players ingame
struct ghostAppearance_t
{
    int GhostModelBodygroup;
    uint32_t GhostModelRGBAColorAsHex;
    uint32_t GhostTrailRGBAColorAsHex;
    uint8_t GhostTrailLength;
    bool GhostTrailEnable;
    bool FlashlightOn;

#ifndef GHOST_SERVER
    ghostAppearance_t(const int bodyGroup, const uint32_t bodyRGBA, const uint32_t trailRGBA, const uint8 trailLen, const bool hasTrail, const bool flashlight)
    {
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
        return *this;
    }
    bool operator==(const ghostAppearance_t &other) const
    {
        return GhostModelBodygroup == other.GhostModelBodygroup &&
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
struct PositionPacket_t : MomentumPacket_t
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
    PositionPacket_t(const QAngle eyeAngle, const Vector position, const Vector velocity, 
        const float viewOffsetZ, const int buttons)
    {
        type = PT_POS_DATA;
        EyeAngle = eyeAngle;
        Position = position;
        Velocity = velocity;

        Buttons = buttons;
        ViewOffset = viewOffsetZ;
    }

    PositionPacket_t(): Buttons(0), ViewOffset(0)
    {
        type = PT_POS_DATA;
    }
    PositionPacket_t(CUtlBuffer &buf)
    {
        type = PT_POS_DATA;
        buf.Get(&EyeAngle, sizeof(QAngle));
        buf.Get(&Position, sizeof(Vector));
        buf.Get(&Velocity, sizeof(Vector));
        Buttons = buf.GetInt();
        ViewOffset = buf.GetFloat();
    }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket_t::Write(buf);
        buf.Put(&EyeAngle, sizeof(QAngle));
        buf.Put(&Position, sizeof(Vector));
        buf.Put(&Velocity, sizeof(Vector));
        buf.PutInt(Buttons);
        buf.PutFloat(ViewOffset);
    }

    PositionPacket_t& operator=(const PositionPacket_t &other)
    {
        Buttons = other.Buttons;
        ViewOffset = other.ViewOffset;
        EyeAngle = other.EyeAngle;
        Position = other.Position;
        Velocity = other.Velocity;
        return *this;
    }

    bool operator==(const PositionPacket_t &other) const
    {
        return EyeAngle == other.EyeAngle &&
            Position == other.Position &&
            ViewOffset == other.ViewOffset &&
            Buttons == other.Buttons &&
            Velocity == other.Velocity;
    }
#endif //NOT_GHOST_SERVER
};

// Used for keeping track of when we recieve certain packets.
// NOTE: The packet used as the Generic (T) here needs to have
// a default constructor and an operator= overload!
template <class T>
struct ReceivedFrame_t
{
    float recvTime;
    T frame;

    ReceivedFrame_t(float recvTime, T recvFrame)
    {
        this->recvTime = recvTime;
        frame = recvFrame;
    }
};


#ifndef GHOST_SERVER

struct SpecUpdatePacket_t : MomentumPacket_t
{
    uint64 specTarget;
    SPECTATE_MSG_TYPE spec_type;

    SpecUpdatePacket_t(uint64 uID, SPECTATE_MSG_TYPE specType)
    {
        type = PT_SPEC_UPDATE;
        specTarget = uID;
        spec_type = specType;
    }

    SpecUpdatePacket_t(CUtlBuffer &buf)
    {
        type = PT_SPEC_UPDATE;
        specTarget = static_cast<uint64_t>(buf.GetInt64());
        spec_type = static_cast<SPECTATE_MSG_TYPE>(buf.GetInt());
    }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket_t::Write(buf);
        buf.PutUint64(specTarget);
        buf.PutInt(spec_type);
    }
};

typedef enum
{
    DECAL_BULLET = 0,
    DECAL_PAINT
    // etc

} DECAL_TYPE;

struct DecalPacket_t : MomentumPacket_t
{
    // Type of decal.
    DECAL_TYPE decal_type;

    Vector vOrigin;
    QAngle vAngle;

    int iWeaponID; // or colorRed
    int iMode;     // or colorGreen
    int iSeed;     // or colorBlue
    float fSpread; // or Radius of decal

    DecalPacket_t() : decal_type(DECAL_BULLET), iWeaponID(0), iMode(0), iSeed(0), fSpread(0)
    {
        type = PT_DECAL_DATA;
    }

    DecalPacket_t(DECAL_TYPE decalType, Vector origin, QAngle angle, int weaponID, int mode, int seed, int spread)
    {
        type = PT_DECAL_DATA;
        decal_type = decalType;
        vOrigin = origin;
        vAngle = angle;
        iWeaponID = weaponID;
        iMode = mode;
        iSeed = seed;
        fSpread = spread;
    }

    DecalPacket_t(CUtlBuffer &buf)
    {
        type = PT_DECAL_DATA;
        decal_type = static_cast<DECAL_TYPE>(buf.GetUnsignedChar());
        buf.Get(&vOrigin, sizeof(Vector));
        buf.Get(&vAngle, sizeof(QAngle));
        iWeaponID = buf.GetInt();
        iMode = buf.GetInt();
        iSeed = buf.GetInt();
        fSpread = buf.GetFloat();
    }

    DecalPacket_t& operator=(const DecalPacket_t &other)
    {
        type = other.type;
        decal_type = other.decal_type;
        vOrigin = other.vOrigin;
        vAngle = other.vAngle;
        iWeaponID = other.iWeaponID;
        iMode = other.iMode;
        iSeed = other.iSeed;
        fSpread = other.fSpread;
        return *this;
    }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket_t::Write(buf);
        buf.PutUnsignedChar(decal_type);
        buf.Put(&vOrigin, sizeof(Vector));
        buf.Put(&vAngle, sizeof(QAngle));
        buf.PutInt(iWeaponID);
        buf.PutInt(iMode);
        buf.PutInt(iSeed);
        buf.PutFloat(fSpread);
    }
};

extern ConVar mm_updaterate;

#endif
