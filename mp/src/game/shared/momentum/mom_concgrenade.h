#pragma once

#include "mom_explosive.h"

#ifdef CLIENT_DLL
class CHudConcEntPanel;
#define CMomConcProjectile C_MomConcProjectile
#endif

#define CONC_MAX_TIME 3.81f

class CMomConcProjectile : public CMomExplosive
{
  public:
    DECLARE_CLASS(CMomConcProjectile, CMomExplosive);
    DECLARE_NETWORKCLASS();

    ~CMomConcProjectile();

    void Spawn() override;
    void Precache() override;

    void BounceSound();

    CNetworkVar(float, m_flDetonateTime);
    float m_flNextBounceSoundTime;

#ifdef GAME_DLL
    DECLARE_DATADESC();

    static CMomConcProjectile *Create(float fTimer, const Vector &position, const Vector &velocity, CBaseEntity *pOwner);

    void PlayFizzleSound() override;

    // Need *some* damage even though we don't actually do any, in order to distinguish
    // the difference between a "dud" conc and an active one. Stickybombs and rockets use the value of
    // "0" to be a dud, conc by default does 0 damage.
    float GetDamageAmount() override { return 1.0f; }

    void DrawRadius();
    void Explode();
    void AffectEntitiesInRadius();

    void Destroy() override;

    float GetGrenadeGravity() { return 0.8f; }
    float GetGrenadeFriction() { return 0.3f; }
    float GetGrenadeElasticity() { return 0.6f; }
    float GetGrenadeRadius() { return 280.0f; }

    void WaterCheck();
    void GrenadeThink();

    void SetDetonateTimerLength(float timer);
    bool IsInWorld() const;

    bool SetHandheld(bool state) { return m_bIsHandheld = state; }
    bool IsHandheld() const { return m_bIsHandheld; }

  private:
    void ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity) override;

    bool m_bIsHandheld;
    bool m_bHitwater;
    float m_flHitwaterTimer;
#else
    void CreateTrailParticles() override;
    void ClientThink() override;

    virtual ShadowType_t ShadowCastType() { return SHADOWS_NONE; }

  private:
    CHudConcEntPanel *m_pEntPanel;
#endif
};