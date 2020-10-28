#include "cbase.h"

#include "mom_entity_run_data.h"

#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
BEGIN_RECV_TABLE_NOBASE(CMomRunEntityData, DT_MomRunEntityData)
RecvPropBool(RECVINFO(m_bIsInZone)),
RecvPropBool(RECVINFO(m_bMapFinished)),
RecvPropInt(RECVINFO(m_iTimerState)),
RecvPropFloat(RECVINFO(m_flStrafeSync)),
RecvPropFloat(RECVINFO(m_flStrafeSync2)),
RecvPropInt(RECVINFO(m_iRunFlags), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iCurrentTrack), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iCurrentZone), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iStartTick), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iRunTime), SPROP_UNSIGNED),
RecvPropFloat(RECVINFO(m_flLastJumpTime)),
RecvPropFloat(RECVINFO(m_flLastJumpVel)),
RecvPropFloat(RECVINFO(m_flLastJumpZPos)),
RecvPropFloat(RECVINFO(m_flTickRate)),
END_RECV_TABLE();
#else
BEGIN_SEND_TABLE_NOBASE(CMomRunEntityData, DT_MomRunEntityData)
SendPropBool(SENDINFO(m_bIsInZone)),
SendPropBool(SENDINFO(m_bMapFinished)),
SendPropInt(SENDINFO(m_iTimerState), 3, SPROP_UNSIGNED),
SendPropFloat(SENDINFO(m_flStrafeSync)),
SendPropFloat(SENDINFO(m_flStrafeSync2)),
SendPropInt(SENDINFO(m_iRunFlags), 8, SPROP_UNSIGNED), // MOM_TODO change the nBits to how many flags we end up supporting!
SendPropInt(SENDINFO(m_iCurrentTrack), 4, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_iCurrentZone), 7, SPROP_UNSIGNED), // 7 bits fits the MAX_ZONES (64 as of writing) number
SendPropInt(SENDINFO(m_iStartTick), -1, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_iRunTime), 24, SPROP_UNSIGNED), // 24 bits fits the ~3.1 million max ticks we allow
SendPropFloat(SENDINFO(m_flLastJumpTime)),
SendPropFloat(SENDINFO(m_flLastJumpVel)),
SendPropFloat(SENDINFO(m_flLastJumpZPos)),
SendPropFloat(SENDINFO(m_flTickRate)),
END_SEND_TABLE();
#endif


CMomRunEntityData::CMomRunEntityData()
{
    m_bIsInZone = false;
    m_bMapFinished = false;
    m_iTimerState = TIMER_STATE_NOT_RUNNING;
    m_flStrafeSync = 0.0f;
    m_flStrafeSync2 = 0.0f;
    m_iRunFlags = 0;
    m_iCurrentTrack = 0;
    m_iCurrentZone = 0;
    m_iStartTick = 0;
    m_iRunTime = 0;
    m_flLastJumpTime = 0.0f;
    m_flLastJumpVel = 0.0f;
    m_flTickRate = gpGlobals->interval_per_tick;
}