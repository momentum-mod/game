//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "decals.h"
#include "fx_impact.h"
#include "iefx.h"
#include "util/mom_util.h"

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
    C_BaseEntity *pEntity =
        ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

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
    C_BaseEntity *pEntity =
        ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);
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

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts
//-----------------------------------------------------------------------------
bool Painting(Vector &vecOrigin, Vector &vecStart, int iHitbox, C_BaseEntity *pEntity, trace_t &tr,
              Color color = Color(255, 255, 255, 255), int nFlags = 0)
{
    VPROF("Painting");

    Assert(pEntity);

    // Clear out the trace
    memset(&tr, 0, sizeof(trace_t));
    tr.fraction = 1.0f;

    // Setup our shot information
    Vector shotDir = vecOrigin - vecStart;
    float flLength = VectorNormalize(shotDir);
    Vector traceExt;
    VectorMA(vecStart, flLength + 8.0f, shotDir, traceExt);

    int decalNumber = decalsystem->GetDecalIndexForName("Painting");

    if (decalNumber == -1)
        return false;

    if (!pEntity)
        return false;

    if ((pEntity->entindex() == 0) && (iHitbox != 0))
    {
        staticpropmgr->AddColorDecalToStaticProp(vecStart, traceExt, iHitbox - 1, decalNumber, true, tr, true,
                                                    color);
    }
    else
    {
        // Here we deal with decals on entities.
        pEntity->AddColoredDecal(vecStart, traceExt, vecOrigin, iHitbox, decalNumber, true, tr, color,
            ADDDECAL_TO_ALL_LODS, nFlags);
    }

    return true;
}

void PaintingCallback(const CEffectData &data)
{
    VPROF_BUDGET("PaintingCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);
    trace_t tr;
    Vector vecOrigin, vecStart, vecShotDir;
    int iMaterial, iDamageType, iHitbox;
    short nSurfaceProp;
    C_BaseEntity *pEntity =
        ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);
    if (!pEntity)
    {
        // This happens for impacts that occur on an object that's then destroyed.
        // Clear out the fraction so it uses the server's data
        tr.fraction = 1.0;
        PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
        return;
    }

    // Let's only allow the world for now...
    if (pEntity->entindex() != 0)
        return;

    Color color;

    // Are we an online entity?
    if (data.m_bCustomColors)
    {
        color.SetRawColor(iDamageType);
        g_pMomentumUtil->UpdatePaintDecalScale(data.m_flScale);
    }
    else
    {
        ConVarRef mom_paintgun_color("mom_paintgun_color"), paintgun_scale("mom_paintgun_scale");
        g_pMomentumUtil->GetColorFromHex(mom_paintgun_color.GetString(), color);
        g_pMomentumUtil->UpdatePaintDecalScale(paintgun_scale.GetFloat());
    }

    // If we hit, perform our custom effects and play the sound
    if (Painting(vecOrigin, vecStart, iHitbox, pEntity, tr, color, data.m_bCustomColors ? 0 : 0x01))
    {
        // Check for custom effects based on the Decal index
        // MOM_TODO: Custom impact effects here? Or at least pass no flecks as a flag
        //PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
    }

    // MOM_TODO: Custom impact sounds here?
    //PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("Painting", PaintingCallback);

void KnifeSlash(const CEffectData &data)
{
    trace_t tr;
    Vector vecOrigin, vecStart, vecShotDir;
    int iMaterial, iDamageType, iHitbox;
    short nSurfaceProp;

    C_BaseEntity *pEntity =
        ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

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
            effects->DecalShoot(decalNumber, pEntity->entindex(), pEntity->GetModel(), pEntity->GetAbsOrigin(),
                                pEntity->GetAbsAngles(), vecOrigin, &vecPerp, 0);
        }
    }

    if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, data.m_fFlags))
    {
        // Check for custom effects based on the Decal index
        PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
    }
}

DECLARE_CLIENT_EFFECT("KnifeSlash", KnifeSlash);
