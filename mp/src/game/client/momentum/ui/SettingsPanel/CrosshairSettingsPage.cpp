#include "cbase.h"

#include "CrosshairSettingsPage.h"

#include <ienginevgui.h>
#include "CrosshairSettingsPreview.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarSlider.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include "controls/ColorPicker.h"

#include <tier0/memdbgon.h>

using namespace vgui;

#define SET_BUTTON_COLOR(button, color)                                                                                \
    button->SetDefaultColor(color, color);                                                                             \
    button->SetArmedColor(color, color);                                                                               \
    button->SetSelectedColor(color, color);

CrosshairSettingsPage::CrosshairSettingsPage(Panel *pParent) : BaseClass(pParent, "CrosshairSettings")
{
    // CvarToggleCheckButton
    m_pCrosshairShow = new CvarToggleCheckButton(this, "CrosshairShow", "#MOM_Settings_Crosshair_Enable", "crosshair");
    m_pCrosshairShow->AddActionSignalTarget(this);
    m_pCrosshairDot = new CvarToggleCheckButton(this, "CrosshairDot", "#MOM_Settings_Crosshair_Dot", "cl_crosshair_dot");
    m_pCrosshairDot->AddActionSignalTarget(this);
    m_pCrosshairAlphaEnable = new CvarToggleCheckButton(this, "CrosshairAlphaEnable", "#MOM_Settings_Crosshair_Alpha", "cl_crosshair_alpha_enable");
    m_pCrosshairAlphaEnable->AddActionSignalTarget(this);
    m_pDynamicFire = new CvarToggleCheckButton(this, "DynamicFire", "#MOM_Settings_Crosshair_Dynamic_Fire", "cl_crosshair_dynamic_fire");
    m_pDynamicFire->AddActionSignalTarget(this);
    m_pDynamicMove = new CvarToggleCheckButton(this, "DynamicMove", "#MOM_Settings_Crosshair_Dynamic_Move", "cl_crosshair_dynamic_move");
    m_pDynamicMove->AddActionSignalTarget(this);
    m_pCrosshairDrawT = new CvarToggleCheckButton(this, "CrosshairDrawT", "#MOM_Settings_Crosshair_T", "cl_crosshair_t");
    m_pCrosshairDrawT->AddActionSignalTarget(this);
    m_pWeaponGap = new CvarToggleCheckButton(this, "WeaponGap", "#MOM_Settings_Crosshair_Gap_Weapon", "cl_crosshair_gap_use_weapon_value");
    m_pWeaponGap->AddActionSignalTarget(this);
    m_pOutlineEnable = new CvarToggleCheckButton(this, "OutlineEnable", "#MOM_Settings_Crosshair_Outline", "cl_crosshair_outline_enable");
    m_pOutlineEnable->AddActionSignalTarget(this);
    m_pScaleEnable = new CvarToggleCheckButton(this, "ScaleEnable", "#MOM_Settings_Crosshair_Scale_Enable", "cl_crosshair_scale_enable");
    m_pScaleEnable->AddActionSignalTarget(this);

    // CvarSlider and TextEntry
    m_pOutlineThicknessSlider = new CvarSlider(this, "OutlineThickness", nullptr, 0.0f, 5.0f, "cl_crosshair_outline_thickness", false);
    m_pOutlineThicknessSlider->AddActionSignalTarget(this);
    m_pOutlineThicknessEntry = new TextEntry(this, "OutlineThicknessEntry");
    m_pOutlineThicknessEntry->SetAllowNumericInputOnly(true);
    m_pOutlineThicknessEntry->AddActionSignalTarget(this);

    m_pCrosshairThicknessSlider = new CvarSlider(this, "CrosshairThickness", nullptr, 0.0f, 5.0f, "cl_crosshair_thickness", false);
    m_pCrosshairThicknessSlider->AddActionSignalTarget(this);
    m_pCrosshairThicknessEntry = new TextEntry(this, "CrosshairThicknessEntry");
    m_pCrosshairThicknessEntry->SetAllowNumericInputOnly(true);
    m_pCrosshairThicknessEntry->AddActionSignalTarget(this);

    m_pCrosshairScaleSlider = new CvarSlider(this, "CrosshairScale", nullptr, 0.0f, 2160.0f, "cl_crosshair_scale", false);
    m_pCrosshairScaleSlider->AddActionSignalTarget(this);
    m_pCrosshairScaleEntry = new TextEntry(this, "CrosshairScaleEntry");
    m_pCrosshairScaleEntry->SetAllowNumericInputOnly(true);
    m_pCrosshairScaleEntry->AddActionSignalTarget(this);

    m_pCrosshairSizeSlider = new CvarSlider(this, "CrosshairSize", nullptr, 0.0f, 200.0f, "cl_crosshair_size", false);
    m_pCrosshairSizeSlider->AddActionSignalTarget(this);
    m_pCrosshairSizeEntry = new TextEntry(this, "CrosshairSizeEntry");
    m_pCrosshairSizeEntry->SetAllowNumericInputOnly(true);
    m_pCrosshairSizeEntry->AddActionSignalTarget(this);

    m_pCrosshairGapSlider = new CvarSlider(this, "CrosshairGap", nullptr, 0.0f, 100.0f, "cl_crosshair_gap", false);
    m_pCrosshairGapSlider->AddActionSignalTarget(this);
    m_pCrosshairGapEntry = new TextEntry(this, "CrosshairGapEntry");
    m_pCrosshairGapEntry->SetAllowNumericInputOnly(true);
    m_pCrosshairGapEntry->AddActionSignalTarget(this);

    //Custom crosshairs are currently implemented by using the string of a texture in hud_textures
    //TODO: allow the user to choose a file instead (including images to convert to .vtfs?)
    m_pCustomFileEntry = new TextEntry(this, "CustomFileEntry");
    m_pCustomFileEntry->SetAllowNonAsciiCharacters(false);
    m_pCustomFileEntry->AddActionSignalTarget(this);

    // ComboBox
    m_pCrosshairStyle = new ComboBox(this, "CrosshairStyle", 4, false);
    m_pCrosshairStyle->AddItem("#MOM_Settings_Crosshair_Style_0", nullptr);
    m_pCrosshairStyle->AddItem("#MOM_Settings_Crosshair_Style_1", nullptr);
    m_pCrosshairStyle->AddItem("#MOM_Settings_Crosshair_Style_2", nullptr);
    m_pCrosshairStyle->AddItem("#MOM_Settings_Crosshair_Style_3", nullptr);
    m_pCrosshairStyle->AddActionSignalTarget(this);

    // ColorPicker
    m_pCrosshairColorButton = new Button(this, "CrosshairColorButton", "", this, "picker_crosshair");
    m_pCrosshairColorPicker = new ColorPicker(this, this);

    //Crosshair preview
    m_pCrosshairPreviewFrame = nullptr;
    m_pCrosshairPreviewPanel = nullptr;

    LoadControlSettings("resource/ui/SettingsPanel_CrosshairSettings.res");
}

