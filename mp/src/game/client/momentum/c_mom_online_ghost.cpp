#include "cbase.h"
#include "c_mom_online_ghost.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumOnlineGhostEntity, DT_MOM_OnlineGhost, CMomentumOnlineGhostEntity)
END_RECV_TABLE();

C_MomentumOnlineGhostEntity::C_MomentumOnlineGhostEntity() : 
m_iv_VecOrigin("C_MomentumOnlineGhostEntity::m_iv_vecOrigin"), m_iv_vecViewOffset("C_MomentumOnlineGhostEntity::m_iv_vecViewOffset"), 
m_iv_QAEyeAngles("C_MomentumOnlineGhostEntity::m_iv_QAEyeAngles")
{
    AddVar(&m_vecViewOffset, &m_iv_vecViewOffset, LATCH_SIMULATION_VAR);
    AddVar(&m_vecOrigin, &m_iv_VecOrigin, LATCH_SIMULATION_VAR);
    AddVar(&m_angAbsRotation, &m_iv_QAEyeAngles, LATCH_SIMULATION_VAR);
    m_pszPlayerName[0] = '\0';
}
void C_MomentumOnlineGhostEntity::Spawn()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);
    ConDColorMsg(Color(255, 0, 255, 255), "Spawned (client side)\n");
}
void C_MomentumOnlineGhostEntity::ClientThink()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);
}