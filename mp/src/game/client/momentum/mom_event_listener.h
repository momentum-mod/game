#pragma once

#include "mom_shareddefs.h"


class C_Momentum_EventListener : public CGameEventListener
{
public:
    C_Momentum_EventListener() : 
        m_bTimerIsRunning(false),
        m_bTimeDidSave(false),
        m_bTimeDidUpload(false),
        m_bPlayerHasPracticeMode(false)
    { }

    void Init();

    void FireGameEvent(IGameEvent* pEvent) override;

    bool m_bTimerIsRunning;
    bool m_bTimeDidSave, m_bTimeDidUpload;
    bool m_bMapIsLinear;

    int m_iMapCheckpointCount;

    bool m_bPlayerHasPracticeMode;

    //MOM_TODO: We're going to hold an unbiased view at both
    //checkpoint and stages. If a map is linear yet has checkpoints,
    //it can be free to use these below to display stats for the player to compare against. 
    int m_iStageJumps[MAX_STAGES], m_iStageStrafes[MAX_STAGES];
    float m_flStageTime[MAX_STAGES], m_flStageEnterTime[MAX_STAGES], m_flStageStrafeSyncAvg[MAX_STAGES], 
        m_flStageStrafeSync2Avg[MAX_STAGES];

    float m_flStageEnterSpeed[MAX_STAGES][2],//The velocity with which you started the stage (exit this stage's start trigger)
        m_flStageVelocityMax[MAX_STAGES][2],//Max velocity for a stage
        m_flStageVelocityAvg[MAX_STAGES][2],//Average velocity in a stage
        m_flStageExitSpeed[MAX_STAGES][2];//The velocity with which you exit the stage (this stage -> next)

    char m_szRunUploadStatus[512];//MOM_TODO: determine best (max) size for this
};

extern C_Momentum_EventListener *g_MOMEventListener;