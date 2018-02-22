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

CMomentumSniper::CMomentumSniper()
{
    m_flIdleInterval = 60.0f;
    m_flTimeToIdleAfterFire = 1.8f;
    m_iRequestedFOV = 0;
}

void CMomentumSniper::SecondaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;
    
    int iFOV = pPlayer->GetFOV();
    float flRate = 0.05f;

    if (iFOV >= pPlayer->GetDefaultFOV())
    {
        m_iRequestedFOV = 40;
        flRate = 0.15f; // Initial zoom is slower than others
    }
    else if (iFOV >= 40)
    {
        m_iRequestedFOV = 15;
    }
    else if (iFOV >= 15)
    {
        m_iRequestedFOV = pPlayer->GetDefaultFOV();
    }

#ifndef CLIENT_DLL

    pPlayer->SetFOV(pPlayer, m_iRequestedFOV, flRate);

    // MOM_TODO: Consider LJ gametype
    // pPlayer->ResetMaxSpeed();
#endif

    m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;
    m_zoomFullyActiveTime = gpGlobals->curtime + 0.15f; // The worst zoom time from above.

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
        pPlayer->m_SrvData.m_iLastZoom = m_iRequestedFOV;

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