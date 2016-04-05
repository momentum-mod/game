
class C_Momentum_EventListener : public IGameEventListener2
{
public:
    C_Momentum_EventListener();

    void FireGameEvent(IGameEvent* pEvent);

    bool m_bRunSaved = false, m_bRunUploaded = false;
    bool m_bTimerIsRunning = false, m_bMapFinished = false;

    bool m_bPlayerInsideStartZone = false , m_bPlayerInsideEndZone = false;

    float m_flStartSpeed, m_flEndSpeed, m_flVelocityMax, m_flVelocityAvg, m_flStrafeSyncAvg, m_flStrafeSync2Avg;
};
