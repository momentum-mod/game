#include "gameui2_interface.h"
#include "panel2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Panel2D::Panel2D() : BaseClass()
{
	
}

Panel2D::Panel2D(vgui::Panel* parent) : BaseClass(parent)
{
	
}

Panel2D::Panel2D(vgui::Panel* parent, const char* panelName) : BaseClass(parent, panelName)
{
	
}

Panel2D::Panel2D(vgui::Panel* parent, const char* panelName, vgui::HScheme scheme) : BaseClass(parent, panelName, scheme)
{
	
}

void Panel2D::Paint()
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

void Panel2D::PaintBlurMask()
{
	
}