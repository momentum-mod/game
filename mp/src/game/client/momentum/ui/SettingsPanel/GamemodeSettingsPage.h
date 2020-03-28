#pragma once

#include "SettingsPage.h"

using namespace vgui;

class GamemodeSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GamemodeSettingsPage, SettingsPage);

    GamemodeSettingsPage(Panel *pParent);
    ~GamemodeSettingsPage();

    // This uses OnCheckbox and not OnModified because we want to be able to enable
    // the other checkboxes regardless of whether the player clicks Apply/OK
    void OnCheckboxChecked(Panel *p) OVERRIDE;

  private:
    CvarToggleCheckButton *m_pRJEnableTrailParticle, *m_pRJEnableExplosionParticle, *m_pRJEnableShootSound,
                          *m_pRJToggleRocketTrailSound, *m_pRJToggleRocketExplosionSound, *m_pRJToggleRocketDecals,
                          *m_pRJEnableCenterFire;

    CvarTextEntry *m_pRJRocketDrawDelayEntry;

    CvarToggleCheckButton *m_pSJEnableTrailParticle, *m_pSJEnableExplosionParticle, *m_pSJToggleStickybombDecals, 
                          *m_pSJEnableExplosionSound, *m_pSJEnableDetonateSuccessSound, *m_pSJEnableDetonateFailSound, 
                          *m_pSJEnableChargeSound, *m_pSJEnableChargeMeter, *m_pSJEnableStickyCounter, *m_pSJStickyCounterAutohide;

    CvarComboBox *m_pSJChargeMeterUnits;

    CvarTextEntry *m_pSJStickyDrawDelayEntry;
};