#pragma once

#include "weapon_base_gun.h"
#include "mom_stickybomb.h"

#ifdef CLIENT_DLL
#define CMomentumStickybombLauncher C_MomentumStickybombLauncher
#endif

enum StickyDetonationStatus_t
{
    DET_STATUS_NONE = 0,            // You have 1+ out but they didn't detonate (are being removed most likely)
    DET_STATUS_FAIL = 1 << 0,       // There was at least one bomb that wasn't armed yet
    DET_STATUS_SUCCESS = 1 << 1     // There was at least one bomb that successfully detonated
};

class CMomentumStickybombLauncher : public CWeaponBaseGun
#ifdef GAME_DLL
, public IEntityListener
#endif
{
  public:
    DECLARE_CLASS(CMomentumStickybombLauncher, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumStickybombLauncher();
    ~CMomentumStickybombLauncher();

    WeaponID_t GetWeaponID() const override { return WEAPON_STICKYLAUNCHER; }

    void Precache() override;

    void PrimaryAttack() override;
    void SecondaryAttack() override;

    bool Deploy() override;
    bool Holster(CBaseCombatWeapon *pSwitchingTo) override;
    void WeaponIdle() override;
    void ItemBusyFrame() override;
    void ItemPostFrame() override;

    void AddStickybomb(CMomStickybomb *pBomb);
    int DetonateRemoteStickybombs(bool bFizzle);
    void DeathNotice(CBaseEntity *pVictim);
    int GetStickybombCount() { return m_iStickybombCount.Get(); }
    float CalculateProjectileSpeed(float flProgress);

    bool IsChargeEnabled() { return m_bIsChargeEnabled.Get(); }
    bool SetChargeEnabled(bool state);
    CMomStickybomb *GetStickyByCount(int count) { return m_iStickybombCount == m_Stickybombs.Size() ? m_Stickybombs[count] : nullptr; }
    void SetChargeBeginTime(float value) { m_flChargeBeginTime = value; }
    float GetChargeBeginTime() { return m_flChargeBeginTime; }
    float GetChargeMaxTime();

  private:
    void LaunchGrenade();
    bool DualFire() override { return true; }
    CMomStickybomb* FireStickybomb(CMomentumPlayer *pPlayer);
    CMomStickybomb* FireProjectile(CMomentumPlayer *pPlayer);

    CNetworkVar(int, m_iStickybombCount);
    CNetworkVar(float, m_flChargeBeginTime);
    float m_flLastDenySoundTime;

    // This var mainly serves to disable the charge animation, sound and turn the charge meter red if set to false
    // Networked so hud code can check the state
    CNetworkVar(bool, m_bIsChargeEnabled);

    // List of active stickybombs
    typedef CHandle<CMomStickybomb> StickybombHandle;
    CUtlVector<StickybombHandle> m_Stickybombs;

    CMomentumStickybombLauncher(const CMomentumStickybombLauncher &) {}

    CNetworkVar(bool, m_bEarlyPrimaryFire);
};