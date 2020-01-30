#pragma once

#include "SettingsPage.h"

class RocketJumpSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(RocketJumpSettingsPage, SettingsPage);

    RocketJumpSettingsPage(Panel *pParent);
    ~RocketJumpSettingsPage();

    void OnApplyChanges() OVERRIDE;
    void LoadSettings() OVERRIDE;

  private:
    vgui::ComboBox *m_pParticlesBox, *m_pSoundsBox, *m_pTrailBox;

    vgui::CvarToggleCheckButton *m_pEnableTFRocketModel, *m_pEnableTFViewModel, *m_pEnableCenterFire,
                                *m_pToggleRocketTrailSound, *m_pToggleRocketDecals;
};