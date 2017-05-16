#pragma once

#include "cbase.h"

//This class handles networking all of the overlap between players and 
//replay files. From an OOP standpoint, this is more efficient than having
//two classes network the same variables.

#ifdef CLIENT_DLL 
#define CMOMRunEntityData C_MOMRunEntityData
#endif

class CMOMRunEntityData
{
public:

    CMOMRunEntityData();

    bool m_bAutoBhop;// Is the player using auto bhop?
    bool m_bIsInZone;//This is true if the player is in a CTriggerTimerStage zone
    bool m_bMapFinished;//Did the player finish the map?
    bool m_bTimerRunning;//Is the timer currently running for this ent?
    int m_iSuccessiveBhops; //How many successive bhops this player has
    float m_flStrafeSync; //eyeangle based, perfect strafes / total strafes
    float m_flStrafeSync2; //acceleration based, strafes speed gained / total strafes
    float m_flLastJumpTime; //The last time that the player jumped
    float m_flLastJumpVel; //Last jump velocity of the player
    uint32 m_iRunFlags;//The run flags (W only/HSW/Scroll etc) of the player
    int m_iCurrentZone;//Current stage/checkpoint the player is on
    int m_iStartTick; //Tick that the entity started its timer
	int m_iStartTickD; //The tick difference between timer and record
    float m_flRunTime; //The time taken to do their most recent run
};