#include "cbase.h"
#include "run_stats.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

CMomRunStats::CMomRunStats(CMomRunStats::data* pData)
{
    m_pData = pData;
    Init(0);
}

CMomRunStats::CMomRunStats(CMomRunStats::data* pData, uint8 size)
{
    m_pData = pData;
    Init(size);
}

CMomRunStats::CMomRunStats(CMomRunStats::data* pData, CBinaryReader *pReader) : m_pData(nullptr) 
{
    m_pData = pData;
    Deserialize(pReader);
}

void CMomRunStats::Init(uint8 size)
{
    if (size > MAX_STAGES)
        size = MAX_STAGES;

    memset(m_pData, 0, sizeof(CMomRunStats::data));
    
    SetTotalZones(size);
}

CMomRunStats &CMomRunStats::operator=(const CMomRunStats &other)
{
    this->m_pData = other.m_pData;
    return *this;
}

void CMomRunStats::FullyCopyStats(CMomRunStats *to)
{
    memcpy(to->m_pData, m_pData, sizeof(CMomRunStats::data));
}

void CMomRunStats::FullyCopyStats(CMomRunStats::data *to)
{
    memcpy(to, m_pData, sizeof(CMomRunStats::data));
}

void CMomRunStats::Deserialize(CBinaryReader *reader)
{
    SetTotalZones(reader->ReadUInt8());

    // NOTE: This range checking might result in unread data.
    if (m_pData->m_iTotalZones > MAX_STAGES)
        SetTotalZones(MAX_STAGES);

    for (int i = 0; i < m_pData->m_iTotalZones + 1; ++i)
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

void CMomRunStats::Serialize(CBinaryWriter *writer) 
{
    writer->WriteUInt8(m_pData->m_iTotalZones);

    for (int i = 0; i < m_pData->m_iTotalZones + 1; ++i)
    {
        //Jumps/Strafes
        writer->WriteUInt32(m_pData->m_iZoneJumps[i]);
        writer->WriteUInt32(m_pData->m_iZoneStrafes[i]);
        //Sync
        writer->WriteFloat(m_pData->m_flZoneStrafeSyncAvg[i]);
        writer->WriteFloat(m_pData->m_flZoneStrafeSync2Avg[i]);
        //Time
        writer->WriteFloat(m_pData->m_flZoneEnterTime[i]);
        writer->WriteFloat(m_pData->m_flZoneTime[i]);
        //Velocity
        writer->WriteFloat(m_pData->m_flZoneVelocityMax3D[i]);
        writer->WriteFloat(m_pData->m_flZoneVelocityMax2D[i]);
        writer->WriteFloat(m_pData->m_flZoneVelocityAvg3D[i]);
        writer->WriteFloat(m_pData->m_flZoneVelocityAvg2D[i]);
        writer->WriteFloat(m_pData->m_flZoneEnterSpeed3D[i]);
        writer->WriteFloat(m_pData->m_flZoneEnterSpeed2D[i]);
        writer->WriteFloat(m_pData->m_flZoneExitSpeed3D[i]);
        writer->WriteFloat(m_pData->m_flZoneExitSpeed2D[i]);
    }
}

uint8 CMomRunStats::GetTotalZones() { return m_pData->m_iTotalZones; }
uint32 CMomRunStats::GetZoneJumps(int zone) { return zone > m_pData->m_iTotalZones ? 0 : m_pData->m_iZoneJumps[zone]; }
uint32 CMomRunStats::GetZoneStrafes(int zone) { return zone > m_pData->m_iTotalZones ? 0 : m_pData->m_iZoneStrafes[zone]; }
float CMomRunStats::GetZoneTime(int zone) { return zone > m_pData->m_iTotalZones ? 0 : m_pData->m_flZoneTime[zone]; }
float CMomRunStats::GetZoneEnterTime(int zone) { return zone > m_pData->m_iTotalZones ? 0 : m_pData->m_flZoneEnterTime[zone]; }
float CMomRunStats::GetZoneStrafeSyncAvg(int zone) { return zone > m_pData->m_iTotalZones ? 0 : m_pData->m_flZoneStrafeSyncAvg[zone]; }
float CMomRunStats::GetZoneStrafeSync2Avg(int zone) { return zone > m_pData->m_iTotalZones ? 0 : m_pData->m_flZoneStrafeSync2Avg[zone]; }
float CMomRunStats::GetZoneEnterSpeed(int zone, bool vel2D)
{
    return zone > m_pData->m_iTotalZones ? 0.0f : (vel2D ? m_pData->m_flZoneEnterSpeed2D[zone] : m_pData->m_flZoneEnterSpeed3D[zone]);
}
float CMomRunStats::GetZoneExitSpeed(int zone, bool vel2D)
{
    return zone > m_pData->m_iTotalZones ? 0.0f : (vel2D ? m_pData->m_flZoneExitSpeed2D[zone] : m_pData->m_flZoneExitSpeed3D[zone]);
}
float CMomRunStats::GetZoneVelocityMax(int zone, bool vel2D)
{
    return zone > m_pData->m_iTotalZones ? 0.0f : (vel2D ? m_pData->m_flZoneVelocityMax2D[zone] : m_pData->m_flZoneVelocityMax3D[zone]);
}
float CMomRunStats::GetZoneVelocityAvg(int zone, bool vel2D)
{
    return zone > m_pData->m_iTotalZones ? 0.0f : (vel2D ? m_pData->m_flZoneVelocityAvg2D[zone] : m_pData->m_flZoneVelocityAvg3D[zone]);
}

void CMomRunStats::SetTotalZones(uint8 zones) { m_pData->m_iTotalZones = zones > MAX_STAGES ? MAX_STAGES : zones; }
void CMomRunStats::SetZoneJumps(int zone, uint32 value)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_iZoneJumps[zone] = value;
}
void CMomRunStats::SetZoneStrafes(int zone, uint32 value)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_iZoneStrafes[zone] = value;
}
void CMomRunStats::SetZoneTime(int zone, float value)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneTime[zone] = value;
}
void CMomRunStats::SetZoneEnterTime(int zone, float value)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneEnterTime[zone] = value;
}
void CMomRunStats::SetZoneStrafeSyncAvg(int zone, float value)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneStrafeSyncAvg[zone] = value;
}
void CMomRunStats::SetZoneStrafeSync2Avg(int zone, float value)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneStrafeSync2Avg[zone] = value;
}
void CMomRunStats::SetZoneEnterSpeed(int zone, float vert, float hor)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneEnterSpeed3D[zone] = vert;
    m_pData->m_flZoneEnterSpeed2D[zone] = hor;
}
void CMomRunStats::SetZoneVelocityMax(int zone, float vert, float hor)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneVelocityMax3D[zone] = vert;
    m_pData->m_flZoneVelocityMax2D[zone] = hor;
}
void CMomRunStats::SetZoneVelocityAvg(int zone, float vert, float hor)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneVelocityAvg3D[zone] = vert;
    m_pData->m_flZoneVelocityAvg2D[zone] = hor;
}
void CMomRunStats::SetZoneExitSpeed(int zone, float vert, float hor)
{
    if (zone > m_pData->m_iTotalZones)
        return;

    m_pData->m_flZoneExitSpeed3D[zone] = vert;
    m_pData->m_flZoneExitSpeed2D[zone] = hor;
}