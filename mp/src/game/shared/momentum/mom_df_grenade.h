#pragma once

#include "basegrenade_shared.h"

#ifdef CLIENT_DLL
#define CMomDFGrenade C_MomDFGrenade
#endif

class CMomDFGrenade : public CBaseGrenade
{
  public:
    DECLARE_CLASS(CMomDFGrenade, CBaseGrenade);
    DECLARE_NETWORKCLASS();

    void Spawn() OVERRIDE;

    // This gets sent to the client and placed in the client's interpolation history
    // so the projectile starts out moving right off the bat.
    CNetworkVector(m_vInitialVelocity);

#ifdef CLIENT_DLL
    virtual void PostDataUpdate(DataUpdateType_t type) OVERRIDE;
#else

    static CMomDFGrenade *Create(const Vector &position, const QAngle &angles, const Vector &velocity,
        const AngularImpulse &angVelocity, CBaseEntity *pOwner, float damageFactor);

    // Think function to emit danger sounds for the AI
    void DetonateThink();

    virtual float GetShakeAmplitude() OVERRIDE { return 0.0f; }
    virtual void Splash() OVERRIDE;

    // Specify what velocity we want the grenade to have on the client immediately.
    // Without this, the entity wouldn't have an interpolation history initially, so it would
    // sit still until it had gotten a few updates from the server.
    void SetupInitialTransmittedGrenadeVelocity(const Vector &velocity);
    
    void Explode(trace_t *pTrace);
    bool DFCanDamage(const CTakeDamageInfo &info, CBaseEntity *other, const Vector &origin);
    void DFRadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore,
                        CBaseEntity *pEntityIgnore);

    void Touch(CBaseEntity *pOther) override;

    float m_flDamageFactor;

  protected:
    // Set the time to detonate ( now + timer )
    void SetDetonateTimerLength(float timer);

  private:
    // Custom collision to allow for constant elasticity on hit surfaces
    virtual void ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity) OVERRIDE;

    float m_flDetonateTime;
#endif
};