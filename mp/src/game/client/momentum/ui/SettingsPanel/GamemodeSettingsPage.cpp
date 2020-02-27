#include "cbase.h"

#include "GamemodeSettingsPage.h"

#include "ienginevgui.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/TextEntry.h>

#include "tier0/memdbgon.h"

using namespace vgui;

GamemodeSettingsPage::GamemodeSettingsPage(Panel *pParent) : BaseClass(pParent, "GamemodeSettings")
{
    // Rocket Jump controls
    m_pRJParticlesBox = new ComboBox(this, "ParticlesBox", 3, false);
    m_pRJParticlesBox->AddItem("#MOM_Settings_RJ_None", nullptr);
    m_pRJParticlesBox->AddItem("#MOM_Momentum", nullptr);
    m_pRJParticlesBox->AddItem("#MOM_Settings_RJ_TF2", nullptr);
    m_pRJParticlesBox->AddActionSignalTarget(this);

    m_pRJSoundsBox = new ComboBox(this, "SoundsBox", 3, false);
    m_pRJSoundsBox->AddItem("#MOM_Settings_RJ_None", nullptr);
    m_pRJSoundsBox->AddItem("#MOM_Momentum", nullptr);
    m_pRJSoundsBox->AddItem("#MOM_Settings_RJ_TF2", nullptr);
    m_pRJSoundsBox->AddActionSignalTarget(this);

    m_pRJTrailBox = new ComboBox(this, "TrailBox", 3, false);
    m_pRJTrailBox->AddItem("#MOM_Settings_RJ_None", nullptr);
    m_pRJTrailBox->AddItem("#MOM_Momentum", nullptr);
    m_pRJTrailBox->AddItem("#MOM_Settings_RJ_TF2", nullptr);
    m_pRJTrailBox->AddActionSignalTarget(this);

    m_pRJEnableTFRocketModel = new CvarToggleCheckButton(this, "EnableTFRocketModel", "#MOM_Settings_RJ_Enable_TF_RocketModel", "mom_rj_use_tf_rocketmodel");
    m_pRJEnableTFViewModel = new CvarToggleCheckButton(this, "EnableTFViewModel", "#MOM_Settings_RJ_Enable_TF_ViewModel", "mom_rj_use_tf_viewmodel");
    m_pRJEnableCenterFire = new CvarToggleCheckButton(this, "EnableCenterFire", "#MOM_Settings_RJ_Enable_Center_Fire", "mom_rj_center_fire");
    m_pRJToggleRocketTrailSound = new CvarToggleCheckButton(this, "ToggleTrailSound", "#MOM_Settings_RJ_Trail_Sound_Enable", "mom_rj_trail_sound_enable");
    m_pRJToggleRocketDecals = new CvarToggleCheckButton(this, "ToggleRocketDecals", "#MOM_Settings_RJ_Rocket_Decals_Enable", "mom_rj_decals_enable");

    // Sticky Jump controls
    m_pSJStickyDrawDelayEntry = new CvarTextEntry(this, "DrawDelayEntry", "mom_sj_stickybomb_drawdelay");
    m_pSJStickyDrawDelayEntry->SetAllowNumericInputOnly(true);
    m_pSJStickyDrawDelayEntry->AddActionSignalTarget(this);

    m_pSJEnableCharge =
        new CvarToggleCheckButton(this, "EnableCharge", "#MOM_Settings_SJ_Enable_Charge", "mom_sj_charge_enable");
    m_pSJEnableChargeMeter = new CvarToggleCheckButton(this, "EnableChargeMeter", "#MOM_Settings_SJ_Enable_Charge_Meter",
                                                     "mom_hud_sj_chargemeter_enable");
    m_pSJEnableStickyCounter = new CvarToggleCheckButton(
        this, "EnableStickyCounter", "#MOM_Settings_SJ_Enable_Sticky_Counter", "mom_hud_sj_stickycount_enable");

    LoadControlSettings("resource/ui/SettingsPanel_GamemodeSettings.res");
}

GamemodeSettingsPage::~GamemodeSettingsPage() {}

void GamemodeSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    ConVarRef mom_rj_particles("mom_rj_particles"), mom_rj_sounds("mom_rj_sounds"), mom_rj_trail("mom_rj_trail");

    mom_rj_particles.SetValue(m_pRJParticlesBox->GetActiveItem());
    mom_rj_sounds.SetValue(m_pRJSoundsBox->GetActiveItem());
    mom_rj_trail.SetValue(m_pRJTrailBox->GetActiveItem());

    m_pRJEnableTFRocketModel->ApplyChanges();
    m_pRJEnableTFViewModel->ApplyChanges();
    m_pRJEnableCenterFire->ApplyChanges();
    m_pRJToggleRocketTrailSound->ApplyChanges();
    m_pRJToggleRocketDecals->ApplyChanges();

    m_pSJStickyDrawDelayEntry->ApplyChanges();
    m_pSJEnableCharge->ApplyChanges();
    m_pSJEnableChargeMeter->ApplyChanges();
    m_pSJEnableStickyCounter->ApplyChanges();
}

void GamemodeSettingsPage::LoadSettings()
{
    ConVarRef mom_rj_particles("mom_rj_particles"), mom_rj_sounds("mom_rj_sounds"), mom_rj_trail("mom_rj_trail");

    m_pRJParticlesBox->ActivateItemByRow(mom_rj_particles.GetInt());
    m_pRJSoundsBox->ActivateItemByRow(mom_rj_sounds.GetInt());
    m_pRJTrailBox->ActivateItemByRow(mom_rj_trail.GetInt());
}