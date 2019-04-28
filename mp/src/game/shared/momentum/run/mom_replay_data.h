#pragma once

#include <momentum/util/serialization.h>
#include "utlbuffer.h"


// HACK: To keep compatibility, store teleport flag in the buttons
// Remember to update me if more button flags are added!!!
#define IN_REPLAY_TELEPORTED            (1 << 27)

// A single frame of the replay.
class CReplayFrame : public ISerializable
{
  public:
    CReplayFrame()
        : m_angEyeAngles(0, 0, 0), m_vPlayerOrigin(0, 0, 0), m_fPlayerViewOffset(0.0f), m_iPlayerButtons(0)
    {
    }

    CReplayFrame(CUtlBuffer &reader)
    {
        m_angEyeAngles.x = reader.GetFloat();
        m_angEyeAngles.y = reader.GetFloat();
        m_angEyeAngles.z = reader.GetFloat();

        m_vPlayerOrigin.x = reader.GetFloat();
        m_vPlayerOrigin.y = reader.GetFloat();
        m_vPlayerOrigin.z = reader.GetFloat();

        m_fPlayerViewOffset = reader.GetFloat();

        m_iPlayerButtons = reader.GetInt();
    }

    CReplayFrame(const QAngle &eye, const Vector &origin, const float viewoffset, int buttons, bool teleported)
        : m_angEyeAngles(eye), m_vPlayerOrigin(origin), m_fPlayerViewOffset(viewoffset), m_iPlayerButtons(buttons)
    {
        if ( teleported )
            m_iPlayerButtons |= IN_REPLAY_TELEPORTED;
    }

  public:
    virtual void Serialize(CUtlBuffer &writer) OVERRIDE
    {
        writer.PutFloat(m_angEyeAngles.x);
        writer.PutFloat(m_angEyeAngles.y);
        writer.PutFloat(m_angEyeAngles.z);

        writer.PutFloat(m_vPlayerOrigin.x);
        writer.PutFloat(m_vPlayerOrigin.y);
        writer.PutFloat(m_vPlayerOrigin.z);

        writer.PutFloat(m_fPlayerViewOffset);

        writer.PutInt(m_iPlayerButtons);
    }

  public:
    inline QAngle EyeAngles() const { return m_angEyeAngles; }
    inline Vector PlayerOrigin() const { return m_vPlayerOrigin; }
    inline float PlayerViewOffset() const { return m_fPlayerViewOffset; }
    inline int PlayerButtons() const { return m_iPlayerButtons; }
    inline bool Teleported() const { return (m_iPlayerButtons & IN_REPLAY_TELEPORTED) ? true : false; }

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

    CReplayHeader(CUtlBuffer &reader)
    {
        reader.GetStringManualCharCount(m_szMapName, sizeof(m_szMapName));
        reader.GetStringManualCharCount(m_szMapHash, sizeof(m_szMapHash));
        reader.GetStringManualCharCount(m_szPlayerName, sizeof(m_szPlayerName));
        char steamID[20];
        reader.GetStringManualCharCount(steamID, sizeof(steamID));
        m_ulSteamID = Q_atoui64(steamID);
        m_fTickInterval = reader.GetFloat();
        m_iRunFlags = reader.GetUnsignedInt();
        char date[20];
        reader.GetStringManualCharCount(date, sizeof(date));
        m_iRunDate = Q_atoui64(date);
        m_iStartTick = reader.GetUnsignedInt();
        m_iStopTick = reader.GetUnsignedInt();
        m_iTrackNumber = reader.GetUnsignedChar();
        m_iZoneNumber = reader.GetUnsignedChar();
    }

  public:
    virtual void Serialize(CUtlBuffer &writer) OVERRIDE
    {
        writer.PutString(m_szMapName);
        writer.PutString(m_szMapHash);
        writer.PutString(m_szPlayerName);
        char temp[20];
        Q_snprintf(temp, 20, "%llu", m_ulSteamID);
        writer.PutString(temp);
        writer.PutFloat(m_fTickInterval);
        writer.PutUnsignedInt(m_iRunFlags);
        char date[20];
        Q_snprintf(date, 20, "%ld", m_iRunDate);
        writer.PutString(date);
        writer.PutUnsignedInt(m_iStartTick);
        writer.PutUnsignedInt(m_iStopTick);
        writer.PutUnsignedChar(m_iTrackNumber);
        writer.PutUnsignedChar(m_iZoneNumber);
    }

    virtual CReplayHeader &operator=(const CReplayHeader &other)
    {
        Q_strncpy(m_szMapName, other.m_szMapName, sizeof(m_szMapName));
        Q_strncpy(m_szMapHash, other.m_szMapHash, sizeof(m_szMapHash));
        Q_strncpy(m_szPlayerName, other.m_szPlayerName, sizeof(m_szPlayerName));
        m_ulSteamID = other.m_ulSteamID;
        m_fTickInterval = other.m_fTickInterval;
        m_iRunFlags = other.m_iRunFlags;
        m_iRunDate = other.m_iRunDate;
        m_iStartTick = other.m_iStartTick;
        m_iStopTick = other.m_iStopTick;
        m_iTrackNumber = other.m_iTrackNumber;
        m_iZoneNumber = other.m_iZoneNumber;
        return *this;
    }

  public:
    char m_szMapName[MAX_MAP_NAME_SAVE];         // The map the run was done in.
    char m_szMapHash[41];                        // The SHA1 of the map the run was done in.
    char m_szPlayerName[MAX_PLAYER_NAME_LENGTH]; // The name of the player that did this run.
    uint64 m_ulSteamID;                          // The steamID of the player that did this run.
    float m_fTickInterval;                       // The tickrate of the run.
    uint32 m_iRunFlags;                          // The flags the player ran with.
    time_t m_iRunDate;                           // The date this run was achieved.
    uint32 m_iStartTick;                         // The tick where the timer was started (difference from record start -> timer start)
    uint32 m_iStopTick;                          // The tick where the timer was stopped
    uint8 m_iTrackNumber;                        // The track number (0 = main map, 1+ = bonus)
    uint8 m_iZoneNumber;                         // The zone number (0 = entire track, 1+ = specific zone)
};