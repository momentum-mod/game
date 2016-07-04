#include "cbase.h"

#include "ReplaysSettingsPage.h"

ReplaysSettingsPage::ReplaysSettingsPage(Panel *pParent) : BaseClass(pParent, "ReplaysSettings")
{
    m_pReplayModelAlphaSlider = FindControl<CCvarSlider>("ReplayModelAlphaSlider");
    m_pReplayModelAlphaSlider->AddActionSignalTarget(this);

    m_pReplayModelAlphaEntry = FindControl<TextEntry>("ReplayModelAlphaEntry");
    m_pReplayModelAlphaEntry->AddActionSignalTarget(this);
}

void ReplaysSettingsPage::LoadSettings()
{
    UpdateReplayEntityAlphaEntry();
}

void ReplaysSettingsPage::OnTextChanged(Panel *p)
{
    BaseClass::OnTextChanged(p);

    if (p == m_pReplayModelAlphaEntry)
    {
        char buf[64];
        m_pReplayModelAlphaEntry->GetText(buf, 64);

        float fValue = float(atof(buf));
        if (fValue >= 1.0)
        {
            m_pReplayModelAlphaSlider->SetSliderValue(fValue);
        }
    }
}

void ReplaysSettingsPage::OnControlModified(Panel *p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pReplayModelAlphaSlider && m_pReplayModelAlphaSlider->HasBeenModified())
    {
        UpdateReplayEntityAlphaEntry();
    }
}

void ReplaysSettingsPage::UpdateReplayEntityAlphaEntry() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), " %.1f", m_pReplayModelAlphaSlider->GetSliderValue());
    m_pReplayModelAlphaEntry->SetText(buf);
}