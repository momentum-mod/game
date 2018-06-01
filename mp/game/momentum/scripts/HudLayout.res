"Resource/HudLayout.res"
{
    //Speedometer
    HudSpeedMeter
        {
                "fieldName"     "HudSpeedMeter"
                "xpos"          "c-60"
                "ypos"          "310"
                "wide"          "120"
                "tall"          "80"
                "visible"       "1"
                "enabled"       "1"
                "NumberFont"    "HudNumbersSmallBold"
                "SmallNumberFont" "HudNumbersExtremelySmall"
                "text_ypos"     "5"
                "digit_ypos"    "15"
                "digit2_ypos"   "30"
                "PrimaryValueColor" "MOM.Panel.Fg"
                "SecondaryValueColor" "Light Gray"
                "LabelColor"    "MOM.Panel.Fg"
                "BgColor"   "Blank"
                "StageAlpha" "0.0"//Used for fading
                "JumpAlpha" "0.0"//Used for fading
        }
    //Timer
    HudTimer
        {
                "fieldName"     "HudTimer"
                "xpos"          "c-50"
                "ypos"          "c+140"
                "wide"          "100"
                "tall"          "55"
                "visible"       "1"
                "enabled"       "1"
                "PaintBackgroundType" "2"
                "TimerFont" "HudNumbersSmallBold"
                "TextFont"      "HudHintTextLarge"
                "SmallTextFont" "HudHintTextSmall"
                "TextColor" "MOM.Panel.Fg"
                "centerTime"    "1" //If true, won't use time_xpos, centering the text instead
                "time_xpos"     "50"
                "time_ypos"     "3"
                "centerCps"     "1" //If true, won't use cps_xpos, centering the text instead
                "cps_xpos"      "50"//Note: checkpoints only show when there's no run (and therefore splits) 
                "cps_ypos"      "19"
                "centerSplit"   "1" //If true, won't use stage_xpos, centering the stage split
                "split_xpos"    "50"
                "split_ypos"    "19"
        }
    //Strafe Sync Meter
    CHudSyncMeter
        {
                "fieldName"     "CHudSyncMeter"
                "xpos"          "c-25"
                "ypos"          "c+198"
                "wide"          "50"
                "tall"          "30"
                "visible"       "1"
                "enabled"       "1"
                "TextFont"      "HudNumbersVerySmall"
                "NumberFont"    "HudNumbersSmall"
                "SmallNumberFont" "HudNumbersVerySmall"
                "PaintBackgroundType" "2"
                "text_xpos"     "15"
                "text_ypos"     "2"
                "digit_xpos"    "12"
                "digit_ypos"    "12"
                "digit2_xpos"   "30"
                "digit2_ypos"   "16"
                "PrimaryValueColor" "MOM.Panel.Fg"
                "SecondaryValueColor" "MOM.Panel.Fg"
                "LabelColor"    "White"
                "BgColor"   "Blank"
        }
    //The design for in-game menus            
    CHudMenuStatic
    {
         "fieldName" "CHudMenuStatic"
         // To override the menu colors, look inside of the ClientScheme.res file (for "MenuColor/MenuBoxColor/MenuItemColor)"!
    }
    // Version warning
    CHudVersionInfo
    {
        "fieldName"     "CHudVersionInfo"
        "xpos"          "10" // This is basically the X pin offset
        "ypos"          "rs1.0" // Used to shove it in the bottom left corner
        "visible"       "1"
        "enabled"       "1"
        "wide"          "20"
        "tall"          "20"
        "TextFont"      "MomHudDropText"
        "AutoResize" "3"
        "PinCorner" "2"
        "PinnedCornerOffsetX" "10"
        "PinnedCornerOffsetY" "5"
        "auto_wide_tocontents" "1"
        "auto_tall_tocontents" "1"
    }
    CHudSyncBar
    {
        "fieldName"     "CHudSyncBar"
        "xpos"          "c-60"
        "ypos"          "c+227"
        "wide"          "120"
        "tall"          "10"
        "InitialValue"  "0"
        "BackgroundColor" "MOM.Panel.Bg"
        "FillColor"     "255 255 255 225"
        "visible"       "1"
        "enabled"       "1"
    }
    
    CHudKeyPressDisplay
    {
        "fieldName"     "CHudKeyPressDisplay"
        "xpos"          "r175"
        "ypos"          "r100"
        "wide"          "150"
        "tall"          "100"
        "visible"       "1"
        "enabled"       "1"
        "top_row_ypos"  "5"
        "mid_row_ypos"  "20"
        "lower_row_ypos"    "35"
        "jump_row_ypos" "45"
        "duck_row_ypos" "55"
        "strafe_count_xpos" "110"
        "jump_count_xpos"   "110"
        "TextFont"      "MomentumIcons"
        "CounterTextFont"       "HudNumbersVerySmall"
        "WordTextFont"  "HudNumbersVerySmall"
        "KeyPressedColor"   "MOM.Panel.Fg"
        "KeyOutlineColor"   "Dark Gray"
    }
    
    CHudCompare
    {
        "fieldName" "CHudCompare"
        "xpos" "50"
        "ypos" "c+50"
        "wide" "200"
        "tall" "150"
        "visible" "1"
        "enabled" "1"
        "PaintBackgroundType" "2"
        "GainColor" "MOM.Compare.Gain"
        "LossColor" "MOM.Compare.Loss"
        "TieColor" "MOM.Compare.Tie"
        "TextFont" "HudHintTextSmall"
        "format_spacing" "2"//Number of pixels between each component of the comparison panel, only if mom_comparisons_format_output has value 1
        "text_xpos" "5"
        "text_ypos" "2"
    }
    
    CHudMapInfo
    {
        "fieldName" "CHudMapInfo"
        "visible" "1"
        "enabled" "1"
        "centerStatus" "1"//If this is 1, the status will be centered above the timer, otherwise the status_xpos will be used
        "status_xpos" "0"
        "status_ypos" "c+125"//y-pos for the map status 
        "mapinfo_xpos" "10"//xpos for the map info (author/difficulty/etc)
        "mapinfo_ypos" "10"//ypos for the map info
        "StatusFont" "MomHudDropText"//Font for the current map area/status
        "MapInfoFont" "MomHudDropText"//Font for the map information
        "TextColor" "MOM.Panel.Fg"
    }
    
    "CHudMapFinishedDialog"
    {
        "fieldName"     "CHudMapFinishedDialog"
        "TextFont"      "Default"
        // See resource/ui/MapFinishedDialog.res to change these
        //"xpos"          "c-110"
        //"ypos"          "c-150"
        //"wide"          "220"
        //"tall"          "180"
    }
    
    CHudSpectatorInfo
    {
        "fieldName" "CHudSpectatorInfo"
        "xpos" "r155"
        "ypos" "c-50"
        "wide" "125"
        "tall" "100"
        "visible" "1"
        "enabled" "1"
        "paintbackground" "0"
        "TextFont" "Default"
    }
    
    HudWeaponSelection
    {
        "fieldName" "HudWeaponSelection"
        "ypos"  "16"    [$WIN32]
        "ypos"  "32"    [$X360]
        "visible" "1"
        "enabled" "1"
        "SmallBoxSize" "32"
        "MediumBoxWide" "95"
        "MediumBoxWide_hidef"   "78"
        "MediumBoxTall" "50"
        "MediumBoxTall_hidef"   "50"
        "MediumBoxWide_lodef"   "74"
        "MediumBoxTall_lodef"   "50"
        "LargeBoxWide" "112"
        "LargeBoxTall" "80"
        "BoxGap" "8"
        "SelectionNumberXPos" "4"
        "SelectionNumberYPos" "4"
        "SelectionGrowTime" "0.4"
        "TextYPos" "64"
    }

    HudCrosshair
    {
        "fieldName" "HudCrosshair"
        "visible" "1"
        "enabled" "1"
        "wide"   "640"
        "tall"   "480"
    }

    HudMessage
    {
        "fieldName" "HudMessage"
        "visible" "1"
        "enabled" "1"
        "wide"   "f0"
        "tall"   "480"
    }

    HudMenu
    {
        "fieldName" "HudMenu"
        "visible" "1"
        "enabled" "1"
        "wide"   "640"
        "tall"   "480"
    }

    HudCloseCaption
    {
        "fieldName" "HudCloseCaption"
        "visible"   "1"
        "enabled"   "1"
        "xpos"      "c-250"
        "ypos"      "276"   [$WIN32]
        "ypos"      "236"   [$X360]
        "wide"      "500"
        "tall"      "136"   [$WIN32]
        "tall"      "176"   [$X360]

        "BgAlpha"   "128"

        "GrowTime"      "0.25"
        "ItemHiddenTime"    "0.2"  // Nearly same as grow time so that the item doesn't start to show until growth is finished
        "ItemFadeInTime"    "0.15"  // Once ItemHiddenTime is finished, takes this much longer to fade in
        "ItemFadeOutTime"   "0.3"
        "topoffset"     "0"     [$WIN32]
        "topoffset"     "0" [$X360]
    }

    HudChat
    {
        "fieldName" "HudChat"
        "visible" "0"
        "enabled" "1"
        "xpos"  "0"
        "ypos"  "0"
        "wide"   "4"
        "tall"   "4"
    }

    HudHistoryResource  [$WIN32]
    {
        "fieldName" "HudHistoryResource"
        "visible" "1"
        "enabled" "1"
        "xpos"  "r252"
        "ypos"  "40"
        "wide"   "248"
        "tall"   "320"

        "history_gap"   "56" [!$OSX]
        "history_gap"   "64" [$OSX]
        "icon_inset"    "38"
        "text_inset"    "36"
        "text_inset"    "26"
        "NumberFont"    "HudNumbersSmall"
    }

    HUDQuickInfo
    {
        "fieldName" "HUDQuickInfo"
        "visible" "1"
        "enabled" "1"
        "wide"   "640"
        "tall"   "480"
    }

    HudWeapon
    {
        "fieldName" "HudWeapon"
        "visible" "1"
        "enabled" "1"
        "wide"   "640"
        "tall"   "480"
    }
    HudAnimationInfo
    {
        "fieldName" "HudAnimationInfo"
        "visible" "1"
        "enabled" "1"
        "wide"   "640"
        "tall"   "480"
    }

    HudPredictionDump
    {
        "fieldName" "HudPredictionDump"
        "visible" "1"
        "enabled" "1"
        "wide"   "640"
        "tall"   "480"
    }

    HudHintDisplay
    {
        "fieldName"             "HudHintDisplay"
        "visible"               "0"
        "enabled"               "1"
        "xpos"                  "c-240"
        "ypos"                  "c60"
        "xpos"  "r148"  [$X360]
        "ypos"  "r338"  [$X360]
        "wide"                  "480"
        "tall"                  "100"
        "HintSize"              "1"
        "text_xpos"             "8"
        "text_ypos"             "8"
        "center_x"              "0" // center text horizontally
        "center_y"              "-1"    // align text on the bottom
        "paintbackground"       "0"
    }   

    HudHintKeyDisplay
    {
        "fieldName" "HudHintKeyDisplay"
        "visible"   "0"
        "enabled"   "1"
        "xpos"      "r120"  [$WIN32]
        "ypos"      "r340"  [$WIN32]
        "xpos"      "r148"  [$X360]
        "ypos"      "r338"  [$X360]
        "wide"      "100"
        "tall"      "200"
        "text_xpos" "8"
        "text_ypos" "8"
        "text_xgap" "8"
        "text_ygap" "8"
        "TextColor" "255 170 0 220"

        "PaintBackgroundType"   "2"
    }

    HudCredits
    {
        "fieldName" "HudCredits"
        "TextFont"  "Default"
        "visible"   "1"
        "xpos"  "0"
        "ypos"  "0"
        "wide"  "640"
        "tall"  "480"
        "TextColor" "255 255 255 192"

    }

    HudCommentary
    {
        "fieldName" "HudCommentary"
        "xpos"  "c-190"
        "ypos"  "350"
        "wide"  "380"
        "tall"  "40"
        "visible" "1"
        "enabled" "1"
        
        "PaintBackgroundType"   "2"
        
        "bar_xpos"      "50"
        "bar_ypos"      "20"
        "bar_height"    "8"
        "bar_width"     "320"
        "speaker_xpos"  "50"
        "speaker_ypos"  "8"
        "count_xpos_from_right" "10"    // Counts from the right side
        "count_ypos"    "8"
        
        "icon_texture"  "vgui/hud/icon_commentary"
        "icon_xpos"     "0"
        "icon_ypos"     "0"     
        "icon_width"    "40"
        "icon_height"   "40"
    }

    AchievementNotificationPanel    
    {
        "fieldName"             "AchievementNotificationPanel"
        "visible"               "1"
        "enabled"               "1"
        "xpos"                  "0"
        "ypos"                  "180"
        "wide"                  "f10"   [$WIN32]
        "wide"                  "f60"   [$X360]
        "tall"                  "100"
    }

    CHudVote
    {
        "fieldName"     "CHudVote"
        "xpos"          "0"         
        "ypos"          "0"
        "wide"          "640"
        "tall"          "480"
        "visible"       "1"
        "enabled"       "1"
        "bgcolor_override"  "0 0 0 0"
        "PaintBackgroundType"   "0" // rounded corners
    }   
}