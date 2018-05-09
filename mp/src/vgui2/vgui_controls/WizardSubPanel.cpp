//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_controls/WizardPanel.h"
#include "vgui_controls/WizardSubPanel.h"
#include "vgui_controls/BuildGroup.h"

#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#include <stdio.h>
using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
WizardSubPanel::WizardSubPanel(Panel *parent, const char *panelName) : EditablePanel(parent, panelName), _wizardPanel(NULL)
{
    InitSettings();
	SetVisible(false);
	m_iDesiredWide = 0;
	m_iDesiredTall = 0;
	SetBuildGroup(GetBuildGroup());
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
WizardSubPanel::~WizardSubPanel()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WizardSubPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(GetSchemeColor("WizardSubPanel.BgColor",pScheme));
}

void WizardSubPanel::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"WizardWide", TYPE_INTEGER},
    {"WizardTall", TYPE_INTEGER}
    END_PANEL_SETTINGS();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WizardSubPanel::GetSettings( KeyValues *outResourceData )
{
	BaseClass::GetSettings(outResourceData);

	outResourceData->SetInt("WizardWide", m_iDesiredWide);
	outResourceData->SetInt("WizardTall", m_iDesiredTall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WizardSubPanel::ApplySettings(KeyValues *inResourceData)
{
	// don't adjust visiblity during settings application (since it's our parent who really controls it)
	bool bVisible = IsVisible();

	BaseClass::ApplySettings(inResourceData);

	m_iDesiredWide = inResourceData->GetInt("WizardWide", 0);
	m_iDesiredTall = inResourceData->GetInt("WizardTall", 0);

	if (GetWizardPanel() && m_iDesiredWide && m_iDesiredTall)
	{
		GetWizardPanel()->SetSize(m_iDesiredWide, m_iDesiredTall);
	}

	SetVisible(bVisible);
}

//-----------------------------------------------------------------------------
// Purpose: gets the size this subpanel would like the wizard to be
//-----------------------------------------------------------------------------
bool WizardSubPanel::GetDesiredSize(int &wide, int &tall)
{
	wide = m_iDesiredWide;
	tall = m_iDesiredTall;

	return (m_iDesiredWide && m_iDesiredTall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
KeyValues *WizardSubPanel::GetWizardData()
{
	return GetWizardPanel()->GetWizardData();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
WizardSubPanel *WizardSubPanel::GetSiblingSubPanelByName(const char *pageName)
{
	return GetWizardPanel()->GetSubPanelByName(pageName);
}
