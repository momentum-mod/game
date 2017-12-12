#include "cbase.h"
#include "dynamicrendertargets.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/itexture.h"

void CDynamicRenderTargets::InitClientRenderTargets(IMaterialSystem *pMaterialSystem,
                                                    IMaterialSystemHardwareConfig *pHardwareConfig)
{
    BaseClass::InitClientRenderTargets(pMaterialSystem, pHardwareConfig);

    m_pMaterialSystem = pMaterialSystem;
    m_pOldViewport = GetViewport();

    InitDynamicRenderTargets();
}

void CDynamicRenderTargets::InitDynamicRenderTargets()
{
    m_pMaterialSystem->BeginRenderTargetAllocation();

    m_MaskGameUITexture.Init(CreateMaskGameUITexture());
    m_DepthBufferTexture.Init(CreateDepthBufferTexture());
    m_BlurX.Init(CreateBlurTexture(true));
    m_BlurY.Init(CreateBlurTexture(false));
    m_FullscreenPP.Init(CreateFullscreenPPTexture());

    m_pMaterialSystem->EndRenderTargetAllocation();
}

void CDynamicRenderTargets::ShutdownClientRenderTargets()
{
    BaseClass::ShutdownClientRenderTargets();

    ShutdownDynamicRenderTargets();

    m_pMaterialSystem = nullptr;
}

void CDynamicRenderTargets::ShutdownDynamicRenderTargets()
{
    m_MaskGameUITexture.Shutdown();
    m_DepthBufferTexture.Shutdown();
}

void CDynamicRenderTargets::PreRender() { UpdateDynamicRenderTargets(); }

void CDynamicRenderTargets::UpdateDynamicRenderTargets()
{
    if (!m_pMaterialSystem)
        return;

    if (m_pOldViewport != GetViewport())
    {
        ShutdownDynamicRenderTargets();
        InitDynamicRenderTargets();

        m_pOldViewport = GetViewport();
    }
}

Vector2D CDynamicRenderTargets::GetViewport()
{
    if (!m_pMaterialSystem)
        return Vector2D(0, 0);

    int32 viewportX, viewportY;
    m_pMaterialSystem->GetBackBufferDimensions(viewportX, viewportY);
    return Vector2D(viewportX, viewportY);
}

ITexture *CDynamicRenderTargets::CreateMaskGameUITexture()
{
    Vector2D viewport = GetViewport();

    return m_pMaterialSystem->CreateNamedRenderTargetTextureEx2(
        "_rt_MaskGameUI", viewport.x, viewport.y, RT_SIZE_FULL_FRAME_BUFFER,
        m_pMaterialSystem->GetBackBufferFormat(), MATERIAL_RT_DEPTH_SHARED, 0, CREATERENDERTARGETFLAGS_HDR);
}

ITexture *CDynamicRenderTargets::CreateDepthBufferTexture()
{
    uint32 flags = NULL;

    ITexture *textureTF2DepthBuffer =
        m_pMaterialSystem->FindTexture("_rt_ResolvedFullFrameDepth", TEXTURE_GROUP_RENDER_TARGET);
    if (textureTF2DepthBuffer)
        flags = textureTF2DepthBuffer->GetFlags();

    Vector2D viewport = GetViewport();
    return m_pMaterialSystem->CreateNamedRenderTargetTextureEx2("_rt_DepthBuffer", viewport.x, viewport.y,
                                                                RT_SIZE_FULL_FRAME_BUFFER, IMAGE_FORMAT_RGBA32323232F,
                                                                MATERIAL_RT_DEPTH_NONE, flags, NULL);
}

ITexture* CDynamicRenderTargets::CreateBlurTexture(bool blurX)
{
    Vector2D viewport = GetViewport();
    return m_pMaterialSystem->CreateNamedRenderTargetTextureEx2(blurX ? "_rt_BlurX" : "_rt_BlurY", viewport.x, viewport.y,
        RT_SIZE_HDR, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_NONE, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_RENDERTARGET, 0);
}

ITexture* CDynamicRenderTargets::CreateFullscreenPPTexture()
{
    Vector2D viewport = GetViewport();
    return m_pMaterialSystem->CreateNamedRenderTargetTextureEx2("_rt_FullscreenPP", viewport.x, viewport.y,
        RT_SIZE_FULL_FRAME_BUFFER, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_NONE, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_RENDERTARGET, 0);
}


static CDynamicRenderTargets g_DynamicRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CDynamicRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION,
                                  g_DynamicRenderTargets);
CDynamicRenderTargets *g_pDynamicRenderTargets = &g_DynamicRenderTargets;