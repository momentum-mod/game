#include "cbase.h"
#include "c_mom_replay_entity.h"


#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumReplayGhostEntity, DT_MOM_ReplayEnt, CMomentumReplayGhostEntity)
RecvPropInt(RECVINFO(m_nReplayButtons)), 
RecvPropInt(RECVINFO(m_iTotalStrafes)), 
RecvPropInt(RECVINFO(m_iTotalJumps)),
RecvPropFloat(RECVINFO(m_flTickRate)), 
RecvPropString(RECVINFO(m_pszPlayerName)),
RecvPropInt(RECVINFO(m_iTotalTimeTicks)), 
RecvPropInt(RECVINFO(m_iCurrentTick)),
RecvPropBool(RECVINFO(m_bIsPaused)),
RecvPropDataTable(RECVINFO_DT(m_RunData), 0, &REFERENCE_RECV_TABLE(DT_MOM_RunEntData)),
RecvPropDataTable(RECVINFO_DT(m_RunStats), SPROP_PROXY_ALWAYS_YES, &REFERENCE_RECV_TABLE(DT_MOM_RunStats)),
END_RECV_TABLE();

C_MomentumReplayGhostEntity::C_MomentumReplayGhostEntity()
    : m_iv_vecViewOffset("C_MomentumReplayGhostEntity::m_iv_vecViewOffset")
{
    AddVar(&m_vecViewOffset, &m_iv_vecViewOffset, LATCH_SIMULATION_VAR);

    m_nReplayButtons = 0;
    m_iTotalStrafes = 0;
    m_iTotalJumps = 0;
    m_iTotalTimeTicks = 0;
    m_iCurrentTick = 0;
    m_flTickRate = 0.0f;
    m_bIsPaused = false;
    m_pszPlayerName[0] = '\0';
    m_RunStats.Init();
}