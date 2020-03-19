"resource/ui/SettingsPanel_GamemodeSettings.res"
{
    // This is the base panel that the scroll panel encapsulates.
    // Wide can be whatever, but tall should be as close to what you need
    // as possible.
    "GamemodeSettings"
    {
        "ControlName" "SettingsPanel"
        "fieldName" "GamemodeSettings"
        "tall" "334"
        "wide" "1000"
    }


    //Individual controls are below
    "RocketJumpLabel"
    {
        "ControlName" "Label"
        "fieldName" "RocketJumpLabel"
        "xpos" "6"
        "ypos" "4"
        "wide" "57"
        "tall" "12"
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
        "Font" "DefaultBoldLarge"
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
        "xpos" "4"
        "ypos" "29"
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
        "xpos" "4"
        "ypos" "49"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "69" // nice
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "89"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "109"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "129"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "149"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos"		"6"
        "ypos"		"169"
        "wide"		"57"
        "tall"		"10"
        "autoResize"		"0"
        "pinCorner"		"0"
        "RoundedCorners"		"15"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_RJ_RocketDrawDelayEntry_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultVerySmall"
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
        "xpos"		"4"
        "ypos"		"0"
        "wide"		"25"
        "tall"		"13"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "6"
        "ypos" "189"
        "wide" "57"
        "tall" "12"
        "autoResize" "0"
        "pinCorner" "0"
        "RoundedCorners" "15"
        "pin_corner_to_sibling" "0"
        "pin_to_sibling_corner" "0"
        "visible" "1"
        "enabled" "1"
        "tabPosition" "0"
        "labelText" "#MOM_GameType_SJ"
        "textAlignment" "west"
        "dulltext" "0"
        "brighttext" "0"
        "Font" "DefaultBoldLarge"
        "wrap" "0"
        "centerwrap" "0"
        "textinsetx" "0"
        "textinsety" "0"
        "auto_wide_tocontents"	"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets" "0"
    }

    "SJEnableExplosionParticle"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SJEnableExplosionParticle"
        "xpos" "4"
        "ypos" "214"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "234"
        "autoResize"		"0"
        "pinCorner"		"0"
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

    "EnableCharge"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "EnableCharge"
        "xpos" "4"
        "ypos" "254"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_Enable_Charge"
        "tooltiptext"   "#MOM_Settings_SJ_Enable_Charge_Tooltip"
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
        "cvar_name"		"mom_sj_charge_enable"
        "Font"		"DefaultVerySmall"
    }

    "EnableChargeMeter"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "EnableChargeMeter"
        "xpos" "4"
        "ypos" "274"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos" "4"
        "ypos" "294"
        "autoResize"		"0"
        "pinCorner"		"0"
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
        "xpos"		"6"
        "ypos"		"314"
        "wide"		"57"
        "tall"		"10"
        "autoResize"		"0"
        "pinCorner"		"0"
        "RoundedCorners"		"15"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_SJ_DrawDelayEntry_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultVerySmall"
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
        "xpos"		"4"
        "ypos"		"0"
        "wide"		"25"
        "tall"		"13"
        "autoResize"		"0"
        "pinCorner"		"0"
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