#pragma once

#include "baseprojectile.h"

#ifdef CLIENT_DLL
#define CMomSticky C_MomSticky
#else
#include "smoke_trail.h"
class CMomentumStickyLauncher;
#endif

class CMomSticky : public CBaseProjectile
{
  public:
    DECLARE_CLASS(CMomSticky, CBaseProjectile);
    DECLARE_NETWORKCLASS();

    // This gets sent to the client and placed in the client's interpolation history
    // so the projectile starts out moving right off the bat.
    CNetworkVector(m_vInitialVelocity);

    CMomSticky();

#ifdef CLIENT_DLL
    virtual int DrawModel(int flags) OVERRIDE;
    virtual void Spawn() OVERRIDE;
    virtual void PostDataUpdate(DataUpdateType_t type) OVERRIDE;

    float m_flSpawnTime;
#else
    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;
    void StickyTouch(CBaseEntity *pOther);
    void Explode(trace_t *pTrace, CBaseEntity *pOther);
    void Destroy(bool bNoGrenadeZone);
    void DestroyTrail();

    float GetRadius() { return m_flRadius; }
    float GetDamage() OVERRIDE { return m_flDamage; }

    void SetRadius(float flRadius) { m_flRadius = flRadius; }
    void SetDamage(float flDamage) OVERRIDE { m_flDamage = flDamage; }

    // Specify what velocity we want the grenade to have on the client immediately.
    // Without this, the entity wouldn't have an interpolation history initially, so it would
    // sit still until it had gotten a few updates from the server.
    void SetupInitialTransmittedGrenadeVelocity(const Vector &velocity);

    CHandle<CMomentumStickyLauncher> m_hOwner;

    static CMomSticky *EmitSticky(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = nullptr);

  protected:
    void CreateSmokeTrail();

    CHandle<RocketTrail> m_hRocketTrail;
    float m_flDamage;
    float m_flRadius;

  private:
    DECLARE_DATADESC();
#endif

public:
    CBaseEntity *GetThrower() { return m_hThrower; }
    void SetThrower(CBaseEntity *pThrower) { m_hThrower = pThrower; }

protected:
    CBaseEntity* m_hThrower;
};