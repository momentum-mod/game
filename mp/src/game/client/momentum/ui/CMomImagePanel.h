#pragma once

#include "vgui_controls/Panel.h"

/**
 * Custom image panel (like BitmapImagePanel) that supports
 * direct rendering PNG/JPG/GIF/BMP/TGA files, without the need
 * for VTF + VMT. This has the drawback of not using shaders, so if any
 * images would need that, they can be created and used the traditional
 * way.
 */
namespace vgui
{
    class CMomImagePanel : public Panel
    {
        DECLARE_CLASS_SIMPLE(CMomImagePanel, Panel);

        CMomImagePanel(Panel* pParent, const char *pName);
        ~CMomImagePanel();

        void LoadImageFrom(const char *pPath);

    protected:
        void Paint() OVERRIDE;
        void PaintBackground() OVERRIDE {};
        void ApplySchemeSettings(IScheme *pScheme) OVERRIDE;

    private:
        void DestroyTexture();
        int m_iTextureID;
    };
}
