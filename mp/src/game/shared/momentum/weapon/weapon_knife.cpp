#include "cbase.h"
#include "effect_dispatch_data.h"
#include "mom_player_shared.h"
#include "util/mom_util.h"
#include "weapon_knife.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#include "momentum/ghost_client.h"
#endif

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Only send to local player if this weapon is the active weapon
// Input  : *pStruct -
//			*pVarData -
//			*pRecipients -
//			objectID -
// Output : void*
//-----------------------------------------------------------------------------
void *SendProxy_SendActiveLocalKnifeDataTable(const SendProp *pProp, const void *pStruct, const void *pVarData,
                                              CSendProxyRecipients *pRecipients, int objectID)
{
    // Get the weapon entity
    CBaseCombatWeapon *pWeapon = (CBaseCombatWeapon *)pVarData;
    if (pWeapon)
    {
        // Only send this chunk of data to the player carrying this weapon
        CBasePlayer *pPlayer = ToBasePlayer(pWeapon->GetOwner());
        if (pPlayer /*&& pPlayer->GetActiveWeapon() == pWeapon*/)
        {
            pRecipients->SetOnly(pPlayer->GetClientIndex());
            return (void *)pVarData;
        }
    }

    return nullptr;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendActiveLocalKnifeDataTable);
#endif

// ----------------------------------------------------------------------------- //
// CKnife tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED(Knife, DT_WeaponKnife)

BEGIN_NETWORK_TABLE_NOBASE(CKnife, DT_LocalActiveWeaponKnifeData)
#ifdef GAME_DLL
    SendPropTime(SENDINFO(m_flSmackTime)),
#else
    RecvPropTime(RECVINFO(m_flSmackTime)),
#endif
END_NETWORK_TABLE();

BEGIN_NETWORK_TABLE(CKnife, DT_WeaponKnife)
#ifdef GAME_DLL
    SendPropDataTable("LocalActiveWeaponKnifeData", 0, &REFERENCE_SEND_TABLE(DT_LocalActiveWeaponKnifeData), SendProxy_SendActiveLocalKnifeDataTable),
#else
    RecvPropDataTable("LocalActiveWeaponKnifeData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalActiveWeaponKnifeData)),
#endif
END_NETWORK_TABLE();

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CKnife)
    DEFINE_PRED_FIELD(m_flSmackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA();
#endif

LINK_ENTITY_TO_CLASS(weapon_knife, CKnife);
PRECACHE_WEAPON_REGISTER(weapon_knife);

#ifndef CLIENT_DLL

BEGIN_DATADESC(CKnife)
DEFINE_FUNCTION(Smack)
END_DATADESC()

#endif

// ----------------------------------------------------------------------------- //
// CKnife implementation.
// ----------------------------------------------------------------------------- //

CKnife::CKnife()
{
    m_bStab = false;
}

bool CKnife::HasPrimaryAmmo() { return true; }

bool CKnife::CanBeSelected() { return true; }

void CKnife::Precache()
{
    BaseClass::Precache();

    PrecacheScriptSound("Weapon_Knife.Deploy");
    PrecacheScriptSound("Weapon_Knife.Slash");
    PrecacheScriptSound("Weapon_Knife.Stab");
    PrecacheScriptSound("Weapon_Knife.Hit");
    PrecacheScriptSound("Weapon_Knife.HitWall");
}

void CKnife::Spawn()
{
    Precache();

    m_iClip1 = -1;
    BaseClass::Spawn();
}

bool CKnife::Deploy()
{
    CPASAttenuationFilter filter(GetOwner()->EarPosition());
    filter.UsePredictionRules();
    EmitSound(filter, GetOwner()->entindex(), "Weapon_Knife.Deploy");

    return BaseClass::Deploy();
}

void CKnife::Holster(int skiplocal)
{
    if (GetPlayerOwner())
    {
        GetPlayerOwner()->m_flNextAttack = gpGlobals->curtime + 0.5f;
    }
}

void CKnife::PrimaryAttack() { DoAttack(false); }

void CKnife::SecondaryAttack() { DoAttack(true); }

void CKnife::DoAttack(bool bIsSecondary)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (pPlayer)
    {
#if !defined(CLIENT_DLL)
        // Move other players back to history positions based on local player's lag
        lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());
#endif
        SwingOrStab(bIsSecondary);
#if !defined(CLIENT_DLL)
        lagcompensation->FinishLagCompensation(pPlayer);
#endif
    }
}

void CKnife::Smack(void)
{
    if (!GetPlayerOwner())
        return;

    m_trHit.m_pEnt = m_pTraceHitEnt;

    MomUtil::KnifeSmack(m_trHit, this, GetPlayerOwner()->GetAbsAngles(), m_bStab);
}

void CKnife::WeaponIdle()
{
    if (m_flTimeWeaponIdle > gpGlobals->curtime)
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    SetWeaponIdleTime(gpGlobals->curtime + 20);

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
    MomUtil::KnifeTrace(pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), bStab, pPlayer, this, &tr,
                                &vForward);

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
    SetWeaponIdleTime(gpGlobals->curtime + 2);

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

void CKnife::ItemPostFrame(void)
{
    if (m_flSmackTime > 0 && gpGlobals->curtime > m_flSmackTime)
    {
        Smack();
        m_flSmackTime = -1;
    }

    BaseClass::ItemPostFrame();
}