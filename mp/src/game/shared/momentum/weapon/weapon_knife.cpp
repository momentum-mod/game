#include "cbase.h"

#include "weapon_knife.h"

#include "effect_dispatch_data.h"
#include "mom_player_shared.h"
#include "util/mom_util.h"
#include "weapon_def.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#include "momentum/ghost_client.h"
#endif

#include "tier0/memdbgon.h"

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

IMPLEMENT_NETWORKCLASS_ALIASED(Knife, DT_WeaponKnife);

BEGIN_NETWORK_TABLE(CKnife, DT_WeaponKnife)
#ifdef GAME_DLL
    SendPropTime(SENDINFO(m_flSmackTime)),
#else
    RecvPropTime(RECVINFO(m_flSmackTime)),
#endif
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CKnife)
    DEFINE_PRED_FIELD(m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();
#endif

LINK_ENTITY_TO_CLASS(weapon_knife, CKnife);
PRECACHE_WEAPON_REGISTER(weapon_knife);

CKnife::CKnife()
{
    m_bStab = false;
    m_iClip1 = -1;
}

void CKnife::PrimaryAttack() { DoAttack(false); }

void CKnife::SecondaryAttack() { DoAttack(true); }

void CKnife::DoAttack(bool bIsSecondary)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer)
    {
#ifndef CLIENT_DLL
        // Move other players back to history positions based on local player's lag
        lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());
#endif

        SwingOrStab(bIsSecondary);

#ifndef CLIENT_DLL
        lagcompensation->FinishLagCompensation(pPlayer);
#endif
    }
}

void CKnife::Smack()
{
    if (!GetPlayerOwner())
        return;

    m_trHit.m_pEnt = m_pTraceHitEnt;

    KnifeSmack(m_trHit, this, GetPlayerOwner()->GetAbsAngles(), m_bStab);
}

void CKnife::WeaponIdle()
{
    if (m_flTimeWeaponIdle > gpGlobals->curtime)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    SetWeaponIdleTime(gpGlobals->curtime + 20.0f);

    // only idle if the slid isn't back
    SendWeaponAnim(ACT_VM_IDLE);
}

bool CKnife::SwingOrStab(bool bStab)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return false;

    trace_t tr;
    Vector vForward;
    KnifeTrace(pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), bStab, pPlayer, this, &tr, &vForward);

    bool bDidHit = tr.fraction < 1.0f;

#ifndef CLIENT_DLL
    // MOM_TODO: Determine if we should pass this into KnifeTrace
    // bool bFirstSwing = (m_flNextPrimaryAttack + 0.4) < gpGlobals->curtime;

    DecalPacket packet = DecalPacket::Knife(pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), bStab);
    g_pMomentumGhostClient->SendDecalPacket(&packet);
#endif

    float fPrimDelay, fSecDelay;

    if (bStab)
    {
        fPrimDelay = fSecDelay = bDidHit ? 1.1f : 1.0f;
    }
    else // swing
    {
        fPrimDelay = bDidHit ? 0.5f : 0.4f;
        fSecDelay = bDidHit ? 0.5f : 0.4f;
    }

    // Send stab or swing anim
    SendWeaponAnim(bDidHit ? ACT_VM_HITCENTER : ACT_VM_MISSCENTER);

    m_flNextPrimaryAttack = gpGlobals->curtime + fPrimDelay;
    m_flNextSecondaryAttack = gpGlobals->curtime + fSecDelay;
    SetWeaponIdleTime(gpGlobals->curtime + 2.0f);

    if (bDidHit)
    {
#ifndef CLIENT_DLL
        // player "shoot" animation
        pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        // delay the decal a bit
        m_trHit = tr;

        // Store the ent in an EHANDLE, just in case it goes away by the time we get into our think function.
        m_pTraceHitEnt = tr.m_pEnt;

        m_bStab = bStab; // store this so we know what hit sound to play

        m_flSmackTime = gpGlobals->curtime + (bStab ? 0.2f : 0.1f);
    }

    return bDidHit;
}

void CKnife::ItemPostFrame()
{
    if (m_flSmackTime > 0.0f && gpGlobals->curtime > m_flSmackTime)
    {
        Smack();
        m_flSmackTime = -1;
    }

    BaseClass::ItemPostFrame();
}

inline void FindHullIntersection(const Vector &vecSrc, trace_t &tr, const Vector &mins, const Vector &maxs, CBaseEntity *pEntity)
{
    const Vector vecHullEnd = vecSrc + ((tr.endpos - vecSrc) * 2.0f);
    trace_t tmpTrace;
    UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &tmpTrace);
    if (tmpTrace.fraction < 1.0f)
    {
        tr = tmpTrace;
        return;
    }

    Vector minmaxs[2] = { mins, maxs };
    Vector vecEnd;
    float distance = 1e6f;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                vecEnd.x = vecHullEnd.x + minmaxs[i][0];
                vecEnd.y = vecHullEnd.y + minmaxs[j][1];
                vecEnd.z = vecHullEnd.z + minmaxs[k][2];

                UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pEntity, COLLISION_GROUP_NONE, &tmpTrace);
                if (tmpTrace.fraction < 1.0f)
                {
                    float thisDistance = (tmpTrace.endpos - vecSrc).Length();
                    if (thisDistance < distance)
                    {
                        tr = tmpTrace;
                        distance = thisDistance;
                    }
                }
            }
        }
    }
}

