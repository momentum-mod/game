//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "OptionsSubMouse.h"
#include "KeyToggleCheckButton.h"
#include "CvarNegateCheckButton.h"
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/CvarSlider.h>

#include "EngineInterface.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include "tier1/convar.h"
#include <stdio.h>
#include <vgui_controls/CvarTextEntry.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

COptionsSubMouse::COptionsSubMouse(vgui::Panel *parent) : PropertyPage(parent, NULL), m_cvarCustomAccel("m_customaccel")
{
    SetSize(20, 20);

	m_pReverseMouseCheckBox = new CvarNegateCheckButton( 
		this, 
		"ReverseMouse", 
		"#GameUI_ReverseMouse", 
		"m_pitch" );
	
	m_pMouseFilterCheckBox = new CvarToggleCheckButton( 
		this, 
		"MouseFilter", 
		"#GameUI_MouseFilter", 
		"m_filter" );

    m_pMouseRawCheckbox = new CvarToggleCheckButton(
        this,
        "MouseRaw",
        "#GameUI_MouseRaw",
        "m_rawinput");

	m_pMouseSensitivitySlider = new CvarSlider( this, "Slider", "#GameUI_MouseSensitivity",
        0.0001f, 20.0f, "sensitivity", true, true);

    m_pMouseSensitivityEntry = new CvarTextEntry(this, "SensitivityLabel", "sensitivity", "%.3f");
    m_pMouseSensitivityEntry->AddActionSignalTarget(this);
    m_pMouseSensitivityEntry->SetAllowNumericInputOnly(true);

    m_pMouseAccelEntry = new CvarTextEntry(this, "MouseAccelerationLabel", "m_customaccel_exponent", "%.3f");
    m_pMouseAccelEntry->AddActionSignalTarget(this);
    m_pMouseAccelEntry->SetAllowNumericInputOnly(true);
    m_pMouseAccelToggle = new CheckButton(this, "MouseAccelerationCheckbox", "#GameUI_MouseAcceleration");
    m_pMouseAccelSlider = new CvarSlider(this, "MouseAccelerationSlider", "#GameUI_MouseAcceleration", 1.0f, 20.0f, "m_customaccel_exponent", true, true);

	LoadControlSettings("resource/optionssubmouse.res");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
COptionsSubMouse::~COptionsSubMouse()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMouse::OnResetData()
{
	m_pReverseMouseCheckBox->Reset();

    m_pMouseAccelToggle->SetSelected(m_cvarCustomAccel.GetInt() == 3);

    bool bEnabled = m_pMouseAccelToggle->IsSelected();
    m_pMouseAccelSlider->SetEnabled(bEnabled);
    m_pMouseAccelEntry->SetEnabled(bEnabled);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void COptionsSubMouse::OnCheckButtonChecked(Panel *panel)
{
    if (panel == m_pMouseAccelToggle)
    {
        bool bMouseAccelEnabled = m_pMouseAccelToggle->IsSelected();
        m_cvarCustomAccel.SetValue(bMouseAccelEnabled ? 3 : 0);
        m_pMouseAccelSlider->SetEnabled(bMouseAccelEnabled);
        m_pMouseAccelEntry->SetEnabled(bMouseAccelEnabled);
    }
}