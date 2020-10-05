#include "cbase.h"

#include "HUDSettingsPanel.h"

#include "clientmode.h"
#include "hud_comparisons.h"
#include "hud_speedometer.h"
#include "hud_speedometer_data.h"
#include "hud_speedometer_label.h"

#include "vgui_controls/AnimationController.h"
#include "vgui_controls/CvarComboBox.h"
#include "vgui_controls/CvarTextEntry.h"
#include "vgui_controls/CvarToggleCheckButton.h"
#include "vgui_controls/Tooltip.h"

#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

using namespace vgui;

HUDSettingsPanel::HUDSettingsPanel(Panel *pParent, Button *pAssociate) : BaseClass(pParent, "HUDPage", pAssociate)
{
    g_pSpeedometerData->Init();

    m_pSpeedometerGameType = new ComboBox(this, "SpeedoGameType", GAMEMODE_COUNT, false);
    for (auto i = 0; i < GAMEMODE_COUNT; i++)
    {
        m_pSpeedometerGameType->AddItem(g_szGameModes[i], nullptr);
    }
    m_pSpeedometerGameType->AddActionSignalTarget(this);

    m_pSpeedometerType = new ComboBox(this, "SpeedoType", SPEEDOMETER_LABEL_TYPE_COUNT, false);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_Absolute", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_Horiz", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_Vert", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_ExplosiveJump", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_LastJump", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_RampBoard", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_RampLeave", nullptr);
    m_pSpeedometerType->AddItem("#MOM_Settings_Speedometer_Type_StageEnterExit", nullptr);
    m_pSpeedometerType->AddActionSignalTarget(this);
    m_pSpeedometerType->SilentActivateItemByRow(SPEEDOMETER_LABEL_TYPE_ABS);

    m_pSpeedometerShow = new CheckButton(this, "SpeedoShow", "#MOM_Settings_Speedometer_Show");
    m_pSpeedometerShow->AddActionSignalTarget(this);

    m_pSpeedometerUnits = new ComboBox(this, "SpeedoUnits", 4, false);
    for (int i = SPEEDOMETER_UNITS_FIRST; i < SPEEDOMETER_UNITS_COUNT; i++)
    {
        m_pSpeedometerUnits->AddItem(g_szSpeedometerUnits[i], nullptr);
    }
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

    m_pHUDSyncShow = new CvarToggleCheckButton(this, "HudSyncShow", "#MOM_Settings_Sync_Show", "mom_hud_strafesync_draw");
    m_pHUDSyncShow->AddActionSignalTarget(this);

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
    // ==== Mapinfo panel
    // - Button appears in the top left of the program, needs formatting fix
    m_pShowMapName =
        new CvarToggleCheckButton(this, "ShowMapName", "#MOM_Settings_Show_MapName", "mom_hud_mapinfo_show_mapname");
    m_pShowMapName->AddActionSignalTarget(this);

    m_pShowMapAuthor =
        new CvarToggleCheckButton(this, "ShowMapAuthor", "#MOM_Settings_Show_MapAuthor", "mom_hud_mapinfo_show_author");
    m_pShowMapAuthor->AddActionSignalTarget(this);

    m_pShowMapDifficulty =
        new CvarToggleCheckButton(this, "ShowMapDifficulty", "#MOM_Settings_Show_MapDifficulty", "mom_hud_mapinfo_show_difficulty");
    m_pShowMapDifficulty->AddActionSignalTarget(this);

    m_pShowMapStatus =
        new CvarToggleCheckButton(this, "ShowMapStatus", "#MOM_Settings_Show_MapStatus", "mom_hud_mapinfo_show_status");
    m_pShowMapStatus->AddActionSignalTarget(this);

    /*
    * Some confusion on how to turn these elements off; they are in the res file but can only be turned off via these cvars:
    * mom_hud_mapinfo_show_mapname
    * mom_hud_mapinfo_show_author
    * mom_hud_mapinfo_show_difficulty
    * mom_hud_mapinfo_show_status
    */


    // ==== Comparisons panel
    m_pCompareShow = new CvarToggleCheckButton(this, "CompareShow", "#MOM_Settings_Compare_Show", "mom_comparisons");
    m_pCompareShow->AddActionSignalTarget(this);
    m_pMaxZones = new CvarTextEntry(this, "Zones", "mom_comparisons_max_zones", 0);
    m_pMaxZones->SetAllowNumericInputOnly(true);
    m_pMaxZones->AddActionSignalTarget(this);
    m_pMaxZonesLabel = new Label(this, "ZonesLabel", "#MOM_Settings_Zones_Label");

    m_pCompareFormat = new CvarToggleCheckButton(this, "CompareFormat", "#MOM_Settings_Compare_Format", "mom_comparisons_format_output");
    m_pCompareFormat->AddActionSignalTarget(this);

    m_pTimeTypeLabel = new Label(this, "TimeTypeLabel", "#MOM_Settings_Compare_Time_Type_Label");
    m_pTimeTypeLabel->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeType = new CvarComboBox(this, "TimeType", "mom_comparisons_time_type");
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_Overall", nullptr);
    m_pTimeType->AddItem("#MOM_Settings_Compare_Time_Type_PerZone", nullptr);
    m_pTimeType->AddActionSignalTarget(this);

    m_pTimeShowOverall = new CvarToggleCheckButton(this, "TimeShowOverall", "#MOM_Settings_Compare_Show_Overall", "mom_comparisons_time_show_overall");
    m_pTimeShowOverall->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowOverall->AddActionSignalTarget(this);
    m_pTimeShowZone = new CvarToggleCheckButton(this, "TimeShowZone", "#MOM_Settings_Compare_Show_Zone", "mom_comparisons_time_show_perzone");
    m_pTimeShowZone->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pTimeShowZone->AddActionSignalTarget(this);

    m_pVelocityShow = new CvarToggleCheckButton(this, "VelShow", "#MOM_Settings_Compare_Show_Velocity", "mom_comparisons_vel_show");
    m_pVelocityShow->AddActionSignalTarget(this);
    m_pVelocityShowAvg = new CvarToggleCheckButton(this, "VelShowAvg", "#MOM_Settings_Compare_Show_Velocity_Avg", "mom_comparisons_vel_show_avg");
    m_pVelocityShowAvg->AddActionSignalTarget(this);
    m_pVelocityShowMax = new CvarToggleCheckButton(this, "VelShowMax", "#MOM_Settings_Compare_Show_Velocity_Max", "mom_comparisons_vel_show_max");
    m_pVelocityShowMax->AddActionSignalTarget(this);
    m_pVelocityShowEnter = new CvarToggleCheckButton(this, "VelShowEnter", "#MOM_Settings_Compare_Show_Velocity_Enter", "mom_comparisons_vel_show_enter");
    m_pVelocityShowEnter->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowEnter->AddActionSignalTarget(this);
    m_pVelocityShowExit = new CvarToggleCheckButton(this, "VelShowExit", "#MOM_Settings_Compare_Show_Velocity_Exit", "mom_comparisons_vel_show_exit");
    m_pVelocityShowExit->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pVelocityShowExit->AddActionSignalTarget(this);

    m_pCompareSyncShow = new CvarToggleCheckButton(this, "CompareSyncShow", "#MOM_Settings_Compare_Show_Sync", "mom_comparisons_sync_show");
    m_pCompareSyncShow->AddActionSignalTarget(this);
    m_pSyncShowS1 = new CvarToggleCheckButton(this, "SyncShowS1", "#MOM_Settings_Compare_Show_Sync1", "mom_comparisons_sync_show_sync1");
    m_pSyncShowS1->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShowS1->AddActionSignalTarget(this);
    m_pSyncShowS2 = new CvarToggleCheckButton(this, "SyncShowS2", "#MOM_Settings_Compare_Show_Sync2", "mom_comparisons_sync_show_sync2");
    m_pSyncShowS2->GetTooltip()->SetTooltipFormatToSingleLine();
    m_pSyncShowS2->AddActionSignalTarget(this);

    m_pJumpShow = new CvarToggleCheckButton(this, "ShowJumps", "#MOM_Settings_Compare_Show_Jumps", "mom_comparisons_jumps_show");
    m_pJumpShow->AddActionSignalTarget(this);
    m_pStrafeShow = new CvarToggleCheckButton(this, "ShowStrafes", "#MOM_Settings_Compare_Show_Strafes", "mom_comparisons_strafe_show");
    m_pStrafeShow->AddActionSignalTarget(this);

    m_pBogusComparisonsPanel = new C_RunComparisons("BogusComparisonsPanel", this);
    m_pBogusComparisonsPanel->SetPaintBackgroundEnabled(true);
    m_pBogusComparisonsPanel->SetPaintBackgroundType(2);
    m_pBogusComparisonsPanel->Reset();
    m_pBogusComparisonsPanel->LoadBogusComparisons();

    LoadControlSettings("resource/ui/settings/Settings_HUD.res");
}

