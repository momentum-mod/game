#include "cbase.h"
#include "weapon_mom_rifle.h"
#include "weapon_mom_sniper.h"
#include "weapon_mom_pistol.h"
#include "weapon_mom_shotgun.h"
#include "weapon_mom_smg.h"
#include "weapon_mom_lmg.h"
#include "weapon_mom_grenade.h"

#include "tier0/memdbgon.h"

// Macro used to override the CS weapons
#ifdef CLIENT_DLL
#define CS_WEP_OVERRIDE(csGun, momGun, entName)                                                                        \
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
#define CS_WEP_OVERRIDE(csGun, momGun, entName)                                                                        \
    class csGun : public momGun                                                                                        \
    {                                                                                                                  \
      public:                                                                                                          \
        DECLARE_CLASS(csGun, momGun);                                                                                  \
        DECLARE_PREDICTABLE();                                                                                         \
        csGun(){};                                                                                                     \
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
CS_WEP_OVERRIDE(CAK47, CMomentumRifle, weapon_ak47);
CS_WEP_OVERRIDE(CWeaponAug, CMomentumRifle, weapon_aug);
CS_WEP_OVERRIDE(CWeaponFamas, CMomentumRifle, weapon_famas);
CS_WEP_OVERRIDE(CWeaponGalil, CMomentumRifle, weapon_galil);
CS_WEP_OVERRIDE(CWeaponM4A1, CMomentumRifle, weapon_m4a1);
CS_WEP_OVERRIDE(CWeaponSG552, CMomentumRifle, weapon_sg552);

//Snipers
CS_WEP_OVERRIDE(CWeaponScout, CMomentumSniper, weapon_scout);
CS_WEP_OVERRIDE(CWeaponG3SG1, CMomentumSniper, weapon_g3sg1);
CS_WEP_OVERRIDE(CWeaponSG550, CMomentumSniper, weapon_sg550);
CS_WEP_OVERRIDE(CWeaponAWP, CMomentumSniper, weapon_awp);

//Pistols
CS_WEP_OVERRIDE(CDeagle, CMomentumPistol, weapon_deagle);
CS_WEP_OVERRIDE(CWeaponElite, CMomentumPistol, weapon_elite);
CS_WEP_OVERRIDE(CWeaponFiveSeven, CMomentumPistol, weapon_fiveseven);
CS_WEP_OVERRIDE(CWeaponGlock, CMomentumPistol, weapon_glock);
CS_WEP_OVERRIDE(CWeaponP228, CMomentumPistol, weapon_p228);
CS_WEP_OVERRIDE(CWeaponUSP, CMomentumPistol, weapon_usp);

//SMGs
CS_WEP_OVERRIDE(CWeaponMAC10, CMomentumSMG, weapon_mac10);
CS_WEP_OVERRIDE(CWeaponMP5Navy, CMomentumSMG, weapon_mp5navy);
CS_WEP_OVERRIDE(CWeaponP90, CMomentumSMG, weapon_p90);
CS_WEP_OVERRIDE(CWeaponTMP, CMomentumSMG, weapon_tmp);
CS_WEP_OVERRIDE(CWeaponUMP45, CMomentumSMG, weapon_ump45);

//Shotguns
CS_WEP_OVERRIDE(CWeaponXM1014, CMomentumShotgun, weapon_xm1014);
CS_WEP_OVERRIDE(CWeaponM3, CMomentumShotgun, weapon_m3);

//LMG
CS_WEP_OVERRIDE(CWeaponM249, CMomentumLMG, weapon_m249);

//Grenades
CS_WEP_OVERRIDE(CHEGrenade, CMomentumGrenade, weapon_hegrenade);
CS_WEP_OVERRIDE(CFlashbang, CMomentumGrenade, weapon_flashbang);
CS_WEP_OVERRIDE(CSmokeGrenade, CMomentumGrenade, weapon_smokegrenade);