#include "GameUI_Interface.h"
#include "frame2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Frame2D::Frame2D(vgui::Panel* parent, const char* panelName, bool showTaskbarIcon, bool bPopup) : BaseClass(parent, panelName, showTaskbarIcon, bPopup)
{

}

void Frame2D::Paint()
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

void Frame2D::PaintBlurMask()
{
	
}