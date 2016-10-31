#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "CVarSlider.h"
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

    void OnCommand(const char *pCommand) OVERRIDE;

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);

private:

    void UpdateSliderEntries() const;

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
    ColorPicker *m_pColorPicker;
    Button *m_pPickColorButton;
    Panel *m_pSampleColor;
};