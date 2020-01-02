#pragma once

#include "mom_stickybomb.h"
#include "weapon_base_gun.h"

#ifdef CLIENT_DLL
#define CMomentumStickybombLauncher C_MomentumStickybombLauncher
#endif

#ifdef GAME_DLL
class CMomentumStickybombLauncher : public CWeaponBaseGun, public IEntityListener
#else
class CMomentumStickybombLauncher : public CWeaponBaseGun
#endif
{
  public:
    DECLARE_CLASS(CMomentumStickybombLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

#ifdef GAME_DLL
    DECLARE_DATADESC();
#endif

    CMomentumStickybombLauncher();
    ~CMomentumStickybombLauncher();

    CWeaponID GetWeaponID() const OVERRIDE { return WEAPON_STICKYLAUNCHER; }

    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;

    CBaseEntity *FireStickybomb(CMomentumPlayer *pPlayer);
    CBaseEntity *FireProjectile(CMomentumPlayer *pPlayer);
    void PrimaryAttack() OVERRIDE;
    void SecondaryAttack() OVERRIDE;
    void LaunchGrenade();

    bool CanDeploy() OVERRIDE;
    bool Deploy() OVERRIDE;
    bool Holster(CBaseCombatWeapon *pSwitchingTo) OVERRIDE;
    void WeaponIdle() OVERRIDE;
    void ItemBusyFrame() OVERRIDE;
    void ItemPostFrame() OVERRIDE;

    void AddStickybomb(CMomStickybomb *pBomb);
    bool DetonateRemoteStickybombs(bool bFizzle);
    void DeathNotice(CBaseEntity *pVictim);
    int GetStickybombCount() { return m_iStickybombCount; }
    float GetProjectileSpeed();
    bool DualFire() OVERRIDE { return true; }

  public:
    float GetChargeBeginTime() { return m_flChargeBeginTime; }
    float GetChargeMaxTime();

#ifdef CLIENT_DLL
    int m_iStickybombCount;
#endif

#ifdef GAME_DLL
  private:
    CNetworkVar(int, m_iStickybombCount);
#endif

    // List of active stickybombs
    typedef CHandle<CMomStickybomb> StickybombHandle;
    CUtlVector<StickybombHandle> m_Stickybombs;

#ifdef CLIENT_DLL
    float m_flChargeBeginTime;
#endif

#ifdef GAME_DLL
    CNetworkVar(float, m_flChargeBeginTime);
#endif

    float m_flLastDenySoundTime;

    CMomentumStickybombLauncher(const CMomentumStickybombLauncher &) {}
};