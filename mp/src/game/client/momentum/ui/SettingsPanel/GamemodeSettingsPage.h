#pragma once

#include "SettingsPage.h"

using namespace vgui;

class GamemodeSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GamemodeSettingsPage, SettingsPage);

    GamemodeSettingsPage(Panel *pParent);
    ~GamemodeSettingsPage();

    void OnApplyChanges() OVERRIDE;
    void LoadSettings() OVERRIDE;

  private:
    ComboBox *m_pRJParticlesBox, *m_pRJSoundsBox, *m_pRJTrailBox;

    CvarToggleCheckButton *m_pRJEnableTFRocketModel, *m_pRJEnableTFViewModel, *m_pRJEnableCenterFire,
        *m_pRJToggleRocketTrailSound, *m_pRJToggleRocketDecals, *m_pSJEnableCharge, *m_pSJEnableChargeMeter,
        *m_pSJEnableStickyCounter;

    CvarTextEntry *m_pSJStickyDrawDelayEntry;
};