CrosshairSettingsPage::~CrosshairSettingsPage() {}

void CrosshairSettingsPage::SetButtonColors()
{
    if (m_pCrosshairColorButton)
    {
        Color crosshairColor;
        if (MomUtil::GetColorFromHex(ConVarRef("cl_crosshair_color").GetString(), crosshairColor))
        {
            SET_BUTTON_COLOR(m_pCrosshairColorButton, crosshairColor);
        }
    }
}

void CrosshairSettingsPage::DestroyBogusCrosshairPanel()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->DeletePanel();

    m_pCrosshairPreviewFrame = nullptr;
    m_pCrosshairPreviewPanel = nullptr;
}

void CrosshairSettingsPage::InitBogusCrosshairPanel()
{
    // Initialize the frame we're putting this into
    m_pCrosshairPreviewFrame = new Frame(nullptr, "CrosshairPreviewFrame");
    m_pCrosshairPreviewFrame->SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
    m_pCrosshairPreviewFrame->SetSize(350, scheme()->GetProportionalScaledValue(275));
    m_pCrosshairPreviewFrame->SetMoveable(false);
    m_pCrosshairPreviewFrame->MoveToFront();
    m_pCrosshairPreviewFrame->SetSizeable(false);
    m_pCrosshairPreviewFrame->SetTitle("#MOM_Settings_Crosshair_Bogus_Title", false);
    m_pCrosshairPreviewFrame->SetTitleBarVisible(true);
    m_pCrosshairPreviewFrame->SetMenuButtonResponsive(false);
    m_pCrosshairPreviewFrame->SetCloseButtonVisible(false);
    m_pCrosshairPreviewFrame->SetMinimizeButtonVisible(false);
    m_pCrosshairPreviewFrame->SetMaximizeButtonVisible(false);
    m_pCrosshairPreviewFrame->PinToSibling("CMomentumSettingsPanel", PIN_TOPRIGHT, PIN_TOPLEFT);
    m_pCrosshairPreviewFrame->InvalidateLayout(true);

    m_pCrosshairPreviewPanel = new C_CrosshairPreview("CrosshairPreviewPanel", m_pCrosshairPreviewFrame);
    m_pCrosshairPreviewPanel->AddActionSignalTarget(this);
    m_pCrosshairPreviewPanel->SetPaintBackgroundEnabled(true);
    m_pCrosshairPreviewPanel->SetPaintBackgroundType(2);
    int x, y, wid, tal;
    m_pCrosshairPreviewFrame->GetClientArea(x, y, wid, tal);
    m_pCrosshairPreviewPanel->SetBounds(x, y, wid, tal);
    m_pCrosshairPreviewPanel->SetScheme(scheme()->GetScheme("ClientScheme"));
    m_pCrosshairPreviewPanel->SetVisible(true);
    m_pCrosshairPreviewPanel->MakeReadyForUse();

    // Finally, set the frame visible (after the bogus panel is loaded and dandy)
    m_pCrosshairPreviewFrame->SetVisible(true);
}

