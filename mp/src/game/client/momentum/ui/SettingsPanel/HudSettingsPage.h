#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/pch_vgui_controls.h>

using namespace vgui;

class HudSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(HudSettingsPage, SettingsPage);

    HudSettingsPage(Panel *pParent);

    ~HudSettingsPage() {}

    void OnApplyChanges() OVERRIDE;

    void LoadSettings() OVERRIDE;

    //This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;

private:
    ComboBox *m_pSpeedometerUnits, *m_pSyncType, *m_pSyncColorize, *m_pSpeedometerColorize;

    CvarToggleCheckButton *m_pSpeedometerShow, *m_pSpeedometerShowLastJump, *m_pSpeedometerShowVerticalVel,
        *m_pSyncShow, *m_pSyncShowBar, *m_pButtonsShow, *m_pShowVersion, *m_pTimerShow;
};
