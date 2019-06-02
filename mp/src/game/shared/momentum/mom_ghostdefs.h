#pragma once

#include "utlbuffer.h"
#include "mom_shareddefs.h"

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
    PT_SAVELOC_REQ,

    PT_COUNT
};

#define DEFAULT_PORT 9000
#define DEFAULT_STEAM_PORT 9001
#define DEFAULT_MASTER_SERVER_PORT 9002

struct MomentumPacket_t
{
    uint8 type;
    virtual ~MomentumPacket_t() {};

    virtual void Write(CUtlBuffer &buf) { buf.PutUnsignedChar(type); }
};

//Describes all data for visual apperence of players ingame
struct GhostAppearance_t
{
    int m_iGhostModelBodygroup;
    uint32 m_iGhostModelRGBAColorAsHex;
    uint32 m_iGhostTrailRGBAColorAsHex;
    uint8 m_iGhostTrailLength;
    bool m_bGhostTrailEnable;
    bool m_bFlashlightOn;

    GhostAppearance_t(const int bodyGroup, const uint32 bodyRGBA, const uint32 trailRGBA, const uint8 trailLen, const bool hasTrail, const bool flashlight)
    {
        m_iGhostModelBodygroup = bodyGroup;
        m_iGhostModelRGBAColorAsHex = bodyRGBA;
        m_iGhostTrailRGBAColorAsHex = trailRGBA;
        m_iGhostTrailLength = trailLen;
        m_bGhostTrailEnable = hasTrail;
        m_bFlashlightOn = flashlight;
    }
    GhostAppearance_t(): m_iGhostModelBodygroup(0), m_iGhostModelRGBAColorAsHex(0), m_iGhostTrailRGBAColorAsHex(0), m_iGhostTrailLength(0), m_bGhostTrailEnable(false), m_bFlashlightOn(false)
    {
    }

    GhostAppearance_t &operator=(const GhostAppearance_t &other) 
    {
        m_iGhostModelBodygroup = other.m_iGhostModelBodygroup;
        m_iGhostModelRGBAColorAsHex = other.m_iGhostModelRGBAColorAsHex;
        m_iGhostTrailRGBAColorAsHex = other.m_iGhostTrailRGBAColorAsHex;
        m_iGhostTrailLength = other.m_iGhostTrailLength;
        m_bGhostTrailEnable = other.m_bGhostTrailEnable;
        m_bFlashlightOn = other.m_bFlashlightOn;
        return *this;
    }
    bool operator==(const GhostAppearance_t &other) const
    {
        return m_iGhostModelBodygroup == other.m_iGhostModelBodygroup &&
            m_iGhostModelRGBAColorAsHex == other.m_iGhostModelRGBAColorAsHex &&
            m_iGhostTrailRGBAColorAsHex == other.m_iGhostTrailRGBAColorAsHex &&
            m_iGhostTrailLength == other.m_iGhostTrailLength &&
            m_bGhostTrailEnable == other.m_bGhostTrailEnable &&
            m_bFlashlightOn == other.m_bFlashlightOn;
    }
};

struct LobbyGhostAppearance_t
{
    GhostAppearance_t appearance;
    char base64[1024]; // Used as a quick verify

    LobbyGhostAppearance_t()
    {
        base64[0] = '\0';
        appearance = GhostAppearance_t();
    }

    LobbyGhostAppearance_t &operator=(const LobbyGhostAppearance_t &other) 
    {
        appearance = other.appearance;
        Q_strncpy(base64, other.base64, sizeof(base64));
        return *this;
    }
};


// Based on CReplayFrame, describes data needed for ghost's physical properties 
struct PositionPacket_t : MomentumPacket_t
{
    int Buttons;
    float ViewOffset;
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



struct SpecUpdatePacket_t : MomentumPacket_t
{
    uint64 specTarget;
    SpectateMessageType_t spec_type;

    SpecUpdatePacket_t(uint64 uID, SpectateMessageType_t specType)
    {
        type = PT_SPEC_UPDATE;
        specTarget = uID;
        spec_type = specType;
    }

    SpecUpdatePacket_t(CUtlBuffer &buf)
    {
        type = PT_SPEC_UPDATE;
        specTarget = static_cast<uint64>(buf.GetInt64());
        spec_type = static_cast<SpectateMessageType_t>(buf.GetInt());
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
    DECAL_PAINT,
    DECAL_KNIFE
    // etc

} DECAL_TYPE;

struct DecalPacket_t : MomentumPacket_t
{
    // Type of decal.
    DECAL_TYPE decal_type;

    Vector vOrigin;
    QAngle vAngle;

    int iWeaponID; // or colorRed or bStab for knife
    int iMode;     // or colorGreen
    int iSeed;     // or colorBlue
    float fSpread; // or Radius of decal

    DecalPacket_t() : decal_type(DECAL_BULLET), iWeaponID(0), iMode(0), iSeed(0), fSpread(0)
    {
        type = PT_DECAL_DATA;
    }

    DecalPacket_t(DECAL_TYPE decalType, Vector origin, QAngle angle, int weaponID, int mode, int seed, float spread)
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
        buf.PutInt(iWeaponID); // MOM_TODO: We don't use all 32 bits here, write a lower precision?
        buf.PutInt(iMode);
        buf.PutInt(iSeed);
        buf.PutFloat(fSpread);
    }
};

struct SavelocReqPacket_t : MomentumPacket_t
{
    // Stage type
    int stage;

    // Stage == 2 ? (The number of savelocs we have to offer)
    // Stage == (3 || 4) ? (The number of savelocs we have chosen to download)
    int saveloc_count;

    // Stage == 3 ? (The selected nums of savelocs to download)
    // Stage == 4 ? (The actual saveloc data, in binary)
    CUtlBuffer dataBuf;

    SavelocReqPacket_t(): stage(0), saveloc_count(0)
    {
        type = PT_SAVELOC_REQ;
        dataBuf.SetBigEndian(false);
    }

    SavelocReqPacket_t(CUtlBuffer &buf)
    {
        type = PT_SAVELOC_REQ;
        stage = buf.GetInt();
        if (stage == 2)
        {
            saveloc_count = buf.GetInt();
        }
        else if (stage > 2)
        {
            dataBuf.CopyBuffer(buf);
            // The CopyBuffer method clears the dataBuf and swallows everything inside of buf.
            // We first need to skip over the 5 bytes used for type + stage
            dataBuf.SeekGet(CUtlBuffer::SEEK_CURRENT, 5);
            // And now pull out the saveloc count
            saveloc_count = dataBuf.GetInt();
            // And now our buffer's Get is currently in the location for reading the data
        }
    }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket_t::Write(buf);
        buf.PutInt(stage);
        if (stage > 1)
        {
            buf.PutInt(saveloc_count);

            if (stage > 2)
                buf.Put(dataBuf.Base(), dataBuf.TellPut());
        }
    }
};

extern ConVar mm_updaterate;