#pragma once
#include "SettingsPage.h"

namespace vgui
{
    class TextEntry;
}

class StickyJumpSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(StickyJumpSettingsPage, SettingsPage);

    StickyJumpSettingsPage(Panel *pParent);
    ~StickyJumpSettingsPage();

    void OnApplyChanges() OVERRIDE;
    void LoadSettings() OVERRIDE;

  private:
    vgui::CvarToggleCheckButton *m_pEnableCharge, *m_pEnableChargeMeter, *m_pEnableStickyCounter;

    vgui::TextEntry *m_pStickyDrawDelayEntry;
};