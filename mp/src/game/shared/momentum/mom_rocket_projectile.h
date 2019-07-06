#pragma once

#ifndef CLIENT_DLL
#include "smoke_trail.h"
#endif

#define MOM_ROCKET_RADIUS (110.0f * 1.1f) // https://github.com/NicknineTheEagle/TF2-Base/blob/master/src/game/shared/tf/tf_weaponbase_rocket.h#L27
#define MOM_ROCKET_SPEED 1100

#ifndef CLIENT_DLL
class CMomentumRocketLauncher;

class CMomentumRocket : public CBaseAnimating
{
    DECLARE_CLASS(CMomentumRocket, CBaseAnimating);

  public:
    CMomentumRocket();

    void Spawn() OVERRIDE;
    void Precache() OVERRIDE;
    void Touch(CBaseEntity *pOther) OVERRIDE;
    void Explode(trace_t *pTrace, CBaseEntity *pOther);

    static float GetRadius() { return MOM_ROCKET_RADIUS; }
    float GetDamage() OVERRIDE { return m_flDamage; }
    void SetDamage(float flDamage) OVERRIDE { m_flDamage = flDamage; }

    CHandle<CMomentumRocketLauncher> m_hOwner;

    static CMomentumRocket *EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner);

  protected:
    void CreateSmokeTrail();

    CHandle<RocketTrail> m_hRocketTrail;
    float m_flDamage;

  private:
    DECLARE_DATADESC();
};
#endif