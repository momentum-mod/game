#include "cbase.h"

#include "HudSettingsPage.h"
#include "vgui_controls/CvarComboBox.h"
#include "vgui_controls/CvarToggleCheckButton.h"

#include "tier0/memdbgon.h"

using namespace vgui;

HudSettingsPage::HudSettingsPage(Panel *pParent) : BaseClass(pParent, "HudSettings")
{
    m_pSpeedometerUnits = new CvarComboBox(this, "SpeedoUnits", 4, false, "mom_hud_speedometer_units");
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_UPS", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_KPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_MPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_Energy", nullptr);
    m_pSpeedometerUnits->AddActionSignalTarget(this);

    m_pSyncType = new CvarComboBox(this, "SyncType", 2, false, "mom_hud_strafesync_type");
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync1", nullptr);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync2", nullptr);
    m_pSyncType->AddActionSignalTarget(this);

    m_pSyncColorize = new CvarComboBox(this, "SyncColorize", 3, false, "mom_hud_strafesync_colorize");
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_None", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_1", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_2", nullptr);
    m_pSyncColorize->AddActionSignalTarget(this);

    m_pSpeedometerShow = new CvarToggleCheckButton(this, "SpeedoShow", "#MOM_Settings_Speedometer_Show", "mom_hud_speedometer");
    m_pSpeedometerShow->AddActionSignalTarget(this);

    m_pSpeedometerHorizShow = new CvarToggleCheckButton(this, "SpeedoHorizShow", "#MOM_Settings_Speedometer_Horiz_Show",
                                                        "mom_hud_speedometer_horiz");
    m_pSpeedometerHorizShow->AddActionSignalTarget(this);

    m_pSpeedometerShowLastJump = new CvarToggleCheckButton(this, "SpeedoShowJump", "#MOM_Settings_Speedometer_Show_Jump", "mom_hud_speedometer_showlastjumpvel");
    m_pSpeedometerShowLastJump->AddActionSignalTarget(this);

    m_pSpeedometerShowStageEnter = new CvarToggleCheckButton(this, "SpeedoShowStageEnter", "#MOM_Settings_Speedometer_Show_StageEnter", "mom_hud_speedometer_showlastjumpvel");
    m_pSpeedometerShowStageEnter->AddActionSignalTarget(this);

    m_pSpeedometerUnitLabels = new CvarToggleCheckButton(this, "SpeedoShowUnitLabels", "#MOM_Settings_Speedometer_Unit_Labels", "mom_hud_speedometer_unit_labels");
    m_pSpeedometerUnitLabels->AddActionSignalTarget(this);

    m_pSpeedometerColorize = new CvarComboBox(this, "SpeedoShowColor", 3, false, "mom_hud_speedometer_colorize");
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

    m_pTimerSoundFailEnable = new CvarToggleCheckButton(this, "TimerSoundFailEnable", "#MOM_Settings_Timer_Sound_Fail_Enable", "mom_timer_sound_fail_enable");
    m_pTimerSoundFailEnable->AddActionSignalTarget(this);

    m_pTimerSoundStartEnable = new CvarToggleCheckButton(this, "TimerSoundStartEnable", "#MOM_Settings_Timer_Sound_Start_Enable", "mom_timer_sound_start_enable");
    m_pTimerSoundStartEnable->AddActionSignalTarget(this);

    m_pTimerSoundStopEnable = new CvarToggleCheckButton(this, "TimerSoundStopEnable", "#MOM_Settings_Timer_Sound_Stop_Enable", "mom_timer_sound_stop_enable");
    m_pTimerSoundStopEnable->AddActionSignalTarget(this);

    m_pTimerSoundFinishEnable = new CvarToggleCheckButton(this, "TimerSoundFinishEnable", "#MOM_Settings_Timer_Sound_Finish_Enable", "mom_timer_sound_finish_enable");
    m_pTimerSoundFinishEnable->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/SettingsPanel_HudSettings.res");
}

void HudSettingsPage::OnCheckboxChecked(Panel *p)
{
    BaseClass::OnCheckboxChecked(p);

    if (p == m_pSpeedometerShow || p == m_pSpeedometerHorizShow || p == m_pSpeedometerShowLastJump || p == m_pSpeedometerShowStageEnter)
    {
        bool bEnableUnits = m_pSpeedometerShow->IsSelected() || m_pSpeedometerHorizShow->IsSelected() ||
                        m_pSpeedometerShowLastJump->IsSelected() || m_pSpeedometerShowStageEnter->IsSelected();
        m_pSpeedometerUnits->SetEnabled(bEnableUnits);
        m_pSpeedometerUnitLabels->SetEnabled(bEnableUnits);

        bool bEnableColorize = m_pSpeedometerShow->IsSelected() || m_pSpeedometerHorizShow->IsSelected();
        m_pSpeedometerColorize->SetEnabled(bEnableColorize);
    }
}