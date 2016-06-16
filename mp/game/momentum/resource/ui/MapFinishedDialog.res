"resource/ui/MapFinishedDialog.res"
{
    "CHudMapFinishedDialog"
    {
        "fieldName"     "CHudMapFinishedDialog"
        "xpos"          "c-300"
        "ypos"          "c-150"
        "wide"          "300"
        "tall"          "175"
        "visible"       "1"
        "enabled"       "1"
        "TextFont"      "Default"
    }
    
    "Prev_Zone"
    {
        "ControlName"   "ImagePanel"
        "fieldName"     "Prev_Zone"
        "xpos"          "2"
        "ypos"          "2"
        "wide"          "8"
        "tall"          "8"
        "scaleImage"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "image"         "left_arrow_button_grey"
    }
    
    "Next_Zone"
    {
        "ControlName"   "ImagePanel"
        "fieldName"     "Next_Zone"
        "xpos"          "50"//Determined by code
        "ypos"          "2"
        "wide"          "8"
        "tall"          "8"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "scaleImage"    "1"
        "image"         "right_arrow_button_grey"
    } 
    
    "Replay_Icon"
    {
        "ControlName"   "ImagePanel"
        "fieldName"     "Replay_Icon"
        "xpos"          "180"
        "ypos"          "80"
        "wide"          "32"
        "tall"          "32"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "scaleImage"    "1"
        "image"         "replay_icon_grey"
    }
    
    "Repeat_Button"
    {
        "ControlName"   "ImagePanel"
        "fieldName"     "Repeat_Button"
        "xpos"          "250"
        "ypos"          "80"
        "wide"          "32"
        "tall"          "32"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "scaleImage"    "1"
        "image"         "loop_button_grey"
    }
    
    "Close_Panel"
    {
        "ControlName"   "ImagePanel"
        "fieldName"     "Close_Panel"
        "xpos"          "180"
        "ypos"          "120"
        "wide"          "32"
        "tall"          "32"
        "autoResize"    "0"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "scaleImage"    "1"
        "image"         "close_button"
    }
    
    "Detach_Mouse"
    {
        "ControlName" "Label"
        "fieldName" "Detach_Mouse"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "150"
        "wide"          "150"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_DetachMouse"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Current_Zone"//This is the label that shows what the current zone on the map finished panel
    {
        "ControlName" "Label"
        "fieldName"  "Current_Zone"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "11"
        "ypos"          "2"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_OverallStats"//This changes based on page number
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Overall_Time"// or "ZoneTime"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Overall_Time"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "10"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_RunTime"//Possibly changing to #MOM_MF_Time_Zone
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Enter_Time"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Enter_Time"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "20"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Zone_Enter"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Strafes"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Strafes"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "30"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Strafes"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Jumps"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Jumps"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "40"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Jumps"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Vel_Enter"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Vel_Enter"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "50"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Velocity_Enter"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Vel_Exit"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Vel_Exit"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "60"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Velocity_Exit"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Vel_Avg"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Vel_Avg"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "70"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Velocity_Avg"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Vel_Max"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Vel_Max"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "80"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Velocity_Max"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Sync1"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Sync1"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "90"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Sync1"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Zone_Sync2"
    {
        "ControlName" "Label"
        "fieldName"  "Zone_Sync2"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "100"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_Sync2"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Run_Save_Status"
    {
        "ControlName" "Label"
        "fieldName"  "Run_Save_Status"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "110"
        "wide"          "100"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_RunNotSaved"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    "Run_Upload_Status"
    {
        "ControlName" "Label"
        "fieldName"  "Run_Upload_Status"
        "font"          "Default"//Set by "TextFont" in HudLayout.res
        "xpos"          "20"
        "ypos"          "120"
        "wide"          "120"
        "tall"          "10"//Set by font size
        "autoResize"    "1"
        "pinCorner"     "0"
        "visible"       "1"
        "enabled"       "1"
        "labelText"     "#MOM_MF_RunNotUploaded"
        "textAlignment" "west"
        "dulltext"      "0"
        "brighttext"    "0"
    }
    
    styles
    {
        status
        {
            bgcolor="PropertySheetBG"
            inset="8 0 0 0"
        }
    }
}
