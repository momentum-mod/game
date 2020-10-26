#pragma once

#include "baseprojectile.h"

#ifdef CLIENT_DLL
#define CMomExplosive C_MomExplosive
#endif

// 146 is used for both sticky and rocket, though rocket has 121 self damage radius,
// see RadiusDamage in mom_gamerules for that
#define MOM_EXPLOSIVE_RADIUS 146.0f

class CMomExplosive : public CBaseProjectile
{
public:
    DECLARE_CLASS(CMomExplosive, CBaseProjectile);
    DECLARE_NETWORKCLASS();

    CMomExplosive();

    void Spawn() override;
    
#ifdef CLIENT_DLL
    virtual float GetDrawDelayTime() { return 0.0f; }
    virtual void CreateTrailParticles() { }

    int DrawModel(int flags) override;
    void OnDataChanged(DataUpdateType_t updateType) override;
    
#else
    virtual float GetDamageAmount() { return 0.0f; }
    virtual void Destroy();
    virtual void Fizzle();
    virtual void ShowFizzleSprite();
    virtual void PlayFizzleSound() {}
    virtual void InitExplosive(CBaseEntity *pOwner, const Vector &velocity, const QAngle &angles);

    float GetDamage() const { return m_fDamage; }
#endif

private:
    // This gets sent to the client and placed in the client's interpolation history
    // so the projectile starts out moving right off the bat.
    CNetworkVector(m_vInitialVelocity);

#ifdef CLIENT_DLL
    void InitializeInterpolationVelocity();
#else
    float m_fDamage;
#endif
};