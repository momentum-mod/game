#include "cbase.h"
#include "c_mom_online_ghost.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumOnlineGhostEntity, DT_MOM_OnlineGhost, CMomentumOnlineGhostEntity)
END_RECV_TABLE();

C_MomentumOnlineGhostEntity::C_MomentumOnlineGhostEntity() 
{
    m_pszPlayerName[0] = '\0';
}
void C_MomentumOnlineGhostEntity::Spawn()
{
    Precache();
    SetNextClientThink(CLIENT_THINK_ALWAYS);
}
void C_MomentumOnlineGhostEntity::ClientThink()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);
}