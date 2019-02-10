#ifndef ULTRALIGHT_UI_SYSTEM_H
#define ULTRALIGHT_UI_SYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "platform.h"

#include <Ultralight/Ultralight.h>

class UltralightUISystem : public CAutoGameSystemPerFrame
{
  public:
    virtual char const *Name() { return "Ultralight UI"; }

    virtual bool Init() OVERRIDE;
    virtual void Shutdown() OVERRIDE;

    virtual void PreRender() OVERRIDE;
    virtual void PostRender() OVERRIDE;

  private:
    ultralight::RefPtr<ultralight::Renderer> m_pRenderer;
    ultralight::GPUDriver *m_pGPUDriver;
};

CBaseGameSystemPerFrame *UltralightUI();

#endif