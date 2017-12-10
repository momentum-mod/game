#include "cbase.h"

#include "AppearanceSettingsPage.h"
#include "ModelPanel.h"
#include "ienginevgui.h"
#include "util/mom_util.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define SET_BUTTON_COLOR(button, color) \
    button->SetDefaultColor(color, color); \
    button->SetArmedColor(color, color); \
    button->SetSelectedColor(color, color);

AppearanceSettingsPage::AppearanceSettingsPage(Panel *pParent) : BaseClass(pParent, "AppearanceSettings"), ghost_color("mom_ghost_color"),
ghost_bodygroup("mom_ghost_bodygroup"), ghost_trail_color("mom_trail_color"), ghost_trail_length("mom_trail_length")
{
    // Outer frame for the model preview
    m_pModelPreviewFrame = new Frame(nullptr, "ModelPreviewFrame");
    m_pModelPreviewFrame->SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pModelPreviewFrame->SetSize(scheme()->GetProportionalScaledValue(200), scheme()->GetProportionalScaledValue(275));
    m_pModelPreviewFrame->SetMoveable(false);
    m_pModelPreviewFrame->MoveToFront();
    m_pModelPreviewFrame->SetSizeable(false);
    m_pModelPreviewFrame->SetTitle("Preview", false); // MOM_TODO: Localize me
    m_pModelPreviewFrame->SetTitleBarVisible(true);
    m_pModelPreviewFrame->SetMenuButtonResponsive(false);
    m_pModelPreviewFrame->SetCloseButtonVisible(true);
    m_pModelPreviewFrame->SetMinimizeButtonVisible(false);
    m_pModelPreviewFrame->SetMaximizeButtonVisible(false);
    m_pModelPreviewFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);

    // Actual model preview
    m_pModelPreview = new CRenderPanel(m_pModelPreviewFrame, "ModelPreview");
    m_pModelPreview->SetPaintBorderEnabled(true);
    m_pModelPreview->SetBorder(scheme()->GetIScheme(GetScheme())->GetBorder("Default"));
    const bool result = m_pModelPreview->LoadModel(ENTITY_MODEL);
    if (result)
        UpdateModelSettings();

    m_pModelPreview->SetVisible(true);
    m_pModelPreview->MakeReadyForUse();


    m_pEnableTrail = FindControl<CvarToggleCheckButton>("EnableTrail");
    m_pPickTrailColorButton = FindControl<Button>("PickTrailColorButton");

    m_pPickBodyColorButton = FindControl<Button>("PickBodyColorButton");

    m_pTrailLengthEntry = FindControl<TextEntry>("TrailEntry");

    m_pBodygroupCombo = FindControl<ComboBox>("BodygroupCombo");
    m_pBodygroupCombo->SetNumberOfEditLines(15);
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
}

AppearanceSettingsPage::~AppearanceSettingsPage()
{
}

void AppearanceSettingsPage::LoadSettings()
{
    if (m_pPickTrailColorButton)
    {
        Color trailColor;
        if (g_pMomentumUtil->GetColorFromHex(ghost_trail_color.GetString(), trailColor))
        {
            SET_BUTTON_COLOR(m_pPickTrailColorButton, trailColor);
        }
    }

    if (m_pPickBodyColorButton)
    {
        Color bodyColor;
        if (g_pMomentumUtil->GetColorFromHex(ghost_color.GetString(), bodyColor))
        {
            SET_BUTTON_COLOR(m_pPickBodyColorButton, bodyColor);
        }
    }

    m_pBodygroupCombo->ActivateItemByRow(ghost_bodygroup.GetInt());
    m_pTrailLengthEntry->SetText(ghost_trail_length.GetString());

    UpdateModelSettings();
}

void AppearanceSettingsPage::OnPageShow()
{
    if (!m_pModelPreviewFrame->IsVisible())
        m_pModelPreviewFrame->Activate();
}

void AppearanceSettingsPage::OnPageHide()
{
    if (m_pModelPreviewFrame)
        m_pModelPreviewFrame->Close();
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
    else if (p == m_pTrailLengthEntry)
    {
        char buf[3];
        m_pTrailLengthEntry->GetText(buf, 3);
        ghost_trail_length.SetValue(buf);
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
        g_pMomentumUtil->GetHexStringFromColor(selected, buf, 32);
        ghost_trail_color.SetValue(buf);
    }
    else if (pTarget == m_pPickBodyColorButton)
    {
        SET_BUTTON_COLOR(m_pPickBodyColorButton, selected);

        char buf[32];
        g_pMomentumUtil->GetHexStringFromColor(selected, buf, 32);
        ghost_color.SetValue(buf);
    }

    UpdateModelSettings();
}

void AppearanceSettingsPage::OnCommand(const char* command)
{
    if (FStrEq(command, "picker_trail"))
    {
        Color trailColor;
        if (g_pMomentumUtil->GetColorFromHex(ghost_trail_color.GetString(), trailColor))
        {
            m_pColorPicker->SetPickerColor(trailColor);
            m_pColorPicker->SetTargetCallback(m_pPickTrailColorButton);
            m_pColorPicker->Show();
        }
    }
    else if (FStrEq(command, "picker_body"))
    {
        Color bodyColor;
        if (g_pMomentumUtil->GetColorFromHex(ghost_color.GetString(), bodyColor))
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
    LoadSettings(); // Called here so the color buttons are properly colored
}

void AppearanceSettingsPage::UpdateModelSettings()
{
    C_BaseFlex *pModel = m_pModelPreview->GetModel();
    if (!pModel)
        return;

    // Player color
    Color ghostRenderColor;
    if (g_pMomentumUtil->GetColorFromHex(ghost_color.GetString(), ghostRenderColor))
    {
        pModel->SetRenderColor(ghostRenderColor.r(), ghostRenderColor.g(), ghostRenderColor.b(), ghostRenderColor.a());
    }
    
    // Player shape
    pModel->SetBodygroup(1, ghost_bodygroup.GetInt());
}