#include "cbase.h"

#include "HudSettingsPage.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/CvarToggleCheckButton.h"

using namespace vgui;

HudSettingsPage::HudSettingsPage(Panel *pParent) : BaseClass(pParent, "HudSettings")
{
    m_pSpeedometerUnits = FindControl<ComboBox>("SpeedoUnits");
    m_pSpeedometerUnits->SetNumberOfEditLines(3);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_UPS", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_KPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_MPH", nullptr);
    m_pSpeedometerUnits->AddActionSignalTarget(this);

    m_pSyncType = FindControl<ComboBox>("SyncType");
    m_pSyncType->SetNumberOfEditLines(2);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync1", nullptr);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync2", nullptr);
    m_pSyncType->AddActionSignalTarget(this);

    m_pSyncColorize = FindControl<ComboBox>("SyncColorize");
    m_pSyncColorize->SetNumberOfEditLines(3);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_None", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_1", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_2", nullptr);
    m_pSyncColorize->AddActionSignalTarget(this);

    m_pSpeedometerShow = FindControl<CvarToggleCheckButton>("SpeedoShow");
    m_pSpeedometerShow->AddActionSignalTarget(this);

    m_pSpeedometerShowLastJump = FindControl<CvarToggleCheckButton>("SpeedoShowJump");
    m_pSpeedometerShowLastJump->AddActionSignalTarget(this);

    m_pSpeedometerShowVerticalVel = FindControl<CvarToggleCheckButton>("ShowSpeedoHvel");
    m_pSpeedometerShowVerticalVel->AddActionSignalTarget(this);


    m_pSpeedometerColorize = FindControl<ComboBox>("SpeedoShowColor");
    m_pSpeedometerColorize->SetNumberOfEditLines(3);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Color_Type_None", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Color_Type_1", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Color_Type_2", nullptr);
    m_pSpeedometerColorize->AddActionSignalTarget(this);

    m_pSyncShow = FindControl<CvarToggleCheckButton>("SyncShow");
    m_pSyncShow->AddActionSignalTarget(this);

    m_pSyncShowBar = FindControl<CvarToggleCheckButton>("SyncShowBar");
    m_pSyncShowBar->AddActionSignalTarget(this);

    m_pButtonsShow = FindControl<CvarToggleCheckButton>("ButtonsShow");
    m_pButtonsShow->AddActionSignalTarget(this);

    m_pTimerShow = FindControl<CvarToggleCheckButton>("TimerShow");
    m_pTimerShow->AddActionSignalTarget(this);
}

void HudSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"),
        sync_color("mom_strafesync_colorize"), speed_color("mom_speedometer_colorize");

    units.SetValue(m_pSpeedometerUnits->GetActiveItem() + 1);
    sync_type.SetValue(m_pSyncType->GetActiveItem() + 1); // Sync type needs +1 added to it before setting convar!
    sync_color.SetValue(m_pSyncColorize->GetActiveItem());
    speed_color.SetValue(m_pSpeedometerColorize->GetActiveItem());
}

void HudSettingsPage::LoadSettings()
{
    ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"),
        sync_color("mom_strafesync_colorize"), speed_color("mom_speedometer_colorize");
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