void HUDSettingsPanel::OnPageShow()
{
    BaseClass::OnPageShow();
    
    m_pSpeedometerGameType->ActivateItemByRow(g_pGameModeSystem->GetGameMode()->GetType());
}

void HUDSettingsPanel::OnMainDialogClosed()
{
    g_pSpeedometerData->Apply();
}

void HUDSettingsPanel::OnCheckboxChecked(Panel *panel)
{
    if (panel == m_pSpeedometerShow)
    {
        GameMode_t gamemode = GameMode_t(m_pSpeedometerGameType->GetCurrentItem());
        SpeedometerLabel_t speedoLabel = SpeedometerLabel_t(m_pSpeedometerType->GetCurrentItem());
        g_pSpeedometerData->SetVisible(gamemode, speedoLabel, m_pSpeedometerShow->IsSelected());
        g_pSpeedometerData->Save();
    }
    // Comparisons
    else if (panel == m_pCompareShow)
    {
        //Turn everything on/off
        bool bEnabled = m_pCompareShow->IsSelected();

        m_pMaxZones->SetEnabled(bEnabled);
        m_pMaxZonesLabel->SetEnabled(bEnabled);
        m_pCompareFormat->SetEnabled(bEnabled);

        //Time
        m_pTimeType->SetEnabled(bEnabled);
        m_pTimeTypeLabel->SetEnabled(bEnabled);
        m_pTimeShowOverall->SetEnabled(bEnabled);
        m_pTimeShowZone->SetEnabled(bEnabled);

        //Velocity
        m_pVelocityShow->SetEnabled(bEnabled);
        m_pVelocityShowExit->SetEnabled(bEnabled);
        m_pVelocityShowAvg->SetEnabled(bEnabled);
        m_pVelocityShowEnter->SetEnabled(bEnabled);
        m_pVelocityShowMax->SetEnabled(bEnabled);

        //Sync
        m_pCompareSyncShow->SetEnabled(bEnabled);
        m_pSyncShowS1->SetEnabled(bEnabled);
        m_pSyncShowS2->SetEnabled(bEnabled);

        //Keypress
        m_pJumpShow->SetEnabled(bEnabled);
        m_pStrafeShow->SetEnabled(bEnabled);

        m_pBogusComparisonsPanel->SetPaintEnabled(bEnabled);
    }
    else if (panel == m_pVelocityShow)
    {
        bool bEnabled = m_pVelocityShow->IsSelected();

        m_pVelocityShowExit->SetEnabled(bEnabled);
        m_pVelocityShowExit->SetSelected(bEnabled);
        m_pVelocityShowAvg->SetEnabled(bEnabled);
        m_pVelocityShowAvg->SetSelected(bEnabled);
        m_pVelocityShowEnter->SetEnabled(bEnabled);
        m_pVelocityShowEnter->SetSelected(bEnabled);
        m_pVelocityShowMax->SetEnabled(bEnabled);
        m_pVelocityShowMax->SetSelected(bEnabled);
    }

    else if (panel == m_pCompareSyncShow)
    {
        bool bEnabled = m_pCompareSyncShow->IsSelected();

        m_pSyncShowS1->SetEnabled(bEnabled);
        m_pSyncShowS1->SetSelected(bEnabled);
        m_pSyncShowS2->SetEnabled(bEnabled);
        m_pSyncShowS2->SetSelected(bEnabled);
    }
}

