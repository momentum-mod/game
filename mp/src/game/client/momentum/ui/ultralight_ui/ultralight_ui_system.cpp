#include "cbase.h"
#include "ultralight_ui_system.h"

#include "ultralight_filesystem.h"
#include "ultralight_overlay.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/ishader.h"
#include "materialsystem/imaterialvar.h"

using namespace ultralight;

static UltralightUISystem g_UltralightSystem;
CBaseGameSystemPerFrame *UltralightUI() { return &g_UltralightSystem; }

static SourceFileSystem g_UltralightFileSystem;
static FontLoader      *g_pUltralightFontLoader = nullptr;

static UltralightOverlay *g_pTestOverlay = nullptr;

class SourceGPUDriver : public GPUDriver, ITextureRegenerator
{
  public:
	  SourceGPUDriver()
        :
		  m_iTextureCount(0) {}

  public: // ITextureRegenerator

	virtual void Release() OVERRIDE {}
	virtual void RegenerateTextureBits( ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pSubRect )
	{
        memcpy(pVTFTexture->ImageData(), m_pCurrentBitmap->raw_pixels(), m_pCurrentBitmap->size());
	}

	virtual void BeginSynchronize() OVERRIDE {};
	virtual void EndSynchronize() OVERRIDE {};

	virtual uint32_t NextTextureId() { return m_iTextureCount++; }

	// Create a texture with a certain ID and optional bitmap. If the Bitmap is
	// empty (Bitmap::IsEmpty), then a RTT Texture should be created instead.
	virtual void CreateTexture(uint32_t texture_id, Ref<Bitmap> bitmap)
	{
		
		ImageFormat format = Ultralight2SourceFormat(bitmap->format());
		ITexture *texture;

		char szTextureName[128];
		Q_snprintf(szTextureName, sizeof(szTextureName), "texture_%i", texture_id);

		if (bitmap->IsEmpty())
		{
			texture = g_pMaterialSystem->CreateProceduralTexture(szTextureName, "ultralight_ui", bitmap->width(),
																	bitmap->height(), format, TEXTUREFLAGS_PROCEDURAL);
			texture->SetTextureRegenerator(this);
		}
		else
		{
			texture = g_pMaterialSystem->CreateProceduralTexture(szTextureName, "ultralight_ui", bitmap->width(),
																	bitmap->height(), format, TEXTUREFLAGS_RENDERTARGET);
		}
		m_Textures.AddToTail(TextureEntry(texture_id, texture));
	}

	// Update an existing non-RTT texture with new bitmap data.
	virtual void UpdateTexture(uint32_t texture_id, Ref<Bitmap> bitmap)
	{
		for (int i = 0; i < m_Textures.Size(); i++)
		{
			if (m_Textures[i].id == texture_id)
			{
                m_pCurrentBitmap = bitmap;
                // It's called "Download", but what it really means is "ReconstructBits"
                m_Textures[i].texture->Download();
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
				// somehow bind
			}
		}
	}

	// Destroy a texture.
	virtual void DestroyTexture(uint32_t texture_id)
	{
		for (int i = 0; i < m_Textures.Size(); i++)
		{
			const TextureEntry& entry = m_Textures[i];
			if (entry.id == texture_id)
			{
				entry.texture->Release();
				entry.texture->DeleteIfUnreferenced();
				m_Textures.FastRemove(i);
				return;
			}
		}
	}

	/******************************
		* Offscreen Rendering        *
		******************************/

	// Generate the next available render buffer ID.
	virtual uint32_t NextRenderBufferId()
	{
	}

	// Create a render buffer with certain ID and buffer description.
	virtual void CreateRenderBuffer(uint32_t render_buffer_id, const RenderBuffer &buffer)
	{
	}

	// Bind a render buffer
	virtual void BindRenderBuffer(uint32_t render_buffer_id) = 0;

	// Clear a render buffer (flush pixels)
	virtual void ClearRenderBuffer(uint32_t render_buffer_id) = 0;

	// Destroy a render buffer
	virtual void DestroyRenderBuffer(uint32_t render_buffer_id) = 0;

	/******************************
		* Geometry                   *
		******************************/

	// Generate the next available geometry ID.
    virtual uint32_t NextGeometryId() { return m_iGeometryCount++; }

	// Create geometry with certain ID and vertex/index data.
	virtual void CreateGeometry(uint32_t geometry_id, const VertexBuffer &vertices, const IndexBuffer &indices)
	{
	}

	// Update existing geometry with new vertex/index data.
	virtual void UpdateGeometry(uint32_t geometry_id, const VertexBuffer &vertices, const IndexBuffer &indices) = 0;

	// Draw geometry using the specific index count/offset and GPUState.
	virtual void DrawGeometry(uint32_t geometry_id, uint32_t indices_count, uint32_t indices_offset,
								const GPUState &state) = 0;

	// Destroy geometry.
	virtual void DestroyGeometry(uint32_t geometry_id) = 0;

	/******************************
		* Command List               *
		******************************/

	// Update command list (you should copy the commands to your own structure).
	virtual void UpdateCommandList(const CommandList &list) = 0;

	// Check if any commands need drawing.
	virtual bool HasCommandsPending() = 0;

	// Iterate through stored command list and dispatch to ClearRenderBuffer or
	// DrawGeometry, respectively. Command list should be cleared at end of call.
	virtual void DrawCommandList() = 0;

  private:
	ImageFormat Ultralight2SourceFormat(BitmapFormat format) {
		switch(format)
		{
        case BitmapFormat::kBitmapFormat_A8:
            return IMAGE_FORMAT_A8;
        case BitmapFormat::kBitmapFormat_RGBA8:
            return IMAGE_FORMAT_RGBA8888;
        default:
            AssertMsg(false, "Unrecognised Ultralight texture format");
		}
	}
  private:
    RefPtr<Bitmap> m_pCurrentBitmap;

	uint32_t m_iTextureCount;
	struct TextureEntry
	{
		TextureEntry(uint32_t id, ITexture* texture) :
			id(id), texture(texture) {}

		uint32_t id;
		ITexture *texture;
	};
	CUtlVector<TextureEntry> m_Textures;

	uint32_t m_iGeometryCount;
	struct GeometryEntry
    {
		
		uint32_t id;
	};
};

bool UltralightUISystem::Init()
{
    g_pUltralightFontLoader = CreateULPlatformFontLoader();

    Config config;
    config.face_winding      = kFaceWinding_Clockwise; // CW in D3D, CCW in OGL
    config.device_scale_hint = 1.0;               // Set DPI to monitor DPI scale
    config.enable_javascript = true;
    config.enable_images     = true;

	Platform &platform = Platform::instance();
    platform.set_config(config);
    platform.set_file_system(&g_UltralightFileSystem);
    platform.set_font_loader(DefaultFontLoader());
    m_pGPUDriver = DefaultGPUDriver(); 
    platform.set_gpu_driver(m_pGPUDriver);

	m_pRenderer = Renderer::Create();

	g_pTestOverlay = new UltralightOverlay(*m_pRenderer, m_pGPUDriver, 300, 300, 0, 0);
    g_pTestOverlay->view()->LoadHTML("<h1 color='red'>HTML UI!!!!</h1>");

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

void UltralightUISystem::PreRender()
{
	
}

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