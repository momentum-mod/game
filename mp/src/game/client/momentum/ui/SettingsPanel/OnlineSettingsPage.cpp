#include "cbase.h"

#include "OnlineSettingsPage.h"
#include "vgui_controls/CvarSlider.h"
#include "vgui_controls/CvarTextEntry.h"
#include <vgui_controls/CvarToggleCheckButton.h>

#include "tier0/memdbgon.h"

using namespace vgui;

OnlineSettingsPage::OnlineSettingsPage(Panel* pParent) : BaseClass(pParent, "OnlineSettings")
{
    m_pEnableGhostRotations = new CvarToggleCheckButton(this, "ToggleRotations", "#MOM_Settings_Online_Rotations", "mom_ghost_online_rotations");
    m_pEnableGhostRotations->AddActionSignalTarget(this);
    m_pEnableGhostSounds = new CvarToggleCheckButton(this, "ToggleSounds", "#MOM_Settings_Online_Sounds", "mom_ghost_online_sounds");
    m_pEnableGhostSounds->AddActionSignalTarget(this);
    m_pEnableEntityPanels = new CvarToggleCheckButton(this, "ToggleEntityPanels", "#MOM_Settings_Entity_Panels", "mom_entpanels_enable");
    m_pEnableEntityPanels->AddActionSignalTarget(this);
    m_pEnableGhostTrails = new CvarToggleCheckButton(this, "ToggleTrails", "#MOM_Settings_Online_Trails", "mom_ghost_online_trail_enable");
    m_pEnableGhostTrails->AddActionSignalTarget(this);
    m_pEnableColorAlphaOverride = new CvarToggleCheckButton(this, "EnableAlphaOverride", "#MOM_Settings_Override_Alpha_Enable", "mom_ghost_online_alpha_override_enable");
    m_pEnableColorAlphaOverride->AddActionSignalTarget(this);
    m_pAlphaOverrideSlider = new CvarSlider(this, "AlphaOverrideSlider", nullptr, 0.0f, 255.0f, "mom_ghost_online_alpha_override", false, true);
    m_pAlphaOverrideSlider->AddActionSignalTarget(this);
    m_pAlphaOverrideInput = new CvarTextEntry(this, "AlphaOverrideEntry", "mom_ghost_online_alpha_override");
    m_pAlphaOverrideInput->SetAllowNumericInputOnly(true);
    m_pAlphaOverrideInput->AddActionSignalTarget(this);

    LoadControlSettings("resource/ui/SettingsPanel_OnlineSettings.res");
}

void OnlineSettingsPage::LoadSettings()
{
    BaseClass::LoadSettings();
}
