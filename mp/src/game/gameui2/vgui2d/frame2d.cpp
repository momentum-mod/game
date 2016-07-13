#include "gameui2_interface.h"
#include "frame2d.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Frame2D::Frame2D(vgui::Panel* parent, const char* panelName, bool showTaskbarIcon, bool bPopup) : BaseClass(parent, panelName, showTaskbarIcon, bPopup)
{

}

void Frame2D::Paint()
{
	BaseClass::Paint();

	if (materials && render && GameUI2().GetMaskTexture() && GameUI2().GetFrustum())
	{
		m_bBlurEnabled = true;

		render->Push2DView(GameUI2().GetView(), NULL, GameUI2().GetMaskTexture(), GameUI2().GetFrustum());

		PaintBlurMask();

		render->PopView(GameUI2().GetFrustum());
	}

	m_bBlurEnabled = false;
}

void Frame2D::PaintBlurMask()
{
	
}