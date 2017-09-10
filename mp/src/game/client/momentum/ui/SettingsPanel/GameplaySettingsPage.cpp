#include "cbase.h"

#include "GameplaySettingsPage.h"
#include "util/mom_util.h"

GameplaySettingsPage::GameplaySettingsPage(Panel *pParent)
    : BaseClass(pParent, "GameplaySettings"), m_TrailColor("mom_trail_color")
{
    m_pYawSpeedSlider = FindControl<CCvarSlider>("YawSpeed");
    m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");

    m_pPlayBlockSound = FindControl<CvarToggleCheckButton<ConVarRef>>("PlayBlockSound");
    m_pSaveCheckpoints = FindControl<CvarToggleCheckButton<ConVarRef>>("SaveCheckpoints");
    m_pEnableTrail = FindControl<CvarToggleCheckButton<ConVarRef>>("EnableTrail");

    m_pPickColorButton = FindControl<Button>("PickColorButton");
    m_pColorPicker = new ColorPicker(this, this);
    m_pColorPicker->SetAutoDelete(true);
}

void GameplaySettingsPage::LoadSettings()
{
    UpdateSliderEntries();
    if (m_pPickColorButton)
    {
        Color trailColor;
        if (g_pMomentumUtil->GetColorFromHex(m_TrailColor.GetString(), trailColor))
        {
            m_pPickColorButton->SetDefaultColor(trailColor, trailColor);
            m_pPickColorButton->SetArmedColor(trailColor, trailColor);
            m_pPickColorButton->SetSelectedColor(trailColor, trailColor);
        }
    }
}

void GameplaySettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);

    if (p == m_pYawSpeedEntry)
    {
        char buf[64];
        m_pYawSpeedEntry->GetText(buf, 64);

        float fValue = static_cast<float>(atof(buf));
        if (fValue >= 1.0)
        {
            m_pYawSpeedSlider->SetSliderValue(fValue);
        }
    }
}

void GameplaySettingsPage::OnControlModified(Panel *p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pYawSpeedSlider && m_pYawSpeedSlider->HasBeenModified())
    {
        UpdateSliderEntries();
    }
}

void GameplaySettingsPage::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");

    m_pPickColorButton->SetDefaultColor(selected, selected);
    m_pPickColorButton->SetArmedColor(selected, selected);
    m_pPickColorButton->SetSelectedColor(selected, selected);

    char buf[32];
    g_pMomentumUtil->GetHexStringFromColor(selected, buf, 32);
    m_TrailColor.SetValue(buf);
}

void GameplaySettingsPage::OnCommand(const char *pCommand)
{
    if (FStrEq(pCommand, "picker"))
    {
        Color trailColor;
        if (g_pMomentumUtil->GetColorFromHex(m_TrailColor.GetString(), trailColor))
        {
            m_pColorPicker->SetPickerColor(trailColor);
            m_pColorPicker->Show();
        }
    }

    BaseClass::OnCommand(pCommand);
}

void GameplaySettingsPage::UpdateSliderEntries() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", m_pYawSpeedSlider->GetSliderValue());
    m_pYawSpeedEntry->SetText(buf);
}