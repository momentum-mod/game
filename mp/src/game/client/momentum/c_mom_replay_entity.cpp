#include "cbase.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumReplayGhostEntity, DT_MOM_ReplayEnt, CMomentumReplayGhostEntity)
RecvPropBool(RECVINFO(m_bIsPaused)),
RecvPropInt(RECVINFO(m_iCurrentTick)),
RecvPropInt(RECVINFO(m_iTotalTicks), SPROP_UNSIGNED),
END_RECV_TABLE();

C_MomentumReplayGhostEntity::C_MomentumReplayGhostEntity()
{
    m_bIsPaused = false;
    m_iCurrentTick = 0;
    m_iTotalTicks = 0;
    m_RunStats.Init();
}

float C_MomentumReplayGhostEntity::GetCurrentRunTime()
{
    int iTotalTicks;
    if (m_Data.m_iTimerState == TIMER_STATE_RUNNING)
    {
        iTotalTicks = m_iCurrentTick - m_Data.m_iStartTick;
    }
    else
    {
        iTotalTicks = m_Data.m_iRunTime;
    }

    return float(iTotalTicks) * m_Data.m_flTickRate;
}