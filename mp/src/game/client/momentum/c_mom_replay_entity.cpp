#include "cbase.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumReplayGhostEntity, DT_MOM_ReplayEnt, CMomentumReplayGhostEntity)
RecvPropBool(RECVINFO(m_bIsPaused)),
RecvPropInt(RECVINFO(m_iCurrentTick)),
RecvPropInt(RECVINFO(m_iStartTickD), SPROP_UNSIGNED),
RecvPropInt(RECVINFO(m_iTotalTicks), SPROP_UNSIGNED),
END_RECV_TABLE();

C_MomentumReplayGhostEntity::C_MomentumReplayGhostEntity() : m_RunStats(&m_SrvData.m_RunStatsData)
{
    m_bIsPaused = false;
    m_iCurrentTick = 0;
    m_iStartTickD = 0;
    m_iTotalTicks = 0;
    m_RunStats.m_pData = &m_SrvData.m_RunStatsData;
    m_RunStats.Init();
}
void C_MomentumReplayGhostEntity::Spawn()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);
}
void C_MomentumReplayGhostEntity::ClientThink()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);
    FetchStdReplayData(this);
}