void HUDSettingsPanel::OnTextChanged(Panel *panel, const char *text)
{
    if (panel != m_pSpeedometerGameType && panel != m_pSpeedometerType && panel != m_pSpeedometerUnits && panel != m_pSpeedometerColorize)
        return;

    GameMode_t gamemode = GameMode_t(m_pSpeedometerGameType->GetCurrentItem());
    SpeedometerLabel_t speedoLabel = SpeedometerLabel_t(m_pSpeedometerType->GetCurrentItem());
    if (panel == m_pSpeedometerGameType || panel == m_pSpeedometerType)
    {
        g_pSpeedometerData->Load();
        m_pSpeedometerShow->SilentSetSelected(g_pSpeedometerData->GetVisible(gamemode, speedoLabel));
        m_pSpeedometerColorize->SilentActivateItemByRow(g_pSpeedometerData->GetColorize(gamemode, speedoLabel));
        m_pSpeedometerUnits->SilentActivateItemByRow(g_pSpeedometerData->GetUnits(gamemode, speedoLabel));
    }
    else if (panel == m_pSpeedometerUnits)
    {
        SpeedometerUnits_t speedoUnits = SpeedometerUnits_t(m_pSpeedometerUnits->GetCurrentItem());
        g_pSpeedometerData->SetUnits(gamemode, speedoLabel, speedoUnits);
        g_pSpeedometerData->Save();
    }
    else if (panel == m_pSpeedometerColorize)
    {
        SpeedometerColorize_t speedoColorize = SpeedometerColorize_t(m_pSpeedometerColorize->GetCurrentItem());
        g_pSpeedometerData->SetColorize(gamemode, speedoLabel, speedoColorize);
        g_pSpeedometerData->Save();
    }
}

