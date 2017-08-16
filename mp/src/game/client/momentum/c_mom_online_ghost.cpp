#include "cbase.h"
#include "c_mom_online_ghost.h"
#include "mom_player_shared.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumOnlineGhostEntity, DT_MOM_OnlineGhost, CMomentumOnlineGhostEntity)
    RecvPropString(RECVINFO(m_pszGhostName)),
    RecvPropInt(RECVINFO(m_uiAccountID), SPROP_UNSIGNED),
    RecvPropInt(RECVINFO(m_nGhostButtons)),
END_RECV_TABLE();

C_MomentumOnlineGhostEntity::C_MomentumOnlineGhostEntity(): m_uiAccountID(0), m_pEntityPanel(nullptr)
{
    m_pszGhostName[0] = '\0';
    m_SteamID = k_steamIDNil;
    m_nGhostButtons = 0;
}

C_MomentumOnlineGhostEntity::~C_MomentumOnlineGhostEntity()
{
    if (m_pEntityPanel)
        m_pEntityPanel->DeletePanel();

    m_pEntityPanel = nullptr;
}

void C_MomentumOnlineGhostEntity::Spawn()
{
    Precache();
    SetNextClientThink(CLIENT_THINK_ALWAYS);

    m_pEntityPanel = new CGhostEntityPanel();
    m_pEntityPanel->Init(this);
}
void C_MomentumOnlineGhostEntity::ClientThink()
{
    SetNextClientThink(CLIENT_THINK_ALWAYS);

    if (m_uiAccountID > 0 && !m_SteamID.IsValid() && steamapicontext->SteamUtils())
    {
        m_SteamID = CSteamID(m_uiAccountID, k_EUniversePublic, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual);
    }

}

void C_MomentumOnlineGhostEntity::SetEntityPanelVisible(bool bVisible)
{
    if (m_pEntityPanel)
        m_pEntityPanel->SetVisible(bVisible);
}
