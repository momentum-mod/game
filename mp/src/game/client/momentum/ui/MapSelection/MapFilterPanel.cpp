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
    m_pMapNameFilter = new TextEntry(this, "MapFilter");//As-is, people can search by map name
    
    //Difficulty filter
    m_pDifficultyLowerBound = new TextEntry(this, "DifficultyLow");
    m_pDifficultyHigherBound = new TextEntry(this, "DifficultyHigh");

    //Hide completed maps
    m_pHideCompletedFilterCheck = new CheckButton(this, "HideCompletedFilterCheck", "#MOM_MapSelector_FilterCompletedMaps");
    
    //Filter staged/linear
    m_pMapLayoutFilter = new ComboBox(this, "MapLayoutFilter", 3, false);

    // Buttons
    m_pResetFiltersButton = new Button(this, "ResetFilters", "#MOM_MapSelector_FilterReset", this, "ResetFilters");
    m_pApplyFiltersButton = new Button(this, "ApplyFilters", "#MOM_MapSelector_FilterApply", this, "ApplyFilters");

    LoadControlSettings("resource/ui/MapSelector/MapFilters.res");

    m_pGameModeFilter->AddItem("#MOM_All", nullptr);//All
    m_pGameModeFilter->AddItem("#MOM_GameType_Surf", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_Bhop", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_KZ", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_RJ", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_Tricksurf", nullptr);
    m_pGameModeFilter->AddItem("#MOM_GameType_Trikz", nullptr);
    m_pGameModeFilter->AddActionSignalTarget(this);

    m_pMapNameFilter->AddActionSignalTarget(this);
    
    m_pDifficultyLowerBound->SetAllowNumericInputOnly(true);
    m_pDifficultyLowerBound->AddActionSignalTarget(this);
    m_pDifficultyHigherBound->SetAllowNumericInputOnly(true);
    m_pDifficultyHigherBound->AddActionSignalTarget(this);

    m_pHideCompletedFilterCheck->AddActionSignalTarget(this);

    m_pMapLayoutFilter->AddItem("#MOM_All", nullptr);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_StagedOnly", nullptr);
    m_pMapLayoutFilter->AddItem("#MOM_MapSelector_LinearOnly", nullptr);
    m_pMapLayoutFilter->AddActionSignalTarget(this);
}

void MapFilterPanel::ReadFiltersFromControls()
{
    //Gamemode
    m_Filters.m_iGameMode = m_pGameModeFilter->GetActiveItem();

    // Map name
    m_pMapNameFilter->GetText(m_Filters.m_szMapName, sizeof(m_Filters.m_szMapName) - 1);
    Q_strlower(m_Filters.m_szMapName);

    // Difficulty
    char buf[12];
    m_pDifficultyLowerBound->GetText(buf, sizeof(buf));
    m_Filters.m_iDifficultyLow = m_pDifficultyLowerBound->GetTextLength() ? Q_atoi(buf) : 0;
    m_pDifficultyHigherBound->GetText(buf, sizeof(buf));
    m_Filters.m_iDifficultyHigh = m_pDifficultyHigherBound->GetTextLength() ? Q_atoi(buf) : 0;

    // Hide completed maps
    m_Filters.m_bHideCompleted = m_pHideCompletedFilterCheck->IsSelected();
    // Showing specfic map layouts?
    m_Filters.m_iMapLayout = m_pMapLayoutFilter->GetActiveItem();
}

void MapFilterPanel::ApplyFiltersToControls()
{
    m_pGameModeFilter->ActivateItemByRow(m_Filters.m_iGameMode);

    m_pMapNameFilter->SetText(m_Filters.m_szMapName);

    m_pMapLayoutFilter->ActivateItemByRow(m_Filters.m_iMapLayout);

    m_pHideCompletedFilterCheck->SetSelected(m_Filters.m_bHideCompleted);

    m_pDifficultyLowerBound->SetText(CFmtStr("%i", m_Filters.m_iDifficultyLow));
    m_pDifficultyHigherBound->SetText(CFmtStr("%i", m_Filters.m_iDifficultyHigh));
}

void MapFilterPanel::LoadFilterSettings(KeyValues *pTabFilters)
{
    m_Filters.FromKV(pTabFilters);

    // apply to the controls and update
    ApplyFiltersToControls();
    UpdateFilterSettings();
}

void MapFilterPanel::UpdateFilterSettings(bool bApply /* = true*/)
{
    // copy filter settings into filter file
    KeyValues *filter = MapSelectorDialog().GetCurrentTabFilterData();
    m_Filters.ToKV(filter);
    MapSelectorDialog().SaveUserData();

    if (bApply)
        ApplyFilters();
}

void MapFilterPanel::ApplyFilters()
{
    MapSelectorDialog().ApplyFiltersToCurrentTab(m_Filters);
}

void MapFilterPanel::ResetFilters()
{
    m_Filters.Reset();
}

void MapFilterPanel::OnCommand(const char* command)
{
    if (FStrEq(command, "ApplyFilters"))
    {
        ReadFiltersFromControls();
        UpdateFilterSettings();
        m_pApplyFiltersButton->SetEnabled(false);
    }
    else if (FStrEq(command, "ResetFilters"))
    {
        ResetFilters();
        ApplyFiltersToControls();
        UpdateFilterSettings();
        m_pResetFiltersButton->SetEnabled(false);
        m_pApplyFiltersButton->SetEnabled(false);
    }
    else
        BaseClass::OnCommand(command);
}

void MapFilterPanel::OnTextChanged(KeyValues* pKv)
{
    m_pApplyFiltersButton->SetEnabled(true);
    m_pResetFiltersButton->SetEnabled(true);
}

void MapFilterPanel::OnCheckButtonChecked(int state)
{
    if (m_pHideCompletedFilterCheck->IsSelected() != m_Filters.m_bHideCompleted)
    {
        m_pApplyFiltersButton->SetEnabled(true);
        m_pResetFiltersButton->SetEnabled(true);
    }
}