#pragma once

#include "cbase.h"

struct RunCompare_t
{
    //Name of the comparison.
    char runName[32];//MOM_TODO: determine a good size for this array. 
    //Note: we're using CUtlVectors here so we don't have to parse the stage/checkpoint number from the .tim file!
    CUtlVector<float> overallSplits,//Stage enter times (overall times)
        stageSplits,//Times spent on stages (stage time)
        stageAvgVels[2],//Average velocities for stages, 0 = 3D vels, 1 = horizontal vels
        stageMaxVels[2],//Maximum velocities for stages, 0 = 3D vels, 1 = horizontal vels
        stageEnterVels[2],//Velocity with which you enter a stage (exit a stage start trigger), 0 = 3D vels, 1 = horizontal vels
        stageExitVels[2],//Velocity with which you leave a stage (one stage -> next), 0 = 3D vels, 1 = horizontal vels
        stageAvgSync1,//Average stage sync1
        stageAvgSync2;//Average stage sync2
    CUtlVector<int> stageJumps,//Number of jumps on this stage
        stageStrafes;//Number of strafes on this stage
};

enum ComparisonString_t
{
    TIME_OVERALL = 1,
    STAGE_TIME,
    VELOCITY_AVERAGE,
    VELOCITY_MAX,
    VELOCITY_ENTER,
    VELOCITY_EXIT,
    STAGE_SYNC1,
    STAGE_SYNC2,
    STAGE_JUMPS,
    STAGE_STRAFES
};