#include "cbase.h"
#include "c_mom_ghost_base.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumGhostBaseEntity, DT_MOM_GHOST_BASE, CMomentumGhostBaseEntity)
END_RECV_TABLE();

C_MomentumGhostBaseEntity::C_MomentumGhostBaseEntity() : m_iv_vecViewOffset("C_MomentumGhostBaseEntity::m_iv_vecViewOffset")
{
    AddVar(&m_vecViewOffset, &m_iv_vecViewOffset, LATCH_SIMULATION_VAR);
}
