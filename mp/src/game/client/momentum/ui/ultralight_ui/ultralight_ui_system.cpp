#include "cbase.h"
#include "ultralight_ui_system.h"

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/ishader.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/itexture.h"
#include "ultralight_filesystem.h"
#include "ultralight_overlay.h"
#include <pixelwriter.h>

#define TEXTURE_GROUP_ULTRALIGHT "UL textures"

using namespace ultralight;

static UltralightUISystem g_UltralightSystem;
CBaseGameSystemPerFrame *UltralightUI() { return &g_UltralightSystem; }

static SourceFileSystem g_UltralightFileSystem;
static FontLoader *g_pUltralightFontLoader = nullptr;

static UltralightOverlay *g_pTestOverlay = nullptr;

class SourceGPUDriver : public GPUDriver
{
  public:
    SourceGPUDriver() : m_iTextureCount(1), m_iGeometryCount(0), m_iRenderBufferCount(0)
    {
        m_FillDrawMaterial.Init("__ulFill", new KeyValues("UL_FILL"));
        m_FillDrawMaterial->Refresh();
        m_FillPathDrawMaterial.Init("__ulFillPath", new KeyValues("UL_FILL_PATH"));
        m_FillDrawMaterial->Refresh();
    }

  public: // GPUDriver
    virtual void BeginSynchronize() OVERRIDE{};
    virtual void EndSynchronize() OVERRIDE{};

    virtual uint32_t NextTextureId() { return m_iTextureCount++; }

    // Create a texture with a certain ID and optional bitmap. If the Bitmap is
    // empty (Bitmap::IsEmpty), then a RTT Texture should be created instead.
    virtual void CreateTexture(uint32_t texture_id, Ref<Bitmap> bitmap)
    {
        ImageFormat format = Ultralight2SourceImageFormat(bitmap->format());
        ITexture *texture = nullptr;

        if (!bitmap->IsEmpty())
        {
            texture = g_pMaterialSystem->CreateTextureFromBits(bitmap->width(), bitmap->height(), 0, format,
                                                               bitmap->size(), (byte *)bitmap->raw_pixels());
            Assert(!texture->IsError());
        }
        m_Textures.AddToTail(TextureEntry(texture_id, texture, format));
    }

    // Update an existing non-RTT texture with new bitmap data.
    virtual void UpdateTexture(uint32_t texture_id, Ref<Bitmap> bitmap)
    {
        for (int i = 0; i < m_Textures.Size(); i++)
        {
            TextureEntry &entry = m_Textures[i];
            if (entry.id == texture_id)
            {
                // HACK: We're recreating the texture every time we want to update it because the procedural/dynamic textures just wouldn't work for me
                ImageFormat format = Ultralight2SourceImageFormat(bitmap->format());

                entry.texture->Release();
                entry.texture->DeleteIfUnreferenced();
                entry.texture = g_pMaterialSystem->CreateTextureFromBits(
                                bitmap->width(), bitmap->height(), 0, format, bitmap->size(), (byte *)bitmap->raw_pixels());
                return;
            }
        }
    }

    // Bind a texture to a certain texture unit.
    virtual void BindTexture(uint8_t texture_unit, uint32_t texture_id)
    {
        for (int i = 0; i < m_Textures.Size(); i++)
        {
            const TextureEntry &entry = m_Textures[i];
            if (entry.id == texture_id)
            {
                char buf[32];
                Q_snprintf(buf, sizeof(buf), "$TEXTURE%i", texture_unit);
                m_FillDrawMaterial->FindVar(buf, nullptr)->SetTextureValue(entry.texture);
                return;
            }
        }
    }

    // Destroy a texture.
    virtual void DestroyTexture(uint32_t texture_id)
    {
        for (int i = 0; i < m_Textures.Size(); i++)
        {
            const TextureEntry &entry = m_Textures[i];
            if (entry.id == texture_id)
            {
                if (entry.texture)
                {
                    entry.texture->Release();
                    entry.texture->DeleteIfUnreferenced();
                }
                m_Textures.FastRemove(i);
                return;
            }
        }
    }

    /******************************
     * Offscreen Rendering        *
     ******************************/

    // Generate the next available render buffer ID.
    virtual uint32_t NextRenderBufferId() { return m_iRenderBufferCount++; }

