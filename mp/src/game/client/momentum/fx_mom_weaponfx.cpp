#include "cbase.h"
#include "fx_impact.h"
#include "tempent.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"
#include "ammodef.h"

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Handle weapon effect callbacks
//-----------------------------------------------------------------------------
void Mom_EjectBrass(int ammoType, const CEffectData &data)
{
    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

    if (!pPlayer)
        return;

    tempents->CSEjectBrass(data.m_vOrigin, data.m_vAngles, data.m_fFlags, ammoType, pPlayer);
}

void Mom_FX_EjectBrass_9mm_Callback(const CEffectData &data)
{
    Mom_EjectBrass(AMMO_TYPE_PISTOL, data);
}

void Mom_FX_EjectBrass_57_Callback(const CEffectData &data)
{
    Mom_EjectBrass(AMMO_TYPE_SMG, data);
}

void Mom_FX_EjectBrass_12Gauge_Callback(const CEffectData &data)
{
    Mom_EjectBrass(AMMO_TYPE_SHOTGUN, data);
}

DECLARE_CLIENT_EFFECT("EjectBrass_9mm", Mom_FX_EjectBrass_9mm_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_12Gauge", Mom_FX_EjectBrass_12Gauge_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_57", Mom_FX_EjectBrass_57_Callback);
