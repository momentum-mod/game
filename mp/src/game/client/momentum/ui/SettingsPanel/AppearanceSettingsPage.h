#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "ColorPicker.h"
#include "vgui_controls/CVarSlider.h"

class CRenderPanel;

class AppearanceSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(AppearanceSettingsPage, SettingsPage);

    AppearanceSettingsPage(Panel *pParent);
    ~AppearanceSettingsPage();

    void LoadSettings() OVERRIDE;
    void OnPageShow() OVERRIDE;
    void OnPageHide() OVERRIDE;
    void OnMainDialogClosed() OVERRIDE;
    void OnMainDialogShow() OVERRIDE;
    void OnTextChanged(Panel *p) OVERRIDE;

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);
    void OnCommand(const char* command) OVERRIDE;
    void ApplySchemeSettings(IScheme* pScheme) OVERRIDE;

private:
    void UpdateModelSettings();
    

    Frame *m_pModelPreviewFrame;
    CRenderPanel *m_pModelPreview;

    ConVarRef ghost_color, ghost_bodygroup, ghost_trail_color, ghost_trail_length; // MOM_TODO add the rest of visible things here

    ComboBox *m_pBodygroupCombo;

    CvarToggleCheckButton *m_pEnableTrail;
    
    TextEntry *m_pTrailLengthEntry;
    ColorPicker *m_pColorPicker;
    Button *m_pPickTrailColorButton, *m_pPickBodyColorButton;
};