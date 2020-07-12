#include "cbase.h"
#include "weapon_mom_sniper.h"
#include "weapon_mom_pistol.h"
#include "weapon_mom_shotgun.h"
#include "weapon_mom_smg.h"
#include "weapon_mom_grenade.h"

#include "tier0/memdbgon.h"

// Macro used to override the CS weapons
#ifdef CLIENT_DLL
#define CS_WEP_OVERRIDE(csGun, momGun, entName, momEntName)                                                            \
    class C_##csGun : public momGun                                                                                    \
    {                                                                                                                  \
      public:                                                                                                          \
        DECLARE_CLASS(C_##csGun, momGun);                                                                              \
        DECLARE_PREDICTABLE();                                                                                         \
        C_##csGun(){};                                                                                                 \
                                                                                                                       \
      private:                                                                                                         \
        C_##csGun(const C_##csGun &){};                                                                                \
    };                                                                                                                 \
    BEGIN_PREDICTION_DATA(C_##csGun)                                                                                   \
    END_PREDICTION_DATA()                                                                                              \
    LINK_ENTITY_TO_CLASS(entName, C_##csGun)                                                                           \
    PRECACHE_WEAPON_REGISTER(entName)
#else
#define CS_WEP_OVERRIDE(csGun, momGun, entName, momEntName)                                                            \
    class csGun : public momGun                                                                                        \
    {                                                                                                                  \
      public:                                                                                                          \
        DECLARE_CLASS(csGun, momGun);                                                                                  \
        DECLARE_PREDICTABLE();                                                                                         \
        csGun(){};                                                                                                     \
        void PostConstructor( const char *pszClassname ) OVERRIDE { BaseClass::PostConstructor(#momEntName); }         \
                                                                                                                       \
      private:                                                                                                         \
        csGun(const csGun &){};                                                                                        \
    };                                                                                                                 \
    BEGIN_PREDICTION_DATA(csGun)                                                                                       \
    END_PREDICTION_DATA()                                                                                              \
    LINK_ENTITY_TO_CLASS(entName, csGun)                                                                               \
    PRECACHE_WEAPON_REGISTER(entName)
#endif

//Rifles
CS_WEP_OVERRIDE(CAK47,        CMomentumMachinegun, weapon_ak47,  weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponAug,   CMomentumMachinegun, weapon_aug,   weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponFamas, CMomentumMachinegun, weapon_famas, weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponGalil, CMomentumMachinegun, weapon_galil, weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponM4A1,  CMomentumMachinegun, weapon_m4a1,  weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponSG552, CMomentumMachinegun, weapon_sg552, weapon_momentum_machinegun);

//Snipers
CS_WEP_OVERRIDE(CWeaponScout, CMomentumSniper, weapon_scout, weapon_momentum_sniper);
CS_WEP_OVERRIDE(CWeaponG3SG1, CMomentumSniper, weapon_g3sg1, weapon_momentum_sniper);
CS_WEP_OVERRIDE(CWeaponSG550, CMomentumSniper, weapon_sg550, weapon_momentum_sniper);
CS_WEP_OVERRIDE(CWeaponAWP,   CMomentumSniper, weapon_awp,   weapon_momentum_sniper);

//Pistols
CS_WEP_OVERRIDE(CDeagle,            CMomentumPistol, weapon_deagle,     weapon_momentum_pistol);
CS_WEP_OVERRIDE(CWeaponElite,       CMomentumPistol, weapon_elite,      weapon_momentum_pistol);
CS_WEP_OVERRIDE(CWeaponFiveSeven,   CMomentumPistol, weapon_fiveseven,  weapon_momentum_pistol);
CS_WEP_OVERRIDE(CWeaponGlock,       CMomentumPistol, weapon_glock,      weapon_momentum_pistol);
CS_WEP_OVERRIDE(CWeaponP228,        CMomentumPistol, weapon_p228,       weapon_momentum_pistol);
CS_WEP_OVERRIDE(CWeaponUSP,         CMomentumPistol, weapon_usp,        weapon_momentum_pistol);

//SMGs
CS_WEP_OVERRIDE(CWeaponMAC10,   CMomentumMachinegun, weapon_mac10,     weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponMP5Navy, CMomentumMachinegun, weapon_mp5navy,   weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponP90,     CMomentumMachinegun, weapon_p90,       weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponTMP,     CMomentumMachinegun, weapon_tmp,       weapon_momentum_machinegun);
CS_WEP_OVERRIDE(CWeaponUMP45,   CMomentumMachinegun, weapon_ump45,     weapon_momentum_machinegun);

//Shotguns
CS_WEP_OVERRIDE(CWeaponXM1014,  CMomentumShotgun, weapon_xm1014,    weapon_momentum_shotgun);
CS_WEP_OVERRIDE(CWeaponM3,      CMomentumShotgun, weapon_m3,        weapon_momentum_shotgun);

//LMG
CS_WEP_OVERRIDE(CWeaponM249, CMomentumMachinegun, weapon_m249, weapon_momentum_machinegun);

//Grenades
CS_WEP_OVERRIDE(CHEGrenade,     CMomentumGrenade, weapon_hegrenade,     weapon_momentum_grenade);
CS_WEP_OVERRIDE(CFlashbang,     CMomentumGrenade, weapon_flashbang,     weapon_momentum_grenade);
CS_WEP_OVERRIDE(CSmokeGrenade,  CMomentumGrenade, weapon_smokegrenade,  weapon_momentum_grenade);