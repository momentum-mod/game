#include "button2d.h"
#include "gameui2_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Button2D::Button2D(vgui::Panel *parent, const char *panelName, const char *text, vgui::Panel *pActionSignalTarget,
                   const char *pCmd)
    : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
{
}

Button2D::Button2D(vgui::Panel *parent, const char *panelName, const wchar_t *text, vgui::Panel *pActionSignalTarget,
                   const char *pCmd)
    : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
{
}

void Button2D::Paint()
{
    BaseClass::Paint();

    if (GameUI2().GetMaterialSystem() && GameUI2().GetRenderView() && GameUI2().GetMaskTexture() && GameUI2().GetFrustum())
    {
        m_bBlurEnabled = true;

        GameUI2().GetRenderView()->Push2DView(GameUI2().GetView(), NULL, GameUI2().GetMaskTexture(), GameUI2().GetFrustum());

        PaintBlurMask();

        GameUI2().GetRenderView()->PopView(GameUI2().GetFrustum());
    }

    m_bBlurEnabled = false;
}

void Button2D::PaintBlurMask() {}
