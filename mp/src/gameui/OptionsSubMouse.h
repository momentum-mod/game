//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OPTIONS_SUB_MOUSE_H
#define OPTIONS_SUB_MOUSE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CvarNegateCheckButton;
class CKeyToggleCheckButton;

namespace vgui
{
    class Label;
    class Panel;
    class CvarToggleCheckButton;
    class CvarSlider;
}

//-----------------------------------------------------------------------------
// Purpose: Mouse Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubMouse : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COptionsSubMouse, vgui::PropertyPage );

public:
	COptionsSubMouse(vgui::Panel *parent);
	~COptionsSubMouse();

	virtual void OnResetData();

private:
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );

	CvarNegateCheckButton		*m_pReverseMouseCheckBox;
	vgui::CvarToggleCheckButton	*m_pMouseFilterCheckBox , *m_pMouseRawCheckbox;

    vgui::CvarSlider					*m_pMouseSensitivitySlider;
    vgui::CvarTextEntry             *m_pMouseSensitivityLabel;

    vgui::CheckButton *m_pMouseAccelToggle;
    vgui::CvarSlider *m_pMouseAccelSlider;
    vgui::CvarTextEntry *m_pMouseAccelLabel;

    ConVarRef m_cvarCustomAccel;
};



#endif // OPTIONS_SUB_MOUSE_H