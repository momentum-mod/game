#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include <vgui_controls/CVarSlider.h>
#include <vgui_controls/Button.h>
#include "ColorPicker.h"

using namespace vgui;

class GameplaySettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GameplaySettingsPage, SettingsPage);

    GameplaySettingsPage(Panel *pParent);

    ~GameplaySettingsPage() {}

    void LoadSettings() OVERRIDE;

    void OnTextChanged(Panel *p) OVERRIDE;

    void OnControlModified(Panel *p) OVERRIDE;

private:
    void UpdateSliderEntries() const;

    CvarToggleCheckButton *m_pPlayBlockSound;
    CvarToggleCheckButton *m_pSaveCheckpoints;
    CCvarSlider *m_pYawSpeedSlider;
    TextEntry *m_pYawSpeedEntry;
};