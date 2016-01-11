//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "fx.h"
#include "decals.h"
#include "fx_quad.h"
#include "fx_sparks.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Handle jeep impacts
//-----------------------------------------------------------------------------
void ImpactJeepCallback(const CEffectData &data)
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

DECLARE_CLIENT_EFFECT("ImpactJeep", ImpactJeepCallback);


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
/*void ImpactCallback(const CEffectData &data)
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

DECLARE_CLIENT_EFFECT("Impact", ImpactCallback);*/

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&normal - 
//			scale - 
//-----------------------------------------------------------------------------
void FX_AirboatGunImpact(const Vector &origin, const Vector &normal, float scale)
{
    // Normal metal spark
    FX_MetalSpark( origin, normal, normal, (int) scale );

    // Add a quad to highlite the hit point
    FX_AddQuad( origin, 
        normal, 
        random->RandomFloat( 16, 32 ),
        random->RandomFloat( 32, 48 ),
        0.75f, 
        1.0f,
        0.0f,
        0.4f,
        random->RandomInt( 0, 360 ), 
        0,
        Vector( 1.0f, 1.0f, 1.0f ), 
        0.05f, 
        "effects/combinemuzzle2_nocull",
        (FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA) );
}

//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts from the airboat gun shooting (cheaper versions)
//-----------------------------------------------------------------------------
void ImpactAirboatGunCallback( const CEffectData &data )
{
    VPROF_BUDGET( "ImpactAirboatGunCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING );

    trace_t tr;
    Vector vecOrigin, vecStart, vecShotDir;
    int iMaterial, iDamageType, iHitbox;
    short nSurfaceProp;
    C_BaseEntity *pEntity = ParseImpactData( data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox );

    if ( !pEntity )
    {
        // This happens for impacts that occur on an object that's then destroyed.
        // Clear out the fraction so it uses the server's data
        tr.fraction = 1.0;
        PlayImpactSound( pEntity, tr, vecOrigin, nSurfaceProp );
        return;
    }

    // If we hit, perform our custom effects and play the sound. Don't create decals
    if ( Impact( vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS ) )
    {
        FX_AirboatGunImpact( vecOrigin, tr.plane.normal, 2 );
    }
}

DECLARE_CLIENT_EFFECT( "AirboatGunImpact", ImpactAirboatGunCallback );


//-----------------------------------------------------------------------------
// Purpose: Handle weapon impacts from the helicopter shooting (cheaper versions)
//-----------------------------------------------------------------------------
void ImpactHelicopterCallback(const CEffectData &data)
{
    VPROF_BUDGET("ImpactHelicopterCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

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

    // If we hit, perform our custom effects and play the sound. Don't create decals
    if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS))
    {
        FX_AirboatGunImpact(vecOrigin, tr.plane.normal, IsXbox() ? 1 : 2);

        // Only do metal + computer custom effects
        if ((iMaterial == CHAR_TEX_METAL) || (iMaterial == CHAR_TEX_COMPUTER))
        {
            PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0, FLAGS_CUSTIOM_EFFECTS_NOFLECKS);
        }
    }

    PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("HelicopterImpact", ImpactHelicopterCallback);

