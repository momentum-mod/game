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

    // This gets sent to the client and placed in the client's interpolation history
    // so the projectile starts out moving right off the bat.
    CNetworkVector(m_vInitialVelocity);

    CMomStickybomb();
    ~CMomStickybomb();

    float GetCreationTime() { return m_flCreationTime; }
    void SetChargeTime(float flChargeTime) { m_flChargeTime = flChargeTime; }

    void UpdateOnRemove() OVERRIDE;

    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;

#ifdef CLIENT_DLL
  public:
    virtual int DrawModel(int flags) OVERRIDE;
    virtual void OnDataChanged(DataUpdateType_t type) OVERRIDE;
    void Simulate() OVERRIDE;

#else

    static CMomStickybomb *Create(const Vector &position, const QAngle &angles, const Vector &velocity,
                                  const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner);
    void InitStickybomb(const Vector &velocity, const AngularImpulse &angVelocity);
    void Explode(trace_t *pTrace, CBaseEntity *pOther);

    float GetRadius() { return m_flRadius; }
    float GetDamage() OVERRIDE { return m_flDamage; }
    void Fizzle();
    void Detonate();
    void RemoveStickybomb(bool bNoGrenadeZone);
    void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent) OVERRIDE;

    void SetRadius(float flRadius) { m_flRadius = flRadius; }
    void SetDamage(float flDamage) OVERRIDE { m_flDamage = flDamage; }

    // Specify what velocity we want the grenade to have on the client immediately.
    // Without this, the entity wouldn't have an interpolation history initially, so it would
    // sit still until it had gotten a few updates from the server.
    void SetupInitialTransmittedGrenadeVelocity(const Vector &velocity);

  protected:
    float m_flDamage;
    float m_flRadius;
    bool m_bFizzle;
    bool m_bUseImpactNormal;
    Vector m_vecImpactNormal;
#endif

  private:
    bool m_bPulsed;
    float m_flChargeTime;
    float m_flCreationTime;
};