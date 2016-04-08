#include "mom_shareddefs.h"

class C_Momentum_EventListener : public IGameEventListener2
{
public:
    C_Momentum_EventListener();
    ~C_Momentum_EventListener(){
        gameeventmanager->RemoveListener(this);
    }

    void FireGameEvent(IGameEvent* pEvent);

    static bool m_bTimerIsRunning, m_bMapFinished;
    static bool m_bTimeDidSave, m_bTimeDidUpload;

    static bool m_bPlayerInsideStartZone, m_bPlayerInsideEndZone;
    static bool m_bPlayerHasPracticeMode;

    static int m_iTotalStrafes, m_iTotalJumps;
    static float m_flStartSpeed, m_flEndSpeed, m_flVelocityMax, m_flVelocityAvg, m_flStrafeSyncAvg, m_flStrafeSync2Avg;

    static int m_iCurrentStage, m_iStageTicks[MAX_STAGES], m_iStageJumps[MAX_STAGES], m_iStageStrafes[MAX_STAGES];
    static float m_flStageStartSpeed[MAX_STAGES], m_flStageVelocityMax[MAX_STAGES],
        m_flStageVelocityAvg[MAX_STAGES], m_flStageStrafeSyncAvg[MAX_STAGES], 
        m_flStageStrafeSync2Avg[MAX_STAGES];

};