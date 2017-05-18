#include "cbase.h"
#include "c_mom_replay_entity.h"


#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumReplayGhostEntity, DT_MOM_ReplayEnt, CMomentumReplayGhostEntity)
//RecvPropInt(RECVINFO(m_SrvData.m_nReplayButtons)), 
//RecvPropInt(RECVINFO(m_iTotalStrafes)), 
//RecvPropInt(RECVINFO(m_iTotalJumps)),
RecvPropFloat(RECVINFO(m_flTickRate)), 
RecvPropString(RECVINFO(m_pszPlayerName)),
RecvPropInt(RECVINFO(m_iTotalTimeTicks)), 
//RecvPropInt(RECVINFO(m_iCurrentTick)),
//RecvPropBool(RECVINFO(m_bIsPaused)),
//RecvPropDataTable(RECVINFO_DT(m_RunData), 0, &REFERENCE_RECV_TABLE(DT_MOM_RunEntData)),
//RecvPropDataTable(RECVINFO_DT(m_RunStats), SPROP_PROXY_ALWAYS_YES, &REFERENCE_RECV_TABLE(DT_MOM_RunStats)),
END_RECV_TABLE();

C_MomentumReplayGhostEntity::C_MomentumReplayGhostEntity() : m_iv_vecViewOffset("C_MomentumReplayGhostEntity::m_iv_vecViewOffset"),
    m_RunStats(&m_SrvData.m_RunStatsData)
{
    AddVar(&m_vecViewOffset, &m_iv_vecViewOffset, LATCH_SIMULATION_VAR);
    m_SrvData.m_nReplayButtons = 0;
    m_SrvData.m_iTotalStrafes = 0;
    m_SrvData.m_iTotalJumps = 0;
    m_iTotalTimeTicks = 0;
    m_SrvData.m_iCurrentTick = 0;
    m_flTickRate = 0.0f;
    m_SrvData.m_bIsPaused = false;
    m_pszPlayerName[0] = '\0';
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