#include "cbase.h"

#include "FileImage.h"

#include "filesystem.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/image/stb_image.h"

#include "tier0/memdbgon.h"

using namespace vgui;

FileImage::FileImage(): m_iX(0), m_iY(0), m_iWide(0), m_iTall(0), m_iRotation(0), m_iTextureID(-1)
{
}

FileImage::~FileImage()
{
    DestroyTexture();
}

bool FileImage::LoadFromFile(const char* pFileName, const char* pPathID /* = "GAME"*/)
{
    // Convert our path to the full drive path for loading
    char fullPath[MAX_PATH];
    g_pFullFileSystem->RelativePathToFullPath_safe(pFileName, pPathID, fullPath);
    // Read our image data
    int w, h, channels;
    unsigned char *pImageData = stbi_load(fullPath, &w, &h, &channels, STBI_rgb_alpha);
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
    surface()->DrawSetTextureRGBA(m_iTextureID, pData, wide, tall, 1, true);
}

void FileImage::Paint()
{
    if (m_iTextureID > -1)
    {
        surface()->DrawSetColor(m_DrawColor);
        surface()->DrawSetTexture(m_iTextureID);

        if (m_iRotation == 0)
        {
            surface()->DrawTexturedRect(m_iX, m_iY, m_iX + m_iWide, m_iY + m_iTall);
        }
        else
        {
            // Rotate about the center of the image
            Vertex_t verts[4];
            Vector2D center(m_iX + (m_iWide * 0.5f), m_iY + (m_iTall * 0.5f));

            // Choose a basis...
            float yawRadians = -m_iRotation * M_PI / 180.0f;
            Vector2D axis[2];
            axis[0].x = cos(yawRadians);
            axis[0].y = sin(yawRadians);
            axis[1].x = -axis[0].y;
            axis[1].y = axis[0].x;

            verts[0].m_TexCoord.Init(0, 0);
            Vector2DMA(center, -0.5f * m_iWide, axis[0], verts[0].m_Position);
            Vector2DMA(verts[0].m_Position, -0.5f * m_iTall, axis[1], verts[0].m_Position);

            verts[1].m_TexCoord.Init(1, 0);
            Vector2DMA(verts[0].m_Position, m_iWide, axis[0], verts[1].m_Position);

            verts[2].m_TexCoord.Init(1, 1);
            Vector2DMA(verts[1].m_Position, m_iTall, axis[1], verts[2].m_Position);

            verts[3].m_TexCoord.Init(0, 1);
            Vector2DMA(verts[0].m_Position, m_iTall, axis[1], verts[3].m_Position);

            surface()->DrawTexturedPolygon(4, verts);
        }
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
