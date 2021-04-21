#pragma once

#include "mom_explosive.h"

#ifdef CLIENT_DLL
#define CMomDFRocket C_MomDFRocket
#endif

enum DFProjectileType_t
{
    DF_ROCKET = 0,
    DF_PLASMA
};

const float damage[] = {100, 15};
const float splashRadius[] = {120, 20};
const float speed[] = {900, 2000};

class CMomDFRocket : public CMomExplosive
{
  public:
    DECLARE_CLASS(CMomDFRocket, CMomExplosive);
    DECLARE_NETWORKCLASS();

    void Spawn() override;

#ifdef CLIENT_DLL
    float GetDrawDelayTime() override;
    void CreateTrailParticles() override;

#else
    float GetDamageAmount() override;
    void Destroy() override;
    void PlayFizzleSound() override;

    void RocketTouch(CBaseEntity *pOther);
    void Explode(trace_t *pTrace, CBaseEntity *pOther);

    bool DFCanDamage(const CTakeDamageInfo &info, CBaseEntity *other, const Vector &origin);
    void DFRadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore,
                        CBaseEntity *pEntityIgnore);

    static CMomDFRocket *EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner,
                                    DFProjectileType_t projType);
    DFProjectileType_t type;

  private:
    void StopTrailSound();
#endif
};