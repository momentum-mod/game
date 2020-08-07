#pragma once

#include "mom_shareddefs.h"

struct SavedLocation_t;
class CTriggerTimerStart;
class CMomentumPlayer;

class CMomentumTimer : public CAutoGameSystemPerFrame
{
  public:
    CMomentumTimer();

    // CAutoGameSystemPerFrame
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void FrameUpdatePreEntityThink() OVERRIDE;

    // HUD messages
    void DispatchCheatsMessage(CMomentumPlayer *pPlayer);
    void DispatchTickrateMessage(CMomentumPlayer *pPlayer);
    void DispatchResetMessage(CMomentumPlayer *pPlayer) const;
    void DispatchTimerEventMessage(CBasePlayer *pPlayer, int iEntIdx, int type) const;

    // ------------- Timer state related messages --------------------------
    // Starts the timer for the given starting tick
    // Returns true if timer successfully started, otherwise false
    bool Start(CMomentumPlayer *pPlayer);
    // Stops the timer
    // If bFinished is true the timer will dispatch a TIMER_EVENT_FINISHED event leading to the time being saved,
    // otherwise TIMER_EVENT_STOPPED is dispatched.
    // If bStopRecording is true the timer will stop the replay recording. If bFinished is true an
    // attempt will be made to also save the replay
    void Stop(CMomentumPlayer *pPlayer, bool bFinished = false, bool bStopRecording = true);
    // Resets timer as well as all player stats
    void Reset(CMomentumPlayer *pPlayer);
    // Is the timer running?
    bool IsRunning() const { return m_bIsRunning; }
    // Set the running status of the timer
    void SetRunning(CMomentumPlayer *pPlayer, bool running);

    // ------------- Timer trigger related methods ----------------------------
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger(int track) const { return (track >= 0 && track < MAX_TRACKS) ? m_hStartTriggers[track] : nullptr; }

    // Sets the given trigger as the start trigger for it's associated track
    void SetStartTrigger(int track, CTriggerTimerStart *pTrigger);


    // Gets the current time for this timer
    int GetCurrentTime() const { return gpGlobals->tickcount - m_iStartTick; }
    // Gets the time for the last run, if there was one
    int GetLastRunTime() const;

    // Practice mode- noclip mode that stops timer
    void EnablePractice(CMomentumPlayer *pPlayer);
    void DisablePractice(CMomentumPlayer *pPlayer);

    int GetTrackNumber() const { return m_iTrackNumber; }

    bool ShouldUseStartZoneOffset() const { return m_bShouldUseStartZoneOffset; }
    void SetShouldUseStartZoneOffset(bool use) { m_bShouldUseStartZoneOffset = use; }
    void SetCanStart(bool canStart) { m_bCanStart = canStart; }

    // creates fraction of a tick to be used as a time "offset" in precisely calculating the real run time.
    void CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, int zoneType, int iZoneNumber);
    void SetIntervalOffset(int stage, float offset) { m_flTickOffsetFix[stage] = offset; }

    // tries to start timer, if successful also sets all the player vars and starts replay
    void TryStart(CMomentumPlayer *pPlayer, bool bUseStartZoneOffset);

  private:
    int m_iStartTick, m_iEndTick;
    bool m_bIsRunning;
    bool m_bCanStart;
    bool m_bWasCheatsMsgShown;

    CHandle<CTriggerTimerStart> m_hStartTriggers[MAX_TRACKS];

    int m_iTrackNumber;

    // PRECISION FIX:
    // this works by adding the starting offset to the final time, since the timer starts after we actually exit the
    // start trigger
    // also, subtract the ending offset from the time, since we end after we actually enter the ending trigger
    float m_flTickOffsetFix[MAX_ZONES]; // index 0 = endzone, 1 = startzone, 2 = stage 2, 3 = stage3, etc
    bool m_bShouldUseStartZoneOffset;
    float m_flDistFixTraceCorners[8]; // array of floats representing the trace distance from each corner of the
                                      // player's collision hull
};

extern CMomentumTimer *g_pMomentumTimer;
