#include "cbase.h"

#include "OfficialTricksTab.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/TextEntry.h"

#include "tier0/memdbgon.h"

using namespace vgui;

OfficialTricksTab::OfficialTricksTab(Panel *pParent): BaseClass(pParent, "OfficialTricksTab")
{
    SetProportional(true);

    m_pTrickNameFilterEntry = nullptr;
    m_pFiltersButton = nullptr;
    m_pOfficialTricksList = nullptr;
}

void OfficialTricksTab::OnResetData()
{
    m_pTrickNameFilterEntry = new TextEntry(this, "TrickNameFilterEntry");
    m_pFiltersButton = new Button(this, "FiltersButton", "#MOM_Tricks_Filters", this, "ToggleFilters");
    m_pOfficialTricksList = new ListPanel(this, "OfficialTricksList");
    m_pOfficialTricksList->SetRowHeightOnFontChange(false);
    m_pOfficialTricksList->SetRowHeight(GetScaledVal(20));
    m_pOfficialTricksList->SetMultiselectEnabled(false);
    m_pOfficialTricksList->SetAutoTallHeaderToFont(true);

    LoadControlSettings("resource/ui/tricks/OfficialTricksTab.res");

    // set up the official tricks list categories
    m_pOfficialTricksList->AddColumnHeader(0, "num_steps", "#MOM_Trick_Steps", GetScaledVal(60));
    m_pOfficialTricksList->AddColumnHeader(1, "tier", "#MOM_MapSelector_Difficulty", GetScaledVal(60));
    m_pOfficialTricksList->AddColumnHeader(2, "name", "#MOM_Trick_Name", GetScaledVal(120));
}

void OfficialTricksTab::OnCommand(const char *command)
{
    if (FStrEq(command, "ToggleFilters"))
    {
        Msg("MOM_TODO: Add trick list filters panel!\n");
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}