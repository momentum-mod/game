#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "tickset.h"
#include "keyvalues.h"
#include <ctime>

class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;

class CTimer
{
    //DECLARE_CLASS_NOBASE(CTimer);
public:
    void Start(int startTick);
    void Stop(bool);
    void DispatchStateMessage();
    void DispatchResetMessage();
    void DispatchCheckpointMessage();
    bool IsRunning();
    void SetRunning(bool running);
    CTriggerTimerStart *GetStartTrigger();
    CTriggerCheckpoint *GetCurrentCheckpoint();
    CTriggerCheckpoint *GetCheckpointAt(int checkpointNumber);
    void SetStartTrigger(CTriggerTimerStart *pTrigger);
    void SetCurrentCheckpointTrigger(CTriggerCheckpoint *pTrigger);
    int GetCurrentCPMenuStep()
    {
        return m_iCurrentStepCP;
    }

    //For leaderboard use later on
    bool IsUsingCPMenu()
    {
        return m_bUsingCPMenu;
    }
    // CheckpointMenu stuff

    void CreateCheckpoint(CBasePlayer*);
    void RemoveLastCheckpoint();
    void RemoveAllCheckpoints()
    {
        checkpoints.RemoveAll();
        m_iCurrentStepCP = -1;
        m_bUsingCPMenu = false;
    }
    void TeleportToCP(CBasePlayer*, int);
    void SetCurrentCPMenuStep(int newNum)
    {
        m_iCurrentStepCP = newNum;
    }
    int GetCPCount()
    {
        return checkpoints.Size();
    }
    // Trigger_Onehop stuff

    // Removes the given Onehop form the hopped list.
    // Returns: True if deleted, False if not found.
    bool RemoveOnehopFromList(CTriggerOnehop* pTrigger);
    // Adds the give Onehop to the hopped list.
    // Returns: Its new index.
    int AddOnehopToListTail(CTriggerOnehop* pTrigger);
    // Finds a Onehop on the hopped list.
    // Returns: Its index. -1 if not found
    int FindOnehopOnList(CTriggerOnehop* pTrigger);
    void RemoveAllOnehopsFromList() { onehops.RemoveAll(); }
    int GetOnehopListCount() { return onehops.Count(); }
    CTriggerOnehop* FindOnehopOnList(int pIndexOnList);

    //Online-related timer commands
    void OnTimeSubmitted(HTTPRequestCompleted_t *pCallback, bool bIOFailure);
    CCallResult<CTimer, HTTPRequestCompleted_t> OnTimeSubmittedCallback;
    void PostTime();
    //MOM_TODO: void LoadOnlineTimes();
    
    //Local-related timer commands
    void LoadLocalTimes(const char*);
    void SaveTime();


    void OnMapEnd(const char *);

private:

    int m_iStartTick;
    bool m_bIsRunning;
    bool m_bIsPaused;

    CTriggerTimerStart *m_pStartTrigger;
    CTriggerCheckpoint *m_pCurrentCheckpoint;

    struct Time
    {
        //The amount of ticks took to complete
        int ticks;
        //Tickrate the run was done on
        float tickrate;
        //Date achieved
        time_t date;
    };

    struct Checkpoint
    {
        Vector pos;
        Vector vel;
        QAngle ang;
    };
    CUtlVector<Checkpoint> checkpoints;
    CUtlVector<CTriggerOnehop*> onehops;
    CUtlVector<Time> localTimes;
    //MOM_TODO: CUtlVector<OnlineTime> onlineTimes;
    int m_iCurrentStepCP = 0;
    bool m_bUsingCPMenu = false;

    const char* c_mapDir = "maps/";
    const char* c_timesExt = ".tim";
};

extern CTimer g_Timer;

#endif // TIMER_H
