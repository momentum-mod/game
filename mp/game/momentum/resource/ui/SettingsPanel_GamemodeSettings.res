"resource/ui/SettingsPanel_GamemodeSettings.res"
{
    // This is the base panel that the scroll panel encapsulates.
    // Wide can be whatever, but tall should be as close to what you need
    // as possible.
    "GamemodeSettings"
    {
        "ControlName" "SettingsPanel"
        "fieldName" "GamemodeSettings"
        "tall" "270"
        "wide" "1000"
    }


    //Individual controls are below
    "RocketJumpLabel"
    {
        "ControlName" "Label"
        "fieldName" "RocketJumpLabel"
        "xpos" "6"
        "ypos" "4"
        "autoResize" "0"
        "pinCorner" "0"
        "RoundedCorners" "15"
        "pin_corner_to_sibling" "0"
        "pin_to_sibling_corner" "0"
        "visible" "1"
        "enabled" "1"
        "tabPosition" "0"
        "labelText" "#MOM_GameType_RJ"
        "textAlignment" "west"
        "dulltext" "0"
        "brighttext" "0"
        "Font" "DefaultBoldSmall"
        "wrap" "0"
        "centerwrap" "0"
        "textinsetx" "0"
        "textinsety" "0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets" "0"
    }

    "RJEnableTrailParticle"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "RJEnableTrailParticle"
        "xpos"		"-8"
        "ypos"		"0"
        "pin_to_sibling" "RocketJumpLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Trail_Particle"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_particle_trail_enable"
        "Font"		"DefaultVerySmall"
    }

    "RJEnableExplosionParticle"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "RJEnableExplosionParticle"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "RJEnableTrailParticle"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Explosion_Particle"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_particle_explosion_enable"
        "Font"		"DefaultVerySmall"
    }

    "ToggleRJShootSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "ToggleRJShootSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "RJEnableExplosionParticle"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Shoot_Sound"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_sound_shoot_enable"
        "Font"		"DefaultVerySmall"
    }

    "ToggleRJTrailSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "ToggleRJTrailSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "ToggleRJShootSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Trail_Sound"
        "tooltiptext" "#MOM_Settings_RJ_Enable_Trail_Sound_TT"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_sound_trail_enable"
        "Font"		"DefaultVerySmall"
    }

    "ToggleRJExplosionSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "ToggleRJExplosionSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "ToggleRJTrailSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Explosion_Sound"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_sound_explosion_enable"
        "Font"		"DefaultVerySmall"
    }

    "ToggleRocketDecals"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "ToggleRocketDecals"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "ToggleRJExplosionSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Rocket_Decals"
        "tooltiptext" "#MOM_Settings_RJ_Enable_Rocket_Decals_TT"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_decals_enable"
        "Font"		"DefaultVerySmall"
    }

    "EnableCenterFire"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "EnableCenterFire"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "ToggleRocketDecals"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_Enable_Center_Fire"
        "tooltiptext" "#MOM_Settings_RJ_Enable_Center_Fire_TT"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_rj_center_fire"
        "Font"		"DefaultVerySmall"
    }

    "RocketDrawDelayEntryLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"RocketDrawDelayEntryLabel"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "RoundedCorners"		"15"
        "pin_to_sibling" "EnableCenterFire"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_RocketDrawDelayEntry_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultSmall"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "tooltiptext" "#MOM_Settings_SJ_RocketDrawDelayEntry_Tooltip"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
    }

    "RocketDrawDelayEntry"
    {
        "ControlName"		"CvarTextEntry"
        "fieldName"		"RocketDrawDelayEntry"
        "xpos"		"3"
        "ypos"		"0"
        "wide"		"25"
        "tall"		"12"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "RocketDrawDelayEntryLabel"
        "pin_corner_to_sibling"		"2"
        "pin_to_sibling_corner"		"3"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textHidden"		"0"
        "editable"		"1"
        "maxchars"		"-1"
        "NumericInputOnly"		"1"
        "unicode"		"0"
        "cvar_name" "mom_rj_rocket_drawdelay"
        "pin_to_sibling" "RocketDrawDelayEntryLabel"
        "pin_corner_to_sibling" "0"
        "pin_to_sibling_corner" "1"
        "font" "DefaultVerySmall"
    }

    "StickyJumpLabel"
    {
        "ControlName" "Label"
        "fieldName" "StickyJumpLabel"
        "xpos"		"8"
        "ypos"		"4"
        "autoResize" "0"
        "pinCorner" "0"
        "pin_to_sibling" "RocketDrawDelayEntryLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "RoundedCorners" "15"
        "visible" "1"
        "enabled" "1"
        "tabPosition" "0"
        "labelText" "#MOM_GameType_SJ"
        "textAlignment" "west"
        "dulltext" "0"
        "brighttext" "0"
        "Font" "DefaultBoldSmall"
        "wrap" "0"
        "centerwrap" "0"
        "textinsetx" "0"
        "textinsety" "0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets" "0"
    }

    "SJEnableTrailParticle"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableTrailParticle"
        "xpos"		"-8"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "StickyJumpLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Trail_Particle"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_sj_particle_trail_enable"
        "Font"		"DefaultVerySmall"
    }
	
    "SJEnableExplosionParticle"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableExplosionParticle"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SJEnableTrailParticle"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Explosion_Particle"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_sj_particle_explosion_enable"
        "Font"		"DefaultVerySmall"
    }

    "SJEnableExplosionSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableExplosionSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SJEnableExplosionParticle"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Explosion_Sound"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_sj_sound_explosion_enable"
        "Font"		"DefaultVerySmall"
    }

    "SJEnableDetonateFailSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableDetonateFailSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SJEnableExplosionSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Detonation_Fail_Sound"
        "tooltiptext"   "#MOM_Settings_SJ_Enable_Detonation_Fail_Sound_TT"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_sj_sound_detonate_fail_enable"
        "Font"		"DefaultVerySmall"
    }

    "SJEnableDetonateSuccessSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableDetonateSuccessSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SJEnableDetonateFailSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Detonation_Success_Sound"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_sj_sound_detonate_success_enable"
        "Font"		"DefaultVerySmall"
    }

    "SJEnableChargeSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableChargeSound"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SJEnableDetonateSuccessSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Charge_Sound"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"0"
        "cvar_name"		"mom_sj_sound_charge_enable"
        "Font"		"DefaultVerySmall"
    }

    "EnableChargeMeter"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "EnableChargeMeter"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SJEnableChargeSound"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_ChargeMeter"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"1"
        "cvar_name"		"mom_hud_sj_chargemeter_enable"
        "Font"		"DefaultVerySmall"
    }

    "EnableStickyCounter"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "EnableStickyCounter"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "EnableChargeMeter"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Sticky_Counter"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"6"
        "textinsety"		"0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Default"		"1"
        "cvar_name"		"mom_hud_sj_stickycount_enable"
        "Font"		"DefaultVerySmall"
    }

    "StickyDrawDelayEntryLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"StickyDrawDelayEntryLabel"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "EnableStickyCounter"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "RoundedCorners"		"15"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_DrawDelayEntry_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultSmall"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "tooltiptext" "#MOM_Settings_SJ_DrawDelayEntry_Tooltip"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
    }

    "StickyDrawDelayEntry"
    {
        "ControlName"		"CvarTextEntry"
        "fieldName"		"StickyDrawDelayEntry"
        "xpos"		"3"
        "ypos"		"0"
        "wide"		"25"
        "tall"		"12"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "StickyDrawDelayEntryLabel"
        "pin_corner_to_sibling"		"2"
        "pin_to_sibling_corner"		"3"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textHidden"		"0"
        "editable"		"1"
        "maxchars"		"-1"
        "NumericInputOnly"		"1"
        "unicode"		"0"
        "cvar_name" "mom_sj_stickybomb_drawdelay"
        "font" "DefaultVerySmall"
        "pin_to_sibling" "StickyDrawDelayEntryLabel"
        "pin_corner_to_sibling" "0"
        "pin_to_sibling_corner" "1"
    }
}