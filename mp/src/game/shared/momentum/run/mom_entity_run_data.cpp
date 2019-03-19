#include "cbase.h"

#include "mom_entity_run_data.h"

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
BEGIN_RECV_TABLE_NOBASE(CMomRunEntityData, DT_MomRunEntityData)
RecvPropBool(RECVINFO(m_bIsInZone)),
RecvPropBool(RECVINFO(m_bMapFinished)),
RecvPropBool(RECVINFO(m_bTimerRunning)),
RecvPropFloat(RECVINFO(m_flStrafeSync)),
RecvPropFloat(RECVINFO(m_flStrafeSync2)),
RecvPropInt(RECVINFO(m_iRunFlags), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iCurrentTrack)),
RecvPropInt(RECVINFO(m_iCurrentZone)),
RecvPropInt(RECVINFO(m_iBonusZone)), // MOM_TODO removeme in favor for current track
RecvPropInt(RECVINFO(m_iOldZone)),
RecvPropInt(RECVINFO(m_iOldBonusZone)),
RecvPropInt(RECVINFO(m_iStartTick)),
RecvPropInt(RECVINFO(m_iRunTimeTicks), SPROP_UNSIGNED),
RecvPropFloat(RECVINFO(m_flLastJumpTime)),
RecvPropFloat(RECVINFO(m_flLastJumpVel)),
RecvPropFloat(RECVINFO(m_flTickRate)),
RecvPropFloat(RECVINFO(m_flRunTime)), // MOM_TODO removeme
END_RECV_TABLE();
#else
BEGIN_SEND_TABLE_NOBASE(CMomRunEntityData, DT_MomRunEntityData)
SendPropBool(SENDINFO(m_bIsInZone)),
SendPropBool(SENDINFO(m_bMapFinished)),
SendPropBool(SENDINFO(m_bTimerRunning)),
SendPropFloat(SENDINFO(m_flStrafeSync)),
SendPropFloat(SENDINFO(m_flStrafeSync2)),
SendPropInt(SENDINFO(m_iRunFlags), -1, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_iCurrentTrack)),
SendPropInt(SENDINFO(m_iCurrentZone)),
SendPropInt(SENDINFO(m_iBonusZone)), // MOM_TODO removeme in favor for current track
SendPropInt(SENDINFO(m_iOldZone)),
SendPropInt(SENDINFO(m_iOldBonusZone)), // MOM_TODO removeme in favor for oldTrackNum
SendPropInt(SENDINFO(m_iStartTick)),
SendPropInt(SENDINFO(m_iRunTimeTicks), -1, SPROP_UNSIGNED),
SendPropFloat(SENDINFO(m_flLastJumpTime)),
SendPropFloat(SENDINFO(m_flLastJumpVel)),
SendPropFloat(SENDINFO(m_flTickRate)),
SendPropFloat(SENDINFO(m_flRunTime)), // MOM_TODO removeme
END_SEND_TABLE();
#endif


CMomRunEntityData::CMomRunEntityData()
{
    m_bIsInZone = false;
    m_bMapFinished = false;
    m_bTimerRunning = false;
    m_flStrafeSync = 0.0f;
    m_flStrafeSync2 = 0.0f;
    m_iRunFlags = 0;
    m_iCurrentTrack = 0;
    m_iCurrentZone = 0;
    m_iBonusZone = 0;
    m_iOldZone = 0;
    m_iOldBonusZone = 0;
    m_iStartTick = 0;
    m_iRunTimeTicks = 0;
    m_flLastJumpTime = 0.0f;
    m_flLastJumpVel = 0.0f;
    m_flRunTime = 0.0f;
    m_flTickRate = gpGlobals->interval_per_tick;
}