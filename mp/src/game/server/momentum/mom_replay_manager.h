#pragma once

#include "cbase.h"
#include "mom_replay_base.h"

class CMomReplayManager
{
public:
	CMomReplayManager();
	~CMomReplayManager();

public:
	CMomReplayBase* StartRecording();
	void StopRecording();
	bool StoreReplay(const char* path);
	CMomReplayBase* GetCurrentReplay();
	CMomReplayBase* LoadReplay(const char* path);
	void StopPlayback();
	bool Recording();
	bool PlayingBack();
};