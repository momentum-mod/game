#include "cbase.h"
#include "ultralight_ui_system.h"

#include "ultralight_filesystem.h"
#include "ultralight_overlay.h"
#include <clientmode.h>
#include <chrono>
#include <vgui_controls/Frame.h>

#include "tier0/memdbgon.h"

using namespace ultralight;

static UltralightUISystem g_UltralightSystem;
UltralightUISystem *UltralightUI() { return &g_UltralightSystem; }

static UltralightFileSystem g_UltralightFileSystem;

static class TestUI *g_pTestOverlay;
std::chrono::time_point<std::chrono::high_resolution_clock> g_iULStartTime;
bool g_bULMeasure = false;


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

class TestUI : public vgui::Frame
{
  public:
	TestUI() : Frame(g_pClientMode->GetViewport(), "TestULUI", false)
	{
        SetBounds(300, 300, 300, 300);
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);

		m_pHTMLOverlay = UltralightUI()->CreateOverlay(this, true);
		m_pHTMLOverlay->SetBounds(0, 0, 300, 300);
		m_pHTMLOverlay->LoadHTMLFromFile("momentum/testui.html");
	}

  public:
    UltralightOverlay *m_pHTMLOverlay;
};

CON_COMMAND(ul_test_js, "")
{
    g_bULMeasure = true;
	char cmd[128];
	Q_snprintf(cmd, sizeof(cmd), "document.getElementById('my-text').innerHTML = '%s'", args.Arg(1));
    g_iULStartTime = std::chrono::high_resolution_clock::now();
    g_pTestOverlay->m_pHTMLOverlay->EvaluateScript(cmd);
}

bool UltralightUISystem::Init()
{
    g_pTestOverlay = new TestUI;
    g_pTestOverlay->Activate();
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
