#include "cbase.h"

#include "OnlineSettingsPanel.h"

#include "mom_shareddefs.h"
#include "util/mom_util.h"

#include "controls/ColorPicker.h"
#include "controls/ModelPanel.h"
#include "vgui_controls/CvarToggleCheckButton.h"
#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/CvarTextEntry.h"
#include "vgui_controls/CvarComboBox.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define SET_BUTTON_COLOR(button, color) \
    button->SetDefaultColor(color, color); \
    button->SetArmedColor(color, color); \
    button->SetSelectedColor(color, color);

OnlineSettingsPanel::OnlineSettingsPanel(Panel *pParent, Button *pAssociate) : BaseClass(pParent, "OnlinePage", pAssociate),
                                                                               ghost_color("mom_ghost_color"), ghost_bodygroup("mom_ghost_bodygroup"),
                                                                               ghost_trail_color("mom_trail_color")
{
    SetSize(20, 20);
    // ============ Appearance settings
    m_pModelPreview = new CRenderPanel(this, "ModelPreview");

    m_pEnableTrail = new CvarToggleCheckButton(this, "EnableTrail", "#MOM_Settings_Enable_Trail", "mom_trail_enable");
    m_pPickTrailColorButton = new Button(this, "PickTrailColorButton", "", this, "picker_trail");

    m_pPickBodyColorButton = new Button(this, "PickBodyColorButton", "", this, "picker_body");

    m_pTrailLengthEntry = new CvarTextEntry(this, "TrailEntry", "mom_trail_length", 1);
    m_pTrailLengthEntry->SetAllowNumericInputOnly(true);

    m_pBodygroupCombo = new CvarComboBox(this, "BodygroupCombo", "mom_ghost_bodygroup");
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_0", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_1", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_2", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_3", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_4", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_5", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_6", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_7", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_8", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_9", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_10", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_11", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_12", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_13", nullptr);
    m_pBodygroupCombo->AddItem("#MOM_Settings_Bodygroup_14", nullptr);
    m_pBodygroupCombo->AddActionSignalTarget(this);

    // Color Picker is shared for trail and body
    m_pColorPicker = new ColorPicker(this, this);

    // ========== Online ghost settings
    m_pEnableGhostRotations = new CvarToggleCheckButton(this, "ToggleRotations", "#GameUI_Enabled", "mom_ghost_online_rotations");
    m_pEnableGhostRotations->AddActionSignalTarget(this);
    m_pEnableGhostSounds = new CvarToggleCheckButton(this, "ToggleSounds", "#GameUI_Enabled", "mom_ghost_online_sounds");
    m_pEnableGhostSounds->AddActionSignalTarget(this);
    m_pEnableEntityPanels = new CvarToggleCheckButton(this, "ToggleEntityPanels", "#GameUI_Enabled", "mom_entpanels_enable");
    m_pEnableEntityPanels->AddActionSignalTarget(this);
    m_pEnableGhostTrails = new CvarToggleCheckButton(this, "ToggleTrails", "#GameUI_Enabled", "mom_ghost_online_trail_enable");
    m_pEnableGhostTrails->AddActionSignalTarget(this);
    m_pEnableColorAlphaOverride = new CvarToggleCheckButton(this, "EnableAlphaOverride", "#GameUI_Enabled", "mom_ghost_online_alpha_override_enable");
    m_pEnableColorAlphaOverride->AddActionSignalTarget(this);
    m_pAlphaOverrideSlider = new CvarSlider(this, "AlphaOverrideSlider", "mom_ghost_online_alpha_override", 0, true);
    m_pAlphaOverrideSlider->AddActionSignalTarget(this);
    m_pAlphaOverrideInput = new CvarTextEntry(this, "AlphaOverrideEntry", "mom_ghost_online_alpha_override", 0);
    m_pAlphaOverrideInput->SetAllowNumericInputOnly(true);
    m_pAlphaOverrideInput->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/settings/Settings_Online.res");
}

