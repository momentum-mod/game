#pragma once

#include "igamesystem.h"

class CMomentumProgress : public CAutoGameSystem
{
  public:
    CMomentumProgress();


    void BeatStage(int stage);

    bool ShouldEnableBrush(int stage);

protected:
    void PostInit() OVERRIDE; // Load
    void Shutdown() OVERRIDE; // Safety save
    void LevelShutdownPostEntity() OVERRIDE; // Safety save


private:
    byte m_CurrentWorld;

    // Keeps track of which stages in which worlds have been beat, using simple bit flags
    short m_progresses[7];//tutorial + worlds 1-5 + bonus world = 7
};

extern CMomentumProgress *g_pMomentumProgress;