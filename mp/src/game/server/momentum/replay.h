#pragma once

#include "cbase.h"
#include "replay_data.h"

class CMomReplay
{
protected:
	CMomReplay(CReplayHeader header) :
		m_rhHeader(header)
	{
	}

	virtual ~CMomReplay() {}

public:
	inline uint8 GetVersion() const { return m_rhHeader.m_ucVersion; }
	virtual const char* GetMapName() { return m_rhHeader.m_szMapName; }
	virtual const char* GetPlayerName() { return m_rhHeader.m_szPlayerName; }
	virtual uint64 GetPlayerSteamID() { return m_rhHeader.m_ulSteamID; }
	virtual float GetTickInterval() { return m_rhHeader.m_fTickInterval; }
	virtual float GetRunTime() { return m_rhHeader.m_fRunTime; }
	virtual int GetRunFlags() { return m_rhHeader.m_iRunFlags; }

public:
	virtual CMomRunStats GetRunStats() = 0;
	virtual CUtlVector<CReplayFrame> GetFrames() = 0;

protected:
	CReplayHeader m_rhHeader;
};