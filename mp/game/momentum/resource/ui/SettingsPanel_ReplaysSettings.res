"resource/ui/SettingsPanel_ReplaysSettings.res"
{
    //This is the base panel that the scroll panel encapsulates.
    //Wide can be whatever, but tall should be as close to what you need
    // as possible.
    "ReplaysSettings"
    {
        "ControlName" "SettingsPanel"
        "fieldName" "ReplaysSettings"
        "tall" "255"
        "wide" "1000"
    }
    
    
    //Individual controls are below
    "PickBodyColorButton"
    {
        "ControlName" "Button"
        "fieldName" "PickBodyColorButton"
        "xpos" "12"
        "ypos" "20"
        "command" "picker_body"
        "visible" "1"
        "enabled" "1"
        "labelText" ""
        "tooltiptext" "#MOM_Settings_Pick_Body_Color_TT"
        "actionsignallevel" "1"
    }
    
    
    "EnableTrail"
	{
	    "ControlName" "CvarToggleCheckButton"
        "fieldName" "EnableTrail"
        "xpos" "12"
        "ypos" "40"
        "wide"		"189"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_Enable_Trail"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"cvar_name"		"mom_trail_enable"
		"cvar_value"		"1"
	}
    
    "PickTrailColorButton"
    {
        "ControlName" "Button"
        "fieldName" "PickTrailColorButton"
        "xpos" "12"
        "ypos" "60"
        "command" "picker_trail"
        "visible" "1"
        "enabled" "1"
        "labelText" ""
        "tooltiptext" "#MOM_Settings_Pick_Trail_Color_TT"
        "actionsignallevel" "1"
    }
}