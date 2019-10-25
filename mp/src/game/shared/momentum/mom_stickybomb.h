#pragma once

#include "baseprojectile.h"

#ifdef CLIENT_DLL
#define CMomStickybomb C_MomStickybomb
#else
#include "smoke_trail.h"
class CMomentumStickybombLauncher;
#endif

class CMomStickybomb : public CBaseProjectile
{
  public:
    DECLARE_CLASS(CMomStickybomb, CBaseProjectile);
    DECLARE_NETWORKCLASS();

    // This gets sent to the client and placed in the client's interpolation history
    // so the projectile starts out moving right off the bat.
    CNetworkVector(m_vInitialVelocity);

    CMomStickybomb();

    float m_flCreationTime;
    bool m_bPulsed;

#ifdef CLIENT_DLL
  public:
    virtual int DrawModel(int flags) OVERRIDE;
    virtual void Spawn() OVERRIDE;
    virtual void PostDataUpdate(DataUpdateType_t type) OVERRIDE;
    float GetCreationTime() { return m_flCreationTime; }
    bool GetHasPulsed() { return m_bPulsed; }

    float m_flSpawnTime;
#else

    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;
    static CMomStickybomb *Create(const Vector &position, const QAngle &angles, const Vector &velocity,
                                  const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner);
    void InitStickybomb(const Vector &velocity, const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner);
    void StickybombTouch(CBaseEntity *pOther);
    void Explode(trace_t *pTrace, CBaseEntity *pOther);
    void DestroyTrail();

    float GetRadius() { return m_flRadius; }
    float GetDamage() OVERRIDE { return m_flDamage; }
    bool GetHasTouchedWorld() { return m_bTouched; }
    void Fizzle();
    void Detonate();
    void RemoveStickybomb(bool bNoGrenadeZone);
    void Pulse();
    void StickybombThink();
    void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent) OVERRIDE;
    void SetChargeTime(float flChargeTime) { m_flChargeTime = flChargeTime; }

    void SetRadius(float flRadius) { m_flRadius = flRadius; }
    void SetDamage(float flDamage) OVERRIDE { m_flDamage = flDamage; }

    // Specify what velocity we want the grenade to have on the client immediately.
    // Without this, the entity wouldn't have an interpolation history initially, so it would
    // sit still until it had gotten a few updates from the server.
    void SetupInitialTransmittedGrenadeVelocity(const Vector &velocity);

    CHandle<CMomentumStickybombLauncher> m_hOwner;

    float m_flChargeTime;

  protected:
    void CreateSmokeTrail();

    CHandle<RocketTrail> m_hRocketTrail;
    float m_flDamage;
    float m_flRadius;
    // CNetworkVar(bool, m_bTouched);
    bool m_bTouched;
    bool m_bFizzle;

  private:
    DECLARE_DATADESC();
#endif
  public:
    CBaseEntity *GetThrower() { return m_hThrower; }
    void SetThrower(CBaseEntity *pThrower) { m_hThrower = pThrower; }

  protected:
    CBaseEntity *m_hThrower;
};