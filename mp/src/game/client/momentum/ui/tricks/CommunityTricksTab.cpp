#include "cbase.h"

#include "CommunityTricksTab.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/TextEntry.h"

#include "tier0/memdbgon.h"

using namespace vgui;

CommunityTricksTab::CommunityTricksTab(Panel *pParent): BaseClass(pParent, "CommunityTricksTab")
{
    SetProportional(true);

    m_pTrickNameFilterEntry = nullptr;
    m_pFiltersButton = nullptr;
    m_pCommunityTricksList = nullptr;
}

void CommunityTricksTab::OnResetData()
{
    m_pTrickNameFilterEntry = new TextEntry(this, "TrickNameFilterEntry");
    m_pFiltersButton = new Button(this, "FiltersButton", "#MOM_Tricks_Filters", this, "ToggleFilters");
    m_pCommunityTricksList = new ListPanel(this, "CommunityTricksList");
    m_pCommunityTricksList->SetRowHeightOnFontChange(false);
    m_pCommunityTricksList->SetRowHeight(GetScaledVal(20));
    m_pCommunityTricksList->SetMultiselectEnabled(false);
    m_pCommunityTricksList->SetAutoTallHeaderToFont(true);

    LoadControlSettings("resource/ui/tricks/CommunityTricksTab.res");

    // set up the official tricks list categories
    m_pCommunityTricksList->AddColumnHeader(0, "num_steps", "#MOM_Trick_Steps", GetScaledVal(60));
    m_pCommunityTricksList->AddColumnHeader(1, "tier", "#MOM_MapSelector_Difficulty", GetScaledVal(60));
    m_pCommunityTricksList->AddColumnHeader(2, "name", "#MOM_Trick_Name", GetScaledVal(120));
}

void CommunityTricksTab::OnCommand(const char *command)
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
