#pragma once

#include "cbase.h"

#include "SettingsPage.h"

class GameplaySettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GameplaySettingsPage, SettingsPage);

    GameplaySettingsPage(Panel *pParent);

    ~GameplaySettingsPage() {}

    void LoadSettings() OVERRIDE;

    void OnTextChanged(Panel *p) OVERRIDE;

    void OnControlModified(Panel *p) OVERRIDE;

    void OnCheckboxChecked(Panel *p) OVERRIDE;

private:
    void UpdateSliderEntries() const;

    vgui::CvarToggleCheckButton *m_pPlayBlockSound;
    vgui::CvarToggleCheckButton *m_pSaveCheckpoints;
    vgui::CvarSlider *m_pYawSpeedSlider;
    vgui::TextEntry *m_pYawSpeedEntry;

    vgui::CvarTextEntry *m_pLowerSpeedCVarEntry;
    vgui::CvarToggleCheckButton *m_pLowerSpeed;
};