#pragma once

#include "game/client/iclientrendertargets.h"
#include "materialsystem/imaterialsystem.h"
#include "baseclientrendertargets.h"

class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CDynamicRenderTargets : public CBaseClientRenderTargets, public CAutoGameSystemPerFrame
{
	DECLARE_CLASS_GAMEROOT(CDynamicRenderTargets, CBaseClientRenderTargets);
public:
	virtual void		InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig);
	virtual void		InitDynamicRenderTargets();

	virtual void		ShutdownClientRenderTargets();
	virtual void		ShutdownDynamicRenderTargets();

	virtual void		PreRender();
	virtual void		UpdateDynamicRenderTargets();

protected:
	virtual Vector2D	GetViewport();

	virtual ITexture*	CreateMaskGameUITexture();
    virtual ITexture*   CreateDepthBufferTexture();
    virtual ITexture*   CreateBlurTexture(bool blurX);
    virtual ITexture*   CreateFullscreenPPTexture();
private:
	Vector2D			m_pOldViewport;

	CTextureReference	m_MaskGameUITexture;
    CTextureReference   m_DepthBufferTexture;
    CTextureReference   m_BlurX, m_BlurY, m_FullscreenPP;

	IMaterialSystem*	m_pMaterialSystem;
};

extern CDynamicRenderTargets* g_pDynamicRenderTargets;