#pragma once

#include <momentum/util/serialization.h>
#include "mom_replay_data.h"
#include "run/run_stats.h"

class CMomentumReplayGhostEntity;

class CMomReplayBase : public ISerializable
{
  protected:
    CMomReplayBase(CReplayHeader header, bool bFull) : m_rhHeader(header), m_pEntity(nullptr)
    {
        m_pszRunHash[0] = '\0';
        m_pszFilePath[0] = '\0';
    }

  public:
    virtual ~CMomReplayBase() {}

  public:
    // All these are virtual so they can be overridden in later versions.
    virtual const char *GetMapName() { return m_rhHeader.m_szMapName; }
    virtual const char *GetMapHash() { return m_rhHeader.m_szMapHash; }
    virtual const char *GetPlayerName() { return m_rhHeader.m_szPlayerName; }
    virtual uint64 GetPlayerSteamID() { return m_rhHeader.m_ulSteamID; }
    virtual float GetTickInterval() { return m_rhHeader.m_fTickInterval; }
    virtual float GetRunTime() { return m_rhHeader.m_fTickInterval * float(m_rhHeader.m_iStopTick - m_rhHeader.m_iStartTick); }
    virtual uint32 GetStartTick() { return m_rhHeader.m_iStartTick; }
    virtual uint32 GetStopTick() { return m_rhHeader.m_iStopTick; }
    virtual uint32 GetRunFlags() { return m_rhHeader.m_iRunFlags; }
    virtual time_t GetRunDate() { return m_rhHeader.m_iRunDate; }
    virtual uint8 GetTrackNumber() { return m_rhHeader.m_iTrackNumber; }
    virtual uint8 GetZoneNumber() { return m_rhHeader.m_iZoneNumber; }
    virtual CMomentumReplayGhostEntity *GetRunEntity() { return m_pEntity; }
    virtual const char *GetRunHash() { return m_pszRunHash; }
    virtual const char *GetFilePath() { return m_pszFilePath; }

  public:
    virtual void SetMapName(const char *name) { Q_strncpy(m_rhHeader.m_szMapName, name, sizeof(m_rhHeader.m_szMapName)); }
    virtual void SetMapHash(const char *hash) { Q_strncpy(m_rhHeader.m_szMapHash, hash, sizeof(m_rhHeader.m_szMapHash));}
    virtual void SetPlayerName(const char *name) { Q_strncpy(m_rhHeader.m_szPlayerName, name, sizeof(m_rhHeader.m_szPlayerName)); }
    virtual void SetPlayerSteamID(uint64 steamID) { m_rhHeader.m_ulSteamID = steamID; }
    virtual void SetTickInterval(float interval) { m_rhHeader.m_fTickInterval = interval; }
    virtual void SetStartTick(uint32 iStart) { m_rhHeader.m_iStartTick = iStart; }
    virtual void SetStopTick(uint32 iStop) { m_rhHeader.m_iStopTick = iStop; }
    virtual void SetRunFlags(uint32 runFlags) { m_rhHeader.m_iRunFlags = runFlags; }
    virtual void SetRunDate(time_t date) { m_rhHeader.m_iRunDate = date; }
    virtual void SetTrackNumber(uint8 track) { m_rhHeader.m_iTrackNumber = track; }
    virtual void SetZoneNumber(uint8 zone) { m_rhHeader.m_iZoneNumber = zone; }
    virtual void SetRunEntity(CMomentumReplayGhostEntity *pEnt) { m_pEntity = pEnt; }
    virtual void SetRunHash(const char *pHash) { Q_strncpy(m_pszRunHash, pHash, sizeof(m_pszRunHash)); }
    virtual void SetFilePath(const char *pFilePath) { Q_strncpy(m_pszFilePath, pFilePath, sizeof(m_pszFilePath)); };

  public:
    virtual uint8 GetVersion() = 0;
    virtual CMomRunStats *GetRunStats() = 0;
    virtual int32 GetFrameCount() = 0;
    virtual CReplayFrame *GetFrame(int32 index) = 0;
    virtual void AddFrame(const CReplayFrame &frame) = 0;
    virtual bool SetFrame(int32 index, const CReplayFrame &frame) = 0;
    virtual CMomRunStats *CreateRunStats(uint8 zones) = 0;
    virtual void RemoveFrames(int num) = 0;

  protected:
    CReplayHeader m_rhHeader;
    CMomentumReplayGhostEntity *m_pEntity;
    char m_pszRunHash[41];
    char m_pszFilePath[MAX_PATH];
};
