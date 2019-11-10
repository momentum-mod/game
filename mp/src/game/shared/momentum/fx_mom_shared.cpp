//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "fx_mom_shared.h"
#include "weapon/weapon_base.h"
#include "mom_player_shared.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#include "mom_ghostdefs.h"
#include "momentum/ghost_client.h"
#include "util/mom_util.h"
#include "momentum/te_shotgun_shot.h"
#else
#include "fx_impact.h"
#endif

#include "tier0/memdbgon.h"

static MAKE_TOGGLE_CONVAR(mom_use_fixed_spread, "1", FCVAR_REPLICATED,
                   "Use fixed spread patterns for shotgun weapons. 1 = ON (default), 0 = OFF");

#ifdef CLIENT_DLL

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

void StartGroupingSounds() {}
void EndGroupingSounds() {}
void FX_WeaponSound(int iEntIndex, WeaponSound_t sound_type, const Vector &vOrigin, CWeaponInfo *pWeaponInfo)
{
    // If we have some sounds from the weapon classname.txt file, play a random one of them
    const char *shootsound = pWeaponInfo->aShootSounds[sound_type];
    if (!shootsound || !shootsound[0])
        return;

    CPASAttenuationFilter filter(vOrigin, shootsound);
    filter.UsePredictionRules();
    filter.MakeReliable();

    CBaseEntity::EmitSound(filter, iEntIndex, shootsound, &vOrigin);
};

#endif

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets(int iEntIndex, const Vector &vOrigin, const QAngle &vAngles, int iWeaponID, int iMode, int iSeed,
                    float flSpread)
{
    bool bDoEffects = true;

    CBaseEntity *pAttacker = CBaseEntity::Instance(iEntIndex);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pAttacker);

    CWeaponInfo *pWeaponInfo = GetWeaponInfo((CWeaponID)iWeaponID);

    if (!pWeaponInfo)
    {
        DevMsg("FX_FireBullets: Cannot find weapon info for ID %i\n", iWeaponID);
        return;
    }

#ifndef CLIENT_DLL
    // if this is server code, send the effect over to client as temp entity
    // Dispatch one message for all the bullet impacts and sounds.
    TE_FireBullets(iEntIndex, vOrigin, vAngles, iWeaponID, iMode, iSeed, flSpread);

    if (pPlayer) // Only send this packet if it was us firing the bullet(s) all along
    {
        DecalPacket decalPacket;
        if (iWeaponID == WEAPON_PAINTGUN)
        {
            Color decalColor;
            if (!MomUtil::GetColorFromHex(ConVarRef("mom_paintgun_color").GetString(), decalColor))
                decalColor = COLOR_WHITE;

            decalPacket = DecalPacket::Paint(vOrigin, vAngles, decalColor, ConVarRef("mom_paintgun_scale").GetFloat());
        }
        else
        {
            decalPacket = DecalPacket::Bullet( vOrigin, vAngles, iWeaponID, iMode, iSeed, flSpread);
        }

        g_pMomentumGhostClient->SendDecalPacket(&decalPacket);
    }

    bDoEffects = false; // no effects on server
#endif

    iSeed++;

    int iDamage = pWeaponInfo->m_iDamage;
    float flRange = pWeaponInfo->m_flRange;
    int iPenetration = pWeaponInfo->m_iPenetration;
    float flRangeModifier = pWeaponInfo->m_flRangeModifier;
    int iAmmoType = pWeaponInfo->iAmmoType;

#ifdef GAME_DLL
    // Weapon sounds are server-only for PAS ability

    static ConVarRef paintgun("mom_paintgun_shoot_sound");

    // Do an extra paintgun check here
    const bool bPreventShootSound = iWeaponID == WEAPON_PAINTGUN && !paintgun.GetBool();

    if (!bPreventShootSound)
        FX_WeaponSound(iEntIndex, SINGLE, vOrigin, pWeaponInfo);
#endif

    // Fire bullets, calculate impacts & effects
    bool bLocalPlayerFired = true;
    if (!pPlayer)
    {
        bLocalPlayerFired = false;
#ifdef GAME_DLL
        pPlayer = CMomentumPlayer::GetLocalPlayer();
#elif defined(CLIENT_DLL)
        pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
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

    int iTotalBullets = pWeaponInfo->m_iBullets;
    bool bNoSpread = false;

    if (iTotalBullets > 1)
        bNoSpread = mom_use_fixed_spread.GetBool();

    for (int iBullet = 0; iBullet < iTotalBullets; iBullet++)
    {
        float x, y;

        if (bNoSpread)
        {
            int iIndex = iBullet;
            while (iIndex > 9)
            {
                iIndex -= 10;
            }

            x = 0.5f * g_vecFixedPattern[iIndex].x;
            y = 0.5f * g_vecFixedPattern[iIndex].y;
        }
        else
        {
            RandomSeed(iSeed); // init random system with this seed

            // Get circular gaussian spread.
            x = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);
            y = RandomFloat(-0.5, 0.5) + RandomFloat(-0.5, 0.5);

            iSeed++; // use new seed for next bullet
        }

        pPlayer->FireBullet(vOrigin, vAngles, flSpread, flRange, iPenetration, iAmmoType, iDamage, flRangeModifier,
                            pAttacker, bDoEffects, x, y);
    }

#ifndef CLIENT_DLL
    if (bLocalPlayerFired)
        lagcompensation->FinishLagCompensation(pPlayer);
#endif

    EndGroupingSounds();
}