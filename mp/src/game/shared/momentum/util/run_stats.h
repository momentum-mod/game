#pragma once

#include "cbase.h"
#include "filesystem.h"
#include "mom_shareddefs.h"
#include "serialization.h"

class CMomRunStats : 
	public ISerializable
{
public:
    // Note: This needs updating every time the struct is updated!
    CMomRunStats(uint8 size = MAX_STAGES)
    {
        if (size > MAX_STAGES)
            size = MAX_STAGES;

        // Set the total number of stages/checkpoints
        m_iTotalZones = size;

        // initialize everything to 0
        // Note: We do m_iTotalZones + 1 because 0 is overall!
        for (int i = 0; i < MAX_STAGES + 1; ++i)
        {
            m_iZoneJumps[i] = 0;
            m_iZoneStrafes[i] = 0;
            m_flZoneStrafeSyncAvg[i] = 0;
            m_flZoneStrafeSync2Avg[i] = 0;
            m_flZoneEnterTime[i] = 0;
            m_flZoneTime[i] = 0;

            for (int k = 0; k < 2; ++k)
            {
                m_flZoneVelocityMax[i][k] = 0;
                m_flZoneVelocityAvg[i][k] = 0;
                m_flZoneEnterSpeed[i][k] = 0;
                m_flZoneExitSpeed[i][k] = 0;
            }
        }
    }

	CMomRunStats(CBinaryReader* reader)
	{
		m_iTotalZones = reader->ReadUInt8();

		// NOTE: This range checking might result in unread data.
		if (m_iTotalZones > MAX_STAGES)
			m_iTotalZones = MAX_STAGES;

		for (int i = 0; i < m_iTotalZones + 1; ++i)
		{
			m_iZoneJumps[i] = reader->ReadUInt32();
			m_iZoneStrafes[i] = reader->ReadUInt32();

			m_flZoneStrafeSyncAvg[i] = reader->ReadFloat();
			m_flZoneStrafeSync2Avg[i] = reader->ReadFloat();
			m_flZoneEnterTime[i] = reader->ReadFloat();
			m_flZoneTime[i] = reader->ReadFloat();

			for (int k = 0; k < 2; ++k)
			{
				m_flZoneVelocityMax[i][k] = reader->ReadFloat();
				m_flZoneVelocityAvg[i][k] = reader->ReadFloat();
				m_flZoneEnterSpeed[i][k] = reader->ReadFloat();
				m_flZoneExitSpeed[i][k] = reader->ReadFloat();
			}
		}
	}

public:
	virtual void Serialize(CBinaryWriter* writer) override
	{
		writer->WriteUInt8(m_iTotalZones);

		for (int i = 0; i < m_iTotalZones + 1; ++i)
		{
			writer->WriteUInt32(m_iZoneJumps[i]);
			writer->WriteUInt32(m_iZoneStrafes[i]);

			writer->WriteFloat(m_flZoneStrafeSyncAvg[i]);
			writer->WriteFloat(m_flZoneStrafeSync2Avg[i]);
			writer->WriteFloat(m_flZoneEnterTime[i]);
			writer->WriteFloat(m_flZoneTime[i]);

			for (int k = 0; k < 2; ++k)
			{
				writer->WriteFloat(m_flZoneVelocityMax[i][k]);
				writer->WriteFloat(m_flZoneVelocityAvg[i][k]);
				writer->WriteFloat(m_flZoneEnterSpeed[i][k]);
				writer->WriteFloat(m_flZoneExitSpeed[i][k]);
			}
		}
	}

public:
    // Note: This needs updating every time the struct is updated!
    CMomRunStats &operator=(const CMomRunStats &other)
    {
		if (this == &other)
			return *this;

        m_iTotalZones = other.m_iTotalZones;

        for (int i = 0; i < MAX_STAGES + 1; ++i)
        {
            m_iZoneJumps[i] = other.m_iZoneJumps[i];
            m_iZoneStrafes[i] = other.m_iZoneStrafes[i];

            m_flZoneStrafeSyncAvg[i] = other.m_flZoneStrafeSyncAvg[i];
            m_flZoneStrafeSync2Avg[i] = other.m_flZoneStrafeSync2Avg[i];
            m_flZoneEnterTime[i] = other.m_flZoneEnterTime[i];
            m_flZoneTime[i] = other.m_flZoneTime[i];

            for (int k = 0; k < 2; ++k)
            {
                m_flZoneVelocityMax[i][k] = other.m_flZoneVelocityMax[i][k];
                m_flZoneVelocityAvg[i][k] = other.m_flZoneVelocityAvg[i][k];
                m_flZoneEnterSpeed[i][k] = other.m_flZoneEnterSpeed[i][k];
                m_flZoneExitSpeed[i][k] = other.m_flZoneExitSpeed[i][k];
            }
        }

        return *this;
    }

public:
	// All these are virtual so they can be overridden in future versions.
	virtual uint8 GetTotalZones() { return m_iTotalZones; }
	virtual uint32 GetZoneJumps(int zone) { return zone > m_iTotalZones ? 0 : m_iZoneJumps[zone]; }
	virtual uint32 GetZoneStrafes(int zone) { return zone > m_iTotalZones ? 0 : m_iZoneStrafes[zone]; }
	virtual float GetZoneTime(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneTime[zone]; }
	virtual float GetZoneEnterTime(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneEnterTime[zone]; }
	virtual float GetZoneStrafeSyncAvg(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneStrafeSyncAvg[zone]; }
	virtual float GetZoneStrafeSync2Avg(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneStrafeSync2Avg[zone]; }
	virtual float* GetZoneEnterSpeed(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneEnterSpeed[zone]; }
	virtual float* GetZoneVelocityMax(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneVelocityMax[zone]; }
	virtual float* GetZoneVelocityAvg(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneVelocityAvg[zone]; }
	virtual float* GetZoneExitSpeed(int zone) { return zone > m_iTotalZones ? 0 : m_flZoneExitSpeed[zone]; }

	virtual void SetTotalZones(uint8 zones) { m_iTotalZones = zones > MAX_STAGES ? MAX_STAGES : zones; }
	virtual void SetZoneJumps(int zone, uint32 value) { if (zone > m_iTotalZones) return; m_iZoneJumps[zone] = value; }
	virtual void SetZoneStrafes(int zone, uint32 value) { if (zone > m_iTotalZones) return; m_iZoneStrafes[zone] = value; }
	virtual void SetZoneTime(int zone, float value) { if (zone > m_iTotalZones) return; m_flZoneTime[zone] = value; }
	virtual void SetZoneEnterTime(int zone, float value) { if (zone > m_iTotalZones) return; m_flZoneEnterTime[zone] = value; }
	virtual void SetZoneStrafeSyncAvg(int zone, float value) { if (zone > m_iTotalZones) return; m_flZoneStrafeSyncAvg[zone] = value; }
	virtual void SetZoneStrafeSync2Avg(int zone, float value) { if (zone > m_iTotalZones) return; m_flZoneStrafeSync2Avg[zone] = value; }
	virtual void SetZoneEnterSpeed(int zone, float vert, float hor) { if (zone > m_iTotalZones) return; m_flZoneEnterSpeed[zone][0] = vert; m_flZoneEnterSpeed[zone][1] = hor; }
	virtual void SetZoneVelocityMax(int zone, float vert, float hor) { if (zone > m_iTotalZones) return; m_flZoneVelocityMax[zone][0] = vert; m_flZoneVelocityMax[zone][1] = hor; }
	virtual void SetZoneVelocityAvg(int zone, float vert, float hor) { if (zone > m_iTotalZones) return; m_flZoneVelocityAvg[zone][0] = vert; m_flZoneVelocityAvg[zone][1] = hor; }
	virtual void SetZoneExitSpeed(int zone, float vert, float hor) { if (zone > m_iTotalZones) return; m_flZoneExitSpeed[zone][0] = vert; m_flZoneExitSpeed[zone][1] = hor; }

private:
    // Note: Passing 0 as the index to any of these will return the overall stat, i.e during the entire run.
    uint8 m_iTotalZones; // Required for the operator= overload

    // Keypress
    uint32 m_iZoneJumps[MAX_STAGES + 1],   // Amount of jumps per stage/checkpoint
        m_iZoneStrafes[MAX_STAGES + 1]; // Amount of strafes per stage/checkpoint

    // Time
    float m_flZoneTime[MAX_STAGES + 1], // The amount of time (seconds) you spent to accomplish (stage) -> (stage + 1)
        m_flZoneEnterTime[MAX_STAGES + 1]; // The time in seconds that you entered the given stage/checkpoint

    // Sync
    float m_flZoneStrafeSyncAvg[MAX_STAGES + 1], // The average sync1 you had over the given stage/checkpoint
        m_flZoneStrafeSync2Avg[MAX_STAGES + 1];  // The average sync2 you had over the given stage/checkpoint

    // Velocity
    // Note: The secondary index is as follows: 0 = 3D Velocity (z included), 1 = Horizontal (XY) Velocity
    float m_flZoneEnterSpeed[MAX_STAGES + 1][2],// The velocity with which you started the stage (exit this stage's start trigger)
        m_flZoneVelocityMax[MAX_STAGES + 1][2], // Max velocity for a stage/checkpoint
        m_flZoneVelocityAvg[MAX_STAGES + 1][2], // Average velocity in a stage/checkpoint
        m_flZoneExitSpeed[MAX_STAGES + 1][2];   // The velocity with which you exit the stage (this stage -> next)
};