
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

    bool m_bPlayerInsideStartZone = false , m_bPlayerInsideEndZone = false;
    bool m_bPlayerHasPracticeMode = false;

    float m_flStartSpeed = 0, m_flEndSpeed, m_flVelocityMax, m_flVelocityAvg, m_flStrafeSyncAvg, m_flStrafeSync2Avg;
    int m_iCurrentStage, m_iStageTicks;
};
