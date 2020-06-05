#include "cbase.h"

#include "HudSettingsPage.h"

#include "hud_speedometer.h"
#include "hud_speedometer_label.h"
#include "hud_speedometer_data.h"

#include "vgui_controls/CvarComboBox.h"
#include "vgui_controls/CvarToggleCheckButton.h"

#include "tier0/memdbgon.h"

using namespace vgui;

HudSettingsPage::HudSettingsPage(Panel *pParent) : BaseClass(pParent, "HudSettings")
{
    m_pSpeedometerGameType = new ComboBox(this, "SpeedoGameType", GAMEMODE_COUNT - 1, false);
    for (auto i = 1; i < GAMEMODE_COUNT; i++)
    {
        m_pSpeedometerGameType->AddItem(g_szGameModes[i], nullptr);
    }
    m_pSpeedometerGameType->AddActionSignalTarget(this);
    m_pSpeedometerGameType->SilentActivateItemByRow(0);

    m_pSpeedometerType = new ComboBox(this, "SpeedoType", SPEEDOMETER_MAX_LABELS, false);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_Absolute", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_Horiz", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_Vert", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_LastJump", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_RampBoardLeave", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_StageEnterExit", nullptr);
    m_pSpeedometerType->AddActionSignalTarget(this);
    m_pSpeedometerType->SilentActivateItemByRow(0);

    m_pSpeedometerShow = new CheckButton(this, "SpeedoShow", "#MOM_Settings_Speedometer_Show");
    m_pSpeedometerShow->AddActionSignalTarget(this);

    m_pSpeedometerUnits = new ComboBox(this, "SpeedoUnits", 4, false);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_UPS", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_KPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_MPH", nullptr);
    m_pSpeedometerUnits->AddItem("#MOM_Settings_Speedometer_Units_Energy", nullptr);
    m_pSpeedometerUnits->AddActionSignalTarget(this);

    m_pSpeedometerColorize = new ComboBox(this, "SpeedoColorize", 4, false);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Colorize_Type_None", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Colorize_Type_1", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Colorize_Type_2", nullptr);
    m_pSpeedometerColorize->AddItem("#MOM_Settings_Speedometer_Colorize_Type_3", nullptr);
    m_pSpeedometerColorize->AddActionSignalTarget(this);

    m_pSyncType = new CvarComboBox(this, "SyncType", "mom_hud_strafesync_type");
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync1", nullptr);
    m_pSyncType->AddItem("#MOM_Settings_Sync_Type_Sync2", nullptr);
    m_pSyncType->AddActionSignalTarget(this);

    m_pSyncColorize = new CvarComboBox(this, "SyncColorize", "mom_hud_strafesync_colorize");
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_None", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_1", nullptr);
    m_pSyncColorize->AddItem("#MOM_Settings_Sync_Color_Type_2", nullptr);
    m_pSyncColorize->AddActionSignalTarget(this);

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

void HudSettingsPage::LoadSettings()
{
    m_pSpeedometerGameType->SilentActivateItemByRow(g_pSpeedometerData->GetCurrentlyLoadedGameMode() - 1);
    LoadSpeedoSetup();
}

void HudSettingsPage::OnCheckboxChecked(Panel* pPanel)
{
    if (pPanel == m_pSpeedometerShow)
    {
        GetSpeedoLabelFromType()->SetVisible(m_pSpeedometerShow->IsSelected());
        g_pSpeedometerData->SaveGamemodeData(m_pSpeedometerGameType->GetCurrentItem() + 1);
    }
}

void HudSettingsPage::OnTextChanged(Panel *pPanel)
{
    if (pPanel == m_pSpeedometerGameType || pPanel == m_pSpeedometerType)
    {
        if (pPanel == m_pSpeedometerGameType) // load if game type changed
        {
            g_pSpeedometerData->LoadGamemodeData(m_pSpeedometerGameType->GetCurrentItem() + 1);
        }
        LoadSpeedoSetup();
    }
    else if (pPanel == m_pSpeedometerUnits)
    {
        GetSpeedoLabelFromType()->SetUnitType(m_pSpeedometerUnits->GetCurrentItem());
        g_pSpeedometerData->SaveGamemodeData(m_pSpeedometerGameType->GetCurrentItem() + 1);
    }
    else if (pPanel == m_pSpeedometerColorize)
    {
        GetSpeedoLabelFromType()->SetColorizeType(m_pSpeedometerColorize->GetCurrentItem());
        g_pSpeedometerData->SaveGamemodeData(m_pSpeedometerGameType->GetCurrentItem() + 1);
    }
}

void HudSettingsPage::LoadSpeedoSetup()
{
    // silently set settings from the currently selected speedo label
    SpeedometerLabel *pSpeedoLabel = GetSpeedoLabelFromType();
    m_pSpeedometerShow->SilentSetSelected(pSpeedoLabel->IsVisible());
    m_pSpeedometerColorize->SilentActivateItemByRow(pSpeedoLabel->GetColorizeType());
    m_pSpeedometerColorize->SetItemEnabled("#MOM_Settings_Speedometer_Colorize_Type_3", pSpeedoLabel->GetSupportsSeparateComparison());
    m_pSpeedometerUnits->SetItemEnabled("#MOM_Settings_Speedometer_Units_Energy", pSpeedoLabel->GetSupportsEnergyUnits());
    m_pSpeedometerUnits->SilentActivateItemByRow(pSpeedoLabel->GetUnitType());
}

SpeedometerLabel* HudSettingsPage::GetSpeedoLabelFromType()
{
    return g_pSpeedometer->GetLabel(m_pSpeedometerType->GetCurrentItem());
}
