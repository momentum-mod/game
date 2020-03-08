#pragma once

#include "weapon_base.h"

#if defined(CLIENT_DLL)
#define CKnife C_Knife
#endif

// ----------------------------------------------------------------------------- //
// CKnife class definition.
// ----------------------------------------------------------------------------- //

class CKnife : public CWeaponBase
{
  public:
    DECLARE_CLASS(CKnife, CWeaponBase);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CKnife();

    // We say yes to this so the weapon system lets us switch to it.
    bool HasPrimaryAmmo() OVERRIDE { return true; }
    bool CanBeSelected() OVERRIDE { return true; }

    void Smack();
    bool SwingOrStab(bool bStab);
    void DoAttack(bool bIsSecondary);
    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;

    void ItemPostFrame() OVERRIDE;

    void WeaponIdle() OVERRIDE;

    WeaponID_t GetWeaponID() const OVERRIDE { return WEAPON_KNIFE; }

    static void KnifeTrace(const Vector &vecShootPos, const QAngle &lookAng, bool bStab, CBaseEntity *pAttacker, CBaseEntity *pSoundSource, trace_t *trOutput, Vector *vForwardOut);
    static void KnifeSmack(const trace_t &tr_in, CBaseEntity *pSoundSource, const QAngle &lookAng, const bool bStab);

private:
    trace_t m_trHit;
    EHANDLE m_pTraceHitEnt;

    CNetworkVar(float, m_flSmackTime);
    bool m_bStab;
};