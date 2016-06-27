#pragma once

#include "cbase.h"

#include "SettingsPage.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/pch_vgui_controls.h>

using namespace vgui;

class HudSettingsPage : public SettingsPage
{
    DECLARE_CLASS_SIMPLE(HudSettingsPage, SettingsPage);

    HudSettingsPage(Panel *pParent) : BaseClass(pParent, "HudSettings")
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

        m_pSpeedometerShow = FindControl<CvarToggleCheckButton<ConVarRef>>("SpeedoShow");
        m_pSpeedometerShow->AddActionSignalTarget(this);

        m_pSpeedometerShowLastJump = FindControl<CvarToggleCheckButton<ConVarRef>>("SpeedoShowJump");
        m_pSpeedometerShowLastJump->AddActionSignalTarget(this);

        m_pSpeedometerShowVerticalVel = FindControl<CvarToggleCheckButton<ConVarRef>>("ShowSpeedoHvel");
        m_pSpeedometerShowVerticalVel->AddActionSignalTarget(this);

        m_pSpeedometerColorize = FindControl<CvarToggleCheckButton<ConVarRef>>("SpeedoShowColor");
        m_pSpeedometerColorize->AddActionSignalTarget(this);

        m_pSyncShow = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShow");
        m_pSyncShow->AddActionSignalTarget(this);

        m_pSyncShowBar = FindControl<CvarToggleCheckButton<ConVarRef>>("SyncShowBar");
        m_pSyncShowBar->AddActionSignalTarget(this);

        m_pButtonsShow = FindControl<CvarToggleCheckButton<ConVarRef>>("ButtonsShow");
        m_pButtonsShow->AddActionSignalTarget(this);

        LoadSettings();
    }

    ~HudSettingsPage() {}

    void OnApplyChanges() override
    {
        BaseClass::OnApplyChanges();

        ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"),
            sync_color("mom_strafesync_colorize");

        units.SetValue(m_pSpeedometerUnits->GetActiveItem() + 1);
        sync_type.SetValue(m_pSyncType->GetActiveItem() + 1); // Sync type needs +1 added to it before setting convar!
        sync_color.SetValue(m_pSyncColorize->GetActiveItem());
    }

    void LoadSettings() override
    {
        ConVarRef units("mom_speedometer_units"), sync_type("mom_strafesync_type"),
            sync_color("mom_strafesync_colorize");
        m_pSpeedometerUnits->ActivateItemByRow(units.GetInt() - 1);
        m_pSyncType->ActivateItemByRow(sync_type.GetInt() - 1);
        m_pSyncColorize->ActivateItemByRow(sync_color.GetInt());
    }

    void OnCheckboxChecked(Panel *p) override
    {
        BaseClass::OnCheckboxChecked(p);
        CheckButton *button = dynamic_cast<CheckButton *>(p);
        if (button)
        {
            if (!Q_stricmp(button->GetName(), "SpeedoShow"))
            {
                m_pSpeedometerShowLastJump->SetEnabled(button->IsSelected());
                m_pSpeedometerShowVerticalVel->SetEnabled(button->IsSelected());
                m_pSpeedometerUnits->SetEnabled(button->IsSelected());
                m_pSpeedometerColorize->SetEnabled(button->IsSelected());
            }
            else if (!Q_stricmp(button->GetName(), "SyncShow"))
            {
                m_pSyncType->SetEnabled(button->IsSelected());
                m_pSyncShowBar->SetEnabled(button->IsSelected());
                m_pSyncColorize->SetEnabled(button->IsSelected());
            }
        }
    }

  private:
    ComboBox *m_pSpeedometerUnits, *m_pSyncType, *m_pSyncColorize;

    CvarToggleCheckButton<ConVarRef> *m_pSpeedometerShow, *m_pSpeedometerShowLastJump, *m_pSpeedometerShowVerticalVel,
        *m_pSpeedometerColorize, *m_pSyncShow, *m_pSyncShowBar, *m_pButtonsShow, *m_pShowVersion;
};