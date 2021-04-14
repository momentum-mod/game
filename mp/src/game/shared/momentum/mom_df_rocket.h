#pragma once

#include "mom_explosive.h"

#ifdef CLIENT_DLL
#define CMomDFRocket C_MomDFRocket
#endif

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

    static CMomDFRocket *EmitRocket(const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner);

  private:
    void SpawnRocketSurprise();
    void StopTrailSound();
#endif
};