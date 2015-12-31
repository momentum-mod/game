#ifndef TIMER_H
#define TIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"
#include "tickset.h"
#include "KeyValues.h"
#include "mom_util.h"
#include <ctime>

class CTriggerTimerStart;
class CTriggerCheckpoint;
class CTriggerOnehop;

class CTimer
{
    //DECLARE_CLASS_NOBASE(CTimer);
public:
    // Strats the timer for the given starting tick
    void Start(int startTick);
    // Stops the timer
    void Stop(bool);
    // MOM_TODO
    // Timer does not think. We'll have to "hack" it
    void Think();
    void DispatchStateMessage();
    void DispatchResetMessage();
    void DispatchCheckpointMessage();
    // Is the timer running?
    bool IsRunning();
    // Set the running status of the timer
    void SetRunning(bool running);
    // Gets the current starting trigger
    CTriggerTimerStart *GetStartTrigger();
    // Gets the current stage checkpoint
    CTriggerCheckpoint *GetCurrentCheckpoint();
    CTriggerCheckpoint *GetCheckpointAt(int checkpointNumber);
    // Seths the given trigger as the start trigger
    void SetStartTrigger(CTriggerTimerStart *pTrigger);
    // Sets the current checkpoint (stage) as the current checkpoint
    void SetCurrentCheckpointTrigger(CTriggerCheckpoint *pTrigger);
    // gets the current menu checkpoint index
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

    // Creates a checkpoint (menu) on the location of the given Entity
    void CreateCheckpoint(CBasePlayer*);
    // Removes last checkpoint (menu) form the checkpoint lists
    void RemoveLastCheckpoint();
    // Removes every checkpoint (menu) on the checkpoint list
    void RemoveAllCheckpoints()
    {
        checkpoints.RemoveAll();
        m_iCurrentStepCP = -1;
        m_bUsingCPMenu = false;
    }
    // Teleports the entity to the checkpoint (menu) with the given index
    void TeleportToCP(CBasePlayer*, int);
    // Sets the current checkpoint (menu) to the desired one with that index
    void SetCurrentCPMenuStep(int newNum)
    {
        m_iCurrentStepCP = newNum;
    }
    // gets the total amount of menu checkpoints
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
    // Removes all onehops from the list
    void RemoveAllOnehopsFromList() { onehops.RemoveAll(); }
    // Returns the count for the onehop list
    int GetOnehopListCount() { return onehops.Count(); }
    // Finds the onehop with the given index on the list
    CTriggerOnehop* FindOnehopOnList(int pIndexOnList);
    //Online-related timer commands

    // Tries to post the current time.
    void PostTime();
    //MOM_TODO: void LoadOnlineTimes();
    //Local-related timer commands

    // Loads local times from given map name
    void LoadLocalTimes(const char*);
    // Saves current time to a local file
    void SaveTime();
    void OnMapEnd(const char *);
    // Cheat detection

    // Have the cheats been turned on in this session?
    bool GotCaughtCheating() { return m_bWereCheatsActivated; };

private:

    int m_iStartTick;
    bool m_bIsRunning;
    bool m_bIsPaused;
    bool m_bWereCheatsActivated;
    ConVar *m_cCheats;
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
    // Extension used for storing local map times
    const char* c_timesExt = ".tim";
};

extern CTimer g_Timer;

#endif // TIMER_H
