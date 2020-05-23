#pragma once

#include "SettingsPage.h"

class SpeedometerLabel;

class HudSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(HudSettingsPage, SettingsPage);

    HudSettingsPage(Panel *pParent);
    ~HudSettingsPage() {}

    void OnCheckboxChecked(Panel *pPanel) override;
    void OnTextChanged(Panel *pPanel) override;
    void LoadSettings() override;

    // to handle speedo changes through console
    void OnPageShow() override { LoadSettings(); }
    void OnSetFocus() override { LoadSettings(); }

private:
    // speedo helper methods
    SpeedometerLabel *GetSpeedoLabelFromType();
    void LoadSpeedoSetup();

    vgui::CvarComboBox *m_pSyncType, *m_pSyncColorize;

    vgui::CvarToggleCheckButton *m_pSyncShow, *m_pSyncShowBar, *m_pButtonsShow, *m_pShowVersion, *m_pTimerShow, *m_pTimerSoundFailEnable,
        *m_pTimerSoundStartEnable, *m_pTimerSoundStopEnable, *m_pTimerSoundFinishEnable;

    // speedo controls
    vgui::ComboBox *m_pSpeedometerGameType, *m_pSpeedometerType, *m_pSpeedometerUnits, *m_pSpeedometerColorize;
    vgui::CheckButton *m_pSpeedometerShow;
};