    // Create a render buffer with certain ID and buffer description.
    virtual void CreateRenderBuffer(uint32_t render_buffer_id, const RenderBuffer &buffer)
    {
        TextureEntry *entry = nullptr;
        for (int i = 0; i < m_Textures.Size(); i++)
        {
            if (m_Textures[i].id == buffer.texture_id)
            {
                entry = &m_Textures[i];
                break;
            }
        }

        if (entry == nullptr)
        {
            Warning("CreateRenderBuffer called with invalid texture_id");
            return;
        }

        g_pMaterialSystem->BeginRenderTargetAllocation();

        MaterialRenderTargetDepth_t depth = buffer.has_depth_buffer ? MATERIAL_RT_DEPTH_SHARED : MATERIAL_RT_DEPTH_NONE;
        entry->texture = g_pMaterialSystem->CreateRenderTargetTexture(buffer.width, buffer.height, RT_SIZE_OFFSCREEN,
                                                                      entry->format, depth);

        g_pMaterialSystem->EndRenderTargetAllocation();

        entry->texture->AddRef();
        m_RenderBuffers.AddToTail(RenderBufferEntry(render_buffer_id, entry->texture));
    }

    // Bind a render buffer
    virtual void BindRenderBuffer(uint32_t render_buffer_id)
    {
        if (render_buffer_id == 0)
        {
            g_pMaterialSystem->GetRenderContext()->SetRenderTarget(nullptr);
            return;
        }

        for (int i = 0; i < m_RenderBuffers.Size(); i++)
        {
            if (m_RenderBuffers[i].id == render_buffer_id)
            {
                g_pMaterialSystem->GetRenderContext()->SetRenderTarget(m_RenderBuffers[i].render_target);
                return;
            }
        }
    }

    // Clear a render buffer (flush pixels)
    virtual void ClearRenderBuffer(uint32_t render_buffer_id)
    {
        for (int i = 0; i < m_RenderBuffers.Size(); i++)
        {
            if (m_RenderBuffers[i].id == render_buffer_id)
            {
                g_pMaterialSystem->GetRenderContext()->PushRenderTargetAndViewport(m_RenderBuffers[i].render_target);
                g_pMaterialSystem->GetRenderContext()->ClearBuffers(true, true);
                g_pMaterialSystem->GetRenderContext()->PopRenderTargetAndViewport();
                return;
            }
        }
    }

    // Destroy a render buffer
    virtual void DestroyRenderBuffer(uint32_t render_buffer_id)
    {
        for (int i = 0; i < m_RenderBuffers.Size(); i++)
        {
            const RenderBufferEntry &entry = m_RenderBuffers[i];
            if (entry.id == render_buffer_id)
            {
                if (entry.render_target)
                {
                    entry.render_target->Release();
                    entry.render_target->DeleteIfUnreferenced();
                }
                m_RenderBuffers.FastRemove(i);
                return;
            }
        }
    }

    /******************************
     * Geometry                   *
     ******************************/

    // Generate the next available geometry ID.
    virtual uint32_t NextGeometryId() { return m_iGeometryCount++; }

    // Create geometry with certain ID and vertex/index data.
    virtual void CreateGeometry(uint32_t geometry_id, const VertexBuffer &vertices, const IndexBuffer &indices)
    {
        IMatRenderContext *pContext = g_pMaterialSystem->GetRenderContext();
        VertexFormat_t format = Ultralight2SourceVertexFormat(vertices.format);
        IMesh *pMesh = pContext->CreateStaticMesh(format, TEXTURE_GROUP_ULTRALIGHT);

        CMeshBuilder builder;
        builder.Begin(pMesh, MATERIAL_TRIANGLE_STRIP, vertices.size, indices.size);
        Ultralight2SourceMesh(builder, vertices, indices);
        builder.End();

        m_Geometries.AddToTail(GeometryEntry(geometry_id, pMesh, vertices.format));
    }

    // Update existing geometry with new vertex/index data.
    virtual void UpdateGeometry(uint32_t geometry_id, const VertexBuffer &vertices, const IndexBuffer &indices)
    {
        for (int i = 0; i < m_Geometries.Size(); i++)
        {
            if (m_Geometries[i].id == geometry_id)
            {
                CMeshBuilder builder;
                builder.BeginModify(m_Geometries[i].mesh, vertices.size, 0, indices.size);
                Ultralight2SourceMesh(builder, vertices, indices);
                builder.EndModify();
                return;
            }
        }
    }

