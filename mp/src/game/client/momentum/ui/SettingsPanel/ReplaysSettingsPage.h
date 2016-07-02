#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "CVarSlider.h"
#include <vgui_controls/Button.h>

using namespace vgui;

class ReplaysSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ReplaysSettingsPage, SettingsPage);

    ReplaysSettingsPage(Panel *pParent) : BaseClass(pParent, "ReplaysSettings")
    {
        m_pReplayModelAlphaSlider = FindControl<CCvarSlider>("ReplayModelAlphaSlider");
        m_pReplayModelAlphaSlider->AddActionSignalTarget(this);

        m_pReplayModelAlphaEntry = FindControl<TextEntry>("ReplayModelAlphaEntry");
        m_pReplayModelAlphaEntry->AddActionSignalTarget(this);

        // There is no cvar for this, not that I can find
        m_pReplayShouldLoop = FindControl<CvarToggleCheckButton<ConVarRef>>("ReplayShouldLoop");
        m_pReplayShouldLoop->AddActionSignalTarget(this);

        m_pReplayInReverse = FindControl<CvarToggleCheckButton<ConVarRef>>("ReplayPlayInReverse");
        m_pReplayInReverse->AddActionSignalTarget(this);
    }

    ~ReplaysSettingsPage() {}

    void LoadSettings() override
    {
        UpdateReplayEntityAlphaEntry();
    }

    void OnTextChanged(Panel *p) override
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

    void OnControlModified(Panel *p) override
    {
        BaseClass::OnControlModified(p);

        if (p == m_pReplayModelAlphaSlider && m_pReplayModelAlphaSlider->HasBeenModified())
        {
            UpdateReplayEntityAlphaEntry();
        }
    }

private:
    void UpdateReplayEntityAlphaEntry() const
    {
        char buf[64];
        Q_snprintf(buf, sizeof(buf), " %.1f", m_pReplayModelAlphaSlider->GetSliderValue());
        m_pReplayModelAlphaEntry->SetText(buf);
    }
    CvarToggleCheckButton<ConVarRef> *m_pReplayShouldLoop, *m_pReplayInReverse;
    CCvarSlider *m_pReplayModelAlphaSlider;
    TextEntry *m_pReplayModelAlphaEntry;
};