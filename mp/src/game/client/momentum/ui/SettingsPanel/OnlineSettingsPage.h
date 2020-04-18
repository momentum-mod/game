#pragma once

#include "SettingsPage.h"

class OnlineSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(OnlineSettingsPage, SettingsPage);

    OnlineSettingsPage(Panel *pParent);

    ~OnlineSettingsPage() {}

    void LoadSettings() OVERRIDE;

    void OnCheckboxChecked(Panel *p) OVERRIDE;

private:
    vgui::CvarSlider *m_pAlphaOverrideSlider;
    vgui::CvarTextEntry *m_pAlphaOverrideInput;
    vgui::CvarToggleCheckButton *m_pEnableGhostRotations, *m_pEnableGhostSounds, *m_pEnableEntityPanels,
        *m_pEnableGhostTrails, *m_pEnableColorAlphaOverride;
};
