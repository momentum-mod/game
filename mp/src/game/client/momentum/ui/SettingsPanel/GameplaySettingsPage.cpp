#include "cbase.h"

#include "GameplaySettingsPage.h"
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/CVarSlider.h>
#include <vgui_controls/CvarToggleCheckButton.h>

using namespace vgui;

GameplaySettingsPage::GameplaySettingsPage(Panel *pParent) : BaseClass(pParent, "GameplaySettings")
{
    m_pYawSpeedSlider = FindControl<CvarSlider>("YawSpeed");
    m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");

    m_pLowerSpeedCVarEntry = FindControl<CvarTextEntry>("LowerSpeedEntry");
    m_pLowerSpeed = FindControl<CvarToggleCheckButton>("LowerWeaponButton");

    m_pPlayBlockSound = FindControl<CvarToggleCheckButton>("PlayBlockSound");
    m_pSaveCheckpoints = FindControl<CvarToggleCheckButton>("SaveCheckpoints");
}

void GameplaySettingsPage::LoadSettings()
{
    if (m_pLowerSpeedCVarEntry && m_pLowerSpeed)
        m_pLowerSpeedCVarEntry->SetEnabled(m_pLowerSpeed->IsSelected());
    UpdateSliderEntries();
    
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

    if (p == m_pYawSpeedSlider)
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

void GameplaySettingsPage::UpdateSliderEntries() const
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", m_pYawSpeedSlider->GetSliderValue());
    m_pYawSpeedEntry->SetText(buf);
}
