#include "cbase.h"
#include "fx_impact.h"
#include "tempent.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"


//-----------------------------------------------------------------------------
// Purpose: Handle weapon effect callbacks
//-----------------------------------------------------------------------------
void Mom_EjectBrass(int shell, const CEffectData &data)
{
    C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

    if (!pPlayer)
        return;

    tempents->CSEjectBrass(data.m_vOrigin, data.m_vAngles, data.m_fFlags, shell, pPlayer);
}

void Mom_FX_EjectBrass_9mm_Callback(const CEffectData &data)
{
    Mom_EjectBrass(CS_SHELL_9MM, data);
}

void Mom_FX_EjectBrass_57_Callback(const CEffectData &data)
{
    Mom_EjectBrass(CS_SHELL_57, data);
}

void Mom_FX_EjectBrass_12Gauge_Callback(const CEffectData &data)
{
    Mom_EjectBrass(CS_SHELL_12GAUGE, data);
}

void Mom_FX_EjectBrass_556_Callback(const CEffectData &data)
{
    Mom_EjectBrass(CS_SHELL_556, data);
}

void Mom_FX_EjectBrass_762Nato_Callback(const CEffectData &data)
{
    Mom_EjectBrass(CS_SHELL_762NATO, data);
}

void Mom_FX_EjectBrass_338Mag_Callback(const CEffectData &data)
{
    Mom_EjectBrass(CS_SHELL_338MAG, data);
}

DECLARE_CLIENT_EFFECT("EjectBrass_9mm", Mom_FX_EjectBrass_9mm_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_12Gauge", Mom_FX_EjectBrass_12Gauge_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_57", Mom_FX_EjectBrass_57_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_556", Mom_FX_EjectBrass_556_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_762Nato", Mom_FX_EjectBrass_762Nato_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_338Mag", Mom_FX_EjectBrass_338Mag_Callback);