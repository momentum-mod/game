#include "cbase.h"

#include "GamemodeSettingsPage.h"

#include <vgui_controls/CvarToggleCheckButton.h>
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/Frame.h>

#include "tier0/memdbgon.h"

using namespace vgui;

GamemodeSettingsPage::GamemodeSettingsPage(Panel *pParent) : BaseClass(pParent, "GamemodeSettings")
{
    // Rocket Jump controls
    m_pRJEnableTrailParticle = new CvarToggleCheckButton(this, "RJEnableTrailParticle", "#MOM_Settings_RJ_Enable_Trail_Particle", "mom_rj_particle_trail_enable"); 
    m_pRJEnableExplosionParticle = new CvarToggleCheckButton(this, "RJEnableExplosionParticle", "#MOM_Settings_RJ_Enable_Explosion_Particle", "mom_rj_particle_explosion_enable");
    m_pRJEnableShootSound = new CvarToggleCheckButton(this, "ToggleRJShootSound", "#MOM_Settings_RJ_Enable_Shoot_Sound", "mom_rj_sound_shoot_enable");
    m_pRJToggleRocketTrailSound = new CvarToggleCheckButton(this, "ToggleRJTrailSound", "#MOM_Settings_RJ_Enable_Trail_Sound", "mom_rj_sound_trail_enable");
    m_pRJToggleRocketExplosionSound = new CvarToggleCheckButton(this, "ToggleRJExplosionSound", "#MOM_Settings_RJ_Enable_Explosion_Sound", "mom_rj_sound_explosion_enable");
    m_pRJToggleRocketDecals = new CvarToggleCheckButton(this, "ToggleRocketDecals", "#MOM_Settings_RJ_Enable_Rocket_Decals", "mom_rj_decals_enable");
    m_pRJEnableCenterFire = new CvarToggleCheckButton(this, "EnableCenterFire", "#MOM_Settings_RJ_Enable_Center_Fire", "mom_rj_center_fire");
    m_pRJRocketDrawDelayEntry = new CvarTextEntry(this, "RocketDrawDelayEntry", "mom_rj_rocket_drawdelay");
    m_pRJRocketDrawDelayEntry->SetAllowNumericInputOnly(true);
    m_pRJRocketDrawDelayEntry->AddActionSignalTarget(this);

    // Sticky Jump controls
    m_pSJEnableExplosionParticle = new CvarToggleCheckButton(this, "SJEnableExplosionParticle", "#MOM_Settings_SJ_Enable_Explosion_Particle", "mom_sj_particle_explosion_enable");
    m_pSJEnableExplosionSound = new CvarToggleCheckButton(this, "SJEnableExplosionSound", "#MOM_Settings_SJ_Enable_Explosion_Sound", "mom_sj_sound_explosion_enable");
    m_pSJEnableCharge = new CvarToggleCheckButton(this, "EnableCharge", "#MOM_Settings_SJ_Enable_Charge", "mom_sj_charge_enable");
    m_pSJEnableChargeMeter = new CvarToggleCheckButton(this, "EnableChargeMeter", "#MOM_Settings_SJ_Enable_Charge_Meter", "mom_hud_sj_chargemeter_enable");
    m_pSJEnableStickyCounter = new CvarToggleCheckButton(this, "EnableStickyCounter", "#MOM_Settings_SJ_Enable_Sticky_Counter", "mom_hud_sj_stickycount_enable");

    m_pSJStickyDrawDelayEntry = new CvarTextEntry(this, "StickyDrawDelayEntry", "mom_sj_stickybomb_drawdelay");
    m_pSJStickyDrawDelayEntry->SetAllowNumericInputOnly(true);
    m_pSJStickyDrawDelayEntry->AddActionSignalTarget(this);


    LoadControlSettings("resource/ui/SettingsPanel_GamemodeSettings.res");
}

GamemodeSettingsPage::~GamemodeSettingsPage() {}