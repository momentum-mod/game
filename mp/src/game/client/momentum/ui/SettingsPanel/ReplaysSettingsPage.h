#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "ColorPicker.h"
#include "CVarSlider.h"

class CRenderPanel;

class ReplaysSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ReplaysSettingsPage, SettingsPage);

    ReplaysSettingsPage(Panel *pParent);
    ~ReplaysSettingsPage();

    void LoadSettings() OVERRIDE;
    void OnPageShow() OVERRIDE;
    void OnPageHide() OVERRIDE;
    void OnMainDialogClosed() OVERRIDE;
    void OnMainDialogShow() OVERRIDE;
    void OnTextChanged(Panel *p) OVERRIDE;

    void OnControlModified(Panel *p) OVERRIDE;

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);
    void OnCommand(const char* command) OVERRIDE;
    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;

private:
    void UpdateModelSettings();
    void UpdateSliderSettings();

    Frame *m_pModelPreviewFrame;
    CRenderPanel *m_pModelPreview;

    ConVarRef ghost_color, ghost_bodygroup, ghost_trail_color; // MOM_TODO add the rest of visible things here

    CvarToggleCheckButton *m_pEnableTrail, *m_pEnableColorAlphaOverride;
    CCvarSlider *m_pAlphaOverrideSlider;
    TextEntry *m_pAlphaOverrideInput;
    ColorPicker *m_pColorPicker;
    Button *m_pPickTrailColorButton, *m_pPickBodyColorButton;
};