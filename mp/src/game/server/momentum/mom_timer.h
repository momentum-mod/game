#pragma once

#include "mom_shareddefs.h"

struct SavedLocation_t;
class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;
class CTriggerStage;
class CTriggerTimerStop;
class CMomentumPlayer;

class CMomentumTimer : CAutoGameSystem
{
  public:
      CMomentumTimer(const char *pName)
        : CAutoGameSystem(pName), m_iZoneCount(0), m_iStartTick(0), m_iEndTick(0), m_iLastZone(0), m_iLastRunDate(0), m_bIsRunning(false),
          m_bWereCheatsActivated(false), m_bMapIsLinear(false), m_pStartTrigger(nullptr), m_pEndTrigger(nullptr),
          m_pCurrentZone(nullptr), m_pLocalTimes(nullptr), m_pStartZoneMark(nullptr)
    {
    }

    //-------- HUD Messages --------------------
    void DispatchResetMessage();
    //Plays the hud_timer effects to a specific player
    void DispatchTimerStateMessage(CBasePlayer* pPlayer, bool isRunning) const;

    // ------------- Timer state related messages --------------------------
    // Strats the timer for the given starting tick
    void Start(int startTick);
    // Stops the timer
    void Stop(bool endTrigger = false, bool bStopRecording = true);
    // Is the timer running?
    bool IsRunning() const { return m_bIsRunning; }
    // Set the running status of the timer
    void SetRunning(bool running);

    // ------------- Timer trigger related methods ----------------------------
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger() const { return m_pStartTrigger; }
    CTriggerTimerStop *GetEndTrigger() const { return m_pEndTrigger; }
    CTriggerStage *GetCurrentStage() const { return m_pCurrentZone; }

    // Sets the given trigger as the start trigger
    void SetStartTrigger(CTriggerTimerStart *pTrigger)
    {
        m_iLastZone = 0; // Allows us to overwrite previous runs
        m_pStartTrigger = pTrigger;
    }

    void SetEndTrigger(CTriggerTimerStop *pTrigger) { m_pEndTrigger = pTrigger; }
    //MOM_TODO: Change this to be the CTriggerZone class
    void SetCurrentZone(CTriggerStage *pTrigger)
    {
        m_pCurrentZone = pTrigger;
    }

    int GetCurrentZoneNumber() const;

    // Calculates the stage count
    // Stores the result on m_iStageCount
    void RequestZoneCount();
    // Gets the total stage count
    int GetZoneCount() const { return m_iZoneCount; };
    float CalculateStageTime(int stageNum);
    // Gets the time for the last run, if there was one
    float GetLastRunTime()
    {
        if (m_iEndTick == 0)
            return 0.0f;
        float originalTime = static_cast<float>(m_iEndTick - m_iStartTick) * gpGlobals->interval_per_tick;
        // apply precision fix, adding offset from start as well as subtracting offset from end.
        // offset from end is 1 tick - fraction offset, since we started trace outside of the end zone.
        return originalTime + m_flTickOffsetFix[1] - (gpGlobals->interval_per_tick - m_flTickOffsetFix[0]);
    }
    // Gets the date achieved for the last run.
    time_t GetLastRunDate() const
    {
        return m_iLastRunDate;
    }

    // Gets the current time for this timer
    float GetCurrentTime() const { return float(gpGlobals->tickcount - m_iStartTick) * gpGlobals->interval_per_tick; }

    //----- Trigger_Onehop stuff -----------------------------------------
    

    //-------- Online-related timer commands -----------------------------
    // MOM_TODO: void LoadOnlineTimes();

    // Level init/shutdown hooks
    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdownPreEntity() OVERRIDE;
    void DispatchMapInfo() const;

    // Practice mode- noclip mode that stops timer
    void EnablePractice(CMomentumPlayer *pPlayer);
    void DisablePractice(CMomentumPlayer *pPlayer);

    // Have the cheats been turned on in this session?
    bool GotCaughtCheating() const { return m_bWereCheatsActivated; };
    void SetCheating(bool newBool)
    {
        UTIL_ShowMessage("CHEATER", UTIL_GetLocalPlayer());
        Stop(false);
        m_bWereCheatsActivated = newBool;
    }

    void SetGameModeConVars();

    void CreateStartMark();
    SavedLocation_t *GetStartMark() const { return m_pStartZoneMark; }
    void ClearStartMark();

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
    CTriggerStage *m_pCurrentZone; // MOM_TODO: Change to be the generic Zone trigger

    KeyValues *m_pLocalTimes;
    // MOM_TODO: KeyValues *m_pOnlineTimes;

    SavedLocation_t *m_pStartZoneMark;

public:
    // PRECISION FIX:
    // this works by adding the starting offset to the final time, since the timer starts after we actually exit the
    // start trigger
    // also, subtract the ending offset from the time, since we end after we actually enter the ending trigger
    float m_flTickOffsetFix[MAX_STAGES]; // index 0 = endzone, 1 = startzone, 2 = stage 2, 3 = stage3, etc
    float m_flZoneEnterTime[MAX_STAGES];

    // creates fraction of a tick to be used as a time "offset" in precicely calculating the real run time.
    void CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, const int zoneType);
    void SetIntervalOffset(int stage, float offset) { m_flTickOffsetFix[stage] = offset; }
    float m_flDistFixTraceCorners[8]; //array of floats representing the trace distance from each corner of the player's collision hull
    typedef enum { ZONETYPE_END, ZONETYPE_START } zoneType;
};

class CTimeTriggerTraceEnum : public IEntityEnumerator
{
  public:
    CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity)
        : m_vecVelocity(velocity), m_pRay(pRay)
    {
        m_flOffset = 0.0f;
    }

    bool EnumEntity(IHandleEntity *pHandleEntity) OVERRIDE;
    float GetOffset() { return m_flOffset; }

  private:
    float m_flOffset;
    Vector m_vecVelocity;
    Ray_t *m_pRay;
};

extern CMomentumTimer *g_pMomentumTimer;