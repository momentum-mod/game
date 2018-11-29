#pragma once

#include <momentum/util/serialization.h>

// A single frame of the replay.
class CReplayFrame : public ISerializable
{
  public:
    CReplayFrame()
        : m_angEyeAngles(0, 0, 0), m_vPlayerOrigin(0, 0, 0), m_fPlayerViewOffset(0.0f), m_iPlayerButtons(0)
    {
    }

    CReplayFrame(CBinaryReader *reader)
    {
        m_angEyeAngles.x = reader->ReadFloat();
        m_angEyeAngles.y = reader->ReadFloat();
        m_angEyeAngles.z = reader->ReadFloat();

        m_vPlayerOrigin.x = reader->ReadFloat();
        m_vPlayerOrigin.y = reader->ReadFloat();
        m_vPlayerOrigin.z = reader->ReadFloat();

        m_fPlayerViewOffset = reader->ReadFloat();

        m_iPlayerButtons = reader->ReadInt32();
    }

    CReplayFrame(const QAngle &eye, const Vector &origin, const Vector &viewoffset, int buttons)
        : m_angEyeAngles(eye), m_vPlayerOrigin(origin), m_fPlayerViewOffset(viewoffset.z), m_iPlayerButtons(buttons)
    {
    }

  public:
    virtual void Serialize(CBinaryWriter *writer) OVERRIDE
    {
        writer->WriteFloat(m_angEyeAngles.x);
        writer->WriteFloat(m_angEyeAngles.y);
        writer->WriteFloat(m_angEyeAngles.z);

        writer->WriteFloat(m_vPlayerOrigin.x);
        writer->WriteFloat(m_vPlayerOrigin.y);
        writer->WriteFloat(m_vPlayerOrigin.z);

        writer->WriteFloat(m_fPlayerViewOffset);

        writer->WriteInt32(m_iPlayerButtons);
    }

  public:
    inline QAngle EyeAngles() const { return m_angEyeAngles; }
    inline Vector PlayerOrigin() const { return m_vPlayerOrigin; }
    inline float PlayerViewOffset() const { return m_fPlayerViewOffset; }
    inline int PlayerButtons() const { return m_iPlayerButtons; }

  private:
    QAngle m_angEyeAngles;
    Vector m_vPlayerOrigin;
    float m_fPlayerViewOffset;
    int m_iPlayerButtons;
};

class CReplayHeader : public ISerializable
{
  public:
    CReplayHeader() {}

    CReplayHeader(CBinaryReader *reader)
    {
        reader->ReadString(m_szMapName, sizeof(m_szMapName) - 1);
        reader->ReadString(m_szMapHash, sizeof(m_szMapHash) - 1);
        reader->ReadString(m_szPlayerName, sizeof(m_szPlayerName) - 1);
        char steamID[20];
        reader->ReadString(steamID, sizeof(steamID) - 1);
        m_ulSteamID = Q_atoui64(steamID);
        m_fTickInterval = reader->ReadFloat();
        m_fRunTime = reader->ReadFloat();
        m_iRunFlags = reader->ReadUInt32();
        char date[20];
        reader->ReadString(date, sizeof(date) - 1);
        m_iRunDate = Q_atoui64(date);
        m_iStartDif = reader->ReadInt32();
        m_iBonusZone = reader->ReadInt32();
    }

  public:
    virtual void Serialize(CBinaryWriter *writer) OVERRIDE
    {
        writer->WriteString(m_szMapName);
        writer->WriteString(m_szMapHash);
        writer->WriteString(m_szPlayerName);
        char temp[20];
        Q_snprintf(temp, 20, "%llu", m_ulSteamID);
        writer->WriteString(temp);
        writer->WriteFloat(m_fTickInterval);
        writer->WriteFloat(m_fRunTime);
        writer->WriteUInt32(m_iRunFlags);
        char date[20];
        Q_snprintf(date, 20, "%llu", m_iRunDate);
        writer->WriteString(date);
        writer->WriteInt32(m_iStartDif);
        writer->WriteInt32(m_iBonusZone);
    }

    virtual CReplayHeader &operator=(const CReplayHeader &other)
    {
        Q_strncpy(m_szMapName, other.m_szMapName, sizeof(m_szMapName));
        Q_strncpy(m_szMapHash, other.m_szMapHash, sizeof(m_szMapHash));
        Q_strncpy(m_szPlayerName, other.m_szPlayerName, sizeof(m_szPlayerName));
        m_ulSteamID = other.m_ulSteamID;
        m_fTickInterval = other.m_fTickInterval;
        m_fRunTime = other.m_fRunTime;
        m_iRunFlags = other.m_iRunFlags;
        m_iRunDate = other.m_iRunDate;
        m_iStartDif = other.m_iStartDif;
        m_iBonusZone = other.m_iBonusZone;
        return *this;
    }

  public:
    char m_szMapName[256];    // The map the run was done in.
    char m_szMapHash[41];     // The SHA1 of the map the run was done in.
    char m_szPlayerName[256]; // The name of the player that did this run.
    uint64 m_ulSteamID;       // The steamID of the player that did this run.
    float m_fTickInterval;    // The tickrate of the run.
    float m_fRunTime;         // The total runtime of the run in seconds.
    uint32 m_iRunFlags;       // The flags the player ran with.
    time_t m_iRunDate;        // The date this run was achieved.
    int m_iStartDif;          // The difference between the tick of the start timer and record
    int m_iBonusZone;         // The bonus zone. This will be needed for leaderboards to check the bonus runs.
};