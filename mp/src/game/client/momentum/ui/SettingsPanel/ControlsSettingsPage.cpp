#include "cbase.h"

#include "ControlsSettingsPage.h"

ControlsSettingsPage::ControlsSettingsPage(Panel *pParent) : BaseClass(pParent, "ControlsSettings")
{
    m_pYawSpeedSlider = FindControl<CCvarSlider>("YawSpeed");
    m_pYawSpeedSlider->AddActionSignalTarget(this);

    m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");
    m_pYawSpeedEntry->AddActionSignalTarget(this);

    m_pPlayBlockSound = FindControl<CvarToggleCheckButton<ConVarRef>>("PlayBlockSound");
    m_pPlayBlockSound->AddActionSignalTarget(this);
}

void ControlsSettingsPage::LoadSettings()
{
    UpdateYawspeedEntry();
}

void ControlsSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);

    if (p == m_pYawSpeedEntry)
    {
        char buf[64];
        m_pYawSpeedEntry->GetText(buf, 64);

        float fValue = float(atof(buf));
        if (fValue >= 1.0)
        {
            m_pYawSpeedSlider->SetSliderValue(fValue);
        }
    }
}

void ControlsSettingsPage::OnControlModified(Panel *p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pYawSpeedSlider && m_pYawSpeedSlider->HasBeenModified())
    {
        UpdateYawspeedEntry();
    }
}

void ControlsSettingsPage::UpdateYawspeedEntry() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), " %.1f", m_pYawSpeedSlider->GetSliderValue());
    m_pYawSpeedEntry->SetText(buf);
}