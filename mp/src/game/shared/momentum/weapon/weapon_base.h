#pragma once

#include "ammodef.h"

#if defined(CLIENT_DLL)
#define CWeaponBase C_WeaponBase
#define CMomentumPlayer C_MomentumPlayer
#endif

class CMomentumPlayer;

class CWeaponBase : public CBaseCombatWeapon
{
  public:
    DECLARE_CLASS(CWeaponBase, CBaseCombatWeapon);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CWeaponBase();

#ifdef GAME_DLL
    DECLARE_DATADESC();

    virtual void CheckRespawn();
    virtual CBaseEntity *Respawn();

    virtual const Vector &GetBulletSpread();
    virtual float GetDefaultAnimSpeed();

    virtual void BulletWasFired(const Vector &vecStart, const Vector &vecEnd);
    virtual bool ShouldRemoveOnRoundRestart();
    virtual bool DefaultReload(int iClipSize1, int iClipSize2, int iActivity);

    void SendReloadEvents();

    void Materialize() OVERRIDE;

    virtual bool IsRemoveable();

#endif

    bool Holster(CBaseCombatWeapon *pSwitchingTo) OVERRIDE;
    void AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles) OVERRIDE;
    float CalcViewmodelBob(void) OVERRIDE;
    bool IsPredicted() const OVERRIDE { return true; }

    // Allow use of both primary and secondary weapon functions at the same time when true
    virtual bool DualFire() { return false; }

    // Pistols reset m_iShotsFired to 0 when the attack button is released.
    bool IsPistol() const { return GetWeaponID() == WEAPON_PISTOL; }

    CMomentumPlayer *GetPlayerOwner() const;

    virtual float GetMaxSpeed() const; // What's the player's max speed while holding this weapon.

  public:
#if defined(CLIENT_DLL)

    void ProcessMuzzleFlashEvent() OVERRIDE;
    bool OnFireEvent(C_BaseViewModel *pViewModel, const Vector &origin, const QAngle &angles, int event, const char *options) OVERRIDE;
    bool ShouldPredict() OVERRIDE;
    void DrawCrosshair() OVERRIDE;
    void OnDataChanged(DataUpdateType_t type) OVERRIDE;

    virtual bool HideViewModelWhenZoomed(void) { return true; }

    float m_flCrosshairDistance;
    int m_iAmmoLastCheck;
    int m_iAlpha;
    int m_iScopeTextureID;
    int m_iCrosshairTextureID; // for white additive texture

    bool m_bInReloadAnimation;
#else

    virtual bool Reload();
    virtual void Spawn();
    virtual bool KeyValue(const char *szKeyName, const char *szValue);

    virtual bool PhysicsSplash(const Vector &centerPoint, const Vector &normal, float rawSpeed, float scaledSpeed);

#endif

    void Precache() override;
    bool IsUseable();
    bool CanDeploy() OVERRIDE;
    bool CanBeSelected() OVERRIDE;
    bool DefaultDeploy(const char *szViewModel, const char *szWeaponModel, int iActivity, const char *szAnimExt) OVERRIDE;
    void DefaultTouch(CBaseEntity *pOther) OVERRIDE; // default weapon touch
    virtual bool DefaultPistolReload();

    bool Deploy() OVERRIDE;
    void Drop(const Vector &vecVelocity) OVERRIDE;
    bool PlayEmptySound();
    void ItemPostFrame() OVERRIDE;

    bool m_bDelayFire; // This variable is used to delay the time between subsequent button pressing.
    float m_flAccuracy;

    void SetExtraAmmoCount(int count) { m_iExtraPrimaryAmmo = count; }
    int GetExtraAmmoCount() { return m_iExtraPrimaryAmmo; }

  private:
    float m_flDecreaseShotsFired;

    CWeaponBase(const CWeaponBase &);

    int m_iExtraPrimaryAmmo;

    float m_nextPrevOwnerTouchTime;
    CMomentumPlayer *m_prevOwner;

    int m_iDefaultExtraAmmo;

    float   m_flBobKickZ; // auto-decaying bob to dip after jumping and landing
    bool    m_bTgtBobKickZ; // kick push to max first, then set this to false to decay
    float   m_flPrevPlayerVelZ; // remember the player's previous z velocity, and use the delta to adjust the BobKick
    float   m_flRollAdj; // crowbar slash uses roll adjustment
};