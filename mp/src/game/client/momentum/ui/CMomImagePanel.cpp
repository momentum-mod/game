#include "cbase.h"

#include "CMomImagePanel.h"
#include "vgui/ISurface.h"
#include "filesystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "image/stb_image.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_BUILD_FACTORY(CMomImagePanel);

CMomImagePanel::CMomImagePanel(Panel* pParent, const char* pName) : BaseClass(pParent, pName), m_iTextureID(-1) {}

CMomImagePanel::~CMomImagePanel()
{
    DestroyTexture();
}

void CMomImagePanel::LoadImageFrom(const char* pPath)
{
    // Clear up any previous texture
    DestroyTexture();
    // Create our new one
    m_iTextureID = surface()->CreateNewTextureID(true);
    // Convert our path to the full drive path for loading
    char fullPath[MAX_PATH];
    g_pFullFileSystem->RelativePathToFullPath_safe(pPath, "MOD", fullPath);
    // Read our image data
    int w, h, channels;
    unsigned char *pImageData = stbi_load(fullPath, &w, &h, &channels, STBI_rgb_alpha);
    if (!pImageData)
    {
        DevMsg("CMomImagePanel: Image at path %s is invalid!\n", fullPath);
        return;
    }

    DevMsg("CMomImagePanel: Image loaded from path: %s with %i channels, size (%i x %i)!\n", fullPath, channels, w, h);

    // Image data gets memcpy'd over in this function...
    surface()->DrawSetTextureRGBA(m_iTextureID, pImageData, w, h, 1, true);
    // ... So we can clean up here
    stbi_image_free(pImageData);
}

void CMomImagePanel::Paint()
{
    if (m_iTextureID > -1)
    {
        surface()->DrawSetColor(255, 255, 255, GetAlpha());
        surface()->DrawSetTexture(m_iTextureID);
        int xpos = GetXPos(), ypos = GetYPos(), wide = GetWide(), tall = GetTall();
        // Note:
        // We currently render based on the size of the panel. Change the size of the panel to change the image render size.
        // MOM_TODO: Make an offset variable?
        surface()->DrawTexturedRect(xpos, ypos, xpos + wide, ypos + tall); 
    }
}

void CMomImagePanel::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
}

void CMomImagePanel::DestroyTexture()
{
    if (surface() && m_iTextureID != -1)
    {
        surface()->DestroyTextureID(m_iTextureID);
        m_iTextureID = -1;
    }
}
