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

#define TEXTURE_GROUP_ULTRALIGHT "UL textures"

using namespace ultralight;

static UltralightUISystem g_UltralightSystem;
CBaseGameSystemPerFrame *UltralightUI() { return &g_UltralightSystem; }

static SourceFileSystem g_UltralightFileSystem;

UltralightOverlay *g_pTestOverlay = nullptr;

bool UltralightUISystem::Init()
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

    g_pTestOverlay = new UltralightOverlay(*m_pRenderer, m_pGPUDriver, 150, 150, 300, 300);
    g_pTestOverlay->view()->LoadHTML("<h1><font color='red'>HTML</font> <font color='green'>UI</font><font color='blue'>!!!!</font></h1>");
    //g_pTestOverlay->view()->LoadURL("http://www.google.com");

    return true;
}

void UltralightUISystem::Shutdown()
{
    delete g_pTestOverlay;
    g_pTestOverlay = nullptr;

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

	//g_pTestOverlay->Draw();
}