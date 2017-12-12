#include "cbase.h"

#include "OnlineSettingsPage.h"
#include "vgui_controls/CVarSlider.h"

using namespace vgui;

OnlineSettingsPage::OnlineSettingsPage(Panel* pParent) : BaseClass(pParent, "OnlineSettings")
{
    


    m_pEnableColorAlphaOverride = FindControl<CvarToggleCheckButton>("EnableAlphaOverride");
    m_pAlphaOverrideSlider = FindControl<CCvarSlider>("AlphaOverrideSlider");
    m_pAlphaOverrideInput = FindControl<TextEntry>("AlphaOverrideEntry");
}

void OnlineSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    
}

void OnlineSettingsPage::LoadSettings()
{

    UpdateSliderSettings();
}

void OnlineSettingsPage::OnCheckboxChecked(Panel* p)
{
}

void OnlineSettingsPage::OnTextChanged(vgui::Panel* panel)
{
    BaseClass::OnTextChanged(panel);

    if (panel == m_pAlphaOverrideInput)
    {
        char buf[64];
        m_pAlphaOverrideInput->GetText(buf, 64);

        float fValue = static_cast<float>(Q_atof(buf));
        if (fValue >= 0.0f && fValue <= 255.0f)
        {
            m_pAlphaOverrideSlider->SetSliderValue(fValue);
        }
    }
}

void OnlineSettingsPage::OnControlModified(vgui::Panel* p)
{
    BaseClass::OnControlModified(p);

    if (p == m_pAlphaOverrideSlider)
    {
        UpdateSliderSettings();
    }
}

void OnlineSettingsPage::UpdateSliderSettings()
{
    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%i", static_cast<int>(m_pAlphaOverrideSlider->GetSliderValue()));
    m_pAlphaOverrideInput->SetText(buf);
}
