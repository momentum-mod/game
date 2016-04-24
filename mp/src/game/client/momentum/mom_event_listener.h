#pragma once

#include "mom_shareddefs.h"


class C_Momentum_EventListener : public IGameEventListener2
{
public:
    C_Momentum_EventListener() {};
    ~C_Momentum_EventListener(){
        if (gameeventmanager)
            gameeventmanager->RemoveListener(this);
    }

    void Init();

    void FireGameEvent(IGameEvent* pEvent);

    bool m_bTimerIsRunning, m_bMapFinished;
    bool m_bTimeDidSave, m_bTimeDidUpload;

    bool m_bPlayerInsideStartZone, m_bPlayerInsideEndZone;
    bool m_bPlayerHasPracticeMode;

    int m_iTotalStrafes, m_iTotalJumps;
    float m_flStartSpeed, m_flEndSpeed, m_flVelocityMax, m_flVelocityAvg, m_flStrafeSyncAvg, m_flStrafeSync2Avg;

    int m_iCurrentStage, m_iStageJumps[MAX_STAGES], m_iStageStrafes[MAX_STAGES];
    float m_flStageTime[MAX_STAGES], m_flStageStartSpeed[MAX_STAGES], m_flStageVelocityMax[MAX_STAGES],
        m_flStageVelocityAvg[MAX_STAGES], m_flStageStrafeSyncAvg[MAX_STAGES], 
        m_flStageStrafeSync2Avg[MAX_STAGES];

};

extern C_Momentum_EventListener *g_MOMEventListener;