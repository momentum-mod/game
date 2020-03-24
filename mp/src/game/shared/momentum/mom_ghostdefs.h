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

        auto iSpecType = buf.GetInt();
        if (iSpecType < SPEC_UPDATE_FIRST || iSpecType > SPEC_UPDATE_LAST)
            iSpecType = SPEC_UPDATE_INVALID;

        spec_type = static_cast<SpectateMessageType_t>(iSpecType);
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
    DECAL_STICKY_SHOOT,
    DECAL_STICKY_DETONATE,

    DECAL_FIRST = DECAL_BULLET,
    DECAL_LAST = DECAL_STICKY_DETONATE,
    DECAL_INVALID = -1
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

struct StickyShootDecalData
{
    Vector velocity;
};

class DecalPacket : public MomentumPacket
{
  private:
    DecalPacket(DecalType decalType, Vector origin, QAngle angle)
    {
        decal_type = decalType;
        vOrigin = origin;
        vAngle = angle;
        Validate();
    }
  public:
    Vector vOrigin;
    
    DecalType decal_type;
    
    QAngle vAngle;

    union DecalData
    {
        DecalData() {}
        BulletDecalData bullet;
        PaintDecalData paint;
        KnifeDecalData knife;
        StickyShootDecalData stickyShoot;
    };
    DecalData data;

    DecalPacket() : decal_type(DECAL_INVALID) {}

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

    static DecalPacket Knife(Vector origin, QAngle angle, bool bStab)
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

    static DecalPacket StickyShoot(const Vector &origin, const QAngle &angle, const Vector &velocity)
    {
        DecalPacket packet(DECAL_STICKY_SHOOT, origin, angle);
        packet.data.stickyShoot.velocity = velocity;
        return packet;
    }

    static DecalPacket StickyDet()
    {
        DecalPacket pack(DECAL_STICKY_DETONATE, vec3_origin, vec3_angle);
        return pack;
    }

    DecalPacket(CUtlBuffer &buf)
    {
        int iDecalType = buf.GetUnsignedChar();
        if (iDecalType < DECAL_FIRST || iDecalType > DECAL_LAST)
            iDecalType = DECAL_INVALID;

        decal_type = static_cast<DecalType>(iDecalType);
        buf.Get(&vOrigin, sizeof(Vector));
        buf.Get(&vAngle, sizeof(QAngle));
        buf.Get(&data, sizeof(data));
        Validate();
    }

    DecalPacket& operator=(const DecalPacket &other)
    {
        decal_type = other.decal_type;
        vOrigin = other.vOrigin;
        vAngle = other.vAngle;
        memcpy(&data, &other.data, sizeof(data));
        Validate();
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

    void Validate()
    {
        if (!vOrigin.IsValid() || !IsEntityPositionReasonable(vOrigin))
            vOrigin = vec3_origin;

        // vAngle is not always angle data (for grenade it's vecThrow)
        if (!vAngle.IsValid())
            vAngle = vec3_angle;

        if (decal_type == DECAL_PAINT)
        {
            data.paint.fDecalRadius = clamp(data.paint.fDecalRadius, 0.001f, 1.0f);
        }
    }
};

enum SavelocRequestStage
{
    SAVELOC_REQ_STAGE_INVALID = -1,
    SAVELOC_REQ_STAGE_DONE,         // Done
    SAVELOC_REQ_STAGE_COUNT_REQ,    // Asking how many savelocs there are
    SAVELOC_REQ_STAGE_COUNT_ACK,    // Telling how many savelocs there are
    SAVELOC_REQ_STAGE_SAVELOC_REQ,  // Requesting specific savelocs at specific indexes
    SAVELOC_REQ_STAGE_SAVELOC_ACK,  // Giving the specific savelocs

    // Internal
    SAVELOC_REQ_STAGE_REQUESTER_LEFT,
    SAVELOC_REQ_STAGE_CLICKED_CANCEL,

    // Bounds for online
    SAVELOC_REQ_STAGE_FIRST = SAVELOC_REQ_STAGE_DONE,
    SAVELOC_REQ_STAGE_LAST = SAVELOC_REQ_STAGE_SAVELOC_ACK
};

class SavelocReqPacket : public MomentumPacket
{
  public:
    // Stage type
    int stage;

    // Stage == _COUNT_ACK ? (The number of savelocs we have to offer)
    // Stage == (_SAVELOC_REQ || _SAVELOC_ACK) ? (The number of savelocs we have chosen to download)
    int saveloc_count;

    // Stage == _SAVELOC_REQ ? (The selected nums of savelocs to download)
    // Stage == _SAVELOC_ACK ? (The actual saveloc data, in binary)
    CUtlBuffer dataBuf;

    SavelocReqPacket(): stage(0), saveloc_count(0)
    {
        dataBuf.SetBigEndian(false);
    }

    SavelocReqPacket(CUtlBuffer &buf)
    {
        stage = buf.GetInt();

        if (stage < SAVELOC_REQ_STAGE_FIRST || stage > SAVELOC_REQ_STAGE_LAST)
            stage = SAVELOC_REQ_STAGE_INVALID;

        if (stage > SAVELOC_REQ_STAGE_COUNT_REQ)
        {
            saveloc_count = buf.GetInt();

            if (stage > SAVELOC_REQ_STAGE_COUNT_ACK && buf.IsValid())
            {
                dataBuf.Clear();
                dataBuf.SetBigEndian(false);
                dataBuf.Put(buf.PeekGet(), buf.GetBytesRemaining());
            }
        }
    }

    PacketType GetType() const OVERRIDE { return PACKET_TYPE_SAVELOC_REQ; }

    void Write(CUtlBuffer& buf) OVERRIDE
    {
        MomentumPacket::Write(buf);
        buf.PutInt(stage);
        if (stage > SAVELOC_REQ_STAGE_COUNT_REQ)
        {
            buf.PutInt(saveloc_count);

            if (stage > SAVELOC_REQ_STAGE_COUNT_ACK)
                buf.Put(dataBuf.Base(), dataBuf.TellPut());
        }
    }
};

extern ConVar mm_updaterate;