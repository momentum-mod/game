#pragma once

// Ammo is infinite
#define INFINITE_AMMO (-2)

enum CWeaponID
{
    WEAPON_NONE = 0,

    WEAPON_PISTOL,
    WEAPON_RIFLE,
    WEAPON_SHOTGUN,
    WEAPON_SMG,
    WEAPON_SNIPER,
    WEAPON_LMG,
    WEAPON_GRENADE,
    WEAPON_KNIFE,
    WEAPON_PAINTGUN,
    WEAPON_ROCKETLAUNCHER,

    WEAPON_MAX, // number of weapons weapon index
};

enum AmmoTracer_t
{
    TRACER_NONE,
    TRACER_LINE,
    TRACER_RAIL,
    TRACER_BEAM,
    TRACER_LINE_AND_WHIZ,
};

enum AmmoType_t
{
    AMMO_TYPE_NONE = -1,
    AMMO_TYPE_PISTOL,
    AMMO_TYPE_SMG,
    AMMO_TYPE_RIFLE,
    AMMO_TYPE_SNIPER,
    AMMO_TYPE_LMG,
    AMMO_TYPE_SHOTGUN,
    AMMO_TYPE_GRENADE,
    AMMO_TYPE_PAINT,

    AMMO_TYPE_MAX
};

class CAmmoBase
{
public:
    CAmmoBase();

    CWeaponID m_WeaponID;

    int     m_iDamageType;
    int     m_iDamageAmount;
    int     m_eTracerType;
    float   m_fDamageForce;
    int     m_iMinSplashSize;
    int     m_iMaxSplashSize;
    int     m_iMaxCarry;
    int     m_iPenetrationAmount;
    float   m_fPenetrationPower;
    float   m_fPenetrationDistance;
    float   m_fRange;
    float   m_fRangeModifier;
    int     m_iNumBullets; // Number of bullets fired per shot
};

#define MAKE_AMMO_CLASS( ammoClassName ) class ammoClassName : public CAmmoBase { public: ammoClassName(); };
#define MAKE_AMMODEF_GETTER(type, funcName, memberVar) \
type funcName(int iAmmoIndex) \
{ \
    if (iAmmoIndex < 0 || iAmmoIndex >= AMMO_TYPE_MAX) \
        return (type)0; \
    return m_AmmoTypes[iAmmoIndex]->memberVar; \
}

MAKE_AMMO_CLASS(CAmmoPistol);
MAKE_AMMO_CLASS(CAmmoSMG);
MAKE_AMMO_CLASS(CAmmoRifle);
MAKE_AMMO_CLASS(CAmmoSniper);
MAKE_AMMO_CLASS(CAmmoLMG);
MAKE_AMMO_CLASS(CAmmoShotgun);
MAKE_AMMO_CLASS(CAmmoGrenade);
MAKE_AMMO_CLASS(CAmmoPaint);

class CAmmoDef
{
  public:
    CAmmoDef();
    virtual ~CAmmoDef();

    CAmmoBase *GetAmmoOfIndex(int nAmmoIndex);
    MAKE_AMMODEF_GETTER(CWeaponID, WeaponID, m_WeaponID);
    MAKE_AMMODEF_GETTER(int, DamageType, m_iDamageType);
    MAKE_AMMODEF_GETTER(int, DamageAmount, m_iDamageAmount);
    MAKE_AMMODEF_GETTER(int, TracerType, m_eTracerType);
    MAKE_AMMODEF_GETTER(float, DamageForce, m_fDamageForce);
    MAKE_AMMODEF_GETTER(int, MinSplashSize, m_iMinSplashSize);
    MAKE_AMMODEF_GETTER(int, MaxSplashSize, m_iMaxSplashSize);
    MAKE_AMMODEF_GETTER(int, MaxCarry, m_iMaxCarry);
    MAKE_AMMODEF_GETTER(int, PenetrationAmount, m_iPenetrationAmount);
    MAKE_AMMODEF_GETTER(float, PenetrationPower, m_fPenetrationPower);
    MAKE_AMMODEF_GETTER(float, PenetrationDistance, m_fPenetrationDistance);
    MAKE_AMMODEF_GETTER(float, Range, m_fRange);
    MAKE_AMMODEF_GETTER(float, RangeModifier, m_fRangeModifier);
    MAKE_AMMODEF_GETTER(int, NumBullets, m_iNumBullets);

  private:
    CUtlVector<CAmmoBase*> m_AmmoTypes;
};

extern CAmmoDef *g_pAmmoDef;