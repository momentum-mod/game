#include "cbase.h"

#include "GameplaySettingsPage.h"
#include <vgui_controls/CvarTextEntry.h>
#include <vgui_controls/CvarSlider.h>
#include <vgui_controls/CvarToggleCheckButton.h>

#include "tier0/memdbgon.h"

using namespace vgui;

GameplaySettingsPage::GameplaySettingsPage(Panel *pParent) : BaseClass(pParent, "GameplaySettings")
{
    m_pYawSpeedSlider = new CvarSlider(this, "YawSpeed", nullptr, 0.0f, 360.0f, "cl_yawspeed", false, true);
    m_pYawSpeedSlider->AddActionSignalTarget(this);
    m_pYawSpeedEntry = new CvarTextEntry(this, "YawSpeedEntry", "cl_yawspeed");
    m_pYawSpeedEntry->SetAllowNumericInputOnly(true);
    m_pYawSpeedEntry->AddActionSignalTarget(this);

    m_pLowerSpeedLabel = new Label(this, "LowerSpeedLabel", "#MOM_Settings_SpeedToLower");
    m_pLowerSpeedCVarEntry = new CvarTextEntry(this, "LowerSpeedEntry", "mom_weapon_speed_lower");
    m_pLowerSpeedCVarEntry->SetAllowNumericInputOnly(true);
    m_pLowerSpeedCVarEntry->AddActionSignalTarget(this);
    m_pLowerSpeed = new CvarToggleCheckButton(this, "LowerWeaponButton", "#MOM_Settings_LowerWeapon", "mom_weapon_speed_lower_enable");
    m_pLowerSpeed->AddActionSignalTarget(this);
    
    m_pOverlappingKeys = new CvarToggleCheckButton(this, "OverlappingKeys", "#MOM_Settings_Overlapping_Keys", "mom_enable_overlapping_keys");
    m_pOverlappingKeys->AddActionSignalTarget(this);
    m_pReleaseForwardOnJump = new CvarToggleCheckButton(this, "ReleaseForwardOnJump", "#MOM_Settings_Release_Forward_On_Jump", "mom_release_forward_on_jump");
    m_pReleaseForwardOnJump->AddActionSignalTarget(this);

    m_pPlayBlockSound = new CvarToggleCheckButton(this, "PlayBlockSound", "#MOM_Settings_Play_BlockSound", "mom_bhop_playblocksound");
    m_pPlayBlockSound->AddActionSignalTarget(this);
    m_pSaveCheckpoints = new CvarToggleCheckButton(this, "SaveCheckpoints", "#MOM_Settings_Save_Checkpoints", "mom_saveloc_save_between_sessions");
    m_pSaveCheckpoints->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/SettingsPanel_GameplaySettings.res");
}

void GameplaySettingsPage::LoadSettings()
{
    m_pLowerSpeedCVarEntry->SetEnabled(m_pLowerSpeed->IsSelected());
}

void GameplaySettingsPage::OnCheckboxChecked(Panel* p)
{
    BaseClass::OnCheckboxChecked(p);
    if (p == m_pLowerSpeed)
    {
        m_pLowerSpeedCVarEntry->SetEnabled(m_pLowerSpeed->IsSelected());
        m_pLowerSpeedLabel->SetEnabled(m_pLowerSpeed->IsSelected());
    }
}
