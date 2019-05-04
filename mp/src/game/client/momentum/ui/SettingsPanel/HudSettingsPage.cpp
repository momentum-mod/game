#include "cbase.h"

#include "HudSettingsPage.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/CvarToggleCheckButton.h"

#include "tier0/memdbgon.h"

using namespace vgui;

HudSettingsPage::HudSettingsPage(Panel *pParent) : BaseClass(pParent, "HudSettings")
{
    m_pSpeedometerUnits = new ComboBox(this, "SpeedoUnits", 4, false);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_UPS", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_KPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_MPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_Energy", nullptr);
    m_pSpeedometerUnits->AddActionSignalTarget(this);

    m_pSyncType = new ComboBox(this, "SyncType", 2, false);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync1", nullptr);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync2", nullptr);
    m_pSyncType->AddActionSignalTarget(this);

    m_pSyncColorize = new ComboBox(this, "SyncColorize", 3, false);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_None", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_1", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_2", nullptr);
    m_pSyncColorize->AddActionSignalTarget(this);

    m_pSpeedometerShow = new CvarToggleCheckButton(this, "SpeedoShow", "#MOM_Settings_Speedometer_Show", "mom_hud_speedometer");
    m_pSpeedometerShow->AddActionSignalTarget(this);

    m_pSpeedometerShowLastJump = new CvarToggleCheckButton(this, "SpeedoShowJump", "#MOM_Settings_Speedometer_Show_Jump", "mom_hud_speedometer_showlastjumpvel");
    m_pSpeedometerShowLastJump->AddActionSignalTarget(this);

    m_pSpeedometerShowVerticalVel = new CvarToggleCheckButton(this, "ShowSpeedoHvel", "#MOM_Settings_Speedometer_Show_Hvel", "mom_hud_speedometer_hvel");
    m_pSpeedometerShowVerticalVel->AddActionSignalTarget(this);

    m_pSpeedometerColorize = new ComboBox(this, "SpeedoShowColor", 3, false);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Color_Type_None", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Color_Type_1", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Color_Type_2", nullptr);
    m_pSpeedometerColorize->AddActionSignalTarget(this);

    m_pSyncShow = new CvarToggleCheckButton(this, "SyncShow", "#MOM_Settings_Sync_Show", "mom_hud_strafesync_draw");
    m_pSyncShow->AddActionSignalTarget(this);

    m_pSyncShowBar = new CvarToggleCheckButton(this, "SyncShowBar", "#MOM_Settings_Sync_Show_Bar", "mom_hud_strafesync_drawbar");
    m_pSyncShowBar->AddActionSignalTarget(this);

    m_pButtonsShow = new CvarToggleCheckButton(this, "ButtonsShow", "#MOM_Settings_Buttons_Show", "mom_hud_showkeypresses");
    m_pButtonsShow->AddActionSignalTarget(this);

    m_pTimerShow = new CvarToggleCheckButton(this, "TimerShow", "#MOM_Settings_Timer_Show", "mom_hud_timer");
    m_pTimerShow->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/SettingsPanel_HudSettings.res");
}

void HudSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    ConVarRef units("mom_hud_speedometer_units"), sync_type("mom_hud_strafesync_type"),
        sync_color("mom_hud_strafesync_colorize"), speed_color("mom_hud_speedometer_colorize");

    units.SetValue(m_pSpeedometerUnits->GetActiveItem() + 1);
    sync_type.SetValue(m_pSyncType->GetActiveItem() + 1); // Sync type needs +1 added to it before setting convar!
    sync_color.SetValue(m_pSyncColorize->GetActiveItem());
    speed_color.SetValue(m_pSpeedometerColorize->GetActiveItem());
}

void HudSettingsPage::LoadSettings()
{
    ConVarRef units("mom_hud_speedometer_units"), sync_type("mom_hud_strafesync_type"),
        sync_color("mom_hud_strafesync_colorize"), speed_color("mom_hud_speedometer_colorize");
    m_pSpeedometerUnits->ActivateItemByRow(units.GetInt() - 1);
    m_pSyncType->ActivateItemByRow(sync_type.GetInt() - 1);
    m_pSyncColorize->ActivateItemByRow(sync_color.GetInt());
    m_pSpeedometerColorize->ActivateItemByRow(speed_color.GetInt());
}

void HudSettingsPage::OnCheckboxChecked(Panel *p)
{
    BaseClass::OnCheckboxChecked(p);

    if (p == m_pSpeedometerShow)
    {
        bool bEnabled = m_pSpeedometerShow->IsSelected();
        m_pSpeedometerShowLastJump->SetEnabled(bEnabled);
        m_pSpeedometerShowVerticalVel->SetEnabled(bEnabled);
        m_pSpeedometerUnits->SetEnabled(bEnabled);
        m_pSpeedometerColorize->SetEnabled(bEnabled);
    }
    else if (p == m_pSyncShow)
    {
        bool bEnabled = m_pSyncShow->IsSelected();
        m_pSyncType->SetEnabled(bEnabled);
        m_pSyncShowBar->SetEnabled(bEnabled);
        m_pSyncColorize->SetEnabled(bEnabled);
    }
}