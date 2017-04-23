#pragma once

#include "cbase.h"
#include "filesystem.h"
#include "../mom_shareddefs.h"
#include "util/serialization.h"

#ifdef CLIENT_DLL
#define CMomRunStats C_MomRunStats
EXTERN_RECV_TABLE(DT_MOM_RunStats);
#else
EXTERN_SEND_TABLE(DT_MOM_RunStats);
#endif

class CMomRunStats : public ISerializable
{
    DECLARE_CLASS_NOBASE(CMomRunStats);
    //DECLARE_EMBEDDED_NETWORKVAR();
    
  public:
    CMomRunStats(uint8 size = MAX_STAGES) : m_pData(&m_data) { Init(size); }
    CMomRunStats(CBinaryReader *pReader) : m_pData(&m_data) { Deserialize(pReader); }

    // Note: This needs updating every time the struct is updated!
    virtual void Init(uint8 size = MAX_STAGES)
    {
        if (size > MAX_STAGES)
            size = MAX_STAGES;

        // Set the total number of stages/checkpoints
        SetTotalZones(size);

        // initialize everything to 0
        // Note: We do m_iTotalZones + 1 because 0 is overall!
        for (int i = 0; i < MAX_STAGES + 1; ++i)
        {
            SetZoneJumps(i, 0);
            SetZoneStrafes(i, 0);
            SetZoneStrafeSyncAvg(i, 0);
            SetZoneStrafeSync2Avg(i, 0);
            SetZoneEnterTime(i, 0.0f);
            SetZoneTime(i, 0.0f);

            SetZoneEnterSpeed(i, 0.0f, 0.0f);
            SetZoneVelocityMax(i, 0.0f, 0.0f);
            SetZoneVelocityAvg(i, 0.0f, 0.0f);
            SetZoneExitSpeed(i, 0.0f, 0.0f);
        }
    }

    // Note: This needs updating every time the struct is updated!
    virtual void Deserialize(CBinaryReader *reader)
    {
        SetTotalZones(reader->ReadUInt8());

        // NOTE: This range checking might result in unread data.
        if (m_data.m_iTotalZones > MAX_STAGES)
            SetTotalZones(MAX_STAGES);

        for (int i = 0; i < m_data.m_iTotalZones + 1; ++i)
        {
            SetZoneJumps(i, reader->ReadUInt32());
            SetZoneStrafes(i, reader->ReadUInt32());

            SetZoneStrafeSyncAvg(i, reader->ReadFloat());
            SetZoneStrafeSync2Avg(i, reader->ReadFloat());
            SetZoneEnterTime(i, reader->ReadFloat());
            SetZoneTime(i, reader->ReadFloat());

            float vel3D = 0.0f, vel2D = 0.0f;
            vel3D = reader->ReadFloat();
            vel2D = reader->ReadFloat();
            SetZoneVelocityMax(i, vel3D, vel2D);
            vel3D = reader->ReadFloat();
            vel2D = reader->ReadFloat();
            SetZoneVelocityAvg(i, vel3D, vel2D);
            vel3D = reader->ReadFloat();
            vel2D = reader->ReadFloat();
            SetZoneEnterSpeed(i, vel3D, vel2D);
            vel3D = reader->ReadFloat();
            vel2D = reader->ReadFloat();
            SetZoneExitSpeed(i, vel3D, vel2D);
        }
    }

    // Note: This needs updating every time the struct is updated!
    void Serialize(CBinaryWriter *writer) OVERRIDE
    {
        writer->WriteUInt8(m_data.m_iTotalZones);

        for (int i = 0; i < m_data.m_iTotalZones + 1; ++i)
        {
            //Jumps/Strafes
            writer->WriteUInt32(m_data.m_iZoneJumps[i]);
            writer->WriteUInt32(m_data.m_iZoneStrafes[i]);
            //Sync
            writer->WriteFloat(m_data.m_flZoneStrafeSyncAvg[i]);
            writer->WriteFloat(m_data.m_flZoneStrafeSync2Avg[i]);
            //Time
            writer->WriteFloat(m_data.m_flZoneEnterTime[i]);
            writer->WriteFloat(m_data.m_flZoneTime[i]);
            //Velocity
            writer->WriteFloat(m_data.m_flZoneVelocityMax3D[i]);
            writer->WriteFloat(m_data.m_flZoneVelocityMax2D[i]);
            writer->WriteFloat(m_data.m_flZoneVelocityAvg3D[i]);
            writer->WriteFloat(m_data.m_flZoneVelocityAvg2D[i]);
            writer->WriteFloat(m_data.m_flZoneEnterSpeed3D[i]);
            writer->WriteFloat(m_data.m_flZoneEnterSpeed2D[i]);
            writer->WriteFloat(m_data.m_flZoneExitSpeed3D[i]);
            writer->WriteFloat(m_data.m_flZoneExitSpeed2D[i]);
        }
    }

