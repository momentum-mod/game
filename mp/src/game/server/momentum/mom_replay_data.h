#pragma once

#include "cbase.h"
#include "util/run_stats.h"
#include "util/serialization.h"

// A single frame of the replay.
class CReplayFrame : public ISerializable
{
  public:
    CReplayFrame() : m_angEyeAngles(0, 0, 0), m_vPlayerOrigin(0, 0, 0), m_iPlayerButtons(0) {}

    CReplayFrame(CBinaryReader *reader)
    {
        m_angEyeAngles.x = reader->ReadFloat();
        m_angEyeAngles.y = reader->ReadFloat();
        m_angEyeAngles.z = reader->ReadFloat();

        m_vPlayerOrigin.x = reader->ReadFloat();
        m_vPlayerOrigin.y = reader->ReadFloat();
        m_vPlayerOrigin.z = reader->ReadFloat();

        m_iPlayerButtons = reader->ReadInt32();
    }

    CReplayFrame(const QAngle &eye, const Vector &origin, int buttons)
        : m_angEyeAngles(eye), m_vPlayerOrigin(origin), m_iPlayerButtons(buttons)
    {
    }

  public:
    virtual void Serialize(CBinaryWriter *writer) override
    {
        writer->WriteFloat(m_angEyeAngles.x);
        writer->WriteFloat(m_angEyeAngles.y);
        writer->WriteFloat(m_angEyeAngles.z);

        writer->WriteFloat(m_vPlayerOrigin.x);
        writer->WriteFloat(m_vPlayerOrigin.y);
        writer->WriteFloat(m_vPlayerOrigin.z);

        writer->WriteInt32(m_iPlayerButtons);
    }

  public:
    inline QAngle EyeAngles() const { return m_angEyeAngles; }
    inline Vector PlayerOrigin() const { return m_vPlayerOrigin; }
    inline int PlayerButtons() const { return m_iPlayerButtons; }

  private:
    QAngle m_angEyeAngles;
    Vector m_vPlayerOrigin;
    int m_iPlayerButtons;
};

class CReplayHeader : public ISerializable
{
  public:
    CReplayHeader() {}

    CReplayHeader(CBinaryReader *reader)
    {
        reader->ReadString(m_szMapName, sizeof(m_szMapName) - 1);
        reader->ReadString(m_szPlayerName, sizeof(m_szPlayerName) - 1);
        m_ulSteamID = reader->ReadUInt64();
        m_fTickInterval = reader->ReadFloat();
        m_fRunTime = reader->ReadFloat();
        m_iRunFlags = reader->ReadUInt32();
        m_iRunDate = reader->ReadInt64();
    }

  public:
    virtual void Serialize(CBinaryWriter *writer) override
    {
        writer->WriteString(m_szMapName);
        writer->WriteString(m_szPlayerName);
        writer->WriteUInt64(m_ulSteamID);
        writer->WriteFloat(m_fTickInterval);
        writer->WriteFloat(m_fRunTime);
        writer->WriteUInt32(m_iRunFlags);
        writer->WriteInt64(m_iRunDate);
    }

  public:
    char m_szMapName[256];    // The map the run was done in.
    char m_szPlayerName[256]; // The name of the player that did this run.
    uint64 m_ulSteamID;       // The steamID of the player that did this run.
    float m_fTickInterval;    // The tickrate of the run.
    float m_fRunTime;         // The total runtime of the run in seconds.
    uint32 m_iRunFlags;       // The flags the player ran with.
    time_t m_iRunDate;        // The date this run was achieved.
};