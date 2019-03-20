#include "cbase.h"
#include "run_stats.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
BEGIN_RECV_TABLE_NOBASE(C_MomRunStats, DT_MomRunStats)
RecvPropInt(RECVINFO(m_iTotalZones), SPROP_UNSIGNED),
//Keypress
RecvPropArray3(RECVINFO_ARRAY(m_iZoneJumps), RecvPropInt(RECVINFO(m_iZoneJumps[0]), SPROP_UNSIGNED | SPROP_CHANGES_OFTEN)),
RecvPropArray3(RECVINFO_ARRAY(m_iZoneStrafes), RecvPropInt(RECVINFO(m_iZoneStrafes[0]), SPROP_UNSIGNED | SPROP_CHANGES_OFTEN)),
//Sync
RecvPropArray3(RECVINFO_ARRAY(m_flZoneStrafeSyncAvg), RecvPropFloat(RECVINFO(m_flZoneStrafeSyncAvg[0]), SPROP_CHANGES_OFTEN)),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneStrafeSync2Avg), RecvPropFloat(RECVINFO(m_flZoneStrafeSync2Avg[0]), SPROP_CHANGES_OFTEN)),
//Time
RecvPropArray3(RECVINFO_ARRAY(m_flZoneEnterTime), RecvPropFloat(RECVINFO(m_flZoneEnterTime[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneTime), RecvPropFloat(RECVINFO(m_flZoneTime[0]))),
//Velocity
RecvPropArray3(RECVINFO_ARRAY(m_flZoneEnterSpeed3D), RecvPropFloat(RECVINFO(m_flZoneEnterSpeed3D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneEnterSpeed2D), RecvPropFloat(RECVINFO(m_flZoneEnterSpeed2D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneExitSpeed3D), RecvPropFloat(RECVINFO(m_flZoneExitSpeed3D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneExitSpeed2D), RecvPropFloat(RECVINFO(m_flZoneExitSpeed2D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneVelocityAvg3D), RecvPropFloat(RECVINFO(m_flZoneVelocityAvg3D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneVelocityAvg2D), RecvPropFloat(RECVINFO(m_flZoneVelocityAvg2D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneVelocityMax3D), RecvPropFloat(RECVINFO(m_flZoneVelocityMax3D[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_flZoneVelocityMax2D), RecvPropFloat(RECVINFO(m_flZoneVelocityMax2D[0]))),
END_RECV_TABLE();
#else
BEGIN_SEND_TABLE_NOBASE(CMomRunStats, DT_MomRunStats)
SendPropInt(SENDINFO(m_iTotalZones), 7, SPROP_UNSIGNED),
//Keypress
SendPropArray3(SENDINFO_ARRAY3(m_iZoneJumps), SendPropInt(SENDINFO_ARRAY(m_iZoneJumps), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN)),
SendPropArray3(SENDINFO_ARRAY3(m_iZoneStrafes), SendPropInt(SENDINFO_ARRAY(m_iZoneStrafes), -1, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN)),
//Sync
SendPropArray3(SENDINFO_ARRAY3(m_flZoneStrafeSyncAvg), SendPropFloat(SENDINFO_ARRAY(m_flZoneStrafeSyncAvg), -1, SPROP_CHANGES_OFTEN)),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneStrafeSync2Avg), SendPropFloat(SENDINFO_ARRAY(m_flZoneStrafeSync2Avg), -1, SPROP_CHANGES_OFTEN)),
//Time
SendPropArray3(SENDINFO_ARRAY3(m_flZoneEnterTime), SendPropFloat(SENDINFO_ARRAY(m_flZoneEnterTime), -1, SPROP_CHANGES_OFTEN)),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneTime), SendPropFloat(SENDINFO_ARRAY(m_flZoneTime), -1, SPROP_CHANGES_OFTEN)),
//Velocity
SendPropArray3(SENDINFO_ARRAY3(m_flZoneEnterSpeed3D), SendPropFloat(SENDINFO_ARRAY(m_flZoneEnterSpeed3D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneEnterSpeed2D), SendPropFloat(SENDINFO_ARRAY(m_flZoneEnterSpeed2D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneExitSpeed3D), SendPropFloat(SENDINFO_ARRAY(m_flZoneExitSpeed3D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneExitSpeed2D), SendPropFloat(SENDINFO_ARRAY(m_flZoneExitSpeed2D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneVelocityAvg3D), SendPropFloat(SENDINFO_ARRAY(m_flZoneVelocityAvg3D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneVelocityAvg2D), SendPropFloat(SENDINFO_ARRAY(m_flZoneVelocityAvg2D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneVelocityMax3D), SendPropFloat(SENDINFO_ARRAY(m_flZoneVelocityMax3D))),
SendPropArray3(SENDINFO_ARRAY3(m_flZoneVelocityMax2D), SendPropFloat(SENDINFO_ARRAY(m_flZoneVelocityMax2D))),
END_SEND_TABLE();
#endif

CMomRunStats::CMomRunStats(uint8 size /* = MAX_STAGES*/)
{
    Init(size);
}

CMomRunStats::CMomRunStats(CUtlBuffer &reader)
{
    Deserialize(reader);
}

void CMomRunStats::Init(uint8 size /* = MAX_STAGES*/)
{
    size = clamp<uint8>(size, 0, MAX_STAGES);

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

void CMomRunStats::FullyCopyFrom(const CMomRunStats &other)
{
    SetTotalZones(other.m_iTotalZones);

    for (auto i = 0; i < MAX_STAGES + 1; ++i)
    {
        SetZoneJumps(i, other.m_iZoneJumps[i]);
        SetZoneStrafes(i, other.m_iZoneStrafes[i]);

        SetZoneStrafeSyncAvg(i, other.m_flZoneStrafeSyncAvg[i]);
        SetZoneStrafeSync2Avg(i, other.m_flZoneStrafeSync2Avg[i]);

        SetZoneEnterTime(i, other.m_flZoneEnterTime[i]);
        SetZoneTime(i, other.m_flZoneTime[i]);

        SetZoneVelocityMax(i, other.m_flZoneVelocityMax3D[i], other.m_flZoneVelocityMax2D[i]);
        SetZoneVelocityAvg(i, other.m_flZoneVelocityAvg3D[i], other.m_flZoneVelocityAvg2D[i]);
        SetZoneEnterSpeed(i, other.m_flZoneEnterSpeed3D[i], other.m_flZoneEnterSpeed2D[i]);
        SetZoneExitSpeed(i, other.m_flZoneExitSpeed3D[i], other.m_flZoneExitSpeed2D[i]);
    }
}

void CMomRunStats::Deserialize(CUtlBuffer &reader)
{
    SetTotalZones(reader.GetUnsignedChar());

    // NOTE: This range checking might result in unread data.
    if (m_iTotalZones > MAX_STAGES)
        SetTotalZones(MAX_STAGES);

    for (int i = 0; i < m_iTotalZones + 1; ++i)
    {
        SetZoneJumps(i, reader.GetUnsignedInt());
        SetZoneStrafes(i, reader.GetUnsignedInt());

        SetZoneStrafeSyncAvg(i, reader.GetFloat());
        SetZoneStrafeSync2Avg(i, reader.GetFloat());
        SetZoneEnterTime(i, reader.GetFloat());
        SetZoneTime(i, reader.GetFloat());

        float vel3D = 0.0f, vel2D = 0.0f;
        vel3D = reader.GetFloat();
        vel2D = reader.GetFloat();
        SetZoneVelocityMax(i, vel3D, vel2D);
        vel3D = reader.GetFloat();
        vel2D = reader.GetFloat();
        SetZoneVelocityAvg(i, vel3D, vel2D);
        vel3D = reader.GetFloat();
        vel2D = reader.GetFloat();
        SetZoneEnterSpeed(i, vel3D, vel2D);
        vel3D = reader.GetFloat();
        vel2D = reader.GetFloat();
        SetZoneExitSpeed(i, vel3D, vel2D);
    }
}

void CMomRunStats::Serialize(CUtlBuffer &writer) 
{
    writer.PutUnsignedChar(m_iTotalZones);

    for (int i = 0; i < m_iTotalZones + 1; ++i)
    {
        //Jumps/Strafes
        writer.PutUnsignedInt(m_iZoneJumps[i]);
        writer.PutUnsignedInt(m_iZoneStrafes[i]);
        //Sync
        writer.PutFloat(m_flZoneStrafeSyncAvg[i]);
        writer.PutFloat(m_flZoneStrafeSync2Avg[i]);
        //Time
        writer.PutFloat(m_flZoneEnterTime[i]);
        writer.PutFloat(m_flZoneTime[i]);
        //Velocity
        writer.PutFloat(m_flZoneVelocityMax3D[i]);
        writer.PutFloat(m_flZoneVelocityMax2D[i]);
        writer.PutFloat(m_flZoneVelocityAvg3D[i]);
        writer.PutFloat(m_flZoneVelocityAvg2D[i]);
        writer.PutFloat(m_flZoneEnterSpeed3D[i]);
        writer.PutFloat(m_flZoneEnterSpeed2D[i]);
        writer.PutFloat(m_flZoneExitSpeed3D[i]);
        writer.PutFloat(m_flZoneExitSpeed2D[i]);
    }
}

uint8 CMomRunStats::GetTotalZones()
{
    return m_iTotalZones;
}
uint32 CMomRunStats::GetZoneJumps(int zone)
{
    return zone > m_iTotalZones ? 0 : m_iZoneJumps[zone];
}
uint32 CMomRunStats::GetZoneStrafes(int zone)
{
    return zone > m_iTotalZones ? 0 : m_iZoneStrafes[zone];
}
float CMomRunStats::GetZoneTime(int zone)
{
    return zone > m_iTotalZones ? 0.0f : m_flZoneTime[zone];
}
float CMomRunStats::GetZoneEnterTime(int zone)
{
    return zone > m_iTotalZones ? 0.0f : m_flZoneEnterTime[zone];
}
float CMomRunStats::GetZoneStrafeSyncAvg(int zone)
{
    return zone > m_iTotalZones ? 0.0f : m_flZoneStrafeSyncAvg[zone];
}
float CMomRunStats::GetZoneStrafeSync2Avg(int zone)
{
    return zone > m_iTotalZones ? 0.0f : m_flZoneStrafeSync2Avg[zone];
}
float CMomRunStats::GetZoneEnterSpeed(int zone, bool vel2D)
{
    return zone > m_iTotalZones
               ? 0.0f
               : (vel2D ? m_flZoneEnterSpeed2D[zone] : m_flZoneEnterSpeed3D[zone]);
}
float CMomRunStats::GetZoneExitSpeed(int zone, bool vel2D)
{
    return zone > m_iTotalZones
               ? 0.0f
               : (vel2D ? m_flZoneExitSpeed2D[zone] : m_flZoneExitSpeed3D[zone]);
}
float CMomRunStats::GetZoneVelocityMax(int zone, bool vel2D)
{
    return zone > m_iTotalZones
               ? 0.0f
               : (vel2D ? m_flZoneVelocityMax2D[zone] : m_flZoneVelocityMax3D[zone]);
}
float CMomRunStats::GetZoneVelocityAvg(int zone, bool vel2D)
{
    return zone > m_iTotalZones
               ? 0.0f
               : (vel2D ? m_flZoneVelocityAvg2D[zone] : m_flZoneVelocityAvg3D[zone]);
}

void CMomRunStats::SetTotalZones(uint8 zones) { m_iTotalZones = clamp<uint8>(zones, 0, MAX_STAGES); }
void CMomRunStats::SetZoneJumps(int zone, uint32 value)
{
    if (zone > m_iTotalZones)
        return;

    m_iZoneJumps.Set(zone, value);
}
void CMomRunStats::SetZoneStrafes(int zone, uint32 value)
{
    if (zone > m_iTotalZones)
        return;

    m_iZoneStrafes.Set(zone, value);
}
void CMomRunStats::SetZoneTime(int zone, float value)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneTime.Set(zone, value);
}
void CMomRunStats::SetZoneEnterTime(int zone, float value)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneEnterTime.Set(zone, value);
}
void CMomRunStats::SetZoneStrafeSyncAvg(int zone, float value)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneStrafeSyncAvg.Set(zone, value);
}
void CMomRunStats::SetZoneStrafeSync2Avg(int zone, float value)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneStrafeSync2Avg.Set(zone, value);
}
void CMomRunStats::SetZoneEnterSpeed(int zone, float vert, float hor)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneEnterSpeed3D.Set(zone, vert);
    m_flZoneEnterSpeed2D.Set(zone, hor);
}
void CMomRunStats::SetZoneVelocityMax(int zone, float vert, float hor)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneVelocityMax3D.Set(zone, vert);
    m_flZoneVelocityMax2D.Set(zone, hor);
}
void CMomRunStats::SetZoneVelocityAvg(int zone, float vert, float hor)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneVelocityAvg3D.Set(zone, vert);
    m_flZoneVelocityAvg2D.Set(zone, hor);
}
void CMomRunStats::SetZoneExitSpeed(int zone, float vert, float hor)
{
    if (zone > m_iTotalZones)
        return;

    m_flZoneExitSpeed3D.Set(zone, vert);
    m_flZoneExitSpeed2D.Set(zone, hor);
}