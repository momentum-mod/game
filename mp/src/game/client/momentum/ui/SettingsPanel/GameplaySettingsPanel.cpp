#include "cbase.h"

#include "GameplaySettingsPanel.h"

#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/CvarTextEntry.h"
#include "vgui_controls/CvarToggleCheckButton.h"
#include "vgui_controls/CvarComboBox.h"
#include "controls/ColorPicker.h"

#include "PaintPreviewPanel.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

#define SET_BUTTON_COLOR(button, color)       \
    button->SetDefaultColor(color, color);    \
    button->SetArmedColor(color, color);      \
    button->SetSelectedColor(color, color);

GameplaySettingsPanel::GameplaySettingsPanel(Panel *pParent, Button *pAssociate)
    : BaseClass(pParent, "GameplayPage", pAssociate), m_cvarPaintColor("mom_paint_color")
{
    // General gameplay settings
    m_pEnableDevConsole = new CvarToggleCheckButton(this, "EnableConsole", "#GameUI_DeveloperConsoleCheck", "con_enable");
    m_pHudFastSwitch = new CvarToggleCheckButton(this, "HudFastSwitch", "#GameUI_FastSwitchCheck", "hud_fastswitch");

    m_pYawSpeedSlider = new CvarSlider(this, "YawSpeed", "cl_yawspeed", 0.0f, 360.0f, 1, true);
    m_pYawSpeedEntry = new CvarTextEntry(this, "YawSpeedEntry", "cl_yawspeed", 1);
    m_pYawSpeedEntry->SetAllowNumericInputOnly(true);

    m_pOverlappingKeys = new CvarToggleCheckButton(this, "OverlappingKeys", "#MOM_Settings_Overlapping_Keys", "mom_enable_overlapping_keys");
    m_pReleaseForwardOnJump = new CvarToggleCheckButton(this, "ReleaseForwardOnJump", "#MOM_Settings_Release_Forward_On_Jump", "mom_release_forward_on_jump");

    m_pPlayBlockSound = new CvarToggleCheckButton(this, "PlayBlockSound", "#MOM_Settings_Play_BlockSound", "mom_bhop_playblocksound");
    m_pSaveCheckpoints = new CvarToggleCheckButton(this, "SaveCheckpoints", "#MOM_Settings_Save_Checkpoints", "mom_saveloc_save_between_sessions");

    m_pDrawViewmodel = new CvarToggleCheckButton(this, "DrawViewmodel", "#MOM_Settings_Draw_Viewmodel", "r_drawviewmodel");

    // Paint settings
    m_pPaintDecalScaleSlider = new CvarSlider(this, "PaintScaleSlider", "mom_paint_scale", 2, true);
    m_pPaintDecalScaleSlider->AddActionSignalTarget(this);
    m_pPaintDecalScaleEntry = new CvarTextEntry(this, "PaintScaleEntry", "mom_paint_scale", 2);
    m_pPaintDecalScaleEntry->SetAllowNumericInputOnly(true);

    m_pTogglePaintApplySound = new CvarToggleCheckButton(this, "TogglePaintApplySound", "#MOM_Settings_Paint_Apply_Sound", "mom_paint_apply_sound");
    m_pTogglePaintLimitToWorld = new CvarToggleCheckButton(this, "TogglePaintLimitToWorld", "#MOM_Settings_Paint_Limit_To_World", "mom_paint_limit_to_world");

    m_pPaintColorPicker = new ColorPicker(this, this);
    m_pPickPaintColorButton = new Button(this, "PickPaintColorButton", "", this, "picker_paint");

    m_pPaintPreviewPanel = new PaintPreviewPanel(this);
    m_pPaintPreviewPanel->SetPaintBackgroundEnabled(true);
    m_pPaintPreviewPanel->SetPaintBackgroundType(2);

    // Safeguard controls
    m_pPracticeModeSafeguards = new CvarComboBox(this, "PracticeModeSafeguard", "mom_run_safeguard_practicemode");
    m_pPracticeModeSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_None", nullptr);
    m_pPracticeModeSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_1", nullptr);
    m_pPracticeModeSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_2", nullptr);

    m_pRestartMapSafeguards = new CvarComboBox(this, "RestartMapSafeguard", "mom_run_safeguard_restart");
    m_pRestartMapSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_None", nullptr);
    m_pRestartMapSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_1", nullptr);
    m_pRestartMapSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_2", nullptr);

    m_pSavelocTeleSafeguards = new CvarComboBox(this, "SavelocTeleSafeguard", "mom_run_safeguard_saveloc_tele");
    m_pSavelocTeleSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_None", nullptr);
    m_pSavelocTeleSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_1", nullptr);
    m_pSavelocTeleSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_2", nullptr);

    m_pChatOpenSafeguards = new CvarComboBox(this, "ChatOpenSafeguard", "mom_run_safeguard_chat_open");
    m_pChatOpenSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_None", nullptr);
    m_pChatOpenSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_1", nullptr);
    m_pChatOpenSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_2", nullptr);

    m_pRestartStageSafeguards = new CvarComboBox(this, "RestartStageSafeguard", "mom_run_safeguard_restart_stage");
    m_pRestartStageSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_None", nullptr);
    m_pRestartStageSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_1", nullptr);
    m_pRestartStageSafeguards->AddItem("#MOM_Settings_Run_Safeguard_Modes_2", nullptr);

    // Gamemode specific settings
    // Rocket Jump controls
    m_pRJEnableTrailParticle = new CvarToggleCheckButton(this, "RJEnableTrailParticle", "#MOM_Settings_RJ_Enable_Trail_Particle", "mom_rj_particle_trail_enable");
    m_pRJEnableExplosionParticle = new CvarToggleCheckButton(this, "RJEnableExplosionParticle", "#MOM_Settings_RJ_Enable_Explosion_Particle", "mom_rj_particle_explosion_enable");
    m_pRJEnableShootSound = new CvarToggleCheckButton(this, "ToggleRJShootSound", "#MOM_Settings_RJ_Enable_Shoot_Sound", "mom_rj_sound_shoot_enable");
    m_pRJToggleRocketTrailSound = new CvarToggleCheckButton(this, "ToggleRJTrailSound", "#MOM_Settings_RJ_Enable_Trail_Sound", "mom_rj_sound_trail_enable");
    m_pRJToggleRocketExplosionSound = new CvarToggleCheckButton(this, "ToggleRJExplosionSound", "#MOM_Settings_RJ_Enable_Explosion_Sound", "mom_rj_sound_explosion_enable");
    m_pRJToggleRocketDecals = new CvarToggleCheckButton(this, "ToggleRocketDecals", "#MOM_Settings_RJ_Enable_Rocket_Decals", "mom_rj_decals_enable");
    m_pRJEnableCenterFire = new CvarToggleCheckButton(this, "EnableCenterFire", "#MOM_Settings_RJ_Enable_Center_Fire", "mom_rj_center_fire");
    m_pRJRocketDrawDelayEntry = new CvarTextEntry(this, "RocketDrawDelayEntry", "mom_rj_rocket_drawdelay", 1);
    m_pRJRocketDrawDelayEntry->SetAllowNumericInputOnly(true);

    // Sticky Jump controls
    m_pSJEnableTrailParticle = new CvarToggleCheckButton(this, "SJEnableTrailParticle", "#MOM_Settings_SJ_Enable_Trail_Particle", "mom_sj_particle_trail_enable");
    m_pSJEnableExplosionParticle = new CvarToggleCheckButton(this, "SJEnableExplosionParticle", "#MOM_Settings_SJ_Enable_Explosion_Particle", "mom_sj_particle_explosion_enable");
    m_pSJToggleStickybombDecals = new CvarToggleCheckButton(this, "SJToggleStickybombDecals", "#MOM_Settings_SJ_Enable_Stickybomb_Decals", "mom_sj_decals_enable");

    m_pSJEnableExplosionSound = new CvarToggleCheckButton(this, "SJEnableExplosionSound", "#MOM_Settings_SJ_Enable_Explosion_Sound", "mom_sj_sound_explosion_enable");
    m_pSJEnableDetonateFailSound = new CvarToggleCheckButton(this, "SJEnableDetonateFailSound", "#MOM_Settings_SJ_Enable_Detonation_Fail_Sound", "mom_sj_sound_detonate_fail_enable");
    m_pSJEnableDetonateSuccessSound = new CvarToggleCheckButton(this, "SJEnableDetonateSuccessSound", "#MOM_Settings_SJ_Enable_Detonation_Success_Sound", "mom_sj_sound_detonate_success_enable");
    m_pSJEnableChargeSound = new CvarToggleCheckButton(this, "SJEnableChargeSound", "#MOM_Settings_SJ_Enable_Charge_Sound", "mom_sj_sound_charge_enable");
    m_pSJEnableShootSound = new CvarToggleCheckButton(this, "SJEnableShootSound", "#MOM_Settings_SJ_Enable_Shoot_Sound", "mom_sj_sound_shot_enable");
    m_pSJChargedShotSoundThreshold = new CvarTextEntry(this, "SJChargedShotSoundThreshold", "mom_sj_sound_shot_charged_threshold", 2);
    m_pSJChargedShotSoundThreshold->SetAllowNumericInputOnly(true);

    m_pSJEnableChargeMeter = new CvarToggleCheckButton(this, "EnableChargeMeter", "#MOM_Settings_SJ_Enable_Charge_Meter", "mom_hud_sj_chargemeter_enable");
    m_pSJEnableChargeMeter->AddActionSignalTarget(this);
    m_pSJEnableStickyCounter = new CvarToggleCheckButton(this, "EnableStickyCounter", "#MOM_Settings_SJ_Enable_Sticky_Counter", "mom_hud_sj_stickycount_enable");
    m_pSJEnableStickyCounter->AddActionSignalTarget(this);

    m_pSJChargeMeterUnits = new CvarComboBox(this, "ChargeMeterUnits", "mom_hud_sj_chargemeter_units");
    m_pSJChargeMeterUnits->AddItem("#MOM_Settings_SJ_ChargeMeter_Units_Type_None", nullptr);
    m_pSJChargeMeterUnits->AddItem("#MOM_Settings_SJ_ChargeMeter_Units_Type_1", nullptr);
    m_pSJChargeMeterUnits->AddItem("#MOM_Settings_SJ_ChargeMeter_Units_Type_2", nullptr);

    m_pSJStickyDrawDelayEntry = new CvarTextEntry(this, "StickyDrawDelayEntry", "mom_sj_stickybomb_drawdelay", 1);
    m_pSJStickyDrawDelayEntry->SetAllowNumericInputOnly(true);

    LoadControlSettings("resource/ui/settings/Settings_Gameplay.res");
}


