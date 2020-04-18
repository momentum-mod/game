"resource/ui/SettingsPanel_GameplaySettings.res"
{
    //This is the base panel that the scroll panel encapsulates.
    //Wide can be whatever, but tall should be as close to what you need
    // as possible.
    "GameplaySettings"
    {
        "ControlName" "SettingsPanel"
        "fieldName" "GameplaySettings"
        "tall" "180"
        "wide" "1000"
    }
    
    
    //Individual controls are below
    
    "YawSpeed"
    {
        "ControlName" "CvarSlider"
        "fieldName" "YawSpeed"
        "xpos" "4"
        "ypos" "16"
        "wide"		"200"
		"tall"		"40"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"leftText"		"#GameUI_Low"
		"rightText"		"#GameUI_High"
        "minvalue" "0"
        "maxvalue" "360"
        "cvar_name" "cl_yawspeed"
        "allowoutofrange" "0"
        "actionsignallevel" "1"
    }
    
    "YawSpeedEntry"
    {
		"ControlName"		"CvarTextEntry"
		"fieldName"		"YawSpeedEntry"
		"xpos"		"206"
		"ypos"		"16"
		"wide"		"35"
		"tall"		"15"
        "font" "DefaultVerySmall"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"3"
		"textHidden"		"0"
		"editable"		"1"
		"maxchars"		"-1"
		"NumericInputOnly"		"1"
		"unicode"		"0"
        "cvar_name" "cl_yawspeed"
        "actionsignallevel" "1"
	}
    
    "YawSpeedLabel"
    {
        "ControlName" "Label"
        "fieldName" "YawSpeedLabel"
        "xpos" "4"
        "ypos" "4"
        "autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_Yaw_Speed"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"0"
		"textinsety"		"0"
        "font" "DefaultSmall"
		"auto_wide_tocontents" "1"
		"use_proportional_insets"		"0"
    }
	
	"PlayBlockSound"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "PlayBlockSound"
        "xpos" "12"
        "ypos" "38"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_Play_BlockSound"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"cvar_name"		"mom_bhop_playblocksound"
        "actionsignallevel" "1"
        "font" "DefaultSmall"
    }
    "PracModeSafeGuard"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "PracModeSafeGuard"
        "xpos" "0"
        "ypos" "0"
        "wide"		"189"
		"tall"		"12"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
        "pin_to_sibling" "PlayBlockSound"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_Practice_Safeguard"
        "tooltiptext"   "#MOM_Settings_Practice_Safeguard_TT"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"cvar_name"		"mom_practice_safeguard"
        "actionsignallevel" "1"
        "font" "DefaultSmall"
    }
    "SaveCheckpoints"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "SaveCheckpoints"
        "xpos" "0"
        "ypos" "0"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
        "pin_to_sibling" "PracModeSafeGuard"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_Save_Checkpoints"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"cvar_name"		"mom_saveloc_save_between_sessions"
        "actionsignallevel" "1"
        "font" "DefaultSmall"
    }
    "LowerWeaponButton"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "LowerWeaponButton"
        "xpos" "0"
        "ypos" "0"
		"autoResize"		"0"
		"pinCorner"		"0"
        "pin_to_sibling" "SaveCheckpoints"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_LowerWeapon"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"cvar_name"		"mom_weapon_speed_lower_enable"
        "actionsignallevel" "1"
        "font" "DefaultSmall"
    }
	"LowerSpeedLabel"
	{
		"ControlName" "Label"
        "fieldName" "LowerSpeedLabel"
        "xpos" "3"
        "ypos" "0"
        "autoResize"		"0"
		"pinCorner"		"0"
        "pin_to_sibling" "LowerSpeedEntry"
		"pin_corner_to_sibling"		"2"
		"pin_to_sibling_corner"		"3"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_SpeedToLower"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"0"
		"textinsety"		"0"
		"auto_wide_tocontents" "1"
        "auto_tall_tocontents" "1"
		"use_proportional_insets"		"0"
        "font" "DefaultSmall"
	}
	"LowerSpeedEntry"
	{
        "ControlName" "CvarTextEntry"
        "fieldName" "LowerSpeedEntry"
		"xpos"		"-4"
		"ypos"		"4"
		"wide"		"25"
		"tall"		"12"
		"autoResize"		"0"
		"pinCorner"		"0"
        "pin_to_sibling" "LowerWeaponButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"3"
		"textHidden"		"0"
		"editable"		"1"
		"maxchars"		"-1"
		"NumericInputOnly"		"1"
		"unicode"		"0"
        "actionsignallevel" "1"
		"NumericInputOnly" "1"
		"cvar_name" "mom_weapon_speed_lower"
        "font" "DefaultVerySmall"
    }
    "WorldLights"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "WorldLights"
        "xpos"		"4"
		"ypos"		"4"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"0"
		"tabPosition"		"13"
		"labelText"		"#MOM_Settings_Worldlight_Shadows"
        "tooltiptext" "#MOM_Settings_Worldlight_Shadows_TT"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"Default"		"0"
		"selected"		"0"
        "cvar_name"		"r_worldlight_castshadows"
        "auto_wide_tocontents" "1"
        "auto_tall_tocontents" "1"
        "pin_to_sibling" "LowerSpeedEntry"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
        "font" "DefaultSmall"
    }
    "OverlappingKeys"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "OverlappingKeys"
        "xpos"		"4"
		"ypos"		"4"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"13"
		"labelText"		"#MOM_Settings_Overlapping_Keys"
        "tooltiptext" "#MOM_Settings_Overlapping_Keys_TT"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"Default"		"0"
		"selected"		"0"
        "cvar_name"		"mom_enable_overlapping_keys"
        "auto_wide_tocontents" "1"
        "auto_tall_tocontents" "1"
        "pin_to_sibling" "LowerSpeedEntry"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
        "font" "DefaultSmall"
    }
    "ReleaseForwardOnJump"
    {
        "ControlName" "CvarToggleCheckButton"
        "fieldName" "ReleaseForwardOnJump"
        "xpos"		"0"
		"ypos"		"0"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"13"
		"labelText"		"#MOM_Settings_Release_Forward_On_Jump"
        "tooltiptext" "#MOM_Settings_Release_Forward_On_Jump_TT"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"Default"		"1"
		"selected"		"0"
        "cvar_name"		"mom_release_forward_on_jump"
        "auto_wide_tocontents" "1"
        "auto_tall_tocontents" "1"
        "pin_to_sibling" "OverlappingKeys"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
        "font" "DefaultSmall"
    }
}