#pragma once

#include "igamesystem.h"

class CMomentumProgress : public CAutoGameSystem
{
  public:
    CMomentumProgress();

    void BeatStage(int stage, int world = -1);
    void Reset();

    bool IsStageBeat(int stage, int world = -1);

    int GetCurrentWorld() const { return m_CurrentWorld; }

    void Save();
    void Load();

protected:
    void PostInit() OVERRIDE; // Load
    void Shutdown() OVERRIDE; // Safety save
    void LevelShutdownPostEntity() OVERRIDE; // Safety save


private:
    byte m_CurrentWorld;

    // Keeps track of which stages in which worlds have been beat, using simple bit flags
    short m_progresses[7];//tutorial + worlds 1-5 + bonus world = 7
    // stored in shorts because ~11 stages fits into 16 bits
};

extern CMomentumProgress *g_pMomentumProgress;