    // Draw geometry using the specific index count/offset and GPUState.
    virtual void DrawGeometry(uint32_t geometry_id, uint32_t indices_count, uint32_t indices_offset,
                              const GPUState &state)
    {
        GeometryEntry *pGeometry = nullptr;
        for (int i = 0; i < m_Geometries.Size(); i++)
        {
            if (m_Geometries[i].id == geometry_id)
            {
                pGeometry = &m_Geometries[i];
                break;
            }
        }

        if (!pGeometry)
        {
            AssertMsg(false, "Invalid geometry id passed");
            return;
        }

        BindRenderBuffer(state.render_buffer_id);
        g_pMaterialSystem->GetRenderContext()->Viewport(0, 0, (int)state.viewport_width, (int)state.viewport_height);

        IMaterial *pMat = (state.shader_type == kShaderType_FillPath) ? m_FillPathDrawMaterial : m_FillDrawMaterial;

        UpdateMaterialVars(state, pMat);

        IMatRenderContext *pContext = g_pMaterialSystem->GetRenderContext();
        pContext->BeginRender();
        pContext->Bind(pMat);
        pGeometry->mesh->Draw();
        pContext->EndRender();
    }

    // Destroy geometry.
    virtual void DestroyGeometry(uint32_t geometry_id)
    {
        for (int i = 0; i < m_Geometries.Size(); i++)
        {
            GeometryEntry &entry = m_Geometries[i];
            if (entry.id == geometry_id)
            {
                g_pMaterialSystem->GetRenderContext()->DestroyStaticMesh(entry.mesh);
                m_Geometries.FastRemove(i);
                return;
            }
        }
    }

    /******************************
     * Command List               *
     ******************************/

    // Update command list (you should copy the commands to your own structure).
    virtual void UpdateCommandList(const CommandList &list) { m_CommandList.CopyArray(list.commands, list.size); };

    // Check if any commands need drawing.
    virtual bool HasCommandsPending() { return !m_CommandList.IsEmpty(); }

    // Iterate through stored command list and dispatch to ClearRenderBuffer or
    // DrawGeometry, respectively. Command list should be cleared at end of call.
    virtual void DrawCommandList()
    {
        if (m_CommandList.IsEmpty())
        {
            return;
        }

        int size = m_CommandList.Size();
        for (int i = 0; i < size; i++)
        {
            const Command &cmd = m_CommandList[i];
            if (cmd.command_type == kCommandType_DrawGeometry)
            {
                DrawGeometry(cmd.geometry_id, cmd.indices_count, cmd.indices_offset, cmd.gpu_state);
            }
            else if (cmd.command_type == kCommandType_ClearRenderBuffer)
            {
                ClearRenderBuffer(cmd.gpu_state.render_buffer_id);
            }
        }

        m_CommandList.RemoveAll();
    }

  private:
    ImageFormat Ultralight2SourceImageFormat(BitmapFormat format)
    {
        switch (format)
        {
        case BitmapFormat::kBitmapFormat_RGBA8:
            return IMAGE_FORMAT_BGRA8888;
        default:
            AssertMsg(false, "Unrecognised Ultralight texture format");
            return IMAGE_FORMAT_BGRA8888;
        }
    }

