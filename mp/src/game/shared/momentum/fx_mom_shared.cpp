//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "fx_mom_shared.h"
#include "weapon/weapon_base.h"
#include "weapon/weapon_def.h"
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

static MAKE_TOGGLE_CONVAR(mom_fixed_spread, "1", FCVAR_REPLICATED, "Use fixed spread patterns for scatter shot weapons. 1 = ON (default), 0 = OFF\n");

static const Vector g_vecFixedPattern[] = {
    Vector(0, 0, 0),        Vector(1, 0, 0),       Vector(-1, 0, 0),        Vector(0, -1, 0),       Vector(0, 1, 0),
    Vector(0.85, -0.85, 0), Vector(0.85, 0.85, 0), Vector(-0.85, -0.85, 0), Vector(-0.85, 0.85, 0), Vector(0, 0, 0),
};

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

void FX_WeaponSound(int iEntIndex, const char *pShootSound, const Vector &vOrigin)
{
    if (!pShootSound || !pShootSound[0])
        return;

    CPASAttenuationFilter filter(vOrigin, pShootSound);
    filter.UsePredictionRules();
    filter.MakeReliable();

    CBaseEntity::EmitSound(filter, iEntIndex, pShootSound, &vOrigin);
};

#endif

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets(int iEntIndex, const Vector &vOrigin, const QAngle &vAngles, int iAmmoType, bool bSecondaryMode, int iSeed, float flSpread)
{
    bool bDoEffects = true;

    CBaseEntity *pAttacker = CBaseEntity::Instance(iEntIndex);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pAttacker);

    const auto hWeaponID = g_pAmmoDef->WeaponID(iAmmoType);
    if (hWeaponID == WEAPON_NONE)
    {
        DevMsg("FX_FireBullets: Cannot find weapon info for given ammo type %i\n", iAmmoType);
        return;
    }

#ifndef CLIENT_DLL
    // if this is server code, send the effect over to client as temp entity
    // Dispatch one message for all the bullet impacts and sounds.
    TE_FireBullets(iEntIndex, vOrigin, vAngles, iAmmoType, bSecondaryMode, iSeed, flSpread);

    if (pPlayer) // Only send this packet if it was us firing the bullet(s) all along
    {
        DecalPacket decalPacket;
        if (iAmmoType == AMMO_TYPE_PAINT)
        {
            Color decalColor;
            if (!MomUtil::GetColorFromHex(ConVarRef("mom_paintgun_color").GetString(), decalColor))
                decalColor = COLOR_WHITE;

            decalPacket = DecalPacket::Paint(vOrigin, vAngles, decalColor, ConVarRef("mom_paintgun_scale").GetFloat());
        }
        else
        {
            decalPacket = DecalPacket::Bullet( vOrigin, vAngles, iAmmoType, bSecondaryMode, iSeed, flSpread);
        }

        g_pMomentumGhostClient->SendDecalPacket(&decalPacket);
    }

    bDoEffects = false; // no effects on server
#endif

    iSeed++;

#ifdef GAME_DLL
    // Weapon sounds are server-only for PAS ability

    static ConVarRef paintgun_shoot_sound("mom_paintgun_shoot_sound");

    // Do an extra paintgun check here
    const bool bPreventShootSound = iAmmoType == AMMO_TYPE_PAINT && !paintgun_shoot_sound.GetBool();

    if (!bPreventShootSound)
    {
        const auto pWeaponScript = g_pWeaponDef->GetWeaponScript(hWeaponID);
        FX_WeaponSound(iEntIndex, pWeaponScript->pKVWeaponSounds->GetString("single_shot"), vOrigin);
    }
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

#ifndef CLIENT_DLL
    // Move other players back to history positions based on local player's lag
    if (bLocalPlayerFired)
        lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());
#else
    StartGroupingSounds();
#endif

    const auto iNumBullets = g_pAmmoDef->NumBullets(iAmmoType);
    const bool bFixedSpread = iNumBullets > 1 && mom_fixed_spread.GetBool();

    for (int iBullet = 0; iBullet < iNumBullets; iBullet++)
    {
        float x, y;

        if (bFixedSpread)
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

        pPlayer->FireBullet(vOrigin, vAngles, flSpread, iAmmoType, pAttacker, bDoEffects, x, y);
    }

