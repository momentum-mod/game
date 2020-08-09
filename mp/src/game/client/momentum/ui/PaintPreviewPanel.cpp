#include "cbase.h"

#include "PaintPreviewPanel.h"
#include "clientmode.h"

#include "mom_shareddefs.h"
#include "util/mom_util.h"

#include <vgui/ISurface.h>

#include "tier0/memdbgon.h"

static void PaintScaleCallback(IConVar *var, const char *pOldValue, float flOldValue);

static ConVar mom_paint_color("mom_paint_color", "0000FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                              "Color of the paint decal");

static MAKE_TOGGLE_CONVAR(mom_paint_apply_sound, "1", FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
                          "Toggles the applying paint noise. 0 = OFF, 1 = ON\n");

static MAKE_CONVAR_C(mom_paint_scale, "1.0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                     "Scale the size of the paint decal\n", 0.001f, 1.0f, PaintScaleCallback);

static void PaintScaleCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    MomUtil::UpdatePaintDecalScale(mom_paint_scale.GetFloat());
}

using namespace vgui;

PaintPreviewPanel::PaintPreviewPanel(Panel *parent) : BaseClass(parent, "PaintPreviewPanel")
{
    SetScheme("ClientScheme");

    m_iDecalTextureID = surface()->CreateNewTextureID();
    surface()->DrawSetTextureFile(m_iDecalTextureID, "decals/paint_decal", true, false);
}

PaintPreviewPanel::~PaintPreviewPanel()
{
    surface()->DestroyTextureID(m_iDecalTextureID);
}

void PaintPreviewPanel::Paint()
{
    BaseClass::Paint();

    Color decalColor;
    if (!MomUtil::GetColorFromHex(mom_paint_color.GetString(), decalColor))
        return;

    surface()->DrawSetTexture(m_iDecalTextureID);
    surface()->DrawSetColor(decalColor);

    float scale = mom_paint_scale.GetFloat();
    int wide, tall;
    GetSize(wide, tall);

    int iDesiredWidth = static_cast<int>(scale / 1.0f * wide),
        iDesiredHeight = static_cast<int>(scale / 1.0f * tall);
    int x = wide / 2 - (iDesiredWidth / 2), y = tall / 2 - (iDesiredHeight / 2);
    surface()->DrawTexturedRect(x, y, x + iDesiredWidth, y + iDesiredHeight);
}
