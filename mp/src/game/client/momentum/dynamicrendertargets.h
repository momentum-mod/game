#pragma once

#include "baseclientrendertargets.h"

class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CDynamicRenderTargets : public CBaseClientRenderTargets, public CAutoGameSystemPerFrame
{
    DECLARE_CLASS_GAMEROOT(CDynamicRenderTargets, CBaseClientRenderTargets);

  public:
    virtual void InitClientRenderTargets(IMaterialSystem *pMaterialSystem,
                                         IMaterialSystemHardwareConfig *pHardwareConfig);
    virtual void InitDynamicRenderTargets();

    virtual void ShutdownClientRenderTargets();
    virtual void ShutdownDynamicRenderTargets();

    virtual void PreRender();
    virtual void UpdateDynamicRenderTargets();

    void PostInit() OVERRIDE;
    void Shutdown() OVERRIDE;

    CMaterialReference GetTriggerOutlineMat() const { return m_TriggerOutlineMat; }

  protected:
    virtual Vector2D GetViewport();

    virtual ITexture *CreateDepthBufferTexture();

  private:
    Vector2D m_pOldViewport;

    CTextureReference m_DepthBufferTexture;
    CMaterialReference m_TriggerOutlineMat;

    IMaterialSystem *m_pMaterialSystem;
};

extern CDynamicRenderTargets *g_pDynamicRenderTargets;