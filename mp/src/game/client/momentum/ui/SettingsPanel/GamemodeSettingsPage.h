#pragma once

#include "SettingsPage.h"

using namespace vgui;

class GamemodeSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GamemodeSettingsPage, SettingsPage);

    GamemodeSettingsPage(Panel *pParent);
    ~GamemodeSettingsPage();

  private:
    CvarToggleCheckButton *m_pRJEnableTrailParticle, *m_pRJEnableExplosionParticle, *m_pRJEnableShootSound,
                          *m_pRJToggleRocketTrailSound, *m_pRJToggleRocketExplosionSound, *m_pRJToggleRocketDecals,
                          *m_pRJEnableCenterFire;

    CvarTextEntry *m_pRJRocketDrawDelayEntry;

    CvarToggleCheckButton *m_pSJEnableExplosionParticle, *m_pSJEnableExplosionSound, *m_pSJEnableCharge,
                          *m_pSJEnableChargeMeter, *m_pSJEnableStickyCounter;

    CvarTextEntry *m_pSJStickyDrawDelayEntry;
};