void HUDSettingsPanel::CursorEnteredCallback(Panel *panel)
{
    int bogusPulse = DetermineBogusPulse(panel);

    m_pBogusComparisonsPanel->SetBogusPulse(bogusPulse);
    if (bogusPulse > 0)
    {
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(this, "PulseComparePanel");
    }
}

void HUDSettingsPanel::CursorExitedCallback(Panel *panel)
{
    if (DetermineBogusPulse(panel) > 0)
    {
        m_pBogusComparisonsPanel->ClearBogusPulse();
        
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(this, "StopPulseComparePanel");
    }
}

int HUDSettingsPanel::DetermineBogusPulse(Panel *panel) const
{
    int bogusPulse = 0;
    if (panel == m_pJumpShow)
    {
        bogusPulse |= ZONE_JUMPS;
    }
    else if (panel == m_pStrafeShow)
    {
        bogusPulse |= ZONE_STRAFES;
    }
    else if (panel == m_pCompareSyncShow)
    {
        bogusPulse |= ZONE_SYNC1 | ZONE_SYNC2;
    }
    else if (panel == m_pSyncShowS1)
    {
        bogusPulse |= ZONE_SYNC1;
    }
    else if (panel == m_pSyncShowS2)
    {
        bogusPulse |= ZONE_SYNC2;
    }
    else if (panel == m_pVelocityShow)
    {
        bogusPulse |= VELOCITY_AVERAGE | VELOCITY_MAX | VELOCITY_ENTER | VELOCITY_EXIT;
    }
    else if (panel == m_pVelocityShowAvg)
    {
        bogusPulse |= VELOCITY_AVERAGE;
    }
    else if (panel == m_pVelocityShowMax)
    {
        bogusPulse |= VELOCITY_MAX;
    }
    else if (panel == m_pVelocityShowEnter)
    {
        bogusPulse |= VELOCITY_ENTER;
    }
    else if (panel == m_pVelocityShowExit)
    {
        bogusPulse |= VELOCITY_EXIT;
    }
    else if (panel == m_pTimeShowOverall)
    {
        bogusPulse |= TIME_OVERALL;
    }
    else if (panel == m_pTimeShowZone)
    {
        bogusPulse |= ZONE_TIME;
    }
    else if (panel == m_pMaxZones || panel == m_pMaxZonesLabel)
    {
        bogusPulse |= ZONE_LABELS;
    }
    else if (panel == m_pTimeType || panel == m_pTimeTypeLabel)
    {
        bogusPulse |= ZONE_LABELS_COMP;
    }
    else if (panel == m_pCompareFormat)
    {
        bogusPulse |= ZONE_TIME | TIME_OVERALL | VELOCITY_AVERAGE | VELOCITY_MAX | VELOCITY_ENTER |
                      VELOCITY_EXIT | ZONE_SYNC1 | ZONE_SYNC2 | ZONE_JUMPS | ZONE_STRAFES;
    }

    return bogusPulse;
}