void GameplaySettingsPanel::OnPageShow()
{
    BaseClass::OnPageShow();

    m_pSJChargeMeterUnits->SetEnabled(m_pSJEnableChargeMeter->IsSelected());

    Color paintColor;
    if (MomUtil::GetColorFromHex(m_cvarPaintColor.GetString(), paintColor))
    {
        SET_BUTTON_COLOR(m_pPickPaintColorButton, paintColor);
    }
}

void GameplaySettingsPanel::OnCheckboxChecked(Panel *panel)
{
    if (panel == m_pSJEnableChargeMeter)
    {
        m_pSJChargeMeterUnits->SetEnabled(m_pSJEnableChargeMeter->IsSelected());
    }
}

void GameplaySettingsPanel::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    Color paintColor;
    if (MomUtil::GetColorFromHex(m_cvarPaintColor.GetString(), paintColor))
    {
        SET_BUTTON_COLOR(m_pPickPaintColorButton, paintColor);
    }
}

void GameplaySettingsPanel::OnCommand(const char *command)
{
    if (FStrEq(command, "picker_paint"))
    {
        Color paintColor;
        if (MomUtil::GetColorFromHex(m_cvarPaintColor.GetString(), paintColor))
        {
            m_pPaintColorPicker->SetPickerColor(paintColor);
            m_pPaintColorPicker->SetTargetCallback(m_pPickPaintColorButton);
            m_pPaintColorPicker->DoModal();
        }
    }
    else
    {
        BaseClass::OnCommand(command);
    }
}

void GameplaySettingsPanel::OnColorSelected(KeyValues *pKv)
{
    const auto selected = pKv->GetColor("color");

    Panel *pTarget = static_cast<Panel *>(pKv->GetPtr("targetCallback"));

    if (pTarget == m_pPickPaintColorButton)
    {
        SET_BUTTON_COLOR(m_pPickPaintColorButton, selected);

        char buf[32];
        MomUtil::GetHexStringFromColor(selected, buf, 32);
        m_cvarPaintColor.SetValue(buf);
    }
}
