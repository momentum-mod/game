#pragma once

#include "SettingsPage.h"

class GameplaySettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GameplaySettingsPage, SettingsPage);

    GameplaySettingsPage(Panel *pParent);

    ~GameplaySettingsPage() {}

    void LoadSettings() OVERRIDE;

    void OnCheckboxChecked(Panel *p) OVERRIDE;

private:
    vgui::CvarToggleCheckButton *m_pOverlappingKeys;
    vgui::CvarToggleCheckButton *m_pReleaseForwardOnJump;

    vgui::CvarToggleCheckButton *m_pPlayBlockSound;
    vgui::CvarToggleCheckButton *m_pSaveCheckpoints;
    vgui::CvarSlider *m_pYawSpeedSlider;
    vgui::CvarTextEntry *m_pYawSpeedEntry;

    vgui::CvarTextEntry *m_pLowerSpeedCVarEntry;
    vgui::CvarToggleCheckButton *m_pLowerSpeed;
    vgui::Label *m_pLowerSpeedLabel;
};