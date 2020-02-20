//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "tier1/KeyValues.h"

#include <vgui_controls/CvarComboBox.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

Panel *Create_CvarComboBox()
{
    return new CvarComboBox( nullptr, nullptr );
}

//not sure which build factory to use here (and if the constructor above is necessary)
DECLARE_BUILD_FACTORY_CUSTOM( CvarComboBox, Create_CvarComboBox );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CvarComboBox::CvarComboBox(Panel *parent, const char *panelName, int numLines, bool allowEdit, char const *cvarname, bool ignoreMissingCvar)
    : ComboBox(parent, panelName, numLines, allowEdit), m_cvar((cvarname) ? cvarname : "", (cvarname) ? ignoreMissingCvar : true)
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
CvarComboBox::~CvarComboBox()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarComboBox::Paint()
{
    if (!m_cvar.IsValid())
    {
        BaseClass::Paint();
        return;
    }

    int value = m_cvar.GetInt();

    if (value != m_iStartValue)
    {
        ActivateItemByRow(value);
        m_iStartValue = value;
    }
    BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the OK / Apply button is pressed.  Changed data should be written into cvar.
//-----------------------------------------------------------------------------
void CvarComboBox::OnApplyChanges()
{
    ApplyChanges();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarComboBox::ApplyChanges()
{
    if (!m_cvar.IsValid())
        return;

    m_iStartValue = GetActiveItem();
    m_cvar.SetValue(m_iStartValue);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarComboBox::Reset()
{
    if (!m_cvar.IsValid())
        return;

    m_iStartValue = m_cvar.GetInt();
    ActivateItemByRow(m_iStartValue);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CvarComboBox::HasBeenModified()
{
    return GetActiveItem() != m_iStartValue;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarComboBox::OnTextChanged()
{
    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CvarComboBox::ApplySettings(KeyValues *inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    const char *cvarName = inResourceData->GetString("cvar_name", nullptr);
    const char *cvarValue = inResourceData->GetString("cvar_value", "");

    if (!cvarName || !Q_stricmp(cvarName, "") || m_cvar.IsValid())
        return; // Doesn't have cvar set up in res file, or we were constructed with one

    //make sure m_iStartValue is set properly here!
    m_iStartValue = (int)cvarValue - (int)'0';

    m_cvar.Init(cvarName, m_bIgnoreMissingCvar);
    if (m_cvar.IsValid())
    {
        ActivateItemByRow(m_cvar.GetInt());
    }
}

void CvarComboBox::GetSettings(KeyValues *pOutResource)
{
    BaseClass::GetSettings(pOutResource);

    pOutResource->SetString("cvar_name", m_cvar.GetName());
    pOutResource->SetString("cvar_value", m_cvar.GetString());
}

void CvarComboBox::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"cvar_name", TYPE_STRING},
    {"cvar_value", TYPE_STRING}
    END_PANEL_SETTINGS();
}