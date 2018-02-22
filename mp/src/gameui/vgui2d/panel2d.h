#pragma once

#include "vgui_controls/EditablePanel.h"

class Panel2D : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(Panel2D, vgui::EditablePanel);

public:
	Panel2D();
	Panel2D(vgui::Panel* parent);
	Panel2D(vgui::Panel* parent, const char* panelName);
	Panel2D(vgui::Panel* parent, const char* panelName, vgui::HScheme scheme);

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
