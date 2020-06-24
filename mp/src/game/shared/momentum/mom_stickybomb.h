#pragma once

#include "mom_explosive.h"

#ifdef CLIENT_DLL
#define CMomStickybomb C_MomStickybomb
#endif

class CMomStickybomb : public CMomExplosive
{
  public:
    DECLARE_CLASS(CMomStickybomb, CMomExplosive);
    DECLARE_NETWORKCLASS();

    CMomStickybomb();
    ~CMomStickybomb();

    void SetChargeTime(float flChargeTime) { m_flChargeTime = flChargeTime; }

    void UpdateOnRemove() OVERRIDE;

    void Spawn() OVERRIDE;

    bool IsArmed() const;
#ifdef CLIENT_DLL
    float GetDrawDelayTime() override;
    void CreateTrailParticles() override;

    void OnDataChanged(DataUpdateType_t type) OVERRIDE;
    void Simulate() OVERRIDE;
#else
    float GetDamageAmount() override { return 120.0f; }
    void InitExplosive(CBaseEntity *pOwner, const Vector &velocity, const QAngle &angles) override;

    static CMomStickybomb *Create(const Vector &position, const QAngle &angles, const Vector &velocity, CBaseEntity *pOwner);

    void Explode(trace_t *pTrace, CBaseEntity *pOther);
    void Destroy(bool bShowFizzleSprite) override;

    void Fizzle();
    void Detonate();
    void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent) OVERRIDE;

    bool DidHitWorld() const { return m_bDidHitWorld; }
#endif

  private:
    IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_fFlags);
#ifdef CLIENT_DLL
    bool m_bPulsed;
#else
    bool m_bFizzle;
    bool m_bDidHitWorld;
    Vector m_vecImpactNormal;
    float m_flCreationTime;
#endif

    float m_flChargeTime;
};