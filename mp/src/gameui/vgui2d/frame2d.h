#pragma once

#include "vgui_controls/Frame.h"

class Frame2D : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(Frame2D, vgui::Frame);

public:
	Frame2D(vgui::Panel* parent, const char* panelName, bool showTaskbarIcon = true, bool bPopup = true);

	virtual void		Paint();
	virtual void 		PaintBlurMask();

	virtual bool		IsBlur()
	{
		return m_bBlurEnabled;
	}

	virtual bool		IsFullyVisible()
	{
		return vgui::ipanel()->IsFullyVisible(GetVPanel());
	}

private:
	bool				m_bBlurEnabled;
};
