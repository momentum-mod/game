#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "CVarSlider.h"
#include <vgui_controls/Button.h>

using namespace vgui;

class ControlsSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ControlsSettingsPage, SettingsPage);

    ControlsSettingsPage(Panel *pParent) : BaseClass(pParent, "ControlsSettings")
    {
        m_pYawSpeedSlider = FindControl<CCvarSlider>("YawSpeed");
        m_pYawSpeedSlider->AddActionSignalTarget(this);

        m_pYawSpeedEntry = FindControl<TextEntry>("YawSpeedEntry");
        m_pYawSpeedEntry->AddActionSignalTarget(this);
    }

    ~ControlsSettingsPage() {}

    void OnApplyChanges() override
    {
        //This handles applying the change to the CCvarSliders and CConvarToggleCheckBox
        BaseClass::OnApplyChanges();

    }

    void LoadSettings() override
    {
        UpdateYawspeedEntry();
    }

    void OnTextChanged(Panel *p) override
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

    void OnControlModified(Panel *p) override
    {
        BaseClass::OnControlModified(p);

        if (p == m_pYawSpeedSlider && m_pYawSpeedSlider->HasBeenModified())
        {
            UpdateYawspeedEntry();
        }
    }

private:

    void UpdateYawspeedEntry() const
    {
        char buf[64];
        Q_snprintf(buf, sizeof(buf), " %.1f", m_pYawSpeedSlider->GetSliderValue());
        m_pYawSpeedEntry->SetText(buf);
    }

    CCvarSlider *m_pYawSpeedSlider;
    TextEntry *m_pYawSpeedEntry;
};