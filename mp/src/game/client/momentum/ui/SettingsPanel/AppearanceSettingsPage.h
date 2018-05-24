#pragma once

#include "cbase.h"

#include "SettingsPage.h"

class CRenderPanel;
namespace vgui
{
    class ColorPicker;
}

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
    void ApplySchemeSettings(vgui::IScheme* pScheme) OVERRIDE;

private:
    void UpdateModelSettings();


    vgui::Frame *m_pModelPreviewFrame;
    CRenderPanel *m_pModelPreview;

    ConVarRef ghost_color, ghost_bodygroup, ghost_trail_color, ghost_trail_length; // MOM_TODO add the rest of visible things here

    vgui::ComboBox *m_pBodygroupCombo;

    vgui::CvarToggleCheckButton *m_pEnableTrail;

    vgui::TextEntry *m_pTrailLengthEntry;
    vgui::ColorPicker *m_pColorPicker;
    vgui::Button *m_pPickTrailColorButton, *m_pPickBodyColorButton;
};