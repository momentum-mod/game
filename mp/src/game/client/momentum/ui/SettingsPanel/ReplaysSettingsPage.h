#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include "CVarSlider.h"
#include <vgui_controls/Button.h>

using namespace vgui;

class ReplaysSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(ReplaysSettingsPage, SettingsPage);

    ReplaysSettingsPage(Panel *pParent);

    ~ReplaysSettingsPage() {}

    void LoadSettings() override;

    void OnTextChanged(Panel *p) override;

    void OnControlModified(Panel *p) override;

private:
    void UpdateReplayEntityAlphaEntry() const;

    CCvarSlider *m_pReplayModelAlphaSlider;
    TextEntry *m_pReplayModelAlphaEntry;
};