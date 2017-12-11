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

#define FDECAL_PERMANENT 0x01 // This decal should not be removed in favor of any new decals
#define FDECAL_REFERENCE 0x02 // This is a decal that's been moved from another level
#define FDECAL_CUSTOM 0x04    // This is a custom clan logo and should not be saved/restored
#define FDECAL_HFLIP 0x08     // Flip horizontal (U/S) axis
#define FDECAL_VFLIP 0x10     // Flip vertical (V/T) axis
#define FDECAL_CLIPTEST 0x20  // Decal needs to be clip-tested
#define FDECAL_NOCLIP 0x40    // Decal is not clipped by containing polygon

// NOTE: There are used by footprints; maybe we separate into a separate struct?
#define FDECAL_USESAXIS 0x80        // Uses the s axis field to determine orientation
#define FDECAL_DYNAMIC 0x100        // Indicates the decal is dynamic
#define FDECAL_SECONDPASS 0x200     // Decals that have to be drawn after everything else
#define FDECAL_DONTSAVE 0x800       // Decal was loaded from adjacent level, don't save out to save file for this level
#define FDECAL_PLAYERSPRAY 0x1000   // Decal is a player spray
#define FDECAL_DISTANCESCALE 0x2000 // Decal is dynamically scaled based on distance.
#define FDECAL_HASUPDATED 0x4000    // Decal has not been updated this frame yet

static ConVar mom_paintgun_ignorez("mom_paintgun_ignorez", "0", FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
                                   "See the paintings through walls");

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Painting(Vector &vecOrigin, Vector &vecStart, int iHitbox, C_BaseEntity *pEntity, trace_t &tr,
              Color color = Color(255, 255, 255, 255), bool bIgnoreZ = true, int nFlags = 0, int maxLODToDecal = -1)
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

    if ((nFlags & IMPACT_NODECAL) == 0)
    {
        int decalNumber = -1;

        if (bIgnoreZ)
        {
            decalNumber = decalsystem->GetDecalIndexForName("PaintingIgnoreZ");
        }
        else
        {
            decalNumber = decalsystem->GetDecalIndexForName("Painting");
        }

        if (decalNumber == -1)
            return false;

        bool bSkipDecal = false;

        if (!bSkipDecal)
        {
            if ((pEntity->entindex() == 0) && (iHitbox != 0))
            {
                staticpropmgr->AddColorDecalToStaticProp(vecStart, traceExt, iHitbox - 1, decalNumber, true, tr, true,
                                                         color);
            }
            else if (pEntity)
            {
                // Here we deal with decals on entities.
                pEntity->AddColoredDecal(vecStart, traceExt, vecOrigin, iHitbox, decalNumber, true, tr, color,
                                         maxLODToDecal);
            }
        }
    }
    else
    {
        // Perform the trace ourselves
        Ray_t ray;
        ray.Init(vecStart, traceExt);

        if ((pEntity->entindex() == 0) && (iHitbox != 0))
        {
            // Special case for world entity with hitbox (that's a static prop)
            ICollideable *pCollideable = staticpropmgr->GetStaticPropByIndex(iHitbox - 1);
            enginetrace->ClipRayToCollideable(ray, MASK_SHOT, pCollideable, &tr);
        }
        else
        {
            if (!pEntity)
                return false;

            enginetrace->ClipRayToEntity(ray, MASK_SHOT, pEntity, &tr);
        }
    }

    return true;
}

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

    Color color;
    ConVarRef mom_paintgun_color("mom_paintgun_color");
    if (g_pMomentumUtil->GetColorFromHex(mom_paintgun_color.GetString(), color))
    {
        // If we hit, perform our custom effects and play the sound
        if (Painting(vecOrigin, vecStart, iHitbox, pEntity, tr, color, mom_paintgun_ignorez.GetBool()))
        {
            // Check for custom effects based on the Decal index
            PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
        }
    }

    PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
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
