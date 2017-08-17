#include "cbase.h"
#include "c_mom_replay_entity.h"


#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumReplayGhostEntity, DT_MOM_ReplayEnt, CMomentumReplayGhostEntity)
END_RECV_TABLE();

C_MomentumReplayGhostEntity::C_MomentumReplayGhostEntity() : m_RunStats(&m_SrvData.m_RunStatsData)
{
    m_SrvData.m_nReplayButtons = 0;
    m_SrvData.m_iTotalStrafes = 0;
    m_SrvData.m_iTotalJumps = 0;
    m_SrvData.m_iTotalTimeTicks = 0;
    m_SrvData.m_iCurrentTick = 0;
    m_SrvData.m_flTickRate = 0.0f;
    m_SrvData.m_bIsPaused = false;
    m_SrvData.m_pszPlayerName[0] = '\0';
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