#ifndef CLIENT_DLL
    if (bLocalPlayerFired)
        lagcompensation->FinishLagCompensation(pPlayer);
#else
    EndGroupingSounds();
#endif
}

// TF2 explosions
#ifndef CLIENT_DLL
class CTETFExplosion : public CBaseTempEntity
{
  public:
    DECLARE_CLASS(CTETFExplosion, CBaseTempEntity);
    DECLARE_SERVERCLASS();

    CTETFExplosion(const char *name);

  public:
    Vector m_vecOrigin;
    Vector m_vecNormal;
    WeaponID_t m_iWeaponID;
};

static CTETFExplosion g_TETFExplosion("TFExplosion");

CTETFExplosion::CTETFExplosion(const char *name) : CBaseTempEntity(name)
{
    m_vecOrigin.Init();
    m_vecNormal.Init();
    m_iWeaponID = WEAPON_NONE;
}

IMPLEMENT_SERVERCLASS_ST(CTETFExplosion, DT_TETFExplosion)
    SendPropVector(SENDINFO_NOCHECK(m_vecOrigin)),
    SendPropVector(SENDINFO_NOCHECK(m_vecNormal), 6, 0, -1.0f, 1.0f),
    SendPropInt(SENDINFO_NOCHECK(m_iWeaponID), Q_log2(WEAPON_MAX) + 1, SPROP_UNSIGNED),
END_SEND_TABLE()

void TE_TFExplosion(IRecipientFilter &filter, const Vector &vecOrigin, const Vector &vecNormal, WeaponID_t iWeaponID)
{
    VectorCopy(vecOrigin, g_TETFExplosion.m_vecOrigin);
    VectorCopy(vecNormal, g_TETFExplosion.m_vecNormal);
    g_TETFExplosion.m_iWeaponID = iWeaponID;

    // Send it over the wire
    g_TETFExplosion.Create(filter);
}

// TF2 Particle effects
class CTETFParticleEffect : public CBaseTempEntity
{
  public:
    DECLARE_CLASS(CTETFParticleEffect, CBaseTempEntity);
    DECLARE_SERVERCLASS();

    CTETFParticleEffect(const char *name);

    void Init();

  public:
    Vector m_vecOrigin;
    Vector m_vecStart;
    QAngle m_vecAngles;

    int m_iParticleSystemIndex;

    int m_nEntIndex;

    int m_iAttachType;
    int m_iAttachmentPointIndex;

    bool m_bResetParticles;
};

static CTETFParticleEffect g_TETFParticleEffect("TFParticleEffect");

CTETFParticleEffect::CTETFParticleEffect(const char *name) : CBaseTempEntity(name) { Init(); }

void CTETFParticleEffect::Init()
{
    m_vecOrigin.Init();
    m_vecStart.Init();
    m_vecAngles.Init();

    m_iParticleSystemIndex = 0;

    m_nEntIndex = -1;

    m_iAttachType = PATTACH_ABSORIGIN;
    m_iAttachmentPointIndex = 0;

    m_bResetParticles = false;
}

IMPLEMENT_SERVERCLASS_ST(CTETFParticleEffect, DT_TETFParticleEffect)
    SendPropVector(SENDINFO_NOCHECK(m_vecOrigin)),
    SendPropVector(SENDINFO_NOCHECK(m_vecStart)),
    SendPropQAngles(SENDINFO_NOCHECK(m_vecAngles), 7),
    SendPropInt(SENDINFO_NOCHECK(m_iParticleSystemIndex), 16, SPROP_UNSIGNED), // probably way too high
    SendPropInt(SENDINFO_NAME(m_nEntIndex, entindex), MAX_EDICT_BITS),
    SendPropInt(SENDINFO_NOCHECK(m_iAttachType), 5, SPROP_UNSIGNED),
    SendPropInt(SENDINFO_NOCHECK(m_iAttachmentPointIndex), Q_log2(MAX_PATTACH_TYPES) + 1, SPROP_UNSIGNED),
    SendPropBool(SENDINFO_NOCHECK(m_bResetParticles)),
