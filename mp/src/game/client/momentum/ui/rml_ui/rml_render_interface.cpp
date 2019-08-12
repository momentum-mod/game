#include "cbase.h"
#include "rml_render_interface.h"

#include "KeyValues.h"
#include "filesystem.h"
#include "fmtstr.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "pixelwriter.h"
#include "utlbuffer.h"

#define STBI_NO_STDIO
#define STBI_NO_HDR
#include "stb/image/stb_image.h"

#define TEXTURE_GROUP_RML "rml"

struct CompiledGeometry
{
    IMesh *mesh;
    ITexture *texture;
};

static VertexFormat_t GetRmlVertexFormat();
static void Rml2SourceMesh(CMeshBuilder &builder, Rml::Core::Vertex *vertices, int num_vertices, int *indices,
                           int num_indices);
static void UpdateMaterialVars(IMaterial *pMaterial, const Rml::Core::Vector2f &translation,
                               CompiledGeometry *pGeometry);

RmlRenderInterface::RmlRenderInterface()
{
    m_pDrawMaterial.Init("__rmlDraw", new KeyValues("rml_draw"));

    m_bScissorRegionEnabled = false;
    m_iScissorLeft = 0;
    m_iScissorRight = 0;
    m_iScissorTop = 0;
    m_iScissorBottom = 0;
}

void RmlRenderInterface::RenderGeometry(Rml::Core::Vertex *vertices, int num_vertices, int *indices, int num_indices,
                                        Rml::Core::TextureHandle texture, const Rml::Core::Vector2f &translation)
{
    // This function should never be called, CompileGeometry should cover all cases
    AssertMsg(false, "Unhandled RmlUi render call");
}

Rml::Core::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Core::Vertex *vertices, int num_vertices,
                                                                      int *indices, int num_indices,
                                                                      Rml::Core::TextureHandle texture)
{
    auto geometry = new CompiledGeometry;

    IMatRenderContext *pContext = materials->GetRenderContext();
    VertexFormat_t format = GetRmlVertexFormat();
    IMesh *pMesh = pContext->CreateStaticMesh(format, TEXTURE_GROUP_RML);
    pContext->Release();

    CMeshBuilder builder;
    builder.Begin(pMesh, MATERIAL_TRIANGLES, num_vertices, num_indices);
    Rml2SourceMesh(builder, vertices, num_vertices, indices, num_indices);
    builder.End();

    geometry->mesh = pMesh;
    geometry->texture = (ITexture *)texture;

    return (Rml::Core::CompiledGeometryHandle)geometry;
}

static VertexFormat_t GetRmlVertexFormat() { return VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2); }

static void Rml2SourceMesh(CMeshBuilder &builder, Rml::Core::Vertex *vertices, int num_vertices, int *indices,
                           int num_indices)
{
    for (int i = 0; i < num_vertices; i++)
    {
        builder.Position3f(vertices[i].position.x, vertices[i].position.y, 0.0f);
        builder.Color4ub(vertices[i].colour.red, vertices[i].colour.green, vertices[i].colour.blue,
                         vertices[i].colour.alpha);
        builder.TexCoord2f(0, vertices[i].tex_coord.x, vertices[i].tex_coord.y);

        builder.AdvanceVertex();
    }

    for (int i = 0; i < num_indices; i++)
    {
        AssertMsg(indices[i] >= 0 && indices[i] < USHRT_MAX, "Indices outside unsigned short range");
        builder.FastIndex((unsigned short)indices[i]);
    }
}

void RmlRenderInterface::RenderCompiledGeometry(Rml::Core::CompiledGeometryHandle geometry,
                                                const Rml::Core::Vector2f &translation)
{
    auto pGeometry = (CompiledGeometry *)geometry;

    UpdateMaterialVars(m_pDrawMaterial, translation, pGeometry);
    m_pDrawMaterial->RefreshPreservingMaterialVars();

    CMatRenderContextPtr pRenderContext(materials);

    int vx, vy, vw, vh;
    pRenderContext->GetViewport(vx, vy, vw, vh);

    pRenderContext->SetScissorRect(m_iScissorLeft, m_iScissorTop, m_iScissorRight, m_iScissorBottom, m_bScissorRegionEnabled);

    pRenderContext->MatrixMode(MATERIAL_VIEW);
    pRenderContext->PushMatrix();
    pRenderContext->LoadIdentity();

    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
    pRenderContext->PushMatrix();
    pRenderContext->LoadIdentity();
    pRenderContext->Scale(1.0f, -1.0f, 1.0f);
    pRenderContext->Ortho(0.0f, 0.0f, vw, vh, -1000.0f, 0.0f);

    pRenderContext->Bind(m_pDrawMaterial);
    pGeometry->mesh->Draw();

    pRenderContext->MatrixMode(MATERIAL_VIEW);
    pRenderContext->PopMatrix();

    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
    pRenderContext->PopMatrix();
}

static void UpdateMaterialVars(IMaterial *pMaterial, const Rml::Core::Vector2f &translation,
                               CompiledGeometry *pGeometry)
{
    if (pGeometry->texture)
    {
        pMaterial->FindVar("$TEXTURE", nullptr)->SetTextureValue(pGeometry->texture);
    }
    else
    {
        pMaterial->FindVar("$TEXTURE", nullptr)->SetUndefined();
    }
    pMaterial->FindVar("$OFFSET_X", nullptr)->SetFloatValue(translation.x);
    pMaterial->FindVar("$OFFSET_Y", nullptr)->SetFloatValue(translation.y);
}

void RmlRenderInterface::ReleaseCompiledGeometry(Rml::Core::CompiledGeometryHandle geometry)
{
    delete (CompiledGeometry *)geometry;
}

void RmlRenderInterface::EnableScissorRegion(bool enable)
{
    m_bScissorRegionEnabled = enable;
}

void RmlRenderInterface::SetScissorRegion(int x, int y, int width, int height)
{
    m_iScissorLeft = x;
    m_iScissorRight = x + width;
    m_iScissorTop = y;
    m_iScissorBottom = y + height;
}

bool RmlRenderInterface::LoadTexture(Rml::Core::TextureHandle &texture_handle, Rml::Core::Vector2i &texture_dimensions,
                                     const Rml::Core::String &source)
{
    CUtlBuffer buf;
    g_pFullFileSystem->ReadFile(source.CString(), "GAME", buf);

    int w, h, channels;
    unsigned char *pImageData =
        stbi_load_from_memory((stbi_uc *)buf.Base(), buf.TellPut(), &w, &h, &channels, STBI_rgb_alpha);
    if (!pImageData)
        return false;

    texture_dimensions.x = w;
    texture_dimensions.y = h;

    return GenerateTexture(texture_handle, pImageData, texture_dimensions);
}

bool RmlRenderInterface::GenerateTexture(Rml::Core::TextureHandle &texture_handle, const Rml::Core::byte *source,
                                         const Rml::Core::Vector2i &source_dimensions)
{
    int size = source_dimensions.x * source_dimensions.y * 4; // 32 bpp
    ITexture *pTexture = materials->CreateTextureFromBits(source_dimensions.x, source_dimensions.y, 1,
                                                          IMAGE_FORMAT_RGBA8888, size, (byte*)source);
    texture_handle = (Rml::Core::TextureHandle)pTexture;
    
    return true;
}

void RmlRenderInterface::ReleaseTexture(Rml::Core::TextureHandle texture_handle)
{
    auto pTexture = (ITexture *)texture_handle;
    pTexture->Release();
}
