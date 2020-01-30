#include "cbase.h"

#include "RocketJumpSettingsPage.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CvarToggleCheckButton.h>

#include "tier0/memdbgon.h"

using namespace vgui;

RocketJumpSettingsPage::RocketJumpSettingsPage(Panel *pParent) : BaseClass(pParent, "RocketJumpSettings")
{
    m_pParticlesBox = new ComboBox(this, "ParticlesBox", 3, false);
    m_pParticlesBox->AddItem("#MOM_Settings_RJ_None", nullptr);
    m_pParticlesBox->AddItem("#MOM_Momentum", nullptr);
    m_pParticlesBox->AddItem("#MOM_Settings_RJ_TF2", nullptr);
    m_pParticlesBox->AddActionSignalTarget(this);

    m_pSoundsBox = new ComboBox(this, "SoundsBox", 3, false);
    m_pSoundsBox->AddItem("#MOM_Settings_RJ_None", nullptr);
    m_pSoundsBox->AddItem("#MOM_Momentum", nullptr);
    m_pSoundsBox->AddItem("#MOM_Settings_RJ_TF2", nullptr);
    m_pSoundsBox->AddActionSignalTarget(this);

    m_pTrailBox = new ComboBox(this, "TrailBox", 3, false);
    m_pTrailBox->AddItem("#MOM_Settings_RJ_None", nullptr);
    m_pTrailBox->AddItem("#MOM_Momentum", nullptr);
    m_pTrailBox->AddItem("#MOM_Settings_RJ_TF2", nullptr);
    m_pTrailBox->AddActionSignalTarget(this);

    m_pEnableTFRocketModel = new CvarToggleCheckButton(this, "EnableTFRocketModel", "#MOM_Settings_RJ_Enable_TF_RocketModel", "mom_rj_use_tf_rocketmodel");
    m_pEnableTFViewModel = new CvarToggleCheckButton(this, "EnableTFViewModel", "#MOM_Settings_RJ_Enable_TF_ViewModel","mom_rj_use_tf_viewmodel");
    m_pEnableCenterFire = new CvarToggleCheckButton(this, "EnableCenterFire", "#MOM_Settings_RJ_Enable_Center_Fire", "mom_rj_center_fire");
    m_pToggleRocketTrailSound = new CvarToggleCheckButton(this, "ToggleTrailSound", "#MOM_Settings_RJ_Trail_Sound_Enable", "mom_rj_trail_sound_enable");
    m_pToggleRocketDecals = new CvarToggleCheckButton(this, "ToggleRocketDecals", "#MOM_Settings_RJ_Rocket_Decals_Enable", "mom_rj_decals_enable");

    LoadControlSettings("resource/ui/SettingsPanel_RocketJumpSettings.res");
}

RocketJumpSettingsPage::~RocketJumpSettingsPage() {}

void RocketJumpSettingsPage::OnApplyChanges()
{
    BaseClass::OnApplyChanges();

    ConVarRef mom_rj_particles("mom_rj_particles"), mom_rj_sounds("mom_rj_sounds"), mom_rj_trail("mom_rj_trail");

    mom_rj_particles.SetValue(m_pParticlesBox->GetActiveItem());
    mom_rj_sounds.SetValue(m_pSoundsBox->GetActiveItem());
    mom_rj_trail.SetValue(m_pTrailBox->GetActiveItem());

    m_pEnableTFRocketModel->ApplyChanges();
    m_pEnableTFViewModel->ApplyChanges();
    m_pEnableCenterFire->ApplyChanges();
    m_pToggleRocketTrailSound->ApplyChanges();
    m_pToggleRocketDecals->ApplyChanges();
}

void RocketJumpSettingsPage::LoadSettings()
{
    ConVarRef mom_rj_particles("mom_rj_particles"), mom_rj_sounds("mom_rj_sounds"), mom_rj_trail("mom_rj_trail");

    m_pParticlesBox->ActivateItemByRow(mom_rj_particles.GetInt());
    m_pSoundsBox->ActivateItemByRow(mom_rj_sounds.GetInt());
    m_pTrailBox->ActivateItemByRow(mom_rj_trail.GetInt());
}