void CrosshairSettingsPage::OnMainDialogClosed()
{
    OnPageHide();
}

void CrosshairSettingsPage::OnMainDialogShow()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->SetVisible(true);

    LoadSettings();
}

void CrosshairSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();
}

void CrosshairSettingsPage::OnCommand(const char *command)
{
    if (FStrEq(command, "picker_crosshair"))
    {
        Color crosshairColor;
        if (MomUtil::GetColorFromHex(ConVarRef("cl_crosshair_color").GetString(), crosshairColor))
        {
            m_pCrosshairColorPicker->SetPickerColor(crosshairColor);
            m_pCrosshairColorPicker->SetTargetCallback(m_pCrosshairColorButton);
            m_pCrosshairColorPicker->Show();
        }
    }

    BaseClass::OnCommand(command);
}

void CrosshairSettingsPage::LoadSettings()
{
    SetButtonColors();
    m_pCrosshairStyle->ActivateItemByRow(ConVarRef("cl_crosshair_style").GetInt());
    m_pCustomFileEntry->SetText(ConVarRef("cl_crosshair_file").GetString());

    m_pOutlineThicknessSlider->SetSliderValue(ConVarRef("cl_crosshair_outline_thickness").GetFloat());
    m_pCrosshairThicknessSlider->SetSliderValue(ConVarRef("cl_crosshair_thickness").GetFloat());
    m_pCrosshairScaleSlider->SetSliderValue(ConVarRef("cl_crosshair_scale").GetFloat());
    m_pCrosshairSizeSlider->SetSliderValue(ConVarRef("cl_crosshair_size").GetFloat());
    m_pCrosshairGapSlider->SetSliderValue(ConVarRef("cl_crosshair_gap").GetFloat());

    UpdateSliderEntries();
    UpdateStyleToggles();
}

void CrosshairSettingsPage::OnPageShow()
{
    BaseClass::OnPageShow();

    if (!m_pCrosshairPreviewFrame)
        InitBogusCrosshairPanel();
    else if (!m_pCrosshairPreviewFrame->IsVisible())
        m_pCrosshairPreviewFrame->Activate();
}

void CrosshairSettingsPage::OnPageHide()
{
    if (m_pCrosshairPreviewFrame)
        m_pCrosshairPreviewFrame->Close();
}

