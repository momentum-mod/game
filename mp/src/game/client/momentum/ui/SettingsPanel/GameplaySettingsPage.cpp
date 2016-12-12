#include "cbase.h"

#include "GameplaySettingsPage.h"

GameplaySettingsPage::GameplaySettingsPage(Panel *pParent)
    : BaseClass(pParent, "GameplaySettings"), m_TrailR("mom_trail_color_r"), m_TrailG("mom_trail_color_g"),
      m_TrailB("mom_trail_color_b"), m_TrailA("mom_trail_color_a")
{
    m_pYawSpeedSlider = FindControl<CCvarSlider>("YawSpeed");
    m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");

    m_pLowerSpeedCVarEntry = FindControl<CCvarTextEntry>("LowerSpeedEntry");
    m_pLowerSpeed = FindControl<CvarToggleCheckButton<ConVarRef>>("LowerWeaponButton");
    m_pPlayBlockSound = FindControl<CvarToggleCheckButton<ConVarRef>>("PlayBlockSound");
    m_pSaveCheckpoints = FindControl<CvarToggleCheckButton<ConVarRef>>("SaveCheckpoints");
    m_pEnableTrail = FindControl<CvarToggleCheckButton<ConVarRef>>("EnableTrail");

    m_pPickColorButton = FindControl<Button>("PickColorButton");
    m_pColorPicker = new ColorPicker(this, this);
    m_pColorPicker->SetAutoDelete(true);
}

void GameplaySettingsPage::LoadSettings()
{
    if (m_pLowerSpeedCVarEntry && m_pLowerSpeed)
        m_pLowerSpeedCVarEntry->SetEnabled(m_pLowerSpeed->IsSelected());
    UpdateSliderEntries();
    if (m_pPickColorButton)
    {
        Color trailColor = Color(m_TrailR.GetInt(), m_TrailG.GetInt(), m_TrailB.GetInt(), m_TrailA.GetInt());
        m_pPickColorButton->SetDefaultColor(trailColor, trailColor);
        m_pPickColorButton->SetArmedColor(trailColor, trailColor);
        m_pPickColorButton->SetSelectedColor(trailColor, trailColor);
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

void GameplaySettingsPage::OnCheckboxChecked(Panel* p)
{
    BaseClass::OnCheckboxChecked(p);
    if (p == m_pLowerSpeed && m_pLowerSpeedCVarEntry)
    {
        m_pLowerSpeedCVarEntry->SetEnabled(m_pLowerSpeed->IsSelected());
    }
}

void GameplaySettingsPage::OnColorSelected(KeyValues *pKv)
{
    Color selected = pKv->GetColor("color");

    m_pPickColorButton->SetDefaultColor(selected, selected);
    m_pPickColorButton->SetArmedColor(selected, selected);
    m_pPickColorButton->SetSelectedColor(selected, selected);

    m_TrailR.SetValue(selected.r());
    m_TrailG.SetValue(selected.g());
    m_TrailB.SetValue(selected.b());
    m_TrailA.SetValue(selected.a());
}

void GameplaySettingsPage::OnCommand(const char *pCommand)
{
    if (FStrEq(pCommand, "picker"))
    {
        m_pColorPicker->SetPickerColor(Color(m_TrailR.GetInt(), m_TrailG.GetInt(), m_TrailB.GetInt(), m_TrailA.GetInt()));
        m_pColorPicker->Show();
    }

    BaseClass::OnCommand(pCommand);
}

void GameplaySettingsPage::UpdateSliderEntries() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", m_pYawSpeedSlider->GetSliderValue());
    m_pYawSpeedEntry->SetText(buf);
}
