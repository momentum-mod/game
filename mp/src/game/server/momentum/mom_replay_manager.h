#pragma once

#include "cbase.h"
#include "mom_replay_base.h"
#include "utlmap.h"

class CMomReplayManager
{
private:
	class CReplayCreatorBase
	{
	public:
		virtual CMomReplayBase* CreateReplay() = 0;
		virtual CMomReplayBase* LoadReplay(CBinaryReader* reader) = 0;
	};

	template <typename T>
	class CReplayCreator :
		public CReplayCreatorBase
	{
	public:
		virtual CMomReplayBase* CreateReplay() override { return new T(); }
		virtual CMomReplayBase* LoadReplay(CBinaryReader* reader) override
		{ 
			Log("Creating stuff\n");
			return new T(reader);
		}
	};

public:
	CMomReplayManager();
	~CMomReplayManager();

public:
	CMomReplayBase* StartRecording();
	void StopRecording();
	bool StoreReplay(const char* path, const char* pathID);
	CMomReplayBase* LoadReplay(const char* path, const char* pathID);
	void StopPlayback();

public:
	inline CMomReplayBase* GetCurrentReplay() const { return m_pCurrentReplay; }
	inline bool Recording() const { return m_bRecording; }
	inline bool PlayingBack() const { return m_bPlayingBack; }

private:
	static bool RegisterCreators();

private:
	CMomReplayBase* m_pCurrentReplay;
	bool m_bRecording;
	bool m_bPlayingBack;
	uint8 m_ucCurrentVersion;

private:
	static CUtlMap<uint8, CReplayCreatorBase*> m_mapCreators;
	static bool m_bDummy;
};