END_SEND_TABLE()

void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName,
                         ParticleAttachment_t iAttachType, CBaseEntity *pEntity, const char *pszAttachmentName,
                         bool bResetAllParticlesOnEntity)
{
    int iAttachment = -1;
    if (pEntity && pEntity->GetBaseAnimating())
    {
        // Find the attachment point index
        iAttachment = pEntity->GetBaseAnimating()->LookupAttachment(pszAttachmentName);
        if (iAttachment == -1)
        {
            Warning("Model '%s' doesn't have attachment '%s' to attach particle system '%s' to.\n",
                    STRING(pEntity->GetBaseAnimating()->GetModelName()), pszAttachmentName, pszParticleName);
            return;
        }
    }
    TE_TFParticleEffect(filter, flDelay, pszParticleName, iAttachType, pEntity, iAttachment,
                        bResetAllParticlesOnEntity);
}

//-----------------------------------------------------------------------------
// Purpose: Yet another overload, lets us supply vecStart
//-----------------------------------------------------------------------------
void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName, Vector vecOrigin,
                         Vector vecStart, QAngle vecAngles, CBaseEntity *pEntity)
{
    int iIndex = GetParticleSystemIndex(pszParticleName);
    TE_TFParticleEffect(filter, flDelay, iIndex, vecOrigin, vecStart, vecAngles, pEntity);
}

void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName,
                         ParticleAttachment_t iAttachType, CBaseEntity *pEntity, int iAttachmentPoint,
                         bool bResetAllParticlesOnEntity)
{
    g_TETFParticleEffect.Init();

    g_TETFParticleEffect.m_iParticleSystemIndex = GetParticleSystemIndex(pszParticleName);
    if (pEntity)
    {
        g_TETFParticleEffect.m_nEntIndex = pEntity->entindex();
    }

    g_TETFParticleEffect.m_iAttachType = iAttachType;
    g_TETFParticleEffect.m_iAttachmentPointIndex = iAttachmentPoint;

    if (bResetAllParticlesOnEntity)
    {
        g_TETFParticleEffect.m_bResetParticles = true;
    }

    g_TETFParticleEffect.Create(filter, flDelay);
}

void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, const char *pszParticleName, Vector vecOrigin,
                         QAngle vecAngles, CBaseEntity *pEntity /*= NULL*/, int iAttachType /*= PATTACH_CUSTOMORIGIN*/)
{
    g_TETFParticleEffect.Init();

    g_TETFParticleEffect.m_iParticleSystemIndex = GetParticleSystemIndex(pszParticleName);

    VectorCopy(vecOrigin, g_TETFParticleEffect.m_vecOrigin);
    VectorCopy(vecAngles, g_TETFParticleEffect.m_vecAngles);

    if (pEntity)
    {
        g_TETFParticleEffect.m_nEntIndex = pEntity->entindex();
        g_TETFParticleEffect.m_iAttachType = iAttachType;
    }

    g_TETFParticleEffect.Create(filter, flDelay);
}

void TE_TFParticleEffect(IRecipientFilter &filter, float flDelay, int iEffectIndex, Vector vecOrigin, Vector vecStart,
                         QAngle vecAngles, CBaseEntity *pEntity)
{
    g_TETFParticleEffect.Init();

    g_TETFParticleEffect.m_iParticleSystemIndex = iEffectIndex;

    VectorCopy(vecOrigin, g_TETFParticleEffect.m_vecOrigin);
    VectorCopy(vecStart, g_TETFParticleEffect.m_vecStart);
    VectorCopy(vecAngles, g_TETFParticleEffect.m_vecAngles);

    if (pEntity)
    {
        g_TETFParticleEffect.m_nEntIndex = pEntity->entindex();
        g_TETFParticleEffect.m_iAttachType = PATTACH_CUSTOMORIGIN;
    }

    g_TETFParticleEffect.Create(filter, flDelay);
}
#endif