    VertexFormat_t Ultralight2SourceVertexFormat(int format)
    {
        switch (format)
        {
        case kVertexBufferFormat_2f_4ub_2f:
            return VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2);
        case kVertexBufferFormat_2f_4ub_2f_2f_28f:
            return VERTEX_POSITION | VERTEX_COLOR | VERTEX_TEXCOORD_SIZE(0, 2) | VERTEX_TEXCOORD_SIZE(1, 2) |
                   VERTEX_TEXCOORD_SIZE(2, 4) | VERTEX_TEXCOORD_SIZE(3, 4) | VERTEX_TEXCOORD_SIZE(4, 4) |
                   VERTEX_TEXCOORD_SIZE(5, 4) | VERTEX_TEXCOORD_SIZE(6, 4) | VERTEX_TEXCOORD_SIZE(7, 4) |
                   VERTEX_USERDATA_SIZE(4);
        default:
            AssertMsg(false, "Unrecognised Ultralight vertex format");
            return 0;
        }
    }

    void Ultralight2SourceMesh(CMeshBuilder &builder, const VertexBuffer &vertices, const IndexBuffer &indices)
    {
        uint32_t vertsize = (vertices.format == kVertexBufferFormat_2f_4ub_2f) ? sizeof(Vertex_2f_4ub_2f)
                                                                            : sizeof(Vertex_2f_4ub_2f_2f_28f);
        uint32_t total = vertices.size / vertsize;
        for (uint32_t i = 0; i < total; i++)
        {
            if (vertices.format == kVertexBufferFormat_2f_4ub_2f)
            {
                auto pVertices = reinterpret_cast<Vertex_2f_4ub_2f *>(vertices.data);
                builder.Position3f(pVertices[i].pos[0], pVertices[i].pos[1], 0.0f);
                builder.Color4ubv(pVertices[i].color);
                builder.TexCoord2fv(0, pVertices[i].obj);
            }
            else if (vertices.format == kVertexBufferFormat_2f_4ub_2f_2f_28f)
            {
                auto pVertices = reinterpret_cast<Vertex_2f_4ub_2f_2f_28f *>(vertices.data);

                Log("Position (%i/%i): {%f, %f}\n", i + 1, total, pVertices[i].pos[0], pVertices[i].pos[1]);
                Log("Color (%i/%i): {%i, %i, %i, %i}\n", i + 1, total, pVertices[i].color[0], pVertices[i].color[1], pVertices[i].color[2], pVertices[i].color[3]);
                Log("Tex (%i/%i): {%f, %f}\n", i + 1, total, pVertices[i].tex[0], pVertices[i].tex[1]);
                Log("Obj (%i/%i): {%f, %f}\n", i + 1, total, pVertices[i].obj[0], pVertices[i].obj[1]);
                Log("Data0 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data0[0], pVertices[i].data0[1], pVertices[i].data0[2], pVertices[i].data0[3]);
                Log("Data1 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data1[0], pVertices[i].data1[1], pVertices[i].data1[2], pVertices[i].data1[3]);
                Log("Data2 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data2[0], pVertices[i].data2[1], pVertices[i].data2[2], pVertices[i].data2[3]);
                Log("Data3 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data3[0], pVertices[i].data3[1], pVertices[i].data3[2], pVertices[i].data3[3]);
                Log("Data4 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data4[0], pVertices[i].data4[1], pVertices[i].data4[2], pVertices[i].data4[3]);
                Log("Data5 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data5[0], pVertices[i].data5[1], pVertices[i].data5[2], pVertices[i].data5[3]);
                Log("Data6 (%i/%i): {%f, %f, %f, %f}\n", i + 1, total, pVertices[i].data6[0], pVertices[i].data6[1], pVertices[i].data6[2], pVertices[i].data6[3]);

                builder.Position3f(pVertices[i].pos[0], pVertices[i].pos[1], 0.0f);
                builder.Color4ubv(pVertices[i].color);
                builder.TexCoord2fv(0, pVertices[i].tex);
                builder.TexCoord2fv(1, pVertices[i].obj);
                builder.TexCoord4fv(2, pVertices[i].data0);
                builder.TexCoord4fv(3, pVertices[i].data1);
                builder.TexCoord4fv(4, pVertices[i].data2);
                builder.TexCoord4fv(5, pVertices[i].data3);
                builder.TexCoord4fv(6, pVertices[i].data4);
                builder.TexCoord4fv(7, pVertices[i].data5);
                builder.UserData(pVertices[i].data6); // No more texcoord slots left...
            }
            builder.AdvanceVertex();
        }

        for (uint32_t i = 0; i < indices.size / sizeof(IndexType); i++)
        {
            auto pIndices = reinterpret_cast<IndexType *>(indices.data);
            builder.FastIndex((unsigned short)pIndices[i]);
        }
    }

    ITexture *FindTexture(uint32_t id)
    {
        for (int i = 0; i < m_Textures.Size(); i++)
        {
            if (m_Textures[i].id == id)
            {
                return m_Textures[i].texture;
            }
        }

        return nullptr;
    }

    void UpdateMaterialVars(const GPUState &gpu_state, IMaterial *pMaterial)
    {
        float state[4];
        state[0] = 0.0f;
        state[1] = gpu_state.viewport_width;
        state[2] = gpu_state.viewport_height;
        state[3] = 0.0f;
        pMaterial->FindVar("$STATE", nullptr)->SetVecValue(state, 4);

        VMatrix transform(gpu_state.transform.data);
        pMaterial->FindVar("$TRANSFORM", nullptr)->SetMatrixValue(transform);

        float scalar[4];
        scalar[0] = gpu_state.uniform_scalar[0];
        scalar[1] = gpu_state.uniform_scalar[1];
        scalar[2] = gpu_state.uniform_scalar[2];
        scalar[3] = gpu_state.uniform_scalar[3];
        pMaterial->FindVar("$SCALAR0", nullptr)->SetVecValue(scalar, 4);

        scalar[0] = gpu_state.uniform_scalar[4];
        scalar[1] = gpu_state.uniform_scalar[5];
        scalar[2] = gpu_state.uniform_scalar[6];
        scalar[3] = gpu_state.uniform_scalar[7];
        pMaterial->FindVar("$SCALAR1", nullptr)->SetVecValue(scalar, 4);

        for (int i = 0; i < 8; i++)
        {
            char buf[32];
            Q_snprintf(buf, sizeof(buf), "$VECTOR%i", i);
            pMaterial->FindVar(buf, nullptr)->SetVecValue(gpu_state.uniform_vector[i].value, 4);
        }

        pMaterial->FindVar("$CLIP_SIZE", nullptr)->SetIntValue(gpu_state.clip_size);
        for (int i = 0; i < gpu_state.clip_size; i++)
        {
            VMatrix clip(gpu_state.clip[i].data);
            char buf[32];
            Q_snprintf(buf, sizeof(buf), "$CLIP%i", i);
            pMaterial->FindVar(buf, nullptr)->SetMatrixValue(clip);
        }

        if (gpu_state.texture_1_id)
        {
            BindTexture(0, gpu_state.texture_1_id);
        }
        if (gpu_state.texture_2_id)
        {
            BindTexture(1, gpu_state.texture_2_id);
        }
    }

  private:
    CMaterialReference m_FillDrawMaterial;
    CMaterialReference m_FillPathDrawMaterial;
    CUtlVector<Command> m_CommandList;

    uint32_t m_iTextureCount;
    struct TextureEntry
    {
        TextureEntry(uint32_t id, ITexture *texture, ImageFormat format) : id(id), texture(texture), format(format) {}

        uint32_t id;
        ImageFormat format;
        ITexture *texture;
    };
    CUtlVector<TextureEntry> m_Textures;

    uint32_t m_iRenderBufferCount;
    struct RenderBufferEntry
    {
        RenderBufferEntry(uint32_t id, ITexture *pRenderTarget) : id(id), render_target(pRenderTarget) {}

        ITexture *render_target;
        uint32_t id;
    };
    CUtlVector<RenderBufferEntry> m_RenderBuffers;

    uint32_t m_iGeometryCount;
    struct GeometryEntry
    {
        GeometryEntry(uint32_t id, IMesh *pMesh, VertexBufferFormat format) : id(id), mesh(pMesh), format(format) {}

        IMesh *mesh;
        VertexBufferFormat format;
        uint32_t id;
    };
    CUtlVector<GeometryEntry> m_Geometries;
};

bool UltralightUISystem::Init()
{
    g_pUltralightFontLoader = CreateULPlatformFontLoader();

    Config config;
    config.face_winding = kFaceWinding_Clockwise; // CW in D3D, CCW in OGL
    config.device_scale_hint = 1.0;               // Set DPI to monitor DPI scale
    config.enable_javascript = true;
    config.enable_images = true;

    Platform &platform = Platform::instance();
    platform.set_config(config);
    platform.set_file_system(&g_UltralightFileSystem);
    platform.set_font_loader(DefaultFontLoader());
    m_pGPUDriver = new SourceGPUDriver();
    platform.set_gpu_driver(m_pGPUDriver);

    m_pRenderer = Renderer::Create();

    g_pTestOverlay = new UltralightOverlay(*m_pRenderer, m_pGPUDriver, 300, 300, 150, 150);
    g_pTestOverlay->view()->LoadHTML("<p color='green'>HTML UI!!!!</p>");

    return true;
}

void UltralightUISystem::Shutdown()
{
    delete g_pUltralightFontLoader;
    g_pUltralightFontLoader = nullptr;

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

    g_pTestOverlay->Draw();
}