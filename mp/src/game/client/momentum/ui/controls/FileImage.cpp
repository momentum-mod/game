#include "cbase.h"

#include "FileImage.h"

#include "filesystem.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"

#include "mom_api_requests.h"
#include "util/mom_util.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_HDR
#include "stb/image/stb_image.h"

#include "tier0/memdbgon.h"

using namespace vgui;

struct ImageCacheEntry
{
    CUtlBuffer m_bufOriginalImage;
};

class CFileImageCache
{
public:
    CFileImageCache() {}

    void AddImageToCache(const char *pPath, const CUtlBuffer &pBuf)
    {
        const auto pEntry = new ImageCacheEntry;
        pEntry->m_bufOriginalImage.CopyBuffer(pBuf);

        const auto index = m_vecImages.AddToTail(pEntry);
        m_dictImages.Insert(pPath, index);
    }

    void RemoveImageFromCache(const char *pImagePath)
    {
        const auto index = m_dictImages.Find(pImagePath);
        if (!m_dictImages.IsValidIndex(index))
            return;

        m_vecImages.PurgeAndDeleteElement(m_dictImages[index]);
        m_dictImages.RemoveAt(index);
    }

    ImageCacheEntry *FindImageByPath(const char *pPath)
    {
        const auto index = m_dictImages.Find(pPath);
        if (m_dictImages.IsValidIndex(index))
        {
            return m_vecImages[m_dictImages[index]];
        }

        return nullptr;
    }

    CUtlVector<ImageCacheEntry *> m_vecImages;
    CUtlDict<int, uint16> m_dictImages;
};

CFileImageCache g_FileImageCache;

FileImage::FileImage(IImage *pDefaultImage /* = nullptr*/): m_iX(0), m_iY(0), m_iImageWide(0), 
    m_iDesiredWide(0), m_iImageTall(0), m_iDesiredTall(0), m_iRotation(0), m_iTextureID(-1), m_pDefaultImage(pDefaultImage)
{
    m_DrawColor = Color(255, 255, 255, 255);
    m_szFileName[0] = '\0';
    m_szPathID[0] = '\0';
}

FileImage::FileImage(const char *pFileName, const char *pPathID, IImage *pDefaultImage) : FileImage(pDefaultImage)
{
    LoadFromFile(pFileName, pPathID);
}

FileImage::~FileImage()
{
    DestroyTexture();
}

bool FileImage::LoadFromFile(const char* pFileName, const char* pPathID /* = "GAME"*/)
{
    Q_strncpy(m_szFileName, pFileName, sizeof(m_szFileName));
    Q_strncpy(m_szPathID, pPathID, sizeof(m_szPathID));

    return LoadFromFileInternal();
}

bool FileImage::LoadFromFileInternal()
{
    CUtlBuffer fileBuf;
    if (!g_pFullFileSystem->ReadFile(m_szFileName, m_szPathID, fileBuf))
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
    if (m_iTextureID != -1)
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
    else if (m_pDefaultImage)
    {
        int defWide, defTall;
        m_pDefaultImage->GetSize(defWide, defTall);
        const auto wide = m_iDesiredWide ? m_iDesiredWide : defWide;
        const auto tall = m_iDesiredTall ? m_iDesiredTall : defTall;
        
        m_pDefaultImage->SetSize(wide, tall);
        m_pDefaultImage->SetPos(m_iX, m_iY);
        m_pDefaultImage->SetColor(m_DrawColor);
        m_pDefaultImage->Paint();
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

bool FileImage::Evict()
{
    if (m_szFileName[0])
        return LoadFromFileInternal();
    return false;
}

void FileImage::DestroyTexture()
{
    if (surface() && m_iTextureID > -1)
    {
        surface()->DestroyTextureID(m_iTextureID);
        m_iTextureID = -1;
    }
}

URLImage::URLImage(IImage *pDefaultImage/* = nullptr*/, bool bDrawProgress /* = false*/) : FileImage(pDefaultImage), 
        m_hRequest(INVALID_HTTPREQUEST_HANDLE), m_bDrawProgressBar(bDrawProgress)
{
    m_szURL[0] = '\0';
    m_fProgress = 0.0f;
    m_uTotalSize = 0;
}

URLImage::URLImage(const char* pURL, IImage* pDefault, bool bDrawProgress) : URLImage(pDefault, bDrawProgress)
{
    LoadFromURL(pURL);
}

URLImage::~URLImage()
{
    if (m_hRequest != INVALID_HTTPREQUEST_HANDLE)
        g_pAPIRequests->CancelDownload(m_hRequest);
}

bool URLImage::LoadFromURL(const char* pURL)
{
    Q_strncpy(m_szURL, pURL, sizeof(m_szURL));

    return LoadFromURLInternal();
}

bool URLImage::LoadFromURLInternal()
{
    m_fProgress = 0.0f;
    m_uTotalSize = 0;
    m_hRequest = g_pAPIRequests->DownloadFile(m_szURL,
                                              UtlMakeDelegate(this, &URLImage::OnFileStreamSize),
                                              UtlMakeDelegate(this, &URLImage::OnFileStreamProgress),
                                              UtlMakeDelegate(this, &URLImage::OnFileStreamEnd),
                                              nullptr);

    return m_hRequest != INVALID_HTTPREQUEST_HANDLE;
}


void URLImage::Paint()
{
    if (m_hRequest != INVALID_HTTPREQUEST_HANDLE && m_bDrawProgressBar)
    {
        surface()->DrawSetColor(Color(0, 0, 0, 255));
        surface()->DrawOutlinedRect(m_iX, m_iY, m_iDesiredWide, m_iDesiredTall);
        // Interp colors
        const Color interp = MomUtil::ColorLerp(m_fProgress, COLOR_RED, COLOR_GREEN);
        surface()->DrawSetColor(interp);
        // Determine width
        const int wide = max(int(float(m_iDesiredWide) * m_fProgress), 0);
        surface()->DrawFilledRect(m_iX + 2, m_iY + 2, wide - 2, m_iDesiredTall - 2);
    }
    else
        FileImage::Paint();
}

bool URLImage::Evict()
{
    if (m_szURL[0])
        return LoadFromURLInternal();
    return false;
}

void URLImage::OnFileStreamSize(KeyValues* pKv)
{
    m_uTotalSize = pKv->GetUint64("size");
}

void URLImage::OnFileStreamProgress(KeyValues* pKv)
{
    if (m_uTotalSize)
        m_fProgress = float(pKv->GetUint64("offset") + pKv->GetUint64("size")) / float(m_uTotalSize);
}

void URLImage::OnFileStreamEnd(KeyValues* pKv)
{
    m_hRequest = INVALID_HTTPREQUEST_HANDLE;

    if (pKv->GetBool("error"))
    {
        DevWarning("Could not load URLImage due to error!\n");
    }
    else
    {
        CUtlBuffer *pBuf = static_cast<CUtlBuffer*>(pKv->GetPtr("buf"));
        if (pBuf)
        {
            LoadFromUtlBuffer(*pBuf);
        }
    }
}