void CrosshairSettingsPage::OnCheckboxChecked(Panel *p)
{
    BaseClass::OnCheckboxChecked(p);

    if (p == m_pCrosshairShow)
    {
        bool bEnabled = m_pCrosshairShow->IsSelected();
        ConVarRef("crosshair").SetValue(bEnabled);

        if (bEnabled)
        {
            UpdateStyleToggles();
            m_pCrosshairAlphaEnable->SetEnabled(true);
            m_pCrosshairColorButton->SetEnabled(true);
            m_pDynamicFire->SetEnabled(true);
            m_pDynamicMove->SetEnabled(true);
            m_pCrosshairStyle->SetEnabled(true);
        }
        else
        {
            m_pCrosshairAlphaEnable->SetEnabled(false);
            m_pCrosshairColorButton->SetEnabled(false);
            m_pDynamicFire->SetEnabled(false);
            m_pDynamicMove->SetEnabled(false);
            m_pCrosshairStyle->SetEnabled(false);

            m_pCrosshairDot->SetEnabled(false);
            m_pCustomFileEntry->SetEnabled(false);
            m_pWeaponGap->SetEnabled(false);
            m_pCrosshairGapSlider->SetEnabled(false);
            m_pCrosshairGapEntry->SetEnabled(false);
            m_pOutlineEnable->SetEnabled(false);
            m_pOutlineThicknessSlider->SetEnabled(false);
            m_pOutlineThicknessEntry->SetEnabled(false);
            m_pScaleEnable->SetEnabled(false);
            m_pCrosshairScaleSlider->SetEnabled(false);
            m_pCrosshairScaleEntry->SetEnabled(false);
            m_pCrosshairSizeSlider->SetEnabled(false);
            m_pCrosshairSizeEntry->SetEnabled(false);
            m_pCrosshairDrawT->SetEnabled(false);
            m_pCrosshairThicknessSlider->SetEnabled(false);
            m_pCrosshairThicknessEntry->SetEnabled(false);
        }
    }
    else if (p == m_pWeaponGap)
    {
        bool bEnabled = m_pWeaponGap->IsSelected();
        ConVarRef("cl_crosshair_gap_use_weapon_value").SetValue(bEnabled);
        m_pCrosshairGapSlider->SetEnabled(!bEnabled && m_pCrosshairStyle->GetActiveItem() != 1);
        m_pCrosshairGapEntry->SetEnabled(!bEnabled && m_pCrosshairStyle->GetActiveItem() != 1);
    }
    else if (p == m_pOutlineEnable)
    {
        bool bEnabled = m_pOutlineEnable->IsSelected();
        ConVarRef("cl_crosshair_outline_enable").SetValue(bEnabled);
        m_pOutlineThicknessSlider->SetEnabled(bEnabled && (m_pCrosshairStyle->GetActiveItem() == 1 || m_pCrosshairStyle->GetActiveItem() == 2));
        m_pOutlineThicknessEntry->SetEnabled(bEnabled && (m_pCrosshairStyle->GetActiveItem() == 1 || m_pCrosshairStyle->GetActiveItem() == 2));
    }
    else if (p == m_pScaleEnable)
    {
        bool bEnabled = m_pScaleEnable->IsSelected();
        ConVarRef("cl_crosshair_scale_enable").SetValue(bEnabled);
        m_pCrosshairScaleSlider->SetEnabled(bEnabled && m_pCrosshairStyle->GetActiveItem() == 1);
        m_pCrosshairScaleEntry->SetEnabled(bEnabled && m_pCrosshairStyle->GetActiveItem() == 1);
    }
    else if (p == m_pCrosshairAlphaEnable)
    {
        ConVarRef("cl_crosshair_alpha_enable").SetValue(m_pCrosshairAlphaEnable->IsSelected());
    }
    else if (p == m_pCrosshairDot)
    {
        ConVarRef("cl_crosshair_dot").SetValue(m_pCrosshairDot->IsSelected());
    }
    else if (p == m_pDynamicFire)
    {
        ConVarRef("cl_crosshair_dynamic_fire").SetValue(m_pDynamicFire->IsSelected());
    }
    else if (p == m_pDynamicMove)
    {
        ConVarRef("cl_crosshair_dynamic_move").SetValue(m_pDynamicMove->IsSelected());
    }
    else if (p == m_pCrosshairDrawT)
    {
        ConVarRef("cl_crosshair_t").SetValue(m_pCrosshairDrawT->IsSelected());
    }
}

void CrosshairSettingsPage::UpdateSliderEntries() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", m_pOutlineThicknessSlider->GetSliderValue());
    m_pOutlineThicknessEntry->SetText(buf);

    Q_snprintf(buf, sizeof(buf), "%.1f", m_pCrosshairThicknessSlider->GetSliderValue());
    m_pCrosshairThicknessEntry->SetText(buf);

    Q_snprintf(buf, sizeof(buf), "%.1f", m_pCrosshairScaleSlider->GetSliderValue());
    m_pCrosshairScaleEntry->SetText(buf);

    Q_snprintf(buf, sizeof(buf), "%.1f", m_pCrosshairSizeSlider->GetSliderValue());
    m_pCrosshairSizeEntry->SetText(buf);

    Q_snprintf(buf, sizeof(buf), "%.1f", m_pCrosshairGapSlider->GetSliderValue());
    m_pCrosshairGapEntry->SetText(buf);
}

void CrosshairSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);
    if (p == m_pCrosshairStyle)
    {
        ConVarRef("cl_crosshair_style").SetValue(m_pCrosshairStyle->GetActiveItem());
        UpdateStyleToggles();
    }
    else if (p == m_pCustomFileEntry)
    {
        char buf[64];
        m_pOutlineThicknessEntry->GetText(buf, 64);
        ConVarRef("cl_crosshair_file").SetValue(buf);
    }
    else if (p == m_pOutlineThicknessEntry)
    {
        char buf[64];
        m_pOutlineThicknessEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 0.0)
        {
            m_pOutlineThicknessSlider->SetSliderValue(fValue);
            ConVarRef("cl_crosshair_outline_thickness").SetValue(fValue);
        }
    }
    else if (p == m_pCrosshairThicknessEntry)
    {
        char buf[64];
        m_pCrosshairThicknessEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 0.0)
        {
            m_pCrosshairThicknessSlider->SetSliderValue(fValue);
            ConVarRef("cl_crosshair_thickness").SetValue(fValue);
        }
    }
    else if (p == m_pCrosshairScaleEntry)
    {
        char buf[64];
        m_pCrosshairScaleEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 0.0)
        {
            m_pCrosshairScaleSlider->SetSliderValue(fValue);
            ConVarRef("cl_crosshair_scale").SetValue(fValue);
        }
    }
    else if (p == m_pCrosshairSizeEntry)
    {
        char buf[64];
        m_pCrosshairSizeEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 0.0)
        {
            m_pCrosshairSizeSlider->SetSliderValue(fValue);
            ConVarRef("cl_crosshair_size").SetValue(fValue);
        }
    }
    else if (p == m_pCrosshairGapEntry)
    {
        char buf[64];
        m_pCrosshairGapEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 0.0)
        {
            m_pCrosshairGapSlider->SetSliderValue(fValue);
            ConVarRef("cl_crosshair_gap").SetValue(fValue);
        }
    }
}

void CrosshairSettingsPage::UpdateStyleToggles() const
{
    if (m_pCrosshairShow->IsSelected())
    {
        if (m_pCrosshairStyle->GetActiveItem() == 0) // dots
        {
            m_pCrosshairDot->SetEnabled(true);
            m_pCustomFileEntry->SetEnabled(false);
            m_pWeaponGap->SetEnabled(true);
            m_pCrosshairGapSlider->SetEnabled(!(m_pWeaponGap->IsSelected()));
            m_pCrosshairGapEntry->SetEnabled(!(m_pWeaponGap->IsSelected()));
            m_pOutlineEnable->SetEnabled(false);
            m_pOutlineThicknessSlider->SetEnabled(false);
            m_pOutlineThicknessEntry->SetEnabled(false);
            m_pScaleEnable->SetEnabled(false);
            m_pCrosshairScaleSlider->SetEnabled(false);
            m_pCrosshairScaleEntry->SetEnabled(false);
            m_pCrosshairSizeSlider->SetEnabled(false);
            m_pCrosshairSizeEntry->SetEnabled(false);
            m_pCrosshairDrawT->SetEnabled(true);
            m_pCrosshairThicknessSlider->SetEnabled(false);
            m_pCrosshairThicknessEntry->SetEnabled(false);
        }
        else if (m_pCrosshairStyle->GetActiveItem() == 1) // cs:s
        {
            m_pCrosshairDot->SetEnabled(true);
            m_pCustomFileEntry->SetEnabled(false);
            m_pWeaponGap->SetEnabled(false);
            m_pCrosshairGapSlider->SetEnabled(false);
            m_pCrosshairGapEntry->SetEnabled(false);
            m_pOutlineEnable->SetEnabled(true);
            m_pOutlineThicknessSlider->SetEnabled(m_pOutlineEnable->IsSelected());
            m_pOutlineThicknessEntry->SetEnabled(m_pOutlineEnable->IsSelected());
            m_pScaleEnable->SetEnabled(true);
            m_pCrosshairScaleSlider->SetEnabled(m_pScaleEnable->IsSelected());
            m_pCrosshairScaleEntry->SetEnabled(m_pScaleEnable->IsSelected());
            m_pCrosshairSizeSlider->SetEnabled(false);
            m_pCrosshairSizeEntry->SetEnabled(false);
            m_pCrosshairDrawT->SetEnabled(true);
            m_pCrosshairThicknessSlider->SetEnabled(false);
            m_pCrosshairThicknessEntry->SetEnabled(false);
        }
        else if (m_pCrosshairStyle->GetActiveItem() == 2) // user var
        {
            m_pCrosshairDot->SetEnabled(true);
            m_pCustomFileEntry->SetEnabled(false);
            m_pWeaponGap->SetEnabled(true);
            m_pCrosshairGapSlider->SetEnabled(!(m_pWeaponGap->IsSelected()));
            m_pCrosshairGapEntry->SetEnabled(!(m_pWeaponGap->IsSelected()));
            m_pOutlineEnable->SetEnabled(true);
            m_pOutlineThicknessSlider->SetEnabled(m_pOutlineEnable->IsSelected());
            m_pOutlineThicknessEntry->SetEnabled(m_pOutlineEnable->IsSelected());
            m_pScaleEnable->SetEnabled(false);
            m_pCrosshairScaleSlider->SetEnabled(false);
            m_pCrosshairScaleEntry->SetEnabled(false);
            m_pCrosshairSizeSlider->SetEnabled(true);
            m_pCrosshairSizeEntry->SetEnabled(true);
            m_pCrosshairDrawT->SetEnabled(true);
            m_pCrosshairThicknessSlider->SetEnabled(true);
            m_pCrosshairThicknessEntry->SetEnabled(true);
        }
        else // custom vtf file
        {
            m_pCrosshairDot->SetEnabled(false);
            m_pCustomFileEntry->SetEnabled(true);
            m_pWeaponGap->SetEnabled(true);
            m_pCrosshairGapSlider->SetEnabled(!(m_pWeaponGap->IsSelected()));
            m_pCrosshairGapEntry->SetEnabled(!(m_pWeaponGap->IsSelected()));
            m_pOutlineEnable->SetEnabled(false);
            m_pOutlineThicknessSlider->SetEnabled(false);
            m_pOutlineThicknessEntry->SetEnabled(false);
            m_pScaleEnable->SetEnabled(false);
            m_pCrosshairScaleSlider->SetEnabled(false);
            m_pCrosshairScaleEntry->SetEnabled(false);
            m_pCrosshairSizeSlider->SetEnabled(true);
            m_pCrosshairSizeEntry->SetEnabled(true);
            m_pCrosshairDrawT->SetEnabled(false);
            m_pCrosshairThicknessSlider->SetEnabled(false);
            m_pCrosshairThicknessEntry->SetEnabled(false);
        }
    }
}

