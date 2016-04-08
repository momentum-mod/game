#include "mom_shareddefs.h"

class C_Momentum_EventListener : public IGameEventListener2
{
public:
    C_Momentum_EventListener();
    ~C_Momentum_EventListener(){
        gameeventmanager->RemoveListener(this);
    }

    void FireGameEvent(IGameEvent* pEvent);

    bool m_bTimerIsRunning = false, m_bMapFinished = false;
    bool m_bTimeDidSave, m_bTimeDidUpload;

    bool m_bPlayerInsideStartZone = false, m_bPlayerInsideEndZone = false;
    bool m_bPlayerHasPracticeMode = false;

    int m_iTotalStrafes = 0, m_iTotalJumps = 0;
    float m_flStartSpeed = 0, m_flEndSpeed, m_flVelocityMax, m_flVelocityAvg, m_flStrafeSyncAvg, m_flStrafeSync2Avg;

    int m_iCurrentStage, m_iStageTicks[MAX_STAGES], m_iStageJumps[MAX_STAGES], m_iStageStrafes[MAX_STAGES];
    float m_flStageStartSpeed[MAX_STAGES], m_flStageVelocityMax[MAX_STAGES],
        m_flStageVelocityAvg[MAX_STAGES], m_flStageStrafeSyncAvg[MAX_STAGES], 
        m_flStageStrafeSync2Avg[MAX_STAGES];

};