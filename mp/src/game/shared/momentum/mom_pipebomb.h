#pragma once

#include "baseprojectile.h"

#ifdef CLIENT_DLL
#define CMomPipebomb C_MomPipebomb
#else
#include "smoke_trail.h"
class CMomentumPipebombLauncher;
#endif

class CMomPipebomb : public CBaseProjectile
{
  public:
    DECLARE_CLASS(CMomPipebomb, CBaseProjectile);
    DECLARE_NETWORKCLASS();

    // This gets sent to the client and placed in the client's interpolation history
    // so the projectile starts out moving right off the bat.
    CNetworkVector(m_vInitialVelocity);

    CMomPipebomb();

#ifdef CLIENT_DLL
    virtual int DrawModel(int flags) OVERRIDE;
    virtual void Spawn() OVERRIDE;
    virtual void PostDataUpdate(DataUpdateType_t type) OVERRIDE;

    float m_flSpawnTime;
#else

    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;
    void PipebombTouch(CBaseEntity *pOther);
    void Explode(trace_t *pTrace, CBaseEntity *pOther);
    void Destroy(bool bNoGrenadeZone);
    void DestroyTrail();

    float GetRadius() { return m_flRadius; }
    float GetDamage() OVERRIDE { return m_flDamage; }
    bool GetHasTouchedWorld() { return m_bTouched; }
    void Fizzle();
    void Detonate();
    void RemovePipebomb(bool bBlinkOut);
    bool ShouldNotDetonate(void);
    void Pulse();
    void DetonateThink(void);
    void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent) OVERRIDE;

    void SetRadius(float flRadius) { m_flRadius = flRadius; }
    void SetDamage(float flDamage) OVERRIDE { m_flDamage = flDamage; }

    // Specify what velocity we want the grenade to have on the client immediately.
    // Without this, the entity wouldn't have an interpolation history initially, so it would
    // sit still until it had gotten a few updates from the server.
    void SetupInitialTransmittedGrenadeVelocity(const Vector &velocity);

    CHandle<CMomentumPipebombLauncher> m_hOwner;

    static CMomPipebomb *EmitPipebomb(const Vector &vecOrigin, const Vector &velocity,
                                      const QAngle &vecAngles,
                                      CBaseEntity *pentOwner = nullptr);

    float m_flChargeTime;
    float m_flCreationTime;
    bool m_bPulsed;
    

  protected:
    void CreateSmokeTrail();

    CHandle<RocketTrail> m_hRocketTrail;
    float m_flDamage;
    float m_flRadius;
    //CNetworkVar(bool, m_bTouched);
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