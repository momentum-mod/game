#include "cbase.h"
#include "run_stats.h"

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
BEGIN_RECV_TABLE_NOBASE(C_MomRunStats, DT_MOM_RunStats)
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
BEGIN_SEND_TABLE_NOBASE(CMomRunStats, DT_MOM_RunStats)
SendPropInt(SENDINFO(m_iTotalZones), -1, SPROP_UNSIGNED),
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