void OnlineSettingsPanel::OnPageShow()
{
    BaseClass::OnPageShow();

    // Appearance settings
    SetButtonColors();

    LoadModelData();

    // Online ghost settings
    m_pAlphaOverrideSlider->SetEnabled(m_pEnableColorAlphaOverride->IsSelected());
    m_pAlphaOverrideInput->SetEnabled(m_pEnableColorAlphaOverride->IsSelected());
}

void OnlineSettingsPanel::OnCheckboxChecked(Panel *panel)
{
    if (panel == m_pEnableColorAlphaOverride)
    {
        m_pAlphaOverrideSlider->SetEnabled(m_pEnableColorAlphaOverride->IsSelected());
        m_pAlphaOverrideInput->SetEnabled(m_pEnableColorAlphaOverride->IsSelected());
    }
}

void OnlineSettingsPanel::OnTextChanged(Panel *panel, const char *text)
{
    if (panel == m_pBodygroupCombo)
    {
        UpdateModelSettings();
    }
}

void OnlineSettingsPanel::OnColorSelected(KeyValues *pKv)
{
    const auto selected = pKv->GetColor("color");

    Panel *pTarget = static_cast<Panel *>(pKv->GetPtr("targetCallback"));

    if (pTarget == m_pPickTrailColorButton)
    {
        SET_BUTTON_COLOR(m_pPickTrailColorButton, selected);

        char buf[32];
        MomUtil::GetHexStringFromColor(selected, buf, 32);
        ghost_trail_color.SetValue(buf);
    }
    else if (pTarget == m_pPickBodyColorButton)
    {
        SET_BUTTON_COLOR(m_pPickBodyColorButton, selected);

        char buf[32];
        MomUtil::GetHexStringFromColor(selected, buf, 32);
        ghost_color.SetValue(buf);
    }

    UpdateModelSettings();
}

void OnlineSettingsPanel::OnCommand(const char *command)
{
    if (FStrEq(command, "picker_trail"))
    {
        Color trailColor;
        if (MomUtil::GetColorFromHex(ghost_trail_color.GetString(), trailColor))
        {
            m_pColorPicker->SetPickerColor(trailColor);
            m_pColorPicker->SetTargetCallback(m_pPickTrailColorButton);
            m_pColorPicker->DoModal();
        }
    }
    else if (FStrEq(command, "picker_body"))
    {
        Color bodyColor;
        if (MomUtil::GetColorFromHex(ghost_color.GetString(), bodyColor))
        {
            m_pColorPicker->SetPickerColor(bodyColor);
            m_pColorPicker->SetTargetCallback(m_pPickBodyColorButton);
            m_pColorPicker->DoModal();
        }
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void OnlineSettingsPanel::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetButtonColors();
}

void OnlineSettingsPanel::SetButtonColors()
{
    Color trailColor;
    if (MomUtil::GetColorFromHex(ghost_trail_color.GetString(), trailColor))
    {
        SET_BUTTON_COLOR(m_pPickTrailColorButton, trailColor);
    }

    Color bodyColor;
    if (MomUtil::GetColorFromHex(ghost_color.GetString(), bodyColor))
    {
        SET_BUTTON_COLOR(m_pPickBodyColorButton, bodyColor);
    }
}

void OnlineSettingsPanel::UpdateModelSettings()
{
    MDLCACHE_CRITICAL_SECTION();
    CModelPanelModel *pModel = m_pModelPreview->GetModel();
    if (!pModel)
        return;

    // Player color
    Color ghostRenderColor;
    if (MomUtil::GetColorFromHex(ghost_color.GetString(), ghostRenderColor))
    {
        pModel->SetRenderColor(ghostRenderColor.r(), ghostRenderColor.g(), ghostRenderColor.b(), ghostRenderColor.a());
    }

    // Player shape
    pModel->SetBodygroup(1, ghost_bodygroup.GetInt());
}

void OnlineSettingsPanel::LoadModelData()
{
    const bool result = m_pModelPreview->LoadModel(ENTITY_MODEL);
    if (result)
        UpdateModelSettings();
}