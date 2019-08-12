#include "cbase.h"
#include "rml_ui_system.h"

#include <RmlUi/Core.h>

#include "tier0/memdbgon.h"

static RmlUiSystem g_RmlUi;

RmlUiSystem::RmlUiSystem()
{
    m_pRmlFileInterface = nullptr;
    m_pRmlSystemInterface = nullptr;
    m_pRmlRenderInterface = nullptr;
    m_pContext = nullptr;
}

RmlUiSystem::~RmlUiSystem()
{
    m_pContext->RemoveReference();
    Rml::Core::Shutdown();
    delete m_pRmlRenderInterface;
    delete m_pRmlSystemInterface;
    delete m_pRmlFileInterface;
}

void RmlUiSystem::PostInit()
{
    m_pRmlFileInterface = new RmlFileInterface;
    Rml::Core::SetFileInterface(m_pRmlFileInterface);

    m_pRmlSystemInterface = new RmlSystemInterface;
    Rml::Core::SetSystemInterface(m_pRmlSystemInterface);

    m_pRmlRenderInterface = new RmlRenderInterface;
    Rml::Core::SetRenderInterface(m_pRmlRenderInterface);

    if (!Rml::Core::Initialise())
    {
        Warning("Failed to initialize RmlUi\n");
        return;
    }

    int width, height;
    engine->GetScreenSize(width, height);
    m_pContext = Rml::Core::CreateContext("default", Rml::Core::Vector2i(width, height));

    if (!m_pContext)
    {
        Warning("Failed to create RmlUi context");
        return;
    }

    // Load default font
    if (!Rml::Core::FontDatabase::LoadFontFace("resource/font/BigNoodleTitling.ttf"))
    {
        Warning("Failed to load default font");
        return;
    }
}

void RmlUiSystem::Update(float frametime)
{
    if (!m_pContext)
        return;
    m_pContext->Update();
}

void RmlUiSystem::LevelInitPostEntity()
{
    if (!m_pContext)
        return;

    auto doc = m_pContext->LoadDocument("test2.rml");
    if (!doc)
    {
        Warning("Failed to load test.rml\n");
    }
    else
    {
        Warning("Successfully loaded test.rml\n");
        doc->Show();
    }
}

void RmlUiSystem::Draw()
{
    if (!m_pContext)
        return;
    m_pContext->Render();
}

RmlUiSystem *RmlUiSystem::Get() { return &g_RmlUi; }