    // Note: This needs updating every time the struct is updated!
    CMomRunStats &operator=(const CMomRunStats &other)
    {
        if (this == &other)
            return *this;
        
        if (other.m_pData == nullptr)
            return *this;

        SetTotalZones(other.m_pData->m_iTotalZones);

        for (int i = 0; i < MAX_STAGES + 1; ++i)
        {
            SetZoneJumps(i, other.m_pData->m_iZoneJumps[i]);
            SetZoneStrafes(i, other.m_pData->m_iZoneStrafes[i]);

            SetZoneStrafeSyncAvg(i, other.m_pData->m_flZoneStrafeSyncAvg[i]);
            SetZoneStrafeSync2Avg(i, other.m_pData->m_flZoneStrafeSync2Avg[i]);

            SetZoneEnterTime(i, other.m_pData->m_flZoneEnterTime[i]);
            SetZoneTime(i, other.m_pData->m_flZoneTime[i]);

            SetZoneVelocityMax(i, other.m_pData->m_flZoneVelocityMax3D[i], other.m_pData->m_flZoneVelocityMax2D[i]);
            SetZoneVelocityAvg(i, other.m_pData->m_flZoneVelocityAvg3D[i], other.m_pData->m_flZoneVelocityAvg2D[i]);
            SetZoneEnterSpeed(i, other.m_pData->m_flZoneEnterSpeed3D[i], other.m_pData->m_flZoneEnterSpeed2D[i]);
            SetZoneExitSpeed(i, other.m_pData->m_flZoneExitSpeed3D[i], other.m_pData->m_flZoneExitSpeed2D[i]);
        }

        return *this;
    }

  public:
    // All these are virtual so they can be overridden in future versions.
    virtual uint8 GetTotalZones() { return m_data.m_iTotalZones; }
    virtual uint32 GetZoneJumps(int zone) { return zone > m_data.m_iTotalZones ? 0 : m_data.m_iZoneJumps[zone]; }
    virtual uint32 GetZoneStrafes(int zone) { return zone > m_data.m_iTotalZones ? 0 : m_data.m_iZoneStrafes[zone]; }
    virtual float GetZoneTime(int zone) { return zone > m_data.m_iTotalZones ? 0 : m_data.m_flZoneTime[zone]; }
    virtual float GetZoneEnterTime(int zone) { return zone > m_data.m_iTotalZones ? 0 : m_data.m_flZoneEnterTime[zone]; }
    virtual float GetZoneStrafeSyncAvg(int zone) { return zone > m_data.m_iTotalZones ? 0 : m_data.m_flZoneStrafeSyncAvg[zone]; }
    virtual float GetZoneStrafeSync2Avg(int zone) { return zone > m_data.m_iTotalZones ? 0 : m_data.m_flZoneStrafeSync2Avg[zone]; }
    virtual float GetZoneEnterSpeed(int zone, bool vel2D)
    {
        return zone > m_data.m_iTotalZones ? 0.0f : (vel2D ? m_data.m_flZoneEnterSpeed2D[zone] : m_data.m_flZoneEnterSpeed3D[zone]);
    }
    virtual float GetZoneExitSpeed(int zone, bool vel2D)
    {
        return zone > m_data.m_iTotalZones ? 0.0f : (vel2D ? m_data.m_flZoneExitSpeed2D[zone] : m_data.m_flZoneExitSpeed3D[zone]);
    }
    virtual float GetZoneVelocityMax(int zone, bool vel2D)
    {
        return zone > m_data.m_iTotalZones ? 0.0f : (vel2D ? m_data.m_flZoneVelocityMax2D[zone] : m_data.m_flZoneVelocityMax3D[zone]);
    }
    virtual float GetZoneVelocityAvg(int zone, bool vel2D)
    {
        return zone > m_data.m_iTotalZones ? 0.0f : (vel2D ? m_data.m_flZoneVelocityAvg2D[zone] : m_data.m_flZoneVelocityAvg3D[zone]);
    }

