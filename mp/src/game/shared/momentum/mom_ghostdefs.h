#pragma once

#include "utlbuffer.h"
#include "mom_shareddefs.h"

enum PacketType
{
    PACKET_TYPE_POSITION = 0,
    PACKET_TYPE_DECAL,
    PACKET_TYPE_SPEC_UPDATE,
    PACKET_TYPE_SAVELOC_REQ,

    PACKET_TYPE_COUNT
};

#define APPEARANCE_BODYGROUP_MIN 0
#define APPEARANCE_BODYGROUP_MAX 14
#define APPEARANCE_TRAIL_LEN_MIN 1
#define APPEARANCE_TRAIL_LEN_MAX 10

struct AppearanceData_t
{
    int m_iBodyGroup;
    uint32 m_iModelRGBAColorAsHex;
    uint32 m_iTrailRGBAColorAsHex;
    uint8 m_iTrailLength;
    bool m_bTrailEnabled;
    bool m_bFlashlightEnabled;

    AppearanceData_t()
    {
        m_iBodyGroup = 11;
        m_iModelRGBAColorAsHex = 0x0000FFFF;
        m_iTrailRGBAColorAsHex = 0xFFFFFFFF;
        m_iTrailLength = APPEARANCE_TRAIL_LEN_MIN;
        m_bTrailEnabled = false;
        m_bFlashlightEnabled = false;
    }

    void ValidateValues()
    {
        m_iBodyGroup = clamp<int>(m_iBodyGroup, APPEARANCE_BODYGROUP_MIN, APPEARANCE_BODYGROUP_MAX);
        m_iTrailLength = clamp<uint8>(m_iTrailLength, APPEARANCE_TRAIL_LEN_MIN, APPEARANCE_TRAIL_LEN_MAX);
    }

    void FromKV(KeyValues *pKV)
    {
        m_iBodyGroup = pKV->GetInt("bodygroup");
        m_iModelRGBAColorAsHex = (uint32)pKV->GetInt("model_color");
        m_iTrailRGBAColorAsHex = (uint32)pKV->GetInt("trail_color");
        m_iTrailLength = pKV->GetInt("trail_length");
        m_bTrailEnabled = pKV->GetBool("trail_enabled");
        m_bFlashlightEnabled = pKV->GetBool("flashlight_enabled");

        ValidateValues();
    }

    void ToKV(KeyValues *pKV) const
    {
        pKV->SetInt("bodygroup", m_iBodyGroup);
        pKV->SetInt("model_color", m_iModelRGBAColorAsHex);
        pKV->SetInt("trail_color", m_iTrailRGBAColorAsHex);
        pKV->SetInt("trail_length", m_iTrailLength);
        pKV->SetBool("trail_enabled", m_bTrailEnabled);
        pKV->SetBool("flashlight_enabled", m_bFlashlightEnabled);
    }
};

class MomentumPacket
{
  public:
    virtual PacketType GetType() const = 0;
    virtual ~MomentumPacket() {};

    virtual void Write(CUtlBuffer &buf) { buf.PutUnsignedChar(GetType()); }
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

    PositionPacket(const QAngle eyeAngle, const Vector position, const Vector velocity, const float viewOffsetZ, const int buttons)
    {
        EyeAngle = eyeAngle;
        Position = position;
        Velocity = velocity;

        Buttons = buttons;
        ViewOffset = viewOffsetZ;

        Validate();
    }

    PositionPacket(): Buttons(0), ViewOffset(0)
    {
        EyeAngle.Init();
        Position.Init();
        Velocity.Init();
    }

    PositionPacket(CUtlBuffer &buf)
    {
        buf.Get(&EyeAngle, sizeof(QAngle));
        buf.Get(&Position, sizeof(Vector));
        buf.Get(&Velocity, sizeof(Vector));
        Buttons = buf.GetInt();
        ViewOffset = buf.GetFloat();

        Validate();
    }

    PacketType GetType() const OVERRIDE { return PACKET_TYPE_POSITION; }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket::Write(buf);
        buf.Put(&EyeAngle, sizeof(QAngle));
        buf.Put(&Position, sizeof(Vector));
        buf.Put(&Velocity, sizeof(Vector));
        buf.PutInt(Buttons);
        buf.PutFloat(ViewOffset);
    }

    void Validate()
    {
        if (!EyeAngle.IsValid() || !IsEntityQAngleReasonable(EyeAngle))
            EyeAngle = vec3_angle;

        if (!Position.IsValid() || !IsEntityPositionReasonable(Position))
            Position = vec3_origin;

        if (!Velocity.IsValid() || !IsEntityVelocityReasonable(Velocity))
            Velocity = vec3_origin;

        ViewOffset = Clamp(ViewOffset, VEC_DUCK_VIEW.z, VEC_VIEW.z);
    }

    PositionPacket& operator=(const PositionPacket &other)
    {
        Buttons = other.Buttons;
        ViewOffset = other.ViewOffset;
        EyeAngle = other.EyeAngle;
        Position = other.Position;
        Velocity = other.Velocity;
        Validate();
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

    PacketType GetType() const OVERRIDE { return PACKET_TYPE_SPEC_UPDATE; }

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

    PacketType GetType() const OVERRIDE { return PACKET_TYPE_DECAL; }

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

    PacketType GetType() const OVERRIDE { return PACKET_TYPE_SAVELOC_REQ; }

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