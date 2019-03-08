#include "cbase.h"
#include "dynamicrendertargets.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/itexture.h"

#include "tier0/memdbgon.h"

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

    m_DepthBufferTexture.Init(CreateDepthBufferTexture());

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

void CDynamicRenderTargets::PostInit()
{
    KeyValues *pVMTKeyValues = new KeyValues("unlitgeneric");
    pVMTKeyValues->SetString("$vertexcolor", "1");
    pVMTKeyValues->SetString("$vertexalpha", "1");
    pVMTKeyValues->SetString("$additive", "1");
    pVMTKeyValues->SetString("$ignorez", "0"); // Change this to 1 to see it through walls
    pVMTKeyValues->SetString("$halflambert", "1");
    pVMTKeyValues->SetString("$selfillum", "1");
    pVMTKeyValues->SetString("$nofog", "1");
    pVMTKeyValues->SetString("$nocull", "1");
    pVMTKeyValues->SetString("$model", "1");
    /*IMaterial *pMat = m_pMaterialSystem->CreateMaterial(); // Refcount = 1
    pMat->DecrementReferenceCount(); // Init adds a ref, so we bring refcount back to 0*/
    m_TriggerOutlineMat.Init("__utilOutlineColor", pVMTKeyValues);
    m_TriggerOutlineMat->Refresh();
}

void CDynamicRenderTargets::Shutdown()
{
    m_TriggerOutlineMat.Shutdown();
}

Vector2D CDynamicRenderTargets::GetViewport()
{
    if (!m_pMaterialSystem)
        return Vector2D(0, 0);

    int32 viewportX, viewportY;
    m_pMaterialSystem->GetBackBufferDimensions(viewportX, viewportY);
    return Vector2D(viewportX, viewportY);
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


static CDynamicRenderTargets g_DynamicRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CDynamicRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION,
                                  g_DynamicRenderTargets);
CDynamicRenderTargets *g_pDynamicRenderTargets = &g_DynamicRenderTargets;