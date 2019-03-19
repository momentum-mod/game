#pragma once

#include "mom_shareddefs.h"

class CBaseMomZoneTrigger;
struct SavedLocation_t;
class CBaseMomentumTrigger;
class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerStage;
class CTriggerZone;
class CTriggerTimerStop;
class CMomentumPlayer;

class CMomentumTimer : public CAutoGameSystemPerFrame
{
  public:
    CMomentumTimer(const char *pName);

  public: // CAutoGameSystemPerFrame
    void PostInit() OVERRIDE;
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;

    virtual void FrameUpdatePreEntityThink() OVERRIDE;

  public:
    // HUD messages
    void DispatchResetMessage() const;
    void DispatchTimerEventMessage(CBasePlayer *pPlayer, int type) const;
    void DispatchMapInfo();

    // ------------- Timer state related messages --------------------------
    // Starts the timer for the given starting tick
    // Returns true if timer successfully started, otherwise false
    bool Start(int startTick, int iBonusZone);
    // Stops the timer
    void Stop(bool endTrigger = false, bool bStopRecording = true);
    // Resets timer as well as all player stats
    void Reset();
    // Is the timer running?
    bool IsRunning() const { return m_bIsRunning; }
    // Set the running status of the timer
    void SetRunning(bool running);

    // ------------- Timer trigger related methods ----------------------------
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger() const { return m_pStartTrigger; }
    CTriggerTimerStop *GetEndTrigger() const { return m_pEndTrigger; }
    CTriggerZone *GetCurrentZone() const { return m_pCurrentZone; }

    // Sets the given trigger as the start trigger
    void SetStartTrigger(CTriggerTimerStart *pTrigger)
    {
        m_iLastZone = 0; // Allows us to overwrite previous runs
        m_pStartTrigger = pTrigger;
    }

    void SetEndTrigger(CTriggerTimerStop *pTrigger) { m_pEndTrigger = pTrigger; }
    void SetCurrentZone(CTriggerZone *pTrigger) { m_pCurrentZone = pTrigger; }
    int GetCurrentZoneNumber() const;

    // Calculates the stage count
    // Stores the result on m_iStageCount
    void RequestZoneCount();
    // Gets the total stage count
    int GetZoneCount() const { return m_iZoneCount; };
    float CalculateStageTime(int stageNum);
    // Gets the time for the last run, if there was one
    float GetLastRunTime();
    // Gets the date achieved for the last run.
    time_t GetLastRunDate() const { return m_iLastRunDate; }

    // Gets the current time for this timer
    float GetCurrentTime() const { return float(gpGlobals->tickcount - m_iStartTick) * gpGlobals->interval_per_tick; }

    //----- Trigger_Onehop stuff -----------------------------------------

    //-------- Online-related timer commands -----------------------------
    // MOM_TODO: void LoadOnlineTimes();

    // Practice mode- noclip mode that stops timer
    void EnablePractice(CMomentumPlayer *pPlayer);
    void DisablePractice(CMomentumPlayer *pPlayer);

    // Have the cheats been turned on in this session?
    bool GotCaughtCheating() const { return m_bWereCheatsActivated; };
    void SetCheating(bool cheating)
    {
        UTIL_ShowMessage("CHEATER", UTIL_GetLocalPlayer());
        Stop(false);
        m_bWereCheatsActivated = cheating;
    }

    void SetGameModeConVars();

    void CreateStartMark();
    SavedLocation_t *GetStartMark() const { return m_pStartZoneMark; }
    void ClearStartMark();

    int GetBonus() const { return m_iBonusZone; }

    bool ShouldUseStartZoneOffset() const { return m_bShouldUseStartZoneOffset; }
    void SetShouldUseStartZoneOffset(bool use) { m_bShouldUseStartZoneOffset = use; }

    // creates fraction of a tick to be used as a time "offset" in precicely calculating the real run time.
    void CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, const int zoneType);
    void SetIntervalOffset(int stage, float offset) { m_flTickOffsetFix[stage] = offset; }

    void OnPlayerEnterZone(CMomentumPlayer *pPlayer, CBaseMomZoneTrigger *pTrigger, int zonenum);
    void OnPlayerExitZone(CMomentumPlayer *pPlayer, CBaseMomZoneTrigger *pTrigger, int zonenum);
    void OnPlayerSpawn(CMomentumPlayer *pPlayer);

  private:
    void OnPlayerJump(KeyValues *kv);
    void OnPlayerLand(KeyValues *kv);

    // tries to start timer, if successful also sets all the player vars and starts replay
    void TryStart(CMomentumPlayer *pPlayer, bool bUseStartZoneOffset);

    void DispatchNoZonesMsg() const;

  private:
    int m_iZoneCount;
    int m_iStartTick, m_iEndTick;
    int m_iLastZone;
    time_t m_iLastRunDate;
    bool m_bIsRunning;
    bool m_bWereCheatsActivated;
    bool m_bMapIsLinear;

    CTriggerTimerStart *m_pStartTrigger;
    CTriggerTimerStop *m_pEndTrigger;
    CTriggerZone *m_pCurrentZone;

    SavedLocation_t *m_pStartZoneMark;

    int m_iBonusZone;

    // PRECISION FIX:
    // this works by adding the starting offset to the final time, since the timer starts after we actually exit the
    // start trigger
    // also, subtract the ending offset from the time, since we end after we actually enter the ending trigger
    float m_flTickOffsetFix[MAX_STAGES]; // index 0 = endzone, 1 = startzone, 2 = stage 2, 3 = stage3, etc
    float m_flZoneEnterTime[MAX_STAGES];
    bool m_bShouldUseStartZoneOffset;
    float m_flDistFixTraceCorners[8]; // array of floats representing the trace distance from each corner of the
                                      // player's collision hull
};

extern CMomentumTimer *g_pMomentumTimer;
