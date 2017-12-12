#pragma once

#include "SettingsPage.h"
#include <vgui_controls/Frame.h>
#include "vgui_controls/CVarSlider.h"

class OnlineSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(OnlineSettingsPage, SettingsPage);

    OnlineSettingsPage(Panel *pParent);

    ~OnlineSettingsPage() {}

    void OnApplyChanges() OVERRIDE;

    void LoadSettings() OVERRIDE;

    //This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;

    void OnTextChanged(vgui::Panel* panel) OVERRIDE;

    void OnControlModified(vgui::Panel* panel) OVERRIDE;

private:
    void UpdateSliderSettings();


    vgui::CCvarSlider *m_pAlphaOverrideSlider;
    vgui::TextEntry *m_pAlphaOverrideInput;
    vgui::CvarToggleCheckButton *m_pEnableColorAlphaOverride;
};
