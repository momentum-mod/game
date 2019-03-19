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

class CMomRunEntityData
{
  public:
    DECLARE_CLASS_NOBASE(CMomRunEntityData);
    DECLARE_EMBEDDED_NETWORKVAR();

    CMomRunEntityData();    

    // If you are adding anything to this class, make sure to add it to the DataTables in the CPP file!

    CNetworkVar(bool, m_bIsInZone);       // This is true if the player is in a CTriggerTimerStage zone
    CNetworkVar(bool, m_bMapFinished);    // Did the player finish the map?
    CNetworkVar(bool, m_bTimerRunning);   // Is the timer currently running for this ent?
    CNetworkVar(float, m_flStrafeSync);   // eyeangle based, perfect strafes / total strafes
    CNetworkVar(float, m_flStrafeSync2);  // acceleration based, strafes speed gained / total strafes
    CNetworkVar(uint32, m_iRunFlags);     // The run flags (W only/HSW/Scroll etc) of the player
    CNetworkVar(int, m_iCurrentTrack);    // The current track that this entity is on
    CNetworkVar(int, m_iCurrentZone);     // Current stage/checkpoint the player is on
    CNetworkVar(int, m_iBonusZone);       // Is timer running under bonus zone? MOM_TODO REMOVEME!!!!
    CNetworkVar(int, m_iOldZone);         // What was the zone before we entered in end zone?
    CNetworkVar(int, m_iOldBonusZone);    // What was the bonus zone before we entered in end zone? MOM_TODO REMOVEME!!!!
    CNetworkVar(int, m_iStartTick);       // Tick that the entity started its timer
    CNetworkVar(float, m_flTickRate);     // Interval per tick the run was done with
    CNetworkVar(float, m_flRunTime);      // The time taken to do their most recent run MOM_TODO REMOVEME!!!!
    CNetworkVar(int, m_iRunTimeTicks);    // The time taken to do their most recent run, in ticks
    CNetworkVar(float, m_flLastJumpTime); // The last time that the player jumped
    CNetworkVar(float, m_flLastJumpVel);  // Last jump velocity of the player

    // If you are adding anything to this class, make sure to add it to the DataTables in the CPP file!
};