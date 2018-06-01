//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "tier1/KeyValues.h"

#include <vgui_controls/CvarToggleCheckButton.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

Panel *Create_CvarToggleCheckButton()
{
	return new CvarToggleCheckButton( nullptr, nullptr );
}

DECLARE_BUILD_FACTORY_CUSTOM( CvarToggleCheckButton, Create_CvarToggleCheckButton );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CvarToggleCheckButton::CvarToggleCheckButton(Panel *parent, const char *panelName, const char *text, char const *cvarname, bool ignoreMissingCvar)
    : CheckButton(parent, panelName, text), m_cvar((cvarname) ? cvarname : "", (cvarname) ? ignoreMissingCvar : true)
{
    InitSettings();
    m_bIgnoreMissingCvar = ignoreMissingCvar;

    if (m_cvar.IsValid())
    {
        Reset();
    }
    AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CvarToggleCheckButton::~CvarToggleCheckButton()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::Paint()
{
    if (!m_cvar.IsValid())
    {
        BaseClass::Paint();
        return;
    }

    bool value = m_cvar.GetBool();

    if (value != m_bStartValue)
    {
        SetSelected(value);
        m_bStartValue = value;
    }
    BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the OK / Apply button is pressed.  Changed data should be written into cvar.
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::OnApplyChanges()
{
    ApplyChanges();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::ApplyChanges()
{
    if (!m_cvar.IsValid())
        return;

    m_bStartValue = IsSelected();
    m_cvar.SetValue(m_bStartValue);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::Reset()
{
    if (!m_cvar.IsValid())
        return;

    m_bStartValue = m_cvar.GetBool();
    SetSelected(m_bStartValue);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CvarToggleCheckButton::HasBeenModified()
{
    return IsSelected() != m_bStartValue;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *panel - 
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::SetSelected(bool state)
{
    BaseClass::SetSelected(state);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::OnButtonChecked()
{
    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarToggleCheckButton::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    const char *cvarName = inResourceData->GetString("cvar_name", "");
    const char *cvarValue = inResourceData->GetString("cvar_value", "");

    if (Q_stricmp(cvarName, "") == 0)
        return;// Doesn't have cvar set up in res file, must have been constructed with it.

    if (Q_stricmp(cvarValue, "1") == 0)
        m_bStartValue = true;
    else
        m_bStartValue = false;

    m_cvar.Init(cvarName, m_bIgnoreMissingCvar);
    if (m_cvar.IsValid())
    {
        SetSelected(m_cvar.GetBool());
    }
}

void CvarToggleCheckButton::GetSettings(KeyValues *pOutResource)
{
    BaseClass::GetSettings(pOutResource);

    pOutResource->SetString("cvar_name", m_cvar.GetName());
    pOutResource->SetString("cvar_value", m_cvar.GetString());
}

void CvarToggleCheckButton::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"cvar_name", TYPE_STRING},
    {"cvar_value", TYPE_STRING}
    END_PANEL_SETTINGS();
}