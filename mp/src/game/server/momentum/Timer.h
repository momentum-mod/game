#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "momentum/tickset.h"
#include "KeyValues.h"
#include "momentum/util/mom_util.h"
#include "filesystem.h"
#include "mom_triggers.h"
#include "GameEventListener.h"
#include "tier1/checksum_sha1.h"
#include "momentum/mom_shareddefs.h"
#include "momentum/mom_gamerules.h"
#include "mom_replay_system.h"
#include "movevars_shared.h"
#include <ctime>

class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;
class CTriggerStage;

struct Time
{
    // overall run stats:
    float time_sec; // The amount of seconds taken to complete
    float tickrate; // Tickrate the run was done on
    time_t date;    // Date achieved
    int flags;

    // stage specific stats:
    CMomRunStats RunStats;

    Time() : time_sec(0), tickrate(0), date(0), flags(0), RunStats() {}
};

class CTimer
{
  public:
      CTimer()
          : m_iZoneCount(0), m_iStartTick(0), m_iEndTick(0), m_iLastZone(0), m_bIsRunning(false),
          m_bWereCheatsActivated(false), m_bMapIsLinear(false), m_pStartTrigger(nullptr), m_pEndTrigger(nullptr),
          m_pCurrentCheckpoint(nullptr), m_pCurrentZone(nullptr), m_iCurrentStepCP(0), m_bUsingCPMenu(false),
          m_pLocalTimes(nullptr)
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
    void Stop(bool = false);
    // Is the timer running?
    bool IsRunning() const { return m_bIsRunning; }
    // Set the running status of the timer
    void SetRunning(bool running);

    // ------------- Timer trigger related methods ----------------------------
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger() const { return m_pStartTrigger.Get(); }
    // Gets the current checkpoint
    CTriggerCheckpoint *GetCurrentCheckpoint() const { return m_pCurrentCheckpoint.Get(); }

    CTriggerTimerStop *GetEndTrigger() const { return m_pEndTrigger.Get(); }
    CTriggerStage *GetCurrentStage() const { return m_pCurrentZone.Get(); }

    // Sets the given trigger as the start trigger
    void SetStartTrigger(CTriggerTimerStart *pTrigger)
    {
        m_iLastZone = 0; // Allows us to overwrite previous runs
        m_pStartTrigger.Set(pTrigger);
    }

    // Sets the current checkpoint
    void SetCurrentCheckpointTrigger(CTriggerCheckpoint *pTrigger) { m_pCurrentCheckpoint.Set(pTrigger); }

    void SetEndTrigger(CTriggerTimerStop *pTrigger) { m_pEndTrigger.Set(pTrigger); }
    //MOM_TODO: Change this to be the CTriggerZone class
    void SetCurrentZone(CTriggerStage *pTrigger)
    {
        m_pCurrentZone.Set(pTrigger);
    }
    int GetCurrentZoneNumber() const { return m_pCurrentZone.Get() && m_pCurrentZone.Get()->GetStageNumber(); }

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
    // Removes the given Onehop form the hopped list.
    // Returns: True if deleted, False if not found.
    bool RemoveOnehopFromList(CTriggerOnehop *pTrigger);
    // Adds the give Onehop to the hopped list.
    // Returns: Its new index.
    int AddOnehopToListTail(CTriggerOnehop *pTrigger);
    // Finds a Onehop on the hopped list.
    // Returns: Its index. -1 if not found
    int FindOnehopOnList(CTriggerOnehop *pTrigger);
    // Removes all onehops from the list
    void RemoveAllOnehopsFromList() { onehops.RemoveAll(); }
    // Returns the count for the onehop list
    int GetOnehopListCount() const { return onehops.Count(); }
    // Finds the onehop with the given index on the list
    CTriggerOnehop *FindOnehopOnList(int pIndexOnList);

    //-------- Generic Time & Run related code
    // Converts the provided run from kvRun into a Time struct.
    void ConvertKVToTime(KeyValues *kvRun, Time &into) const;
    //Converts a given Time struct into a KeyValues object into kvInto.
    //Note: kvInto must be declared BEFORE going into the function!
    void ConvertTimeToKV(KeyValues *kvInto, Time *from) const;
    //-------- Online-related timer commands -----------------------------
    // Tries to post the current time.
    void PostTime();
    // MOM_TODO: void LoadOnlineTimes();

    //------- Local-related timer commands -------------------------------
    // Loads local times from given map name
    void LoadLocalTimes(const char *);
    // Add a new time to the local times KV
    void AddNewTime(Time* t) const;
    // Saves current time to a local file
    void SaveTimeToFile() const;
    // Unloads loaded times
    void UnloadLoadedLocalTimes()
    {
        if (m_pLocalTimes)
            m_pLocalTimes->deleteThis();
        m_pLocalTimes = nullptr;
    }
    void OnMapEnd(const char *);
    void OnMapStart(const char *);
    void DispatchMapInfo() const;
    // Practice mode- noclip mode that stops timer
    // void PracticeMove(); MOM_TODO: NOT IMPLEMENTED
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

  private:
    int m_iZoneCount;
    int m_iStartTick, m_iEndTick;
    int m_iLastZone;
    time_t m_iLastRunDate;
    bool m_bIsRunning;
    bool m_bWereCheatsActivated;
    bool m_bMapIsLinear;

    CHandle<CTriggerTimerStart> m_pStartTrigger;
    CHandle<CTriggerTimerStop> m_pEndTrigger;
    CHandle<CTriggerCheckpoint> m_pCurrentCheckpoint;
    CHandle<CTriggerStage> m_pCurrentZone; // MOM_TODO: Change to be the generic Zone trigger

    
    CUtlVector<CTriggerOnehop *> onehops;
    KeyValues *m_pLocalTimes;
    // MOM_TODO: KeyValues *m_pOnlineTimes;

    int m_iCurrentStepCP;
    bool m_bUsingCPMenu;
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
    CTimeTriggerTraceEnum(Ray_t *pRay, Vector velocity, int zoneType, int cornerNum)
        : m_iZoneType(zoneType), m_pRay(pRay), m_iCornerNumber(cornerNum)
    {
    }

    bool EnumEntity(IHandleEntity *pHandleEntity) override;

  private:
    int m_iZoneType;
    int m_iCornerNumber;
    Ray_t *m_pRay;
};

extern CTimer *g_Timer;

#endif // TIMER_H