#include "cbase.h"

#include "AppearanceSettingsPage.h"

#include "ienginevgui.h"
#include "util/mom_util.h"
#include "mom_shareddefs.h"

#include <vgui_controls/Frame.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include "controls/ModelPanel.h"
#include "controls/ColorPicker.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define SET_BUTTON_COLOR(button, color) \
    button->SetDefaultColor(color, color); \
    button->SetArmedColor(color, color); \
    button->SetSelectedColor(color, color);

AppearanceSettingsPage::AppearanceSettingsPage(Panel *pParent) : BaseClass(pParent, "AppearanceSettings"), ghost_color("mom_ghost_color"), 
    ghost_bodygroup("mom_ghost_bodygroup"), ghost_trail_color("mom_trail_color"), m_bModelPreviewFrameIsFadingOut(false)
{
    // Outer frame for the model preview
    m_pModelPreviewFrame = new Frame(nullptr, "ModelPreviewFrame");
    m_pModelPreviewFrame->SetProportional(true);
    m_pModelPreviewFrame->SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pModelPreviewFrame->SetSize(scheme()->GetProportionalScaledValue(200), scheme()->GetProportionalScaledValue(275));
    m_pModelPreviewFrame->SetMoveable(false);
    m_pModelPreviewFrame->MoveToFront();
    m_pModelPreviewFrame->SetSizeable(false);
    m_pModelPreviewFrame->SetTitle("#MOM_Settings_Tab_Appearance", false);
    m_pModelPreviewFrame->SetTitleBarVisible(true);
    m_pModelPreviewFrame->SetMenuButtonResponsive(false);
    m_pModelPreviewFrame->SetCloseButtonVisible(false);
    m_pModelPreviewFrame->SetMinimizeButtonVisible(false);
    m_pModelPreviewFrame->SetMaximizeButtonVisible(false);
    m_pModelPreviewFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);
    m_pModelPreviewFrame->InvalidateLayout(true);
    m_pModelPreviewFrame->MakeReadyForUse();

    // Actual model preview
    m_pModelPreview = new CRenderPanel(m_pModelPreviewFrame, "ModelPreview");
    int x, y, wid, tall;
    m_pModelPreviewFrame->GetClientArea(x, y, wid, tall);
    m_pModelPreview->SetBounds(x, y, wid, tall);
    m_pModelPreview->SetPaintBorderEnabled(false);
    m_pModelPreview->MakeReadyForUse();
    m_pModelPreview->SetVisible(true);

    m_pEnableTrail = new CvarToggleCheckButton(this, "EnableTrail", "#MOM_Settings_Enable_Trail", "mom_trail_enable");
    m_pPickTrailColorButton = new Button(this, "PickTrailColorButton", "", this, "picker_trail");

    m_pPickBodyColorButton = new Button(this, "PickBodyColorButton", "", this, "picker_body");

    m_pTrailLengthEntry = new CvarTextEntry(this, "TrailEntry", "mom_trail_length");
    m_pTrailLengthEntry->SetAllowNumericInputOnly(true);

    m_pBodygroupCombo = new ComboBox(this, "BodygroupCombo", 15, false);
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

    LoadControlSettings("resource/ui/SettingsPanel_AppearanceSettings.res");
}

AppearanceSettingsPage::~AppearanceSettingsPage()
{
}

void AppearanceSettingsPage::SetButtonColors()
{
    if (m_pPickTrailColorButton)
    {
        Color trailColor;
        if (MomUtil::GetColorFromHex(ghost_trail_color.GetString(), trailColor))
        {
            SET_BUTTON_COLOR(m_pPickTrailColorButton, trailColor);
        }
    }

    if (m_pPickBodyColorButton)
    {
        Color bodyColor;
        if (MomUtil::GetColorFromHex(ghost_color.GetString(), bodyColor))
        {
            SET_BUTTON_COLOR(m_pPickBodyColorButton, bodyColor);
        }
    }
}

void AppearanceSettingsPage::LoadSettings()
{
    SetButtonColors();

    m_pBodygroupCombo->ActivateItemByRow(ghost_bodygroup.GetInt());

    const bool result = m_pModelPreview->LoadModel(ENTITY_MODEL);
    if (result)
        UpdateModelSettings();
}

void AppearanceSettingsPage::OnPageShow()
{
    if (!m_pModelPreviewFrame->IsVisible() || m_bModelPreviewFrameIsFadingOut)
    {
        m_pModelPreviewFrame->Activate();
        m_bModelPreviewFrameIsFadingOut = false;
    }
}

void AppearanceSettingsPage::OnPageHide()
{
    if (m_pModelPreviewFrame)
    {
        m_pModelPreviewFrame->Close();
        m_bModelPreviewFrameIsFadingOut = true;
    }
}

void AppearanceSettingsPage::OnMainDialogClosed()
{
    OnPageHide();
}

void AppearanceSettingsPage::OnMainDialogShow()
{
    OnPageShow();
}

void AppearanceSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);
    
    if (p == m_pBodygroupCombo)
    {
        ghost_bodygroup.SetValue(m_pBodygroupCombo->GetActiveItem());
        UpdateModelSettings();
    }
}

void AppearanceSettingsPage::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");

    Panel *pTarget = static_cast<Panel*>(pKv->GetPtr("targetCallback"));

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

void AppearanceSettingsPage::OnCommand(const char* command)
{
    if (FStrEq(command, "picker_trail"))
    {
        Color trailColor;
        if (MomUtil::GetColorFromHex(ghost_trail_color.GetString(), trailColor))
        {
            m_pColorPicker->SetPickerColor(trailColor);
            m_pColorPicker->SetTargetCallback(m_pPickTrailColorButton);
            m_pColorPicker->Show();
        }
    }
    else if (FStrEq(command, "picker_body"))
    {
        Color bodyColor;
        if (MomUtil::GetColorFromHex(ghost_color.GetString(), bodyColor))
        {
            m_pColorPicker->SetPickerColor(bodyColor);
            m_pColorPicker->SetTargetCallback(m_pPickBodyColorButton);
            m_pColorPicker->Show();
        }
    }


    BaseClass::OnCommand(command);
}

void AppearanceSettingsPage::ApplySchemeSettings(IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetButtonColors();
}

void AppearanceSettingsPage::OnScreenSizeChanged(int oldwide, int oldtall)
{
    BaseClass::OnScreenSizeChanged(oldwide, oldtall);

    DestroyModelPanel();
}

void AppearanceSettingsPage::UpdateModelSettings()
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

void AppearanceSettingsPage::DestroyModelPanel()
{
    if (m_pModelPreviewFrame)
    {
        m_pModelPreviewFrame->DeletePanel();
    }

    m_pModelPreviewFrame = nullptr;
    m_pModelPreview = nullptr;
}