void CKnife::KnifeTrace(const Vector &vecShootPos, const QAngle &lookAng, bool bStab, CBaseEntity *pAttacker,
                        CBaseEntity *pSoundSource, trace_t *trOutput, Vector *vForwardOut)
{
    float fRange = bStab ? 32.0f : 48.0f; // knife range

    AngleVectors(lookAng, vForwardOut);
    Vector vecSrc = vecShootPos;
    Vector vecEnd = vecSrc + *vForwardOut * fRange;

    UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pAttacker, COLLISION_GROUP_NONE, trOutput);

#ifndef CLIENT_DLL
    if (pAttacker->IsPlayer())
    {
        //check for hitting glass
        CTakeDamageInfo glassDamage(pAttacker, pAttacker, 42.0f, DMG_BULLET | DMG_NEVERGIB);
        pSoundSource->TraceAttackToTriggers(glassDamage, trOutput->startpos, trOutput->endpos, *vForwardOut);
    }
#endif

    if (trOutput->fraction >= 1.0f)
    {
        Vector head_hull_mins(-16, -16, -18);
        Vector head_hull_maxs(16, 16, 18);
        UTIL_TraceHull(vecSrc, vecEnd, head_hull_mins, head_hull_maxs, MASK_SOLID, pAttacker, COLLISION_GROUP_NONE, trOutput);
        if (trOutput->fraction < 1.0f)
        {
            // Calculate the point of intersection of the line (or hull) and the object we hit
            // This is and approximation of the "best" intersection
            CBaseEntity *pHit = trOutput->m_pEnt;
            if (!pHit || pHit->IsBSPModel())
                FindHullIntersection(vecSrc, *trOutput, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, pAttacker);
            //vecEnd = trOutput->endpos; // This is the point on the actual surface (the hull could have hit space)
        }
    }

    bool bDidHit = trOutput->fraction < 1.0f;

    if (!bDidHit)
    {
        const auto pSound = g_pWeaponDef->GetWeaponSound(WEAPON_KNIFE, "swish");

        CPASAttenuationFilter filter(trOutput->endpos, pSound);
        filter.UsePredictionRules();
        EmitSound(filter, pSoundSource->entindex(), pSound);
    }
#ifndef CLIENT_DLL
    else if (pAttacker->IsPlayer())
    {
        CMomentumPlayer *pPlayer = static_cast<CMomentumPlayer *>(pAttacker);
        if (pPlayer->m_bHasPracticeMode)
            return;

        CBaseEntity *pHitEntity = trOutput->m_pEnt;

        ClearMultiDamage();

        const float flDamage = bStab ? 65.0f : 20.0f;

        CTakeDamageInfo info(pAttacker, pAttacker, flDamage, DMG_BULLET | DMG_NEVERGIB);

        CalculateMeleeDamageForce(&info, *vForwardOut, trOutput->endpos, 1.0f / flDamage);
        pHitEntity->DispatchTraceAttack(info, *vForwardOut, trOutput);
        ApplyMultiDamage();
    }
#endif
}

void CKnife::KnifeSmack(const trace_t &trIn, CBaseEntity *pSoundSource, const QAngle &lookAng, const bool bStab)
{
    if (!trIn.m_pEnt || (trIn.surface.flags & SURF_SKY))
        return;

    if (trIn.fraction == 1.0f)
        return;

    if (trIn.m_pEnt)
    {
        CPASAttenuationFilter filter(trIn.endpos);
        filter.UsePredictionRules();

        const auto pWepScript = g_pWeaponDef->GetWeaponScript(WEAPON_KNIFE);
        const char *pSound = pWepScript->pKVWeaponSounds->GetString("hit_wall");

        const auto pRunEnt = dynamic_cast<CMomRunEntity *>(trIn.m_pEnt);
        if (pRunEnt)
        {
            pSound = pWepScript->pKVWeaponSounds->GetString(bStab ? "stab" : "hit");
        }

        EmitSound(filter, pSoundSource->entindex(), pSound);
    }

    CEffectData data;
    data.m_vOrigin = trIn.endpos;
    data.m_vStart = trIn.startpos;
    data.m_nSurfaceProp = trIn.surface.surfaceProps;
    data.m_nDamageType = DMG_SLASH;
    data.m_nHitBox = trIn.hitbox;
#ifdef CLIENT_DLL
    data.m_hEntity = trIn.m_pEnt->GetRefEHandle();
#else
    data.m_nEntIndex = trIn.m_pEnt->entindex();
#endif

    CPASFilter filter(data.m_vOrigin);
    data.m_vAngles = lookAng;
    data.m_fFlags = 0x1; //IMPACT_NODECAL;

    te->DispatchEffect(filter, 0.0f, data.m_vOrigin, "KnifeSlash", data);
}