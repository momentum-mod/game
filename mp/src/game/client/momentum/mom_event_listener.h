#pragma once

#include "mom_shareddefs.h"


class C_Momentum_EventListener : public CGameEventListener
{
public:
    C_Momentum_EventListener() : 
        m_bTimerIsRunning(false),
        m_bTimeDidSave(false),
        m_bTimeDidUpload(false),
        m_bPlayerHasPracticeMode(false),
        m_iCurrentStage(0)
    { }

    void Init();

    void FireGameEvent(IGameEvent* pEvent) override;

    bool m_bTimerIsRunning;// , m_bMapFinished;
    bool m_bTimeDidSave, m_bTimeDidUpload;

    //bool m_bPlayerInsideStartZone, m_bPlayerInsideEndZone;
    bool m_bPlayerHasPracticeMode;

    int m_iCurrentStage, m_iStageJumps[MAX_STAGES], m_iStageStrafes[MAX_STAGES];
    float m_flStageTime[MAX_STAGES], m_flStageEnterTime[MAX_STAGES], m_flStageStrafeSyncAvg[MAX_STAGES], 
        m_flStageStrafeSync2Avg[MAX_STAGES];

    float m_flStageStartSpeed[MAX_STAGES][2], m_flStageVelocityMax[MAX_STAGES][2],
        m_flStageVelocityAvg[MAX_STAGES][2], m_flStageExitSpeed[MAX_STAGES][2];

    char m_szRunUploadStatus[512];//MOM_TODO: determine best (max) size for this
};

extern C_Momentum_EventListener *g_MOMEventListener;