#include "cbase.h"

#include "StickyJumpSettingsPage.h"

#include "ienginevgui.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"

#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/TextEntry.h>

#include "tier0/memdbgon.h"

using namespace vgui;

StickyJumpSettingsPage::StickyJumpSettingsPage(Panel *pParent) : BaseClass(pParent, "StickyJumpSettings")
{
    m_pStickyDrawDelayEntry = new TextEntry(this, "DrawDelayEntry");
    m_pStickyDrawDelayEntry->SetAllowNumericInputOnly(true);
    m_pStickyDrawDelayEntry->AddActionSignalTarget(this);

    m_pEnableCharge =
        new CvarToggleCheckButton(this, "EnableCharge", "#MOM_Settings_SJ_Enable_Charge", "mom_sj_firing_mode");
    m_pEnableChargeMeter = new CvarToggleCheckButton(this, "EnableChargeMeter", "#MOM_Settings_SJ_Enable_Charge_Meter",
                                                     "mom_hud_sj_chargemeter_enable");
    m_pEnableStickyCounter = new CvarToggleCheckButton(
        this, "EnableStickyCounter", "#MOM_Settings_SJ_Enable_Sticky_Counter", "mom_hud_sj_stickycount_enable");

    LoadControlSettings("resource/ui/SettingsPanel_StickyJumpSettings.res");
}

StickyJumpSettingsPage::~StickyJumpSettingsPage() {}

void StickyJumpSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    ConVarRef mom_sj_stickybomb_drawdelay("mom_sj_stickybomb_drawdelay");

    mom_sj_stickybomb_drawdelay.SetValue(m_pStickyDrawDelayEntry->GetValueAsFloat());

    m_pEnableCharge->ApplyChanges();
    m_pEnableChargeMeter->ApplyChanges();
    m_pEnableStickyCounter->ApplyChanges();
}

void StickyJumpSettingsPage::LoadSettings()
{
    ConVarRef mom_sj_stickybomb_drawdelay("mom_sj_stickybomb_drawdelay");

    char buf[64];
    Q_snprintf(buf, sizeof(buf), "%.1f", mom_sj_stickybomb_drawdelay.GetFloat());

    m_pStickyDrawDelayEntry->SetText(buf);
}