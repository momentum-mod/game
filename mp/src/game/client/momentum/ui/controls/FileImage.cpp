#include "cbase.h"

#include "FileImage.h"

#include "filesystem.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"

#include "mom_api_requests.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_HDR
#include "stb/image/stb_image.h"

#include "tier0/memdbgon.h"

using namespace vgui;

FileImage::FileImage(): m_iX(0), m_iY(0), m_iImageWide(0), m_iDesiredWide(0), m_iImageTall(0), m_iDesiredTall(0),
                        m_iRotation(0), m_iTextureID(-1)
{
    m_DrawColor = Color(255, 255, 255, 255);
}

FileImage::~FileImage()
{
    DestroyTexture();
}

bool FileImage::LoadFromFile(const char* pFileName, const char* pPathID /* = "GAME"*/)
{
    // Convert our path to the full drive path for loading
    CUtlBuffer fileBuf;
    if (!g_pFullFileSystem->ReadFile(pFileName, pPathID, fileBuf))
        return false;

    return LoadFromUtlBuffer(fileBuf);
}

bool FileImage::LoadFromUtlBuffer(CUtlBuffer &buf)
{
    // Read our image data
    int w, h, channels;
    unsigned char *pImageData = stbi_load_from_memory((stbi_uc*) buf.Base(), buf.TellPut(), &w, &h, &channels, STBI_rgb_alpha);
    if (!pImageData)
        return false;

    // Image data gets memcpy'd over in this function...
    LoadFromRGBA(pImageData, w, h);

    // ... So we can clean up here
    stbi_image_free(pImageData);

    return true;
}

void FileImage::LoadFromRGBA(const uint8* pData, int wide, int tall)
{
    // Clear up any previous texture
    DestroyTexture();
    // Create our new one
    m_iTextureID = surface()->CreateNewTextureID(true);
    // Image data gets memcpy'd over in this function...
    surface()->DrawSetTextureRGBAEx(m_iTextureID, pData, wide, tall, IMAGE_FORMAT_RGBA8888);
    
    m_iImageWide = wide;
    m_iImageTall = tall;

    if (m_iDesiredWide == 0)
        m_iDesiredWide = wide;
    if (m_iDesiredTall == 0)
        m_iDesiredTall = tall;
}

void FileImage::Paint()
{
    if (m_iTextureID > -1)
    {
        surface()->DrawSetColor(m_DrawColor);
        surface()->DrawSetTexture(m_iTextureID);

        if (m_iRotation == 0)
        {
            surface()->DrawTexturedRect(m_iX, m_iY, m_iX + m_iDesiredWide, m_iY + m_iDesiredTall);
        }
        else
        {
            // Rotate about the center of the image
            Vertex_t verts[4];
            Vector2D center(m_iX + (m_iDesiredWide * 0.5f), m_iY + (m_iDesiredTall * 0.5f));

            // Choose a basis...
            float yawRadians = -m_iRotation * M_PI / 180.0f;
            Vector2D axis[2];
            axis[0].x = cos(yawRadians);
            axis[0].y = sin(yawRadians);
            axis[1].x = -axis[0].y;
            axis[1].y = axis[0].x;

            verts[0].m_TexCoord.Init(0, 0);
            Vector2DMA(center, -0.5f * m_iDesiredWide, axis[0], verts[0].m_Position);
            Vector2DMA(verts[0].m_Position, -0.5f * m_iDesiredTall, axis[1], verts[0].m_Position);

            verts[1].m_TexCoord.Init(1, 0);
            Vector2DMA(verts[0].m_Position, m_iDesiredWide, axis[0], verts[1].m_Position);

            verts[2].m_TexCoord.Init(1, 1);
            Vector2DMA(verts[1].m_Position, m_iDesiredTall, axis[1], verts[2].m_Position);

            verts[3].m_TexCoord.Init(0, 1);
            Vector2DMA(verts[0].m_Position, m_iDesiredTall, axis[1], verts[3].m_Position);

            surface()->DrawTexturedPolygon(4, verts);
        }
    }
}

void FileImage::GetSize(int& wide, int& tall)
{
    if (m_iDesiredWide == 0 && m_iDesiredTall == 0)
        GetContentSize(wide, tall);
    else
    {
        wide = m_iDesiredWide; 
        tall = m_iDesiredTall;
    }
}

void FileImage::DestroyTexture()
{
    if (surface() && m_iTextureID != -1)
    {
        surface()->DestroyTextureID(m_iTextureID);
        m_iTextureID = -1;
    }
}

URLImage::URLImage(): m_pDefaultImage(nullptr), m_hRequest(INVALID_HTTPREQUEST_HANDLE), m_bValid(false)
{
}

bool URLImage::LoadFromURL(const char* pURL, IImage *pDefaultImage /*= nullptr*/)
{
    m_pDefaultImage = pDefaultImage;

    m_hRequest = g_pAPIRequests->DownloadFile(pURL, 
                                 UtlMakeDelegate(this, &URLImage::OnFileStreamStart),
                                 UtlMakeDelegate(this, &URLImage::OnFileStreamProgress),
                                 UtlMakeDelegate(this, &URLImage::OnFileStreamEnd),
                                 nullptr);

    return m_hRequest != INVALID_HTTPREQUEST_HANDLE;
}

void URLImage::Paint()
{
    if (m_bValid)
        FileImage::Paint();
    else if (m_pDefaultImage)
    {
        m_pDefaultImage->SetSize(m_iDesiredWide, m_iDesiredTall);
        m_pDefaultImage->SetPos(m_iX, m_iY);
        m_pDefaultImage->SetColor(m_DrawColor);
        m_pDefaultImage->Paint();
    }
    // MOM_TODO: else { progressBar->Paint() }
}

void URLImage::OnFileStreamStart(KeyValues* pKv)
{
    // MOM_TODO also put a progress bar here and paint it, if no default image?
}

void URLImage::OnFileStreamProgress(KeyValues* pKv)
{
    // MOM_TODO update that progress bar here?
}

void URLImage::OnFileStreamEnd(KeyValues* pKv)
{
    if (pKv->GetBool("error"))
    {
        DevWarning("Could not load URLImage due to error!\n");
    }
    else
    {
        CUtlBuffer *pBuf = (CUtlBuffer*)pKv->GetPtr("buf");
        if (pBuf)
        {
            m_bValid = LoadFromUtlBuffer(*pBuf);
        }
    }
}
