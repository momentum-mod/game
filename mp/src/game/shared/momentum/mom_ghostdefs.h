#pragma once

#include "utlbuffer.h"
#include "mom_shareddefs.h"

enum PacketType
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

class MomentumPacket
{
  public:
    virtual PacketType GetType() const = 0;
    virtual ~MomentumPacket() {};

    virtual void Write(CUtlBuffer &buf) { buf.PutUnsignedChar(GetType()); }
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

    GhostAppearance_t(const int bodyGroup, const uint32 bodyRGBA, const uint32 trailRGBA, const uint8 trailLen, const bool hasTrail, const bool flashlightOn)
    {
        m_iGhostModelBodygroup = bodyGroup;
        m_iGhostModelRGBAColorAsHex = bodyRGBA;
        m_iGhostTrailRGBAColorAsHex = trailRGBA;
        m_iGhostTrailLength = trailLen;
        m_bGhostTrailEnable = hasTrail;
        m_bFlashlightOn = flashlightOn;
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
class PositionPacket : public MomentumPacket
{
  public:
    int Buttons;
    float ViewOffset;
    QAngle EyeAngle;
    Vector Position;
    Vector Velocity;
    PositionPacket(const QAngle eyeAngle, const Vector position, const Vector velocity, 
        const float viewOffsetZ, const int buttons)
    {
        EyeAngle = eyeAngle;
        Position = position;
        Velocity = velocity;

        Buttons = buttons;
        ViewOffset = viewOffsetZ;
    }

    PositionPacket(): Buttons(0), ViewOffset(0)
    {
    }
    PositionPacket(CUtlBuffer &buf)
    {
        buf.Get(&EyeAngle, sizeof(QAngle));
        buf.Get(&Position, sizeof(Vector));
        buf.Get(&Velocity, sizeof(Vector));
        Buttons = buf.GetInt();
        ViewOffset = buf.GetFloat();
    }

    PacketType GetType() const OVERRIDE { return PT_POS_DATA; }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket::Write(buf);
        buf.Put(&EyeAngle, sizeof(QAngle));
        buf.Put(&Position, sizeof(Vector));
        buf.Put(&Velocity, sizeof(Vector));
        buf.PutInt(Buttons);
        buf.PutFloat(ViewOffset);
    }

    PositionPacket& operator=(const PositionPacket &other)
    {
        Buttons = other.Buttons;
        ViewOffset = other.ViewOffset;
        EyeAngle = other.EyeAngle;
        Position = other.Position;
        Velocity = other.Velocity;
        return *this;
    }

    bool operator==(const PositionPacket &other) const
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



class SpecUpdatePacket : public MomentumPacket
{
  public:
    uint64 specTarget;
    SpectateMessageType_t spec_type;

    SpecUpdatePacket(uint64 uID, SpectateMessageType_t specType)
    {
        specTarget = uID;
        spec_type = specType;
    }

    SpecUpdatePacket(CUtlBuffer &buf)
    {
        specTarget = static_cast<uint64>(buf.GetInt64());
        spec_type = static_cast<SpectateMessageType_t>(buf.GetInt());
    }

    PacketType GetType() const OVERRIDE { return PT_SPEC_UPDATE; }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket::Write(buf);
        buf.PutUint64(specTarget);
        buf.PutInt(spec_type);
    }
};

enum DecalType
{
    DECAL_BULLET = 0,
    DECAL_PAINT,
    DECAL_KNIFE,
    DECAL_ROCKET,
    // etc
};

struct BulletDecalData
{
    int iAmmoType;
    int iMode;
    int iSeed;
    float fSpread;
};

struct PaintDecalData
{
    Color color;
    float fDecalRadius;
};

struct KnifeDecalData
{
    bool bStab;
};

class DecalPacket : public MomentumPacket
{
  private:
    DecalPacket(DecalType decalType, Vector origin, QAngle angle)
    {
        decal_type = decalType;
        vOrigin = origin;
        vAngle = angle;
    }
  public:
    Vector vOrigin;
    
    DecalType decal_type;
    
    QAngle vAngle;

    union DecalData
    {
        DecalData() {}
        BulletDecalData bullet; // When decal type is DECAL_BULLET
        PaintDecalData paint;   // When decal type is DECAL_PAINT
        KnifeDecalData knife;   // When decal type is DECAL_KNIFE
    };
    DecalData data;

    DecalPacket() {}

    static DecalPacket Bullet(Vector origin, QAngle angle, int iAmmoType, int iMode, int iSeed, float fSpread)
    {
        DecalPacket packet(DECAL_BULLET, origin, angle);
        packet.data.bullet.iAmmoType = iAmmoType;
        packet.data.bullet.iMode = iMode;
        packet.data.bullet.iSeed = iSeed;
        packet.data.bullet.fSpread = fSpread;
        return packet;
    }

    static DecalPacket Paint(Vector origin, QAngle angle, Color color, float fDecalRadius)
    {
        DecalPacket packet(DECAL_PAINT, origin, angle);
        packet.data.paint.color = color;
        packet.data.paint.fDecalRadius = fDecalRadius;
        return packet;
    }

    static DecalPacket Knife( Vector origin, QAngle angle, bool bStab )
    {
        DecalPacket packet(DECAL_KNIFE, origin, angle);
        packet.data.knife.bStab = bStab;
        return packet;
    }

    static DecalPacket Rocket(Vector origin, QAngle angle)
    {
        DecalPacket pack(DECAL_ROCKET, origin, angle);
        return pack;
    }

    DecalPacket(CUtlBuffer &buf)
    {
        decal_type = static_cast<DecalType>(buf.GetUnsignedChar());
        buf.Get(&vOrigin, sizeof(Vector));
        buf.Get(&vAngle, sizeof(QAngle));
        buf.Get(&data, sizeof(data));
    }

    DecalPacket& operator=(const DecalPacket &other)
    {
        decal_type = other.decal_type;
        vOrigin = other.vOrigin;
        vAngle = other.vAngle;
        memcpy(&data, &other.data, sizeof(data));
        return *this;
    }

    PacketType GetType() const OVERRIDE { return PT_DECAL_DATA; }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket::Write(buf);
        buf.PutUnsignedChar(decal_type);
        buf.Put(&vOrigin, sizeof(Vector));
        buf.Put(&vAngle, sizeof(QAngle));
        buf.Put(&data, sizeof(data));
    }
};

class SavelocReqPacket : public MomentumPacket
{
  public:
    // Stage type
    int stage;

    // Stage == 2 ? (The number of savelocs we have to offer)
    // Stage == (3 || 4) ? (The number of savelocs we have chosen to download)
    int saveloc_count;

    // Stage == 3 ? (The selected nums of savelocs to download)
    // Stage == 4 ? (The actual saveloc data, in binary)
    CUtlBuffer dataBuf;

    SavelocReqPacket(): stage(0), saveloc_count(0)
    {
        dataBuf.SetBigEndian(false);
    }

    SavelocReqPacket(CUtlBuffer &buf)
    {
        stage = buf.GetInt();
        if (stage > 1)
        {
            saveloc_count = buf.GetInt();

            if (stage > 2)
            {
                dataBuf.Clear();
                dataBuf.Put(buf.PeekGet(), buf.GetBytesRemaining());
            }
        }
    }

    PacketType GetType() const OVERRIDE { return PT_SAVELOC_REQ; }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket::Write(buf);
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