#pragma once

#include "Sprite.h"
#include "SpriteTrail.h"
#include "basegrenade_shared.h"

#ifdef CLIENT_DLL
class CHudConcEntPanel;
#define CMomConcProjectile C_MomConcProjectile
#define CMomConcGlow C_MomConcGlow
#endif

class CMomConcGlow : public CSprite
{
  public:
    DECLARE_CLASS(CMomConcGlow, CSprite);

    DECLARE_NETWORKCLASS();
    DECLARE_DATADESC();

    int ObjectCaps() override { return BaseClass::ObjectCaps() & (~FCAP_ACROSS_TRANSITION | FCAP_DONT_SAVE); }

#ifdef CLIENT_DLL
    bool IsTransparent() override { return true; }
    RenderGroup_t GetRenderGroup() override { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
    int DrawModel(int flags) override;
    void OnDataChanged(DataUpdateType_t updateType) override;
    bool ShouldDraw() { return (IsEffectActive(EF_NODRAW) == false); }
#else
    static CMomConcGlow *Create(const Vector &origin, CBaseEntity *pOwner = nullptr);
#endif
};

class CMomConcProjectile : public CBaseGrenade
{
  public:
    DECLARE_CLASS(CMomConcProjectile, CBaseGrenade);
    DECLARE_NETWORKCLASS();

    ~CMomConcProjectile();

    bool m_bIsHandheld;
    CNetworkVar(bool, m_bIsOn);
    CNetworkVector(m_vInitialVelocity);

    void Spawn() override;
    void Precache() override;

    float GetDamage() override { return 0.0f; }
    float GetMaxTimer() { return 3.81f; }

    bool SetHandheld(bool state) { return m_bIsHandheld = state; }
    bool IsHandheld() { return m_bIsHandheld; }

    void BounceSound() override;

    CNetworkVarForDerived(float, m_flSpawnTime);
    CNetworkVar(float, m_flDetonateTime);
    float m_flNextBounceSoundTime;

#ifdef GAME_DLL
    DECLARE_DATADESC();

    void DrawRadius(float flRadius);
    void Explode(trace_t *pTrace, int bitsDamageType) override;

    // Delete entity without explosion
    void Remove();

    float GetShakeAmplitude() override { return 0.0f; }
    float GetGrenadeGravity() { return 0.8f; }
    float GetGrenadeFriction() { return 0.3f; }
    float GetGrenadeElasticity() { return 0.6f; }
    float GetGrenadeRadius() { return 280.0f; }
    float GetGrenadeDamage() { return 0.0f; }

    void WaterCheck();
    void GrenadeThink();

    void CreateTrail();
    void CreateGlowSprite();

    // Specify what velocity we want to have on the client immediately.
    // Without this, the entity wouldn't have an interpolation history initially, so it would
    // sit still until it had gotten a few updates from the server.
    void SetupInitialTransmittedVelocity(const Vector &velocity);
    void SetDetonateTimerLength(float timer);
    bool IsInWorld() const;

    bool m_bHitwater;
    float m_flHitwaterTimer;

  private:
    void ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity) override;
#else
    CMomConcProjectile() {}
    CMomConcProjectile(const CMomConcProjectile &) {}

    // Add initial velocity into the interpolation history so that interp works okay
    void OnDataChanged(DataUpdateType_t type) override;
    void ClientThink() override;

    void DoIdleEffect();

    virtual ShadowType_t ShadowCastType() { return SHADOWS_NONE; }

  private:
    CHudConcEntPanel *m_pEntPanel;
#endif
  private:
    CHandle<CSpriteTrail> m_pTrail;
    CHandle<CMomConcGlow> m_hGlowSprite;
};