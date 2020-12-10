#pragma once

// This class handles networking all of the overlap between players and
// replay files. From an OOP standpoint, this is more efficient than having
// two classes network the same variables.

#ifdef CLIENT_DLL
#define CMomRunEntityData C_MomRunEntityData
EXTERN_RECV_TABLE(DT_MomRunEntityData);
#else
EXTERN_SEND_TABLE(DT_MomRunEntityData);
#endif

enum TimerState_t
{
    TIMER_STATE_NOT_RUNNING = 0,
    TIMER_STATE_RUNNING, // Officially running (replay, official run attempt)
    TIMER_STATE_PRACTICE // Artificially running (loaded saveloc)
};

class CMomRunEntityData
{
  public:
    DECLARE_CLASS_NOBASE(CMomRunEntityData);
    DECLARE_EMBEDDED_NETWORKVAR();

    CMomRunEntityData();    

    // If you are adding anything to this class, make sure to add it to the DataTables in the CPP file!

    CNetworkVar(bool, m_bIsInZone);       // This is true if the player is in a CTriggerTimerStage zone
    CNetworkVar(bool, m_bMapFinished);    // Did the player finish the map?
    CNetworkVar(int, m_iTimerState);     // Is the timer currently running for this ent? See TimerState_t
    CNetworkVar(float, m_flStrafeSync);   // eyeangle based, perfect strafes / total strafes
    CNetworkVar(float, m_flStrafeSync2);  // acceleration based, strafes speed gained / total strafes
    CNetworkVar(uint32, m_iRunFlags);     // The run flags (W only/HSW/Scroll etc) of the player
    CNetworkVar(int, m_iCurrentTrack);    // The current track that this entity is on
    CNetworkVar(int, m_iCurrentZone);     // Current stage/checkpoint the player is on
    CNetworkVar(int, m_iStartTick);       // Tick that the entity started its timer
    CNetworkVar(float, m_flTickRate);     // Interval per tick the run was done with
    CNetworkVar(int, m_iRunTime);         // The time taken to do their most recent run, in ticks
    CNetworkVar(float, m_flLastJumpTime); // The last time that the player jumped
    CNetworkVar(float, m_flLastJumpVel);  // Last jump velocity of the player
    CNetworkVar(float, m_flLastJumpZPos); // Z coordinate of player on jump

    // If you are adding anything to this class, make sure to add it to the DataTables in the CPP file!
};