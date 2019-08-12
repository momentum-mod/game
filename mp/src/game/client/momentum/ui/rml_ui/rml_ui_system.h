#pragma once

#include "rml_render_interface.h"
#include "rml_system_interface.h"
#include "rml_filesystem_interface.h"
#include "igamesystem.h"

class RmlUiSystem : public CAutoGameSystemPerFrame
{
  public:
    RmlUiSystem();
    ~RmlUiSystem();

    virtual void PostInit() OVERRIDE;

    // Gets called each frame
    virtual void Update(float frametime) OVERRIDE;

    virtual void LevelInitPostEntity();

    void Draw();

  public:
    static RmlUiSystem *Get();

  private:
    RmlFileInterface* m_pRmlFileInterface;
    RmlSystemInterface* m_pRmlSystemInterface;
    RmlRenderInterface* m_pRmlRenderInterface;
    Rml::Core::Context *m_pContext;
};