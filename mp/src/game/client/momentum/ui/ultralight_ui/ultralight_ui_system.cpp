#include "cbase.h"
#include "ultralight_ui_system.h"

#include <pixelwriter.h>
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/ishader.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/itexture.h"
#include "ultralight_filesystem.h"
#include "ultralight_overlay.h"

#include "tier0/memdbgon.h"

using namespace ultralight;

static UltralightUISystem g_UltralightSystem;
UltralightUISystem *UltralightUI() { return &g_UltralightSystem; }

static UltralightFileSystem g_UltralightFileSystem;

bool UltralightUISystem::Init()
{
    return true;
}

void UltralightUISystem::Shutdown()
{
    m_pGPUDriver = nullptr;
}

void UltralightUISystem::PreRender() {}

void UltralightUISystem::PostRender()
{
    m_pRenderer->Update();

    m_pGPUDriver->BeginSynchronize();
    m_pRenderer->Render();
    m_pGPUDriver->EndSynchronize();

    if (m_pGPUDriver->HasCommandsPending())
    {
        m_pGPUDriver->DrawCommandList();
    }
}

UltralightUISystem::UltralightUISystem()
{
    Config config;
    config.face_winding = kFaceWinding_Clockwise; // CW in D3D, CCW in OGL
    config.device_scale_hint = 1.0;               // Set DPI to monitor DPI scale
    config.enable_javascript = true;
    config.enable_images = true;

    Platform &platform = Platform::instance();
    platform.set_config(config);
    platform.set_file_system(&g_UltralightFileSystem);
    platform.set_font_loader(DefaultFontLoader());
    m_pGPUDriver = DefaultGPUDriver();
    platform.set_gpu_driver(m_pGPUDriver);

    m_pRenderer = Renderer::Create();
}

UltralightOverlay *UltralightUISystem::CreateOverlay(vgui::Panel *pParentPanel, bool bTransparent)
{
    if (!m_pRenderer || !m_pGPUDriver)
    {
        return nullptr;
    }

    auto pOverlay = new UltralightOverlay(*m_pRenderer, pParentPanel, bTransparent);
    return pOverlay;
}