    virtual void SetTotalZones(uint8 zones) { m_data.m_iTotalZones = zones > MAX_STAGES ? MAX_STAGES : zones; }
    virtual void SetZoneJumps(int zone, uint32 value)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_iZoneJumps[zone] = value;
    }
    virtual void SetZoneStrafes(int zone, uint32 value)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_iZoneStrafes[zone] = value;
    }
    virtual void SetZoneTime(int zone, float value)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneTime[zone] = value;
    }
    virtual void SetZoneEnterTime(int zone, float value)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneEnterTime[zone] = value;
    }
    virtual void SetZoneStrafeSyncAvg(int zone, float value)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneStrafeSyncAvg[zone] = value;
    }
    virtual void SetZoneStrafeSync2Avg(int zone, float value)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneStrafeSync2Avg[zone] = value;
    }
    virtual void SetZoneEnterSpeed(int zone, float vert, float hor)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneEnterSpeed3D[zone] = vert;
        m_data.m_flZoneEnterSpeed2D[zone] = hor;
    }
    virtual void SetZoneVelocityMax(int zone, float vert, float hor)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneVelocityMax3D[zone] = vert;
        m_data.m_flZoneVelocityMax2D[zone] = hor;
    }
    virtual void SetZoneVelocityAvg(int zone, float vert, float hor)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneVelocityAvg3D[zone] = vert;
        m_data.m_flZoneVelocityAvg2D[zone] = hor;
    }
    virtual void SetZoneExitSpeed(int zone, float vert, float hor)
    {
        if (zone > m_data.m_iTotalZones)
            return;

        m_data.m_flZoneExitSpeed3D[zone] = vert;
        m_data.m_flZoneExitSpeed2D[zone] = hor;
    }

    /*
     * We encapsulate the raw data in its own struct to allow a memcpy of just the data
     * rather than a memcpy of CMomRunStats which would also copy pointers to virtuals
     */ 
    struct data 
    {
        // Note: Passing 0 as the index to any of these will return the overall stat, i.e during the entire run.
        
        uint8 m_iTotalZones; // Required for the operator= overload

        // Keypress
        uint32 m_iZoneJumps[MAX_STAGES + 1], // Amount of jumps per stage/checkpoint
            m_iZoneStrafes[MAX_STAGES + 1];  // Amount of strafes per stage/checkpoint

        // Time
        float m_flZoneTime[MAX_STAGES + 1], // The amount of time (seconds) you spent to accomplish (stage) -> (stage + 1)
            m_flZoneEnterTime[MAX_STAGES + 1]; // The time in seconds that you entered the given stage/checkpoint

        // Sync
        float m_flZoneStrafeSyncAvg[MAX_STAGES + 1], // The average sync1 you had over the given stage/checkpoint
            m_flZoneStrafeSync2Avg[MAX_STAGES + 1];  // The average sync2 you had over the given stage/checkpoint

        // Velocity
        float m_flZoneEnterSpeed3D[MAX_STAGES + 1], m_flZoneEnterSpeed2D[MAX_STAGES + 1],// The 3D velocity with which you started the stage (exit this stage's start trigger)
            m_flZoneVelocityMax3D[MAX_STAGES + 1], m_flZoneVelocityMax2D[MAX_STAGES + 1],// Max velocity for a stage/checkpoint
            m_flZoneVelocityAvg3D[MAX_STAGES + 1], m_flZoneVelocityAvg2D[MAX_STAGES + 1],// Average velocity in a stage/checkpoint
            m_flZoneExitSpeed3D[MAX_STAGES + 1], m_flZoneExitSpeed2D[MAX_STAGES + 1];// The velocity with which you exit the stage (this stage -> next)
    };
    
    //A public pointer to the data kind of undermines the encapsulation, but it is necessary
    CMomRunStats::data *m_pData;
    
private:
    CMomRunStats::data m_data;
};


