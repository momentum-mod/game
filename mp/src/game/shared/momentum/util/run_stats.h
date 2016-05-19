#pragma once

#include "cbase.h"
#include "mom_shareddefs.h"

struct RunStats_t
{
    //MOM_TODO: We're going to hold an unbiased view at both
    //checkpoint and stages. If a map is linear yet has checkpoints,
    //it can be free to use these below to display stats for the player to compare against.

    //Note: Passing 0 as the index to any of these will return overall.
   
    //Keypress
    int m_iStageJumps[MAX_STAGES],//Amount of jumps per stage
        m_iStageStrafes[MAX_STAGES];//Amount of strafes per stage

    //Time
    float m_flStageTime[MAX_STAGES], //The amount of time (seconds) you spent to accomplish (stage) -> (stage + 1)
        m_flStageEnterTime[MAX_STAGES]; //The time in seconds that you entered the given stage

    //Sync
    float m_flStageStrafeSyncAvg[MAX_STAGES],//The average sync1 you had over the given stage
        m_flStageStrafeSync2Avg[MAX_STAGES];//The average sync2 you had over the given stage

    //Velocity
    //Note: The secondary index is as follows: 0 = 3D Velocity (z included), 1 = Horizontal (XY) Velocity
    float m_flStageEnterSpeed[MAX_STAGES][2],//The velocity with which you started the stage (exit this stage's start trigger)
        m_flStageVelocityMax[MAX_STAGES][2],//Max velocity for a stage
        m_flStageVelocityAvg[MAX_STAGES][2],//Average velocity in a stage
        m_flStageExitSpeed[MAX_STAGES][2];//The velocity with which you exit the stage (this stage -> next)
};