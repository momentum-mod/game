#pragma once

#include "mom_stickybomb.h"
#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumStickybombLauncher C_MomentumStickybombLauncher
#endif

// List of active stickybombs
typedef CHandle<CMomStickybomb> StickybombHandle;

class CMomentumStickybombLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumStickybombLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
    
    CMomentumStickybombLauncher();
    
    float m_flChargeBeginTime;
    float m_flLastDenySoundTime;
#ifdef CLIENT_DLL    
    int m_iStickybombCount;
#else
    CNetworkVar(int, m_iStickybombCount);
#endif

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;
    bool Deploy() OVERRIDE;
    bool Holster(CBaseCombatWeapon *pSwitchingTo) OVERRIDE;
    void ItemBusyFrame() OVERRIDE;
    void AddStickybomb(CMomStickybomb *pBomb);
    bool DetonateRemoteStickybombs(bool bFizzle);
    void DeathNotice(CBaseEntity *pVictim);
    int GetStickybombCount(void) { return m_iStickybombCount; }
    float GetProjectileSpeed(void);
    void WeaponIdle() OVERRIDE;
    CBaseEntity *FireStickybomb(CMomentumPlayer *pPlayer);
    CBaseEntity *FireProjectile(CMomentumPlayer *pPlayer);

    CUtlVector<StickybombHandle> m_Stickybombs;

    void GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward);
    float GetChargeMaxTime(void);

    bool CanDeploy() OVERRIDE;

    CWeaponID GetWeaponID() const OVERRIDE { return WEAPON_STICKYLAUNCHER; }

  private:
    void StickybombLauncherFire();
    void StickybombLauncherDetonate();
    void StickybombLauncherHolster();
};