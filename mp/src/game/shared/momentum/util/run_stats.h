#pragma once

#include "cbase.h"
#include "filesystem.h"
#include "mom_shareddefs.h"

struct RunStats_t
{
    RunStats_t(int size = MAX_STAGES)
    {
        // Set the total number of stages/checkpoints
        m_iTotalZones = size;

        // initialize everything to 0
        for (int i = 0; i < m_iTotalZones; i++)
        {
            m_iZoneJumps[i] = 0;
            m_iZoneStrafes[i] = 0;
            m_flZoneStrafeSyncAvg[i] = 0;
            m_flZoneStrafeSync2Avg[i] = 0;
            m_flZoneEnterTime[i] = 0;
            m_flZoneTime[i] = 0;
            for (int k = 0; k < 2; k++)
            {
                m_flZoneVelocityMax[i][k] = 0;
                m_flZoneVelocityAvg[i][k] = 0;
                m_flZoneEnterSpeed[i][k] = 0;
                m_flZoneExitSpeed[i][k] = 0;
            }
        }
    }

    //Note: This needs updating every time the struct is updated!
    void Read(FileHandle_t file)
    {
        filesystem->Read(&m_iTotalZones, sizeof(m_iTotalZones), file);
        for (int i = 0; i < m_iTotalZones; i++)
        {
            filesystem->Read(&m_iZoneJumps[i], sizeof(m_iZoneJumps[i]), file);
            filesystem->Read(&m_iZoneStrafes[i], sizeof(m_iZoneStrafes[i]), file);
            filesystem->Read(&m_flZoneStrafeSyncAvg[i], sizeof(m_flZoneStrafeSyncAvg[i]), file);

            filesystem->Read(&m_flZoneStrafeSyncAvg[i], sizeof(m_flZoneStrafeSyncAvg[i]), file);
            filesystem->Read(&m_flZoneStrafeSync2Avg[i], sizeof(m_flZoneStrafeSync2Avg[i]), file);
            filesystem->Read(&m_flZoneEnterTime[i], sizeof(m_flZoneEnterTime[i]), file);
            filesystem->Read(&m_flZoneTime[i], sizeof(m_flZoneTime[i]), file);

            for (int k = 0; k < 2; k++)
            {
                filesystem->Read(&m_flZoneVelocityMax[i][k], sizeof(m_flZoneVelocityMax[i][k]), file);
                filesystem->Read(&m_flZoneVelocityAvg[i][k], sizeof(m_flZoneVelocityAvg[i][k]), file);
                filesystem->Read(&m_flZoneEnterSpeed[i][k], sizeof(m_flZoneEnterSpeed[i][k]), file);
                filesystem->Read(&m_flZoneExitSpeed[i][k], sizeof(m_flZoneExitSpeed[i][k]), file);
            }
        }
    }

    //Note: This needs updating every time the struct is updated!
    void Write(FileHandle_t file) const
    {
        filesystem->Write(&m_iTotalZones, sizeof(m_iTotalZones), file);
        for (int i = 0; i < m_iTotalZones; i++)
        {
            filesystem->Write(&m_iZoneJumps[i], sizeof(m_iZoneJumps[i]), file);
            filesystem->Write(&m_iZoneStrafes[i], sizeof(m_iZoneStrafes[i]), file);
            filesystem->Write(&m_flZoneStrafeSyncAvg[i], sizeof(m_flZoneStrafeSyncAvg[i]), file);

            filesystem->Write(&m_flZoneStrafeSyncAvg[i], sizeof(m_flZoneStrafeSyncAvg[i]), file);
            filesystem->Write(&m_flZoneStrafeSync2Avg[i], sizeof(m_flZoneStrafeSync2Avg[i]), file);
            filesystem->Write(&m_flZoneEnterTime[i], sizeof(m_flZoneEnterTime[i]), file);
            filesystem->Write(&m_flZoneTime[i], sizeof(m_flZoneTime[i]), file);

            for (int k = 0; k < 2; k++)
            {
                filesystem->Write(&m_flZoneVelocityMax[i][k], sizeof(m_flZoneVelocityMax[i][k]), file);
                filesystem->Write(&m_flZoneVelocityAvg[i][k], sizeof(m_flZoneVelocityAvg[i][k]), file);
                filesystem->Write(&m_flZoneEnterSpeed[i][k], sizeof(m_flZoneEnterSpeed[i][k]), file);
                filesystem->Write(&m_flZoneExitSpeed[i][k], sizeof(m_flZoneExitSpeed[i][k]), file);
            }
        }
    }

    RunStats_t &operator=(const RunStats_t &other)
    {
        m_iTotalZones = other.m_iTotalZones;
        for (int i = 0; i < other.m_iTotalZones; i++)
        {
            m_iZoneJumps[i] = other.m_iZoneJumps[i];
            m_iZoneStrafes[i] = other.m_iZoneStrafes[i];

            m_flZoneStrafeSyncAvg[i] = other.m_flZoneStrafeSyncAvg[i];
            m_flZoneStrafeSync2Avg[i] = other.m_flZoneStrafeSync2Avg[i];
            m_flZoneEnterTime[i] = other.m_flZoneEnterTime[i];
            m_flZoneTime[i] = other.m_flZoneTime[i];

            for (int k = 0; k < 2; k++)
            {
                m_flZoneVelocityMax[i][k] = other.m_flZoneVelocityMax[i][k];
                m_flZoneVelocityAvg[i][k] = other.m_flZoneVelocityAvg[i][k];
                m_flZoneEnterSpeed[i][k] = other.m_flZoneEnterSpeed[i][k];
                m_flZoneExitSpeed[i][k] = other.m_flZoneExitSpeed[i][k];
            }
        }
        return *this;
    }

    // Note: Passing 0 as the index to any of these will return the overall stat, i.e during the entire run.
    int m_iTotalZones; //Required for the operator= overload

    // Keypress
    int m_iZoneJumps[MAX_STAGES],    // Amount of jumps per stage/checkpoint
        m_iZoneStrafes[MAX_STAGES]; // Amount of strafes per stage/checkpoint

    // Time
    float m_flZoneTime[MAX_STAGES],    // The amount of time (seconds) you spent to accomplish (stage) -> (stage + 1)
        m_flZoneEnterTime[MAX_STAGES]; // The time in seconds that you entered the given stage/checkpoint

    // Sync
    float m_flZoneStrafeSyncAvg[MAX_STAGES], // The average sync1 you had over the given stage/checkpoint
        m_flZoneStrafeSync2Avg[MAX_STAGES];  // The average sync2 you had over the given stage/checkpoint

    // Velocity
    // Note: The secondary index is as follows: 0 = 3D Velocity (z included), 1 = Horizontal (XY) Velocity
    float m_flZoneEnterSpeed[MAX_STAGES][2], // The velocity with which you started the stage (exit this stage's start trigger)
        m_flZoneVelocityMax[MAX_STAGES][2], // Max velocity for a stage/checkpoint
        m_flZoneVelocityAvg[MAX_STAGES][2], // Average velocity in a stage/checkpoint
        m_flZoneExitSpeed[MAX_STAGES][2];   // The velocity with which you exit the stage (this stage -> next)
};