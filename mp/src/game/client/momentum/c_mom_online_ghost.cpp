#include "cbase.h"
#include "c_mom_online_ghost.h"
#include "steam/steam_api.h"
#include "GhostEntityPanel.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumOnlineGhostEntity, DT_MOM_OnlineGhost, CMomentumOnlineGhostEntity)
    RecvPropInt(RECVINFO(m_uiAccountID), SPROP_UNSIGNED),
    RecvPropBool(RECVINFO(m_bSpectating)),
    RecvPropFloat(RECVINFO(m_vecViewOffset[0])),
    RecvPropFloat(RECVINFO(m_vecViewOffset[1])),
    RecvPropFloat(RECVINFO(m_vecViewOffset[2])),
END_RECV_TABLE();

C_MomentumOnlineGhostEntity::C_MomentumOnlineGhostEntity(): m_uiAccountID(0), m_bSpectating(false), m_pEntityPanel(nullptr)
{
    m_SteamID = 0;
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

    if (m_uiAccountID > 0 && m_SteamID == 0 && SteamUtils())
    {
        m_SteamID = CSteamID(m_uiAccountID, k_EUniversePublic, SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual).ConvertToUint64();
    }

    m_pEntityPanel->SetVisible(!(m_bSpectating || m_bSpectated));
}
