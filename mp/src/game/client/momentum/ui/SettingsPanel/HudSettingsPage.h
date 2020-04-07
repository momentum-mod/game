#pragma once

#include "SettingsPage.h"

class HudSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(HudSettingsPage, SettingsPage);

    HudSettingsPage(Panel *pParent);

    ~HudSettingsPage() {}

    //This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;

private:
    vgui::CvarComboBox *m_pSpeedometerUnits, *m_pSyncType, *m_pSyncColorize, *m_pSpeedometerColorize;

    vgui::CvarToggleCheckButton *m_pSpeedometerShow, *m_pSpeedometerHorizShow, *m_pSpeedometerShowLastJump, *m_pSpeedometerShowStageEnter,
        *m_pSpeedometerUnitLabels, *m_pSyncShow, *m_pSyncShowBar, *m_pButtonsShow, *m_pShowVersion, *m_pTimerShow;
};
