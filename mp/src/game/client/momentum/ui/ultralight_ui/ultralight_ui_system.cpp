#include "cbase.h"
#include "ultralight_ui_system.h"

#include "ultralight_filesystem.h"
#include "ultralight_overlay.h"

#include "tier0/memdbgon.h"

using namespace ultralight;

static UltralightUISystem g_UltralightSystem;
UltralightUISystem *UltralightUI() { return &g_UltralightSystem; }

static UltralightFileSystem g_UltralightFileSystem;

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

    m_pRenderer = Renderer::Create();
}

bool UltralightUISystem::Init()
{
    return true;
}

void UltralightUISystem::Shutdown()
{
}

void UltralightUISystem::PreRender()
{
    m_pRenderer->Update();
    m_pRenderer->Render();
}

void UltralightUISystem::PostRender()
{
}

UltralightOverlay *UltralightUISystem::CreateOverlay(vgui::Panel *pParentPanel, bool bTransparent)
{
    if (!m_pRenderer)
    {
        return nullptr;
    }

    auto pOverlay = new UltralightOverlay(*m_pRenderer, pParentPanel, bTransparent);
    return pOverlay;
}
