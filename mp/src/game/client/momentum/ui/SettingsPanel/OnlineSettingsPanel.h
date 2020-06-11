#pragma once

#include "SettingsPanel.h"

namespace vgui
{
    class ColorPicker;
}

class CRenderPanel;

class OnlineSettingsPanel : public SettingsPanel
{
public:
    DECLARE_CLASS_SIMPLE(OnlineSettingsPanel, SettingsPanel);

    OnlineSettingsPanel(Panel *pParent, vgui::Button *pAssociate);

    void OnPageShow() override;

    void OnCheckboxChecked(Panel *panel) override;
    void OnTextChanged(Panel *panel, const char *text) override;

protected:

    // From the color picker
    MESSAGE_FUNC_PARAMS(OnColorSelected, "ColorSelected", pKv);
    void OnCommand(const char *command) override;
    void ApplySchemeSettings(vgui::IScheme *pScheme) override;

private:
    // Appearance settings
    void SetButtonColors();
    void UpdateModelSettings();
    void LoadModelData();

    vgui::DHANDLE<CRenderPanel> m_pModelPreview;

    ConVarRef ghost_color, ghost_bodygroup, ghost_trail_color;

    vgui::CvarComboBox *m_pBodygroupCombo;

    vgui::CvarToggleCheckButton *m_pEnableTrail;

    vgui::CvarTextEntry *m_pTrailLengthEntry;
    vgui::ColorPicker *m_pColorPicker;
    vgui::Button *m_pPickTrailColorButton, *m_pPickBodyColorButton;

    // Online ghost settings
    vgui::CvarSlider *m_pAlphaOverrideSlider;
    vgui::CvarTextEntry *m_pAlphaOverrideInput;
    vgui::CvarToggleCheckButton *m_pEnableGhostRotations, *m_pEnableGhostSounds, *m_pEnableEntityPanels,
        *m_pEnableGhostTrails, *m_pEnableColorAlphaOverride;
};