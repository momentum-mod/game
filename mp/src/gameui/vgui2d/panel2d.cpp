#include "GameUI_Interface.h"
#include "panel2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Panel2D::Panel2D() : BaseClass(nullptr, "Panel")
{
	
}

Panel2D::Panel2D(vgui::Panel* parent) : BaseClass(parent, "Panel")
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

    if (GameUI().GetMaterialSystem() && GameUI().GetRenderView() && GameUI().GetMaskTexture() && GameUI().GetFrustum())
    {
        m_bBlurEnabled = true;

        GameUI().GetRenderView()->Push2DView(GameUI().GetView(), NULL, GameUI().GetMaskTexture(), GameUI().GetFrustum());

        PaintBlurMask();

        GameUI().GetRenderView()->PopView(GameUI().GetFrustum());
    }

	m_bBlurEnabled = false;
}

void Panel2D::PaintBlurMask()
{
	
}