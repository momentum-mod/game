#pragma once

#include "mom_pipebomb.h"
#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumPipebombLauncher C_MomentumPipebombLauncher
#endif

class CMomentumPipebombLauncher : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumPipebombLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumPipebombLauncher();

    float m_flChargeBeginTime;
    float m_flLastDenySoundTime;

#ifdef CLIENT_DLL
    int m_iPipebombCount;
#endif

#ifdef GAME_DLL
    CNetworkVar(int, m_iPipebombCount);
#endif

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;
    bool Deploy() OVERRIDE;
    bool Holster(CBaseCombatWeapon *pSwitchingTo) OVERRIDE;
    void ItemBusyFrame() OVERRIDE;
    void AddPipeBomb(CMomPipebomb *pBomb);
    bool DetonateRemotePipebombs(bool bFizzle);
    void DeathNotice(CBaseEntity *pVictim);
    int GetPipebombCount(void) { return m_iPipebombCount; }
    float GetProjectileSpeed(void);
    void WeaponIdle() OVERRIDE;
    CBaseEntity *FirePipebomb(CMomentumPlayer *pPlayer);
    CBaseEntity *FireProjectile(CMomentumPlayer *pPlayer);

    // List of active pipebombs
    typedef CHandle<CMomPipebomb> PipebombHandle;
    CUtlVector<PipebombHandle> m_Pipebombs;

    void GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward);
    float GetChargeMaxTime(void);

        bool CanDeploy() OVERRIDE;

    CWeaponID GetWeaponID() const OVERRIDE { return WEAPON_PIPEBOMBLAUNCHER; }

  private:
    void PipebombLauncherFire();
    void PipebombLauncherDetonate();
    void PipebombLauncherHolster();
};