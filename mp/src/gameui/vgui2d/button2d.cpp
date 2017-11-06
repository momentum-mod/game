#include "button2d.h"
#include "GameUI_Interface.h"

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

    if (GameUI().GetMaterialSystem() && GameUI().GetRenderView() && GameUI().GetMaskTexture() && GameUI().GetFrustum())
    {
        m_bBlurEnabled = true;

        GameUI().GetRenderView()->Push2DView(GameUI().GetView(), NULL, GameUI().GetMaskTexture(), GameUI().GetFrustum());

        PaintBlurMask();

        GameUI().GetRenderView()->PopView(GameUI().GetFrustum());
    }

    m_bBlurEnabled = false;
}

void Button2D::PaintBlurMask() {}
