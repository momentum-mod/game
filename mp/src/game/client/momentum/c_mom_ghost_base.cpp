#include "cbase.h"

#include "c_mom_ghost_base.h"

#include "steam/isteamutils.h"

#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_MomentumGhostBaseEntity, DT_MOM_GHOST_BASE, CMomentumGhostBaseEntity)
RecvPropInt(RECVINFO(m_nGhostButtons)),
RecvPropInt(RECVINFO(m_iDisabledButtons)),
RecvPropBool(RECVINFO(m_bBhopDisabled)),
RecvPropInt(RECVINFO(m_AccountID)),
RecvPropString(RECVINFO(m_szGhostName)),
RecvPropBool(RECVINFO(m_bSpectated)),
RecvPropInt(RECVINFO(m_fFlags)),
RecvPropDataTable(RECVINFO_DT(m_Data), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunEntityData)),
RecvPropDataTable(RECVINFO_DT(m_RunStats), SPROP_PROXY_ALWAYS_YES | SPROP_CHANGES_OFTEN, &REFERENCE_RECV_TABLE(DT_MomRunStats)),
END_RECV_TABLE();

C_MomentumGhostBaseEntity::C_MomentumGhostBaseEntity(): m_iv_vecViewOffset("C_MomentumGhostBaseEntity::m_iv_vecViewOffset")
{
    m_AccountID = 0;
    m_SteamID = 0;
    m_nGhostButtons = 0;
    m_iDisabledButtons = 0;
    m_bBhopDisabled = false;
    m_bSpectated = false;
    m_szGhostName.GetForModify()[0] = '\0';
    m_RunStats.Init();
    AddVar(&m_vecViewOffset, &m_iv_vecViewOffset, LATCH_SIMULATION_VAR);
}

void C_MomentumGhostBaseEntity::PostDataUpdate(DataUpdateType_t updateType)
{
    BaseClass::PostDataUpdate(updateType);

    if (updateType == DATA_UPDATE_CREATED)
    {
        if (m_AccountID > 0 && m_SteamID == 0 && SteamUtils())
        {
            m_SteamID = CSteamID(m_AccountID, k_EUniversePublic, SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual).ConvertToUint64();
        }
    }
}

float C_MomentumGhostBaseEntity::GetCurrentRunTime()
{
    return 0.0f;
}