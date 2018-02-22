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

class CCvarNegateCheckButton;
class CKeyToggleCheckButton;

namespace vgui
{
    class Label;
    class Panel;
    class CvarToggleCheckButton;
    class CCvarSlider;
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
	virtual void OnApplyChanges();

protected:
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	MESSAGE_FUNC_PTR( OnControlModified, "ControlModified", panel );
    MESSAGE_FUNC_PTR( OnTextChanged, "TextChanged", panel );
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel )
	{
		OnControlModified( panel );
	}

    void UpdateSensitivityLabel();
    void UpdateAccelLabel();
	void UpdateJoystickPanels();

	CCvarNegateCheckButton		*m_pReverseMouseCheckBox;
	vgui::CvarToggleCheckButton	*m_pMouseFilterCheckBox ,*m_pMouseRawCheckbox, *m_pJoystickCheckBox,
        *m_pJoystickSouthpawCheckBox, *m_pReverseJoystickCheckBox;

    vgui::CCvarSlider					*m_pMouseSensitivitySlider;
    vgui::TextEntry             *m_pMouseSensitivityLabel;

    vgui::CheckButton *m_pMouseAccelToggle;
    vgui::CCvarSlider *m_pMouseAccelSlider;
    vgui::TextEntry *m_pMouseAccelLabel;

    vgui::CCvarSlider					*m_pJoyYawSensitivitySlider;
	vgui::Label					*m_pJoyYawSensitivityPreLabel;
    vgui::CCvarSlider					*m_pJoyPitchSensitivitySlider;
	vgui::Label					*m_pJoyPitchSensitivityPreLabel;
};



#endif // OPTIONS_SUB_MOUSE_H