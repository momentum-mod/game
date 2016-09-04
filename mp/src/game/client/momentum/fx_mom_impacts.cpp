//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "iefx.h"
#include "decals.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Handle gauss impacts
//-----------------------------------------------------------------------------
void ImpactGaussCallback(const CEffectData &data)
{
    trace_t tr;
    Vector vecOrigin, vecStart, vecShotDir;
    int iMaterial, iDamageType, iHitbox;
    short nSurfaceProp;
    C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

    if (!pEntity)
    {
        // This happens for impacts that occur on an object that's then destroyed.
        // Clear out the fraction so it uses the server's data
        tr.fraction = 1.0;
        PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
        return;
    }

    // If we hit, perform our custom effects and play the sound
    if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
    {
        // Check for custom effects based on the Decal index
        PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 2);
    }

    PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("ImpactGauss", ImpactGaussCallback);


//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
void ImpactCallback(const CEffectData &data)
{
    VPROF_BUDGET("ImpactCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);
    trace_t tr;
    Vector vecOrigin, vecStart, vecShotDir;
    int iMaterial, iDamageType, iHitbox;
    short nSurfaceProp;
    C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);
    if (!pEntity)
    {
        // This happens for impacts that occur on an object that's then destroyed.
        // Clear out the fraction so it uses the server's data
        tr.fraction = 1.0;
        PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
        return;
    }
    // If we hit, perform our custom effects and play the sound
    if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
    {
        // Check for custom effects based on the Decal index
        PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
    }
    PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}
DECLARE_CLIENT_EFFECT("Impact", ImpactCallback);


void KnifeSlash(const CEffectData &data)
{
    trace_t tr;
    Vector vecOrigin, vecStart, vecShotDir;
    int iMaterial, iDamageType, iHitbox;
    short nSurfaceProp;

    C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

    if (!pEntity)
        return;

    int decalNumber = decalsystem->GetDecalIndexForName(GetImpactDecal(pEntity, iMaterial, iDamageType));
    if (decalNumber == -1)
        return;

    // vector perpendicular to the slash direction
    // so we can align the slash decal to that
    Vector vecPerp;
    AngleVectors(data.m_vAngles, nullptr, &vecPerp, nullptr);

    ConVarRef decals("r_decals");

    if (decals.GetInt())
    {
        if ((pEntity->entindex() == 0) && (iHitbox != 0))
        {
            // Setup our shot information
            Vector shotDir = vecOrigin - vecStart;
            float flLength = VectorNormalize(shotDir);
            Vector traceExt;
            VectorMA(vecStart, flLength + 8.0f, shotDir, traceExt);

            // Special case for world entity with hitbox (that's a static prop):
            // In this case, we've hit a static prop. Decal it!
            staticpropmgr->AddDecalToStaticProp(vecStart, traceExt, iHitbox - 1, decalNumber, true, tr);
        }
        else
        {
            effects->DecalShoot(decalNumber,
                pEntity->entindex(),
                pEntity->GetModel(),
                pEntity->GetAbsOrigin(),
                pEntity->GetAbsAngles(),
                vecOrigin,
                &vecPerp,
                0);
        }

    }

    if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, data.m_fFlags))
    {
        // Check for custom effects based on the Decal index
        PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
    }
}

DECLARE_CLIENT_EFFECT("KnifeSlash", KnifeSlash);