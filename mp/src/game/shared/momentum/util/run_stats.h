#pragma once

#include "cbase.h"
#include "mom_shareddefs.h"

struct RunStats_t
{
    RunStats_t(int size = MAX_STAGES)
    {
        m_iTotalStages = size;
        //dynamically allocate the size of the run stats
        m_iStageJumps = new int[size];
        m_iStageStrafes = new int[size];

        m_flStageStrafeSyncAvg = new float[size];
        m_flStageStrafeSync2Avg = new float[size];
        m_flStageEnterTime = new float[size];
        m_flStageTime = new float[size];

        //the 2d arrays are basically an array of pointers to arrays.
        m_flStageEnterSpeed = new float*[size];
        m_flStageExitSpeed = new float*[size];
        m_flStageVelocityMax = new float*[size];
        m_flStageVelocityAvg = new float*[size];

        for (int m = 0; m < size; ++m)
        {
            m_flStageEnterSpeed[m] = new float[2];
            m_flStageExitSpeed[m] = new float[2];
            m_flStageVelocityMax[m] = new float[2];
            m_flStageVelocityAvg[m] = new float[2];
        }

        //initialize everything to 0
        for (int i = 0; i < size; i++)
        {
            m_iStageJumps[i] = 0;
            m_iStageStrafes[i] = 0;

            m_flStageStrafeSyncAvg[i] = 0;
            m_flStageStrafeSync2Avg[i] = 0;
            m_flStageEnterTime[i] = 0;
            m_flStageTime[i] = 0;
            for (int k = 0; k < 2; k++)
            {
                m_flStageEnterSpeed[i][k] = 0;
                m_flStageExitSpeed[i][k] = 0;
                m_flStageVelocityMax[i][k] = 0;
                m_flStageVelocityAvg[i][k] = 0;
            }
        }
    }
    /*
    MOM_TODO:
    @tuxxi: this somehow breaks future memory allocation and will cause issues when trying 
    to access m_flVelocityAvg in a newly created RunStats object. No idea why. Commenting out for now, FIX BEFORE RELEASE!!!!

    ~RunStats_t()
    {
        delete[] m_iStageJumps;
        delete[] m_iStageStrafes;

        delete[] m_flStageEnterTime;
        delete[] m_flStageTime;
        delete[] m_flStageStrafeSyncAvg;
        delete[] m_flStageStrafeSync2Avg;

        for (int i = 0; i < m_iTotalStages; ++i)
        {
            delete[] m_flStageEnterSpeed[i];
            delete[] m_flStageExitSpeed[i];
            delete[] m_flStageVelocityMax[i];
            delete[] m_flStageVelocityAvg[i];
        }
        delete[] m_flStageEnterSpeed;
        delete[] m_flStageExitSpeed;
        delete[] m_flStageVelocityMax;
        delete[] m_flStageVelocityAvg;
    }
    */
    RunStats_t& operator=(const RunStats_t& other)
    {
        for (int i = 0; i < other.m_iTotalStages; i++)
        {
            m_iStageJumps[i] = other.m_iStageJumps[i];
            m_iStageStrafes[i] = other.m_iStageStrafes[i];

            m_flStageStrafeSyncAvg[i] = other.m_flStageStrafeSyncAvg[i];
            m_flStageStrafeSync2Avg[i] = other.m_flStageStrafeSync2Avg[i];
            m_flStageEnterTime[i] = other.m_flStageEnterTime[i];
            m_flStageTime[i] = other.m_flStageTime[i];

            for (int k = 0; k < 2; k++)
            {
                m_flStageVelocityMax[i][k] = other.m_flStageVelocityMax[i][k];
                m_flStageVelocityAvg[i][k] = other.m_flStageVelocityAvg[i][k];
                m_flStageEnterSpeed[i][k] = other.m_flStageEnterSpeed[i][k];
                m_flStageExitSpeed[i][k] = other.m_flStageExitSpeed[i][k];
            }
        }
        return *this;
    }
    //MOM_TODO: We're going to hold an unbiased view at both
    //checkpoint and stages. If a map is linear yet has checkpoints,
    //it can be free to use these below to display stats for the player to compare against.


    //Note: These are by initally created as null pointers, but the constructor will by default create arrays of size MAX_STAGES
    //Note: Passing 0 as the index to any of these will return the overall stat, i.e during the entire run.

    int m_iTotalStages = MAX_STAGES;                         //required for the operator= overload

    //Keypress
    int *m_iStageJumps = nullptr,               //Amount of jumps per stage
        *m_iStageStrafes = nullptr;             //Amount of strafes per stage

    //Time
    float *m_flStageTime = nullptr,             //The amount of time (seconds) you spent to accomplish (stage) -> (stage + 1)
        *m_flStageEnterTime = nullptr;          //The time in seconds that you entered the given stage

    //Sync
    float *m_flStageStrafeSyncAvg = nullptr,    //The average sync1 you had over the given stage
        *m_flStageStrafeSync2Avg = nullptr;     //The average sync2 you had over the given stage

    //Velocity
    //Note: The secondary index is as follows: 0 = 3D Velocity (z included), 1 = Horizontal (XY) Velocity
    float **m_flStageEnterSpeed = nullptr,      //The velocity with which you started the stage (exit this stage's start trigger)
        **m_flStageVelocityMax = nullptr,       //Max velocity for a stage
        **m_flStageVelocityAvg = nullptr,       //Average velocity in a stage
        **m_flStageExitSpeed = nullptr;         //The velocity with which you exit the stage (this stage -> next)
};