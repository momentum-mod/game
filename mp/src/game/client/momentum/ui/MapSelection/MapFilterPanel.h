#pragma once

#include "vgui_controls/EditablePanel.h"
#include "IMapList.h"

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

    MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", pKv);
    MESSAGE_FUNC_INT(OnCheckButtonChecked, "CheckButtonChecked", state);

private:
    // filter controls
    vgui::ComboBox *m_pGameModeFilter;
    vgui::TextEntry *m_pMapNameFilter, *m_pDifficultyLowerBound, *m_pDifficultyHigherBound;
    vgui::CheckButton *m_pHideCompletedFilterCheck;//Used for local maps only
    vgui::ComboBox *m_pMapLayoutFilter;//0 = ALL, 1 = LINEAR ONLY, 2 = STAGED ONLY
    vgui::Button *m_pApplyFiltersButton, *m_pResetFiltersButton;

    // filter data
    MapFilters_t m_Filters;
};
