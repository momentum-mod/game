#include "cbase.h"
#include "weapon_mom_sniper.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumSniper, DT_MomentumSniper);

BEGIN_NETWORK_TABLE(CMomentumSniper, DT_MomentumSniper)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumSniper)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_sniper, CMomentumSniper);
PRECACHE_WEAPON_REGISTER(weapon_momentum_sniper);

void CMomentumSniper::SecondaryAttack()
{
#ifndef CLIENT_DLL
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
    {
        Assert(pPlayer != nullptr);
        return;
    }

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
    {
        pPlayer->SetFOV(pPlayer, 40, 0.15f);
    }
    else if (pPlayer->GetFOV() == 40)
    {
        pPlayer->SetFOV(pPlayer, 15, 0.05);
    }
    else if (pPlayer->GetFOV() == 15)
    {
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.05f);
    }

    // MOM_TODO: Consider LJ gametype
    // pPlayer->ResetMaxSpeed();
#endif

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
    m_zoomFullyActiveTime = gpGlobals->curtime + 0.15; // The worst zoom time from above.

#ifndef CLIENT_DLL
    // If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
    // Let the server play it since if only the client plays it, it's liable to get played twice cause of
    // a prediction error. joy.
    EmitSound("Default.Zoom"); // zoom sound
#endif
}

void CMomentumSniper::PrimaryAttack(void) { SniperFire(); }

void CMomentumSniper::SniperFire()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (pPlayer->GetFOV() != pPlayer->GetDefaultFOV())
    {
        pPlayer->m_SrvData.m_bResumeZoom = true;
        pPlayer->m_SrvData.m_iLastZoom = pPlayer->GetFOV();

#ifndef CLIENT_DLL
        pPlayer->SetFOV(pPlayer, pPlayer->GetDefaultFOV(), 0.05f);
#endif
    }

    if (!CSBaseGunFire(0.0f, 1.25f, true))
        return;

    QAngle angle = pPlayer->GetPunchAngle();
    angle.x -= 2;
    pPlayer->SetPunchAngle(angle);
}

// MOM_TODO: Consider LJ gametype
float CMomentumSniper::GetMaxSpeed() const
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
    {
        Assert(pPlayer != nullptr);
        return BaseClass::GetMaxSpeed();
    }

    if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
        return BaseClass::GetMaxSpeed();

    return 220; // zoomed in.
}