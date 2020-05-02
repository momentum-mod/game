"resource/ui/SettingsPanel_HudSettings.res"
{
    //This is the base panel that the scroll panel encapsulates.
    //Wide can be whatever, but tall should be as close to what you need
    // as possible.
    "HudSettings"
    {
        "ControlName" "SettingsPanel"
        "fieldName" "HudSettings"
        "tall" "300"
        "wide" "1000"
    }
    
    
    //Individual controls are below
    "SpeedoLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"SpeedoLabel"
        "xpos"		"6"
        "ypos"		"4"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultBoldSmall"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
    }
    "SpeedoShow"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SpeedoShow"
        "xpos"		"-8"
        "ypos"		"0"
        "pin_to_sibling" "SpeedoLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Show"
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
        "cvar_name"		"mom_hud_speedometer"
        "Font"		"DefaultVerySmall"
    }
	"SpeedoHorizShow"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SpeedoHorizShow"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pin_to_sibling" "SpeedoShow"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Horiz_Show"
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
        "cvar_name"		"mom_hud_speedometer_horiz"
        "Font"		"DefaultVerySmall"
    }
    "SpeedoShowJump"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SpeedoShowJump"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoHorizShow"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Show_Jump"
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
        "cvar_name"		"mom_hud_speedometer_lastjumpvel"
        "Font"		"DefaultVerySmall"
    }
    "SpeedoShowStageEnter"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SpeedoShowStageEnter"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoShowJump"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Show_StageEnter"
        "tooltiptext" 	"#MOM_Settings_Speedometer_Show_StageEnter_TT"
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
        "cvar_name"		"mom_hud_speedometer_showenterspeed"
        "Font"		"DefaultVerySmall"
    }
	"SpeedoShowUnitLabels"
	{
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SpeedoShowUnitLabels"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoShowStageEnter"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Unit_Labels"
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
        "cvar_name"		"mom_hud_speedometer_unit_labels"
        "Font"		"DefaultVerySmall"
	}
    "SpeedoUnitsLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"SpeedoUnitsLabel"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoShowUnitLabels"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Units_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "Font"		"DefaultSmall"
    }
    "SpeedoUnits"
    {
        "ControlName"		"CvarComboBox"
        "fieldName"		"SpeedoUnits"
        "xpos"		"3"
        "ypos"		"0"
        "wide"		"130"
        "tall"		"12"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoUnitsLabel"
        "pin_corner_to_sibling"		"2"
        "pin_to_sibling_corner"		"3"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textHidden"		"0"
        "editable"		"0"
        "maxchars"		"-1"
        "NumericInputOnly"		"0"
        "unicode"		"0"
        "cvar_name"		"mom_hud_speedometer_units"
        "font" "DefaultVerySmall"
    }   
    "SpeedoColorLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"SpeedoColorLabel"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoUnitsLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Speedometer_Color_Type_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "font" "DefaultSmall"
    }
    "SpeedoShowColor"
    {
        "ControlName"		"CvarComboBox"
        "fieldName"		"SpeedoShowColor"
        "xpos"		"3"
        "ypos"		"0"
        "wide"		"165"
        "tall"		"12"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoColorLabel"
        "pin_corner_to_sibling"		"2"
        "pin_to_sibling_corner"		"3"
        "visible"		"1"
        "enabled"		"1"
        "tooltiptext" 	"#MOM_Settings_Speedometer_Color_TT"
        "tabPosition"		"0"
        "textHidden"		"0"
        "editable"		"0"
        "maxchars"		"-1"
        "NumericInputOnly"		"0"
        "unicode"		"0"
        "cvar_name"     "mom_hud_speedometer_colorize"
        "font" "DefaultVerySmall"
    }
	
    
    
    
    
    // Key presses
    "KeysLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"KeysLabel"
        "xpos"		"8"
        "ypos"		"4"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SpeedoColorLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Buttons_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultBoldSmall"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
    }
    "ButtonsShow"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"ButtonsShow"
        "xpos"		"-8"
        "ypos"		"2"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "KeysLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Buttons_Show"
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
        "cvar_name"		"mom_hud_showkeypresses"
        "font" "DefaultVerySmall"
    }
    
    
    // Strafe sync
    "SyncLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"SyncLabel"
        "xpos"		"8"
        "ypos"		"5"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "ButtonsShow"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Sync_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultBoldSmall"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
    }
    "SyncShow"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SyncShow"
        "xpos"		"-8"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Sync_Show"
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
        "cvar_name"		"mom_hud_strafesync_draw"
        "font" "DefaultVerySmall"
    }
    "SyncShowBar"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"SyncShowBar"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncShow"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Sync_Show_Bar"
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
        "cvar_name"		"mom_hud_strafesync_drawbar"
        "font" "DefaultVerySmall"
    }
    "SyncTypeLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"SyncTypeLabel"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncShowBar"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Sync_Type_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
        "font" "DefaultSmall"
    }
    "SyncType"
    {
        "ControlName"		"CvarComboBox"
        "fieldName"		"SyncType"
        "xpos"		"3"
        "ypos"		"0"
        "wide"		"185"
        "tall"		"12"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncTypeLabel"
        "pin_corner_to_sibling"		"2"
        "pin_to_sibling_corner"		"3"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textHidden"		"0"
        "editable"		"0"
        "maxchars"		"-1"
        "NumericInputOnly"		"0"
        "unicode"		"0"
        "cvar_name"     "mom_hud_strafesync_type"
        "font" "DefaultVerySmall"
    }
    "SyncColorTypeLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"SyncColorTypeLabel"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncTypeLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Sync_Color_Type_Label"
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
    "SyncColorize"
    {
        "ControlName"		"CvarComboBox"
        "fieldName"		"SyncColorize"
        "xpos"		"3"
        "ypos"		"0"
        "wide"		"75"
        "tall"		"12"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncColorTypeLabel"
        "pin_corner_to_sibling"		"2"
        "pin_to_sibling_corner"		"3"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textHidden"		"0"
        "editable"		"0"
        "maxchars"		"-1"
        "NumericInputOnly"		"0"
        "unicode"		"0"
        "cvar_name"     "mom_hud_strafesync_colorize"
        "font" "DefaultVerySmall"
    }
    
    // Timer
    "TimerLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"TimerLabel"
        "xpos"		"8"
        "ypos"		"4"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "SyncColorTypeLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Timer_Label"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "Font"		"DefaultBoldSmall"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
        "auto_wide_tocontents"		"1"
        "auto_tall_tocontents" "1"
        "use_proportional_insets"		"0"
    }
    "TimerShow"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"TimerShow"
        "xpos"		"-8"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "TimerLabel"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Timer_Show"
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
        "cvar_name"		"mom_hud_timer"
        "Font"		"DefaultVerySmall"
    }
    "TimerSoundFailEnable"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"TimerSoundFailEnable"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "TimerShow"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Timer_Sound_Fail_Enable"
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
        "cvar_name"		"mom_timer_sound_fail_enable"
        "Font"		"DefaultVerySmall"
    }
    "TimerSoundStartEnable"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"TimerSoundStartEnable"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "TimerSoundFailEnable"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Timer_Sound_Start_Enable"
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
        "cvar_name"		"mom_timer_sound_start_enable"
        "Font"		"DefaultVerySmall"
    }
    "TimerSoundStopEnable"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"TimerSoundStopEnable"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "TimerSoundStartEnable"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Timer_Sound_Stop_Enable"
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
        "cvar_name"		"mom_timer_sound_stop_enable"
        "Font"		"DefaultVerySmall"
    }
    "TimerSoundFinishEnable"
    {
        "ControlName"		"CvarToggleCheckButton"
        "fieldName"		"TimerSoundFinishEnable"
        "xpos"		"0"
        "ypos"		"0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "pin_to_sibling" "TimerSoundStopEnable"
        "pin_corner_to_sibling"		"0"
        "pin_to_sibling_corner"		"2"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "labelText"		"#MOM_Settings_Timer_Sound_Finish_Enable"
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
        "cvar_name"		"mom_timer_sound_finish_enable"
        "Font"		"DefaultVerySmall"
    }
}