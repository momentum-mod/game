#include "cbase.h"

#include "MapFilterPanel.h"
#include "MapSelectorDialog.h"
#include "IMapList.h"
#include "mom_map_cache.h"
#include "fmtstr.h"

#include "vgui_controls/CheckButton.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ComboBox.h"

#include "tier0/memdbgon.h"

using namespace vgui;

MapFilterPanel::MapFilterPanel(Panel *pParent) : EditablePanel(pParent, "MapFilterPanel")
{
    ResetFilters();

    // filter controls
    m_pGameModeFilter = new ComboBox(this, "GameModeFilter", 7, false);
    m_pGameModeFilter->AddItem("#MOM_All", nullptr);//All
    m_pGameModeFilter->AddItem("#MOM_GameType_Surf", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_Bhop", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_KZ", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_RJ", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_Tricksurf", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_Trikz", nullptr);
    m_pGameModeFilter->AddActionSignalTarget(this);

    m_pMapNameFilter = new TextEntry(this, "MapFilter");//As-is, people can search by map name
    m_pMapNameFilter->AddActionSignalTarget(this);

    //Difficulty filter
    m_pDifficultyLowerBound = new TextEntry(this, "DifficultyLow");
    m_pDifficultyLowerBound->SetAllowNumericInputOnly(true);
    m_pDifficultyLowerBound->AddActionSignalTarget(this);
    m_pDifficultyHigherBound = new TextEntry(this, "DifficultyHigh");
    m_pDifficultyHigherBound->SetAllowNumericInputOnly(true);
    m_pDifficultyHigherBound->AddActionSignalTarget(this);

    //Hide completed maps
    m_pHideCompletedFilterCheck = new CheckButton(this, "HideCompletedFilterCheck", "#MOM_MapSelector_FilterCompletedMaps");
    m_pHideCompletedFilterCheck->AddActionSignalTarget(this);

    //Filter staged/linear
    m_pMapLayoutFilter = new ComboBox(this, "MapLayoutFilter", 3, false);
    m_pMapLayoutFilter->AddItem("#MOM_All", nullptr);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_StagedOnly", nullptr);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_LinearOnly", nullptr);
    m_pMapLayoutFilter->AddActionSignalTarget(this);

    m_pResetFiltersButton = new Button(this, "ResetFilters", "#MOM_MapSelector_FilterReset", this, "ResetFilters");
    m_pApplyFiltersButton = new Button(this, "ApplyFilters", "#MOM_MapSelector_FilterApply", this, "ApplyFilters");

    LoadControlSettings("resource/ui/MapSelector/MapFilters.res");
}

void MapFilterPanel::ReadFiltersFromControls()
{
    //Gamemode
    m_iGameModeFilter = m_pGameModeFilter->GetActiveItem();

    // Map name
    m_pMapNameFilter->GetText(m_szMapNameFilter, sizeof(m_szMapNameFilter) - 1);
    Q_strlower(m_szMapNameFilter);

    // Difficulty
    char buf[12];
    m_pDifficultyLowerBound->GetText(buf, sizeof(buf));
    m_iDifficultyLowBound = m_pDifficultyLowerBound->GetTextLength() ? Q_atoi(buf) : 0;
    m_pDifficultyHigherBound->GetText(buf, sizeof(buf));
    m_iDifficultyHighBound = m_pDifficultyHigherBound->GetTextLength() ? Q_atoi(buf) : 0;

    // Hide completed maps
    m_bFilterHideCompleted = m_pHideCompletedFilterCheck->IsSelected();
    // Showing specfic map layouts?
    m_iMapLayoutFilter = m_pMapLayoutFilter->GetActiveItem();
}

void MapFilterPanel::ApplyFiltersToControls()
{
    m_pGameModeFilter->ActivateItemByRow(m_iGameModeFilter);

    m_pMapNameFilter->SetText(m_szMapNameFilter);

    m_pMapLayoutFilter->ActivateItemByRow(m_iMapLayoutFilter);

    m_pHideCompletedFilterCheck->SetSelected(m_bFilterHideCompleted);

    m_pDifficultyLowerBound->SetText(CFmtStr("%i", m_iDifficultyLowBound));
    m_pDifficultyHigherBound->SetText(CFmtStr("%i", m_iDifficultyHighBound));
}

void MapFilterPanel::LoadFilterSettings(KeyValues *pTabFilters)
{
    //Game-mode selection
    m_iGameModeFilter = pTabFilters->GetInt("type");

    //"Map"
    Q_strncpy(m_szMapNameFilter, pTabFilters->GetString("name"), sizeof(m_szMapNameFilter));

    //Map layout
    m_iMapLayoutFilter = pTabFilters->GetInt("layout");

    //HideCompleted maps
    m_bFilterHideCompleted = pTabFilters->GetBool("HideCompleted");

    //Difficulty
    m_iDifficultyLowBound = pTabFilters->GetInt("difficulty_low");
    m_iDifficultyHighBound = pTabFilters->GetInt("difficulty_high");

    // apply to the controls and update
    ApplyFiltersToControls();
    UpdateFilterSettings();
}

void MapFilterPanel::UpdateFilterSettings(bool bApply /* = true*/)
{
    // copy filter settings into filter file
    KeyValues *filter = MapSelectorDialog().GetCurrentTabFilterData();

    filter->SetInt("type", m_iGameModeFilter);
    filter->SetString("name", m_szMapNameFilter);
    filter->SetInt("difficulty_low", m_iDifficultyLowBound);
    filter->SetInt("difficulty_high", m_iDifficultyHighBound);
    filter->SetBool("HideCompleted", m_bFilterHideCompleted);
    filter->SetInt("layout", m_iMapLayoutFilter);

    MapSelectorDialog().SaveUserData();

    if (bApply)
        ApplyFilters();
}

void MapFilterPanel::ApplyFilters()
{
    MapSelectorDialog().ApplyFiltersToCurrentTab();
}

void MapFilterPanel::ResetFilters()
{
    m_szMapNameFilter[0] = '\0';
    m_iDifficultyHighBound = m_iDifficultyLowBound = m_iGameModeFilter = m_iMapLayoutFilter = 0;
    m_bFilterHideCompleted = false;
}

void MapFilterPanel::OnCommand(const char* command)
{
    if (FStrEq(command, "ApplyFilters"))
    {
        ReadFiltersFromControls();
        UpdateFilterSettings();
    }
    else if (FStrEq(command, "ResetFilters"))
    {
        ResetFilters();
        ApplyFiltersToControls();
        UpdateFilterSettings();
    }
    else
        BaseClass::OnCommand(command);
}