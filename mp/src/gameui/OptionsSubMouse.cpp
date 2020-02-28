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
#include <vgui_controls/TextEntry.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

COptionsSubMouse::COptionsSubMouse(vgui::Panel *parent) : PropertyPage(parent, NULL)
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
        0.0001f, 20.0f, "sensitivity", true);

    m_pMouseSensitivityLabel = new TextEntry(this, "SensitivityLabel");
    m_pMouseSensitivityLabel->AddActionSignalTarget(this);

    m_pMouseAccelLabel = new TextEntry(this, "MouseAccelerationLabel");
    m_pMouseAccelLabel->AddActionSignalTarget(this);
    m_pMouseAccelToggle = new CheckButton(this, "MouseAccelerationCheckbox", "#GameUI_MouseAcceleration");
    m_pMouseAccelSlider = new CvarSlider(this, "MouseAccelerationSlider", "#GameUI_MouseAcceleration", 1.0f, 20.0f, "m_customaccel_exponent", true);

	LoadControlSettings("resource/optionssubmouse.res");

    //float sensitivity = engine->pfnGetCvarFloat( "sensitivity" );
	ConVarRef var( "sensitivity" );
	if ( var.IsValid() )
	{
		float sensitivity = var.GetFloat();

		char buf[64];
		Q_snprintf(buf, sizeof(buf), "%.3f", sensitivity);
		m_pMouseSensitivityLabel->SetText(buf);
	}
    ConVarRef accel("m_customaccel_exponent");
    if (accel.IsValid())
    {
        float acc = accel.GetFloat();

        char buf[64];
        Q_snprintf(buf, sizeof(buf), "%.3f", acc);
        m_pMouseAccelLabel->SetText(buf);
    }
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
	m_pMouseFilterCheckBox->Reset();
    m_pMouseRawCheckbox->Reset();
	m_pMouseSensitivitySlider->Reset();
    m_pMouseAccelSlider->Reset();

    m_pMouseAccelToggle->SetSelected(ConVarRef("m_customaccel").GetInt() == 3);

    bool bEnabled = m_pMouseAccelToggle->IsSelected();
    m_pMouseAccelSlider->SetEnabled(bEnabled);
    m_pMouseAccelLabel->SetEnabled(bEnabled);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMouse::OnApplyChanges()
{
	m_pReverseMouseCheckBox->ApplyChanges();
	m_pMouseFilterCheckBox->ApplyChanges();
    m_pMouseRawCheckbox->ApplyChanges();
	m_pMouseSensitivitySlider->ApplyChanges();
    m_pMouseAccelSlider->ApplyChanges();

    bool bMouseAccelEnabled = m_pMouseAccelToggle->IsSelected();
    ConVarRef("m_customaccel").SetValue(bMouseAccelEnabled ? 3 : 0);
}

//-----------------------------------------------------------------------------
// Purpose: sets background color & border
//-----------------------------------------------------------------------------
void COptionsSubMouse::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMouse::OnControlModified(Panel *panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));

    // the HasBeenModified() check is so that if the value is outside of the range of the
    // slider, it won't use the slider to determine the display value but leave the
    // real value that we determined in the constructor
    if (panel == m_pMouseSensitivitySlider && m_pMouseSensitivitySlider->HasBeenModified())
    {
        UpdateSensitivityLabel();
    }
    else if (panel == m_pMouseAccelSlider && m_pMouseAccelSlider->HasBeenModified())
    {
        UpdateAccelLabel();
    }
    else if (panel == m_pMouseAccelToggle)
    {
        bool bEnabled = m_pMouseAccelToggle->IsSelected();
        m_pMouseAccelSlider->SetEnabled(bEnabled);
        m_pMouseAccelLabel->SetEnabled(bEnabled);
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMouse::OnTextChanged(Panel *panel)
{
    if (panel == m_pMouseSensitivityLabel)
    {
        char buf[64];
        m_pMouseSensitivityLabel->GetText(buf, 64);

        float fValue = Q_atof(buf);
        if (fValue >= 0.0001f)
        {
            m_pMouseSensitivitySlider->SetSliderValue(fValue);
            PostActionSignal(new KeyValues("ApplyButtonEnable"));
        }
    }
    else if (panel == m_pMouseAccelLabel)
    {
        char buf[64];
        m_pMouseAccelLabel->GetText(buf, 64);

        float fVal = Q_atof(buf);
        if (fVal > 1.0f)
        {
            m_pMouseAccelSlider->SetSliderValue(fVal);
            PostActionSignal(new KeyValues("ApplyButtonEnable"));
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubMouse::UpdateSensitivityLabel()
{
    char buf[64];
    Q_snprintf(buf, sizeof( buf ), "%.3f", m_pMouseSensitivitySlider->GetSliderValue());
    m_pMouseSensitivityLabel->SetText(buf);
}

void COptionsSubMouse::UpdateAccelLabel()
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.3f", m_pMouseAccelSlider->GetSliderValue());
    m_pMouseAccelLabel->SetText(buf);
}