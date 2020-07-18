#pragma once

class ConcParticle : public SimpleParticle
{
  public:
    ConcParticle() {}
    float m_flOffset;
    float m_flRefract;
};

class CConcEmitter : public CSimpleEmitter
{
  public:
    DECLARE_CLASS(CConcEmitter, CSimpleEmitter);

    static CSmartPtr<CConcEmitter> Create(const char *pDebugName);

    virtual void RenderParticles(CParticleRenderIterator *pIterator);
    virtual void SimulateParticles(CParticleSimulateIterator *pIterator)
    {
        CSimpleEmitter::SimulateParticles(pIterator);
    };

    ConcParticle *AddConcParticle();

    static PMaterialHandle GetMaterial() { return m_hMaterial; };

  protected:
    CConcEmitter(const char *pDebugName);
    virtual ~CConcEmitter();

  private:
    CConcEmitter(const CConcEmitter &);

    static PMaterialHandle m_hMaterial;
};
