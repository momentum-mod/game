#include "cbase.h"
#include "c_mom_player.h"

#include "tier0/memdbgon.h"



IMPLEMENT_CLIENTCLASS_DT(C_MomentumPlayer, DT_MOM_Player, CMomentumPlayer)
//RecvPropDataTable(RECVINFO_DT(m_HL2Local), 0, &REFERENCE_RECV_TABLE(DT_HL2Local)),
//RecvPropBool(RECVINFO(m_fIsSprinting)),
END_RECV_TABLE()


C_MomentumPlayer::C_MomentumPlayer()
{

}

C_MomentumPlayer::~C_MomentumPlayer()
{

}