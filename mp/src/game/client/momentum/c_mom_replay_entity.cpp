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