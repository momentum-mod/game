//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_EFFECTS_H
#define C_EFFECTS_H
#ifdef _WIN32
#pragma once
#endif

#include "env_wind_shared.h"
#include "precipitation_shared.h"

class CPrecipitationParticle
{
  public:
    Vector m_Pos;
    Vector m_Velocity;
    float m_SpawnTime; // Note: Tweak with this to change lifetime
    float m_Mass;
    float m_Ramp;

    float m_flMaxLifetime;
    int m_nSplitScreenPlayerSlot;
};

class AshDebrisEffect : public CSimpleEmitter
{
  public:
    AshDebrisEffect(const char *pDebugName) : CSimpleEmitter(pDebugName) {}

    static CSmartPtr<AshDebrisEffect> Create(const char *pDebugName);

    virtual float UpdateAlpha(const SimpleParticle *pParticle);
    virtual float UpdateRoll(SimpleParticle *pParticle, float timeDelta);

  private:
    AshDebrisEffect(const AshDebrisEffect &);
};

//-----------------------------------------------------------------------------
// Precipitation blocker entity
//-----------------------------------------------------------------------------
class C_PrecipitationBlocker : public C_BaseEntity
{
  public:
    DECLARE_CLASS(C_PrecipitationBlocker, C_BaseEntity);
    DECLARE_CLIENTCLASS();

    C_PrecipitationBlocker();
    virtual ~C_PrecipitationBlocker();
};

//-----------------------------------------------------------------------------
// Precipitation base entity
//-----------------------------------------------------------------------------
class CClient_Precipitation : public C_BaseEntity
{
    class CPrecipitationEffect;
    friend class CClient_Precipitation::CPrecipitationEffect;

  public:
    DECLARE_CLASS(CClient_Precipitation, C_BaseEntity);
    DECLARE_CLIENTCLASS();

    CClient_Precipitation();
    virtual ~CClient_Precipitation();

    // Inherited from C_BaseEntity
    virtual void Precache();
    virtual bool IsTransparent() { return true; }
    virtual bool IsTwoPass() { return true; }

    void Render();

    // Computes where we're gonna emit
    bool ComputeEmissionArea(Vector &origin, Vector2D &size, C_BaseCombatCharacter *pCharacter) const;

  private:
    // Creates a single particle
    CPrecipitationParticle *CreateParticle();

    virtual void OnDataChanged(DataUpdateType_t updateType);
    virtual void ClientThink();

    void Simulate(float dt);

    // Renders the particle
    void RenderParticle(CPrecipitationParticle *pParticle, CMeshBuilder &mb) const;

    void CreateWaterSplashes();

    // Emits the actual particles
    void EmitParticles(float fTimeDelta);

    // Gets the tracer width and speed
    float GetWidth() const;
    float GetLength() const;
    float GetSpeed() const;

    // Gets the remaining lifetime of the particle
    float GetRemainingLifetime(CPrecipitationParticle *pParticle) const;

    // Computes the wind vector
    static void ComputeWindVector();

    // simulation methods
    bool SimulateRain(CPrecipitationParticle *pParticle, float dt);
    bool SimulateSnow(CPrecipitationParticle *pParticle, float dt) const;

    void CreateParticlePrecip(void);
    void InitializeParticlePrecip(void);
    void DispatchOuterParticlePrecip(C_BasePlayer *pPlayer, const Vector &vForward);
    void DispatchInnerParticlePrecip(C_BasePlayer *pPlayer, const Vector &vForward);
    void DestroyOuterParticlePrecip();
    void DestroyInnerParticlePrecip();

    void UpdateParticlePrecip(C_BasePlayer *pPlayer);
    float GetDensity() const { return m_flDensity; }

  private:
    void CreateAshParticle(void);
    void CreateRainOrSnowParticle(const Vector &vSpawnPosition, const Vector &vEndPosition,
                                  const Vector &vVelocity); // TERROR: adding end pos for lifetime calcs

    // Information helpful in creating and rendering particles
    IMaterial *m_MatHandle; // material used

    float m_Color[4];                  // precip color
    float m_Lifetime;                  // Precip lifetime
    float m_InitialRamp;               // Initial ramp value
    float m_Speed;                     // Precip speed
    float m_Width;                     // Tracer width
    float m_Remainder;                 // particles we should render next time
    PrecipitationType_t m_nPrecipType; // Precip type
    float m_flHalfScreenWidth;         // Precalculated each frame.

    float m_flDensity;

#ifdef INFESTED_DLL
    int m_nSnowDustAmount;
#endif

    // Some state used in rendering and simulation
    // Used to modify the rain density and wind from the console
    static ConVar s_raindensity;
    static ConVar s_rainwidth;
    static ConVar s_rainlength;
    static ConVar s_rainspeed;

    static Vector s_WindVector; // Stores the wind speed vector

    CUtlLinkedList<CPrecipitationParticle> m_Particles;
    CUtlVector<Vector> m_Splashes;

  protected:
    CSmartPtr<AshDebrisEffect> m_pAshEmitter;
    TimedEvent m_tAshParticleTimer;
    TimedEvent m_tAshParticleTraceTimer;
    bool m_bActiveAshEmitter;
    Vector m_vAshSpawnOrigin;
    int m_iAshCount;

    float m_flParticleInnerDist;         // The distance at which to start drawing the inner system
    const char *m_pParticleInnerNearDef; // Name of the first inner system
    const char *m_pParticleInnerFarDef;  // Name of the second inner system
    const char *m_pParticleOuterDef;     // Name of the outer system
    HPARTICLEFFECT m_pParticlePrecipInnerNear;
    HPARTICLEFFECT m_pParticlePrecipInnerFar;
    HPARTICLEFFECT m_pParticlePrecipOuter;
    TimedEvent m_tParticlePrecipTraceTimer;
    bool m_bActiveParticlePrecipEmitter;
    bool m_bParticlePrecipInitialized;

  private:
    CClient_Precipitation(const CClient_Precipitation &); // not defined, not accessible
};

extern CUtlVector<CClient_Precipitation *> g_Precipitations;

//-----------------------------------------------------------------------------
// EnvWind - global wind info
//-----------------------------------------------------------------------------
class C_EnvWind : public C_BaseEntity
{
  public:
    C_EnvWind();

    DECLARE_CLIENTCLASS();
    DECLARE_CLASS(C_EnvWind, C_BaseEntity);

    virtual void OnDataChanged(DataUpdateType_t updateType);
    virtual bool ShouldDraw(void) { return false; }

    virtual void ClientThink();

    const CEnvWindShared &WindShared() const { return m_EnvWindShared; }

  private:
    C_EnvWind(const C_EnvWind &);

    CEnvWindShared m_EnvWindShared;
};



// Draw rain effects.
void DrawPrecipitation();


#endif // C_EFFECTS_H
