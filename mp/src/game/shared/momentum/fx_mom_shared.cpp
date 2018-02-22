//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "fx_mom_shared.h"
#include "weapon/weapon_csbase.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#include "momentum/ghost_client.h"
#include "mom_ghostdefs.h"
#include "util/mom_util.h"
#endif


#ifdef CLIENT_DLL

#include "fx_impact.h"

// this is a cheap ripoff from CBaseCombatWeapon::WeaponSound():
void FX_WeaponSound(
    int iPlayerIndex,
    WeaponSound_t sound_type,
    const Vector &vOrigin,
    CCSWeaponInfo *pWeaponInfo)
{

    // If we have some sounds from the weapon classname.txt file, play a random one of them
    const char *shootsound = pWeaponInfo->aShootSounds[sound_type];
    if (!shootsound || !shootsound[0])
        return;

    CBroadcastRecipientFilter filter; // this is client side only
    if (!te->CanPredict())
        return;

    CBaseEntity::EmitSound(filter, iPlayerIndex, shootsound, &vOrigin);
}

class CGroupedSound
{
public:
    string_t m_SoundName;
    Vector m_vPos;
};

CUtlVector<CGroupedSound> g_GroupedSounds;


// Called by the ImpactSound function.
void ShotgunImpactSoundGroup(const char *pSoundName, const Vector &vEndPos)
{
    int i;
    // Don't play the sound if it's too close to another impact sound.
    for (i = 0; i < g_GroupedSounds.Count(); i++)
    {
        CGroupedSound *pSound = &g_GroupedSounds[i];

        if (vEndPos.DistToSqr(pSound->m_vPos) < 300 * 300)
        {
            if (Q_stricmp(pSound->m_SoundName, pSoundName) == 0)
                return;
        }
    }

    // Ok, play the sound and add it to the list.
    CLocalPlayerFilter filter;
    C_BaseEntity::EmitSound(filter, NULL, pSoundName, &vEndPos);

    i = g_GroupedSounds.AddToTail();
    g_GroupedSounds[i].m_SoundName = pSoundName;
    g_GroupedSounds[i].m_vPos = vEndPos;
}


void StartGroupingSounds()
{
    Assert(g_GroupedSounds.Count() == 0);
    SetImpactSoundRoute(ShotgunImpactSoundGroup);
}


void EndGroupingSounds()
{
    g_GroupedSounds.Purge();
    SetImpactSoundRoute(NULL);
}

#else

#include "momentum/te_shotgun_shot.h"

// Server doesn't play sounds anyway.
void StartGroupingSounds() {}
void EndGroupingSounds() {}
void FX_WeaponSound ( int iPlayerIndex,
    WeaponSound_t sound_type,
    const Vector &vOrigin,
    CCSWeaponInfo *pWeaponInfo ) {};

#endif

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets(
    int	iPlayerIndex,
    const Vector &vOrigin,
    const QAngle &vAngles,
    int	iWeaponID,
    int	iMode,
    int iSeed,
    float flSpread
    )
{
    bool bDoEffects = true;

#ifdef CLIENT_DLL
    CMomentumPlayer *pPlayer = ToCMOMPlayer(ClientEntityList().GetBaseEntity(iPlayerIndex));
#else
    CMomentumPlayer *pPlayer = ToCMOMPlayer( UTIL_PlayerByIndex( iPlayerIndex) );
#endif

    CCSWeaponInfo *pWeaponInfo = GetWeaponInfo((CSWeaponID) iWeaponID);

    if (!pWeaponInfo)
    {
        DevMsg("FX_FireBullets: Cannot find weapon info for ID %i\n", iWeaponID);
        return;
    }

#ifndef CLIENT_DLL
    // if this is server code, send the effect over to client as temp entity
    // Dispatch one message for all the bullet impacts and sounds.
    TE_FireBullets( 
        iPlayerIndex,
        vOrigin, 
        vAngles, 
        iWeaponID,
        iMode,
        iSeed,
        flSpread
        );

    if (pPlayer) // Only send this packet if it was us firing the bullet(s) all along
    {
        DecalPacket_t decalPacket;
        if (iWeaponID == WEAPON_PAINTGUN)
        {
            Color decalColor;
            if (!g_pMomentumUtil->GetColorFromHex(ConVarRef("mom_paintgun_color").GetString(), decalColor))
                decalColor = COLOR_WHITE;

            decalPacket = DecalPacket_t(DECAL_PAINT, vOrigin, vAngles, decalColor.GetRawColor(), 0, 0, ConVarRef("mom_paintgun_scale").GetFloat());
        }
        else
        {
            decalPacket = DecalPacket_t(DECAL_BULLET, vOrigin, vAngles, iWeaponID, iMode, iSeed, flSpread);
        }

        g_pMomentumGhostClient->SendDecalPacket(&decalPacket);
    }

    bDoEffects = false; // no effects on server
#endif

    iSeed++;

    int		iDamage = pWeaponInfo->m_iDamage;
    float	flRange = pWeaponInfo->m_flRange;
    int		iPenetration = pWeaponInfo->m_iPenetration;
    float	flRangeModifier = pWeaponInfo->m_flRangeModifier;
    int		iAmmoType = pWeaponInfo->iAmmoType;

    if (bDoEffects)
    {
        // Do an extra paintgun check here
        const bool bPreventShootSound = (iWeaponID == WEAPON_PAINTGUN && !ConVarRef("mom_paintgun_shoot_sound").GetBool());
        
        if (!bPreventShootSound)
            FX_WeaponSound(iPlayerIndex, SINGLE, vOrigin, pWeaponInfo);
    }


    // Fire bullets, calculate impacts & effects
    bool bLocalPlayerFired = true;
    if (!pPlayer)
    {
        bLocalPlayerFired = false;
#ifdef GAME_DLL
        pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
#elif defined(CLIENT_DLL)
        pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
#endif
        if (!pPlayer) // If we still can't get a player to shoot through, then get outta 'ere
            return;
    }

    StartGroupingSounds();

#ifndef CLIENT_DLL
    // Move other players back to history positions based on local player's lag
    if (bLocalPlayerFired)
        lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());
#endif

    for (int iBullet = 0; iBullet < pWeaponInfo->m_iBullets; iBullet++)
    {
        RandomSeed(iSeed);	// init random system with this seed

        // Get circular gaussian spread.
        float x, y;
        x = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);
        y = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);

        iSeed++; // use new seed for next bullet

        pPlayer->FireBullet(
            vOrigin,
            vAngles,
            flSpread,
            flRange,
            iPenetration,
            iAmmoType,
            iDamage,
            flRangeModifier,
            pPlayer,
            bDoEffects,
            x, y);
    }

#if !defined (CLIENT_DLL)
    if (bLocalPlayerFired)
        lagcompensation->FinishLagCompensation(pPlayer);
#endif

    EndGroupingSounds();
}