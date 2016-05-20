#include "cbase.h"
#include "c_mom_replay_entity.h"

#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_MomentumReplayGhostEntity, DT_MOM_ReplayEnt, CMomentumReplayGhostEntity)
//MOM_TODO: Network the rest of the variables that the ghost entity will be sending
RecvPropInt(RECVINFO(m_nReplayButtons)),
RecvPropInt(RECVINFO(m_iTotalStrafes)),
RecvPropDataTable(RECVINFO_DT(m_RunData), 0, &REFERENCE_RECV_TABLE(DT_MOM_RunEntData))
END_RECV_TABLE();

C_MomentumReplayGhostEntity::C_MomentumReplayGhostEntity()
{
    m_nReplayButtons = 0;
    m_iTotalStrafes = 0;
}