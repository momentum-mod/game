#include "cbase.h"
#include "ultralight_overlay.h"

#include <Ultralight/Buffer.h>
#include <Ultralight/Renderer.h>
#include <Ultralight/platform/Config.h>
#include <Ultralight/platform/Platform.h>
#include <clientmode.h>
#include <vgui/ISurface.h>

using namespace ultralight;
using namespace vgui;

static IndexType patternCW[] = {0, 1, 3, 1, 2, 3};
static IndexType patternCCW[] = {0, 3, 1, 1, 3, 2};

static ImageFormat Ultralight2SourceImageFormat(BitmapFormat format)
{
    switch (format)
    {
    case BitmapFormat::kBitmapFormat_RGBA8:
        return IMAGE_FORMAT_RGBA8888;
    default:
        AssertMsg(false, "Unrecognised Ultralight texture format");
        return IMAGE_FORMAT_ARGB8888;
    }
}

UltralightOverlay::UltralightOverlay(Ref<Renderer> renderer, GPUDriver *driver, int x, int y, int width, int height)
    : m_pView(renderer->CreateView(width, height, true)), m_iWidth(width), m_iHeight(height), m_iXPos(x), m_iYPos(y),
      m_bDirty(true), m_pDriver(driver)
{
    m_iTextureId = surface()->CreateNewTextureID(true);
}

UltralightOverlay::~UltralightOverlay()
{
    if (m_vVertices.Size())
        m_pDriver->DestroyGeometry(m_iGeometryId);
    surface()->DestroyTextureID(m_iTextureId);
}

void UltralightOverlay::Draw()
{
    UpdateGeometry();
    m_pDriver->DrawGeometry(m_iGeometryId, 6, 0, m_GPUState);

    bool dirty = view()->is_bitmap_dirty();
    const RefPtr<Bitmap> bitmap = view()->bitmap();
    if (dirty)
    {
        surface()->DrawSetTextureRGBAEx(m_iTextureId, (const unsigned char *)bitmap->raw_pixels(), bitmap->width(),
                                        bitmap->height(), Ultralight2SourceImageFormat(bitmap->format()));
    }
    else
    {
        surface()->DrawSetTexture(m_iTextureId);
    }
    surface()->DrawTexturedRect(m_iXPos, m_iYPos, m_iXPos + width(), m_iYPos + height());
}

void UltralightOverlay::FireKeyEvent(const KeyEvent &evt) { view()->FireKeyEvent(evt); }

void UltralightOverlay::FireMouseEvent(const MouseEvent &evt)
{
    if (evt.type == MouseEvent::kType_MouseDown && evt.button == MouseEvent::kButton_Left)
        m_bHasFocus = Contains(evt.x, evt.y);
    else if (evt.type == MouseEvent::kType_MouseMoved)
        m_bHasHover = Contains(evt.x, evt.y);

    MouseEvent rel_evt = evt;
    rel_evt.x -= m_iXPos;
    rel_evt.y -= m_iYPos;

    view()->FireMouseEvent(rel_evt);
}

void UltralightOverlay::FireScrollEvent(const ScrollEvent &evt)
{
    // if (m_bHasHover)
    view()->FireScrollEvent(evt);
}

void UltralightOverlay::Resize(int width, int height)
{
    if (width == m_iWidth && height == m_iHeight)
        return;

    m_pView->Resize(width, height);

    m_iWidth = width;
    m_iHeight = height;
    m_bDirty = true;
    UpdateGeometry();

    // Update these now since they were invalidated
    RenderTarget target = m_pView->render_target();
    m_GPUState.texture_1_id = target.texture_id;
}

void UltralightOverlay::UpdateGeometry()
{
    bool initial_creation = false;
    RenderTarget target = m_pView->render_target();

    if (m_vVertices.IsEmpty())
    {
        m_vVertices.SetSize(4);
        m_vIndices.SetSize(6);

        auto &config = Platform::instance().config();
        if (config.face_winding == kFaceWinding_Clockwise)
            memcpy(m_vIndices.Base(), patternCW, sizeof(IndexType) * m_vIndices.Size());
        else
            memcpy(m_vIndices.Base(), patternCCW, sizeof(IndexType) * m_vIndices.Size());

        memset(&m_GPUState, 0, sizeof(m_GPUState));
        Matrix identity;
        identity.SetIdentity();

        int width, height;
        engine->GetScreenSize(width, height);
        m_GPUState.viewport_width = (float)width;
        m_GPUState.viewport_height = (float)height;
        m_GPUState.transform = ConvertAffineTo4x4(identity);
        m_GPUState.enable_blend = true;
        m_GPUState.enable_texturing = true;
        m_GPUState.shader_type = kShaderType_Fill;
        m_GPUState.render_buffer_id = 0; // default render target view (screen)
        m_GPUState.texture_1_id = target.texture_id;

        initial_creation = true;
    }

    if (!m_bDirty)
        return;

    Vertex_2f_4ub_2f_2f_28f v;
    memset(&v, 0, sizeof(v));

    v.data0[0] = 1; // Fill Type: Image

    v.color[0] = 255;
    v.color[1] = 255;
    v.color[2] = 255;
    v.color[3] = 255;

    float left = static_cast<float>(m_iXPos);
    float top = static_cast<float>(m_iYPos);
    float right = static_cast<float>(m_iXPos + width());
    float bottom = static_cast<float>(m_iYPos + height());

    // TOP LEFT
    v.pos[0] = v.obj[0] = left;
    v.pos[1] = v.obj[1] = top;
    v.tex[0] = target.uv_coords.left;
    v.tex[1] = target.uv_coords.top;

    m_vVertices[0] = v;

    // TOP RIGHT
    v.pos[0] = v.obj[0] = right;
    v.pos[1] = v.obj[1] = top;
    v.tex[0] = target.uv_coords.right;
    v.tex[1] = target.uv_coords.top;

    m_vVertices[1] = v;

    // BOTTOM RIGHT
    v.pos[0] = v.obj[0] = right;
    v.pos[1] = v.obj[1] = bottom;
    v.tex[0] = target.uv_coords.right;
    v.tex[1] = target.uv_coords.bottom;

    m_vVertices[2] = v;

    // BOTTOM LEFT
    v.pos[0] = v.obj[0] = left;
    v.pos[1] = v.obj[1] = bottom;
    v.tex[0] = target.uv_coords.left;
    v.tex[1] = target.uv_coords.bottom;

    m_vVertices[3] = v;

    ultralight::VertexBuffer vbuffer;
    vbuffer.format = ultralight::kVertexBufferFormat_2f_4ub_2f_2f_28f;
    vbuffer.size = static_cast<uint32_t>(sizeof(ultralight::Vertex_2f_4ub_2f_2f_28f) * m_vVertices.Size());
    vbuffer.data = (uint8_t *)m_vVertices.Base();

    ultralight::IndexBuffer ibuffer;
    ibuffer.size = static_cast<uint32_t>(sizeof(ultralight::IndexType) * m_vIndices.Size());
    ibuffer.data = (uint8_t *)m_vIndices.Base();

    if (initial_creation)
    {
        m_iGeometryId = m_pDriver->NextGeometryId();
        m_pDriver->CreateGeometry(m_iGeometryId, vbuffer, ibuffer);
    }
    else
    {
        m_pDriver->UpdateGeometry(m_iGeometryId, vbuffer, ibuffer);
    }

    m_bDirty = false;
}