void CrosshairSettingsPage::OnControlModified(Panel *p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pOutlineThicknessSlider)
    {
        ConVarRef("cl_crosshair_outline_thickness").SetValue(m_pOutlineThicknessSlider->GetSliderValue());
    }
    else if (p == m_pCrosshairThicknessSlider)
    {
        ConVarRef("cl_crosshair_thickness").SetValue(m_pCrosshairThicknessSlider->GetSliderValue());
    }
    else if (p == m_pCrosshairScaleSlider)
    {
        ConVarRef("cl_crosshair_scale").SetValue(m_pCrosshairScaleSlider->GetSliderValue());
    }
    else if (p == m_pCrosshairSizeSlider)
    {
        ConVarRef("cl_crosshair_size").SetValue(m_pCrosshairSizeSlider->GetSliderValue());
    }
    else if (p == m_pCrosshairGapSlider)
    {
        ConVarRef("cl_crosshair_gap").SetValue(m_pCrosshairGapSlider->GetSliderValue());
    }

    UpdateSliderEntries();
}

void CrosshairSettingsPage::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");

    Panel *pTarget = static_cast<Panel *>(pKv->GetPtr("targetCallback"));

    if (pTarget == m_pCrosshairColorButton)
    {
        SET_BUTTON_COLOR(m_pCrosshairColorButton, selected);
        char buf[32];
        MomUtil::GetHexStringFromColor(selected, buf, 32);
        ConVarRef("cl_crosshair_color").SetValue(buf);
    }
}

void CrosshairSettingsPage::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetButtonColors();
}

void CrosshairSettingsPage::OnScreenSizeChanged(int oldwide, int oldtall)
{
    BaseClass::OnScreenSizeChanged(oldwide, oldtall);

    DestroyBogusCrosshairPanel();
}

void CrosshairSettingsPage::OnCrosshairPreviewResize(int wide, int tall)
{
    int scaledPad = scheme()->GetProportionalScaledValue(15);
    m_pCrosshairPreviewFrame->SetSize(wide + scaledPad, tall + float(scaledPad) * 1.5f);
    m_pCrosshairPreviewPanel->SetPos(m_pCrosshairPreviewFrame->GetXPos() + scaledPad / 2,
                                     m_pCrosshairPreviewFrame->GetYPos() + scaledPad);
}