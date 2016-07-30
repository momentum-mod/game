#include "cbase.h"
#include "weapon_csbasegun.h"
#include "weapon_mom_smg.h"

#include "tier0/memdbgon.h"

#if defined( CLIENT_DLL )
#define CWeaponMAC10 C_WeaponMAC10
#define CWeaponMP5Navy C_WeaponMP5Navy
#define CWeaponP90 C_WeaponP90
#define CWeaponTMP C_WeaponTMP
#define CWeaponUMP45 C_WeaponUMP45
#endif

//MAC10
class CWeaponMAC10 : public CMomentumSMG
{
public:
    DECLARE_CLASS(CWeaponMAC10, CMomentumSMG);
    DECLARE_PREDICTABLE();

    CWeaponMAC10() {}
};

BEGIN_PREDICTION_DATA(CWeaponMAC10)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_mac10, CWeaponMAC10);
PRECACHE_WEAPON_REGISTER(weapon_mac10);

//MP5NAVY
class CWeaponMP5Navy : public CMomentumSMG
{
public:
    DECLARE_CLASS(CWeaponMP5Navy, CMomentumSMG);
    DECLARE_PREDICTABLE();

    CWeaponMP5Navy() {}
};

BEGIN_PREDICTION_DATA(CWeaponMP5Navy)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_mp5navy, CWeaponMP5Navy);
PRECACHE_WEAPON_REGISTER(weapon_mp5navy);


//P90
class CWeaponP90 : public CMomentumSMG
{
public:
    DECLARE_CLASS(CWeaponP90, CMomentumSMG);
    DECLARE_PREDICTABLE();

    CWeaponP90() {}
};

BEGIN_PREDICTION_DATA(CWeaponP90)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_p90, CWeaponP90);
PRECACHE_WEAPON_REGISTER(weapon_p90);

//TMP
class CWeaponTMP : public CMomentumSMG
{
public:
    DECLARE_CLASS(CWeaponTMP, CMomentumSMG);
    DECLARE_PREDICTABLE();

    CWeaponTMP() {}
};

BEGIN_PREDICTION_DATA(CWeaponTMP)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_tmp, CWeaponTMP);
PRECACHE_WEAPON_REGISTER(weapon_tmp);

//UMP45
class CWeaponUMP45 : public CMomentumSMG
{
public:
    DECLARE_CLASS(CWeaponUMP45, CMomentumSMG);
    DECLARE_PREDICTABLE();

    CWeaponUMP45() {}
};

BEGIN_PREDICTION_DATA(CWeaponUMP45)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_ump45, CWeaponUMP45);
PRECACHE_WEAPON_REGISTER(weapon_ump45);