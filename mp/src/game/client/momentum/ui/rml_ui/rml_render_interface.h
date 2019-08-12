#pragma once

#include "platform.h"
#include "materialsystem/MaterialSystemUtil.h"
#include <RmlUi/Core/RenderInterface.h>

class RmlRenderInterface : public Rml::Core::RenderInterface
{
  public:
    RmlRenderInterface();

    // Called by RmlUi when it wants to render geometry that the application does not wish to optimise.
    virtual void RenderGeometry(Rml::Core::Vertex *vertices, int num_vertices, int *indices, int num_indices,
                                Rml::Core::TextureHandle texture, const Rml::Core::Vector2f &translation);

    // Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
    virtual Rml::Core::CompiledGeometryHandle CompileGeometry(Rml::Core::Vertex *vertices, int num_vertices,
                                                              int *indices, int num_indices,
                                                              Rml::Core::TextureHandle texture) OVERRIDE;

    // Called by RmlUi when it wants to render application-compiled geometry.
    virtual void RenderCompiledGeometry(Rml::Core::CompiledGeometryHandle geometry,
                                        const Rml::Core::Vector2f &translation) OVERRIDE;

    // Called by RmlUi when it wants to release application-compiled geometry.
    virtual void ReleaseCompiledGeometry(Rml::Core::CompiledGeometryHandle geometry) OVERRIDE;

    // Called by RmlUi when it wants to enable or disable scissoring to clip content.
    virtual void EnableScissorRegion(bool enable) OVERRIDE;

    // Called by RmlUi when it wants to change the scissor region.
    virtual void SetScissorRegion(int x, int y, int width, int height) OVERRIDE;

    // Called by RmlUi when a texture is required by the library.
    virtual bool LoadTexture(Rml::Core::TextureHandle &texture_handle, Rml::Core::Vector2i &texture_dimensions,
                             const Rml::Core::String &source);

    // Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
    virtual bool GenerateTexture(Rml::Core::TextureHandle &texture_handle, const Rml::Core::byte *source,
                                 const Rml::Core::Vector2i &source_dimensions);

    // Called by RmlUi when a loaded texture is no longer required.
    virtual void ReleaseTexture(Rml::Core::TextureHandle texture_handle) OVERRIDE;

    // Returns the native horizontal texel offset for the renderer.
    virtual float GetHorizontalTexelOffset() OVERRIDE { return -0.5f; }

    // Returns the native vertical texel offset for the renderer.
    virtual float GetVerticalTexelOffset() OVERRIDE { return -0.5f; }

  private:
    CMaterialReference m_pDrawMaterial;
    bool m_bScissorRegionEnabled;
    int m_iScissorLeft;
    int m_iScissorRight;
    int m_iScissorTop;
    int m_iScissorBottom;
};