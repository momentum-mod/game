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

    void LoadSettings() OVERRIDE;

    void OnTextChanged(Panel *p) OVERRIDE;

    void OnControlModified(Panel *p) OVERRIDE;

private:

    void UpdateSlideEntries() const;

    CvarToggleCheckButton<ConVarRef> *m_pPlayBlockSound;
    CvarToggleCheckButton<ConVarRef> *m_pSaveCheckpoints;
    CvarToggleCheckButton<ConVarRef> *m_pEnableTrail;
    CCvarSlider *m_pYawSpeedSlider;
    CCvarSlider *m_pTrailColorRSlider;
    CCvarSlider *m_pTrailColorGSlider;
    CCvarSlider *m_pTrailColorBSlider;
    CCvarSlider *m_pTrailColorASlider;
    TextEntry *m_pYawSpeedEntry;
    TextEntry *m_pTrailColorREntry;
    TextEntry *m_pTrailColorGEntry;
    TextEntry *m_pTrailColorBEntry;
    TextEntry *m_pTrailColorAEntry;
    Panel *m_pSampleColor;
};