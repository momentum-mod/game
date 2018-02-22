#include "cbase.h"
#include "PaintGunScreenPanel.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"
#include "vgui/ISurface.h"

DECLARE_VGUI_SCREEN_FACTORY(PaintGunScreenPanel, "paintgun_screen");

using namespace vgui;

PaintGunScreenPanel::PaintGunScreenPanel(Panel *pParent, const char *pName)
    : BaseClass(pParent, pName), m_cvarPaintColor("mom_paintgun_color"),
      m_cvarDecalScale("mom_paintgun_scale")
{
    SetScheme("ClientScheme");
    SetPaintBackgroundEnabled(false);

    m_iDecalTextureID = surface()->CreateNewTextureID();
    surface()->DrawSetTextureFile(m_iDecalTextureID, "decals/paint_decal", true, false);
}

PaintGunScreenPanel::~PaintGunScreenPanel()
{
    surface()->DestroyTextureID(m_iDecalTextureID);
}

void PaintGunScreenPanel::Paint()
{
    BaseClass::Paint();

    Color decalColor;
    if (g_pMomentumUtil->GetColorFromHex(m_cvarPaintColor.GetString(), decalColor))
    {
        surface()->DrawSetTexture(m_iDecalTextureID);
        surface()->DrawSetColor(decalColor);

        float scale = m_cvarDecalScale.GetFloat();
        int wide, tall;
        GetSize(wide, tall);
        int iDesiredWidth = static_cast<int>(scale / 1.0f * wide), iDesiredHeight = static_cast<int>(scale / 1.0f * tall);
        int x = wide / 2 - (iDesiredWidth / 2), y = tall / 2 - (iDesiredHeight / 2);
        surface()->DrawTexturedRect(x, y, x + iDesiredWidth, y + iDesiredHeight);
    }
}
