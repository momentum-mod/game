#include "cbase.h" 
#include "weapon_csbase.h"
#include "weapon_mom_pistol.h"


#ifdef CLIENT_DLL
#include "c_te_effect_dispatch.h"
#define CDEagle C_DEagle
#define CWeaponElite C_WeaponElite
#define CWeaponFiveSeven C_WeaponFiveSeven
#define CWeaponGlock C_WeaponGlock
#define CWeaponP228 C_WeaponP228
#define CWeaponUSP C_WeaponUSP
#endif

#include "tier0/memdbgon.h"


class CDEagle : public CMomentumPistol
{
public:
    DECLARE_CLASS(CDEagle, CMomentumPistol);
    DECLARE_PREDICTABLE();

    CDEagle() {};
};

BEGIN_PREDICTION_DATA(CDEagle)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_deagle, CDEagle);
PRECACHE_WEAPON_REGISTER(weapon_deagle);


/*
ELITES
*/
class CWeaponElite : public CMomentumPistol
{
public:
    DECLARE_CLASS(CWeaponElite, CMomentumPistol);
    DECLARE_PREDICTABLE();

    CWeaponElite() {};
};

BEGIN_PREDICTION_DATA(CWeaponElite)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_elite, CWeaponElite);
PRECACHE_WEAPON_REGISTER(weapon_elite);

// FIVESEVEN
class CWeaponFiveSeven : public CMomentumPistol
{
public:
    DECLARE_CLASS(CWeaponFiveSeven, CMomentumPistol);
    DECLARE_PREDICTABLE();

    CWeaponFiveSeven() {};
};

BEGIN_PREDICTION_DATA(CWeaponFiveSeven)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_fiveseven, CWeaponFiveSeven);
PRECACHE_WEAPON_REGISTER(weapon_fiveseven);

//GLOCK
class CWeaponGlock : public CMomentumPistol
{
public:
    DECLARE_CLASS(CWeaponGlock, CMomentumPistol);
    DECLARE_PREDICTABLE();

    CWeaponGlock() {};
};

BEGIN_PREDICTION_DATA(CWeaponGlock)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_glock, CWeaponGlock);
PRECACHE_WEAPON_REGISTER(weapon_glock);


//P228
class CWeaponP228 : public CMomentumPistol
{
public:
    DECLARE_CLASS(CWeaponP228, CMomentumPistol);
    DECLARE_PREDICTABLE();

    CWeaponP228() {};
};

BEGIN_PREDICTION_DATA(CWeaponP228)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_p228, CWeaponP228);
PRECACHE_WEAPON_REGISTER(weapon_p228);

//USP
class CWeaponUSP : public CMomentumPistol
{
public:
    DECLARE_CLASS(CWeaponUSP, CMomentumPistol);
    DECLARE_PREDICTABLE();

    CWeaponUSP() {};
};

BEGIN_PREDICTION_DATA(CWeaponUSP)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_usp, CWeaponUSP);
PRECACHE_WEAPON_REGISTER(weapon_usp);