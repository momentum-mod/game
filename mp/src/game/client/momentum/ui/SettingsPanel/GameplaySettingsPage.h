#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "CVarSlider.h"
#include <vgui_controls/Button.h>

using namespace vgui;

class GameplaySettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(GameplaySettingsPage, SettingsPage);

    GameplaySettingsPage(Panel *pParent);

    ~GameplaySettingsPage() {}

    void LoadSettings() override;

    void OnTextChanged(Panel *p) override;

    void OnControlModified(Panel *p) override;

private:

    void UpdateYawspeedEntry() const;

    CvarToggleCheckButton<ConVarRef> *m_pPlayBlockSound;
    CCvarSlider *m_pYawSpeedSlider;
    TextEntry *m_pYawSpeedEntry;
};