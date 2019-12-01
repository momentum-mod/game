#include "cbase.h"
#include "ammodef.h"

#include "tier0/memdbgon.h"

CAmmoBase *CAmmoDef::GetAmmoOfIndex(int nAmmoIndex)
{
    if (nAmmoIndex < 0 || nAmmoIndex >= AMMO_TYPE_MAX)
        return nullptr;

    return m_AmmoTypes[nAmmoIndex];
}

CAmmoBase::CAmmoBase()
{
    m_WeaponID = WEAPON_NONE;
    m_iDamageType = DMG_BULLET;
    m_iDamageAmount = 42;
    m_eTracerType = TRACER_LINE;
    m_fDamageForce = 2400.0f;
    m_iMinSplashSize = 10;
    m_iMaxSplashSize = 14;
    m_iMaxCarry = -2;
    m_iPenetrationAmount = 1;
    m_fPenetrationPower = 0.0f;
    m_fPenetrationDistance = 0.0f;
    m_fRange = 8192.0f;
    m_fRangeModifier = 1.0f;
    m_iNumBullets = 1;
}

CAmmoPistol::CAmmoPistol()
{
    m_WeaponID = WEAPON_PISTOL;
    m_iDamageAmount = 25;
    m_fDamageForce = 2000.0f;
    m_iMinSplashSize = 5;
    m_iMaxSplashSize = 10;
    m_fPenetrationPower = 21.0f;
    m_fPenetrationDistance = 800.0f;
    m_fRangeModifier = 0.75f;
}

CAmmoSMG::CAmmoSMG()
{
    m_WeaponID = WEAPON_SMG;
    m_iDamageAmount = 26;
    m_iMinSplashSize = 4;
    m_iMaxSplashSize = 8;
    m_fPenetrationPower = 30.0f;
    m_fPenetrationDistance = 2000.0f;
    m_fRange = 4096.0f;
    m_fRangeModifier = 0.84f;
}

CAmmoRifle::CAmmoRifle()
{
    m_WeaponID = WEAPON_RIFLE;
    m_iDamageAmount = 36;
    m_iPenetrationAmount = 2;
    m_fPenetrationPower = 39.0f;
    m_fPenetrationDistance = 5000.0f;
    m_fRangeModifier = 0.98f;
}

CAmmoSniper::CAmmoSniper()
{
    m_WeaponID = WEAPON_SNIPER;
    m_iDamageAmount = 75;
    m_iPenetrationAmount = 3;
    m_fPenetrationPower = 39.0f;
    m_fPenetrationDistance = 5000.0f;
    m_fRangeModifier = 0.98f;
}

CAmmoShotgun::CAmmoShotgun()
{
    m_WeaponID = WEAPON_SHOTGUN;
    m_iDamageAmount = 22;
    m_fDamageForce = 600.0f;
    m_iMinSplashSize = 3;
    m_iMaxSplashSize = 6;
    m_fRangeModifier = 0.70f;
    m_iNumBullets = 9;
}

CAmmoLMG::CAmmoLMG()
{
    m_WeaponID = WEAPON_LMG;
    m_iDamageAmount = 35;
    m_iPenetrationAmount = 2;
    m_fPenetrationPower = 35.0f;
    m_fPenetrationDistance = 4000.0f;
    m_fRangeModifier = 0.97f;
}

CAmmoGrenade::CAmmoGrenade()
{
    m_WeaponID = WEAPON_GRENADE;
    m_iDamageType = DMG_BLAST;
}

CAmmoPaint::CAmmoPaint()
{
    m_WeaponID = WEAPON_PAINTGUN;
    m_iDamageAmount = 1;
    m_iPenetrationAmount = 0;
}


CAmmoDef::CAmmoDef()
{
    m_AmmoTypes.EnsureCount(AMMO_TYPE_MAX);
    m_AmmoTypes[AMMO_TYPE_PISTOL] = new CAmmoPistol;
    m_AmmoTypes[AMMO_TYPE_SMG] = new CAmmoSMG;
    m_AmmoTypes[AMMO_TYPE_RIFLE] = new CAmmoRifle;
    m_AmmoTypes[AMMO_TYPE_SNIPER] = new CAmmoSniper;
    m_AmmoTypes[AMMO_TYPE_LMG] = new CAmmoLMG;
    m_AmmoTypes[AMMO_TYPE_SHOTGUN] = new CAmmoShotgun;
    m_AmmoTypes[AMMO_TYPE_GRENADE] = new CAmmoGrenade;
    m_AmmoTypes[AMMO_TYPE_PAINT] = new CAmmoPaint;
}

CAmmoDef::~CAmmoDef()
{
    m_AmmoTypes.PurgeAndDeleteElements();
}

static CAmmoDef s_AmmoDef;
CAmmoDef *g_pAmmoDef = &s_AmmoDef;