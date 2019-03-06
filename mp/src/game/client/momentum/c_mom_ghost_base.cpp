#include "cbase.h"
#include "c_mom_ghost_base.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumGhostBaseEntity, DT_MOM_GHOST_BASE, CMomentumGhostBaseEntity)
RecvPropInt(RECVINFO(m_iDisabledButtons)),
RecvPropBool(RECVINFO(m_bBhopDisabled))
END_RECV_TABLE();

C_MomentumGhostBaseEntity::C_MomentumGhostBaseEntity() : m_iv_vecViewOffset("C_MomentumGhostBaseEntity::m_iv_vecViewOffset")
{
    m_iDisabledButtons = 0;
    m_bBhopDisabled = false;
    AddVar(&m_vecViewOffset, &m_iv_vecViewOffset, LATCH_SIMULATION_VAR);
}
