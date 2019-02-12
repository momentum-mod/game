#ifndef ULTRALIGHT_UI_SYSTEM_H
#define ULTRALIGHT_UI_SYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "platform.h"

#include <Ultralight/Ultralight.h>

class UltralightOverlay;
namespace vgui
{
    class Panel;
}

class UltralightUISystem : public CAutoGameSystemPerFrame
{
  public:
    UltralightUISystem();

    virtual char const *Name() { return "Ultralight UI"; }

    virtual bool Init() OVERRIDE;
    virtual void Shutdown() OVERRIDE;

    virtual void PreRender() OVERRIDE;
    virtual void PostRender() OVERRIDE;

	UltralightOverlay *CreateOverlay(vgui::Panel *pParentPanel, int x, int y, int width, int height);
  private:
    ultralight::RefPtr<ultralight::Renderer> m_pRenderer;
    ultralight::GPUDriver *m_pGPUDriver;
};

UltralightUISystem *UltralightUI();

#endif