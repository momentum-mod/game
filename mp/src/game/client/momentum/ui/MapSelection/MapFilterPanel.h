#pragma once

#include "vgui_controls/EditablePanel.h"

class MapFilterPanel : public vgui::EditablePanel
{
    DECLARE_CLASS_SIMPLE(MapFilterPanel, vgui::EditablePanel);

    MapFilterPanel(Panel *pParent);

    void ReadFiltersFromControls();
    void ApplyFiltersToControls();
    void LoadFilterSettings(KeyValues *pTabFilters);
    void UpdateFilterSettings(bool bApply = true);
    void ApplyFilters();
    void ResetFilters();

protected:
    void OnCommand(const char* command) OVERRIDE;

private:
    // filter controls
    vgui::ComboBox *m_pGameModeFilter;
    vgui::TextEntry *m_pMapNameFilter, *m_pDifficultyLowerBound, *m_pDifficultyHigherBound;
    vgui::CheckButton *m_pHideCompletedFilterCheck;//Used for local maps only
    vgui::ComboBox *m_pMapLayoutFilter;//0 = ALL, 1 = LINEAR ONLY, 2 = STAGED ONLY
    vgui::Button *m_pApplyFiltersButton, *m_pResetFiltersButton;

    // filter data
    int m_iGameModeFilter;
    char m_szMapNameFilter[32];
    int	m_iDifficultyLowBound; // Lower bound for the difficulty, maps have to be >= this
    int m_iDifficultyHighBound; // High bound, maps have to be <= this
    bool m_bFilterHideCompleted;//Hide completed maps
    int m_iMapLayoutFilter;//Map is non-linear (has stages)
};
