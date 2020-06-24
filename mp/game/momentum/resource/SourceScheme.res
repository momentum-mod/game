///////////////////////////////////////////////////////////
// Tracker scheme resource file
//
// sections:
//		Colors			- all the colors used by the scheme
//		BaseSettings	- contains settings for app to use to draw controls
//		Fonts			- list of all the fonts used by app
//		Borders			- description of all the borders
//
///////////////////////////////////////////////////////////
Scheme
{
    //////////////////////// COLORS ///////////////////////////
    // color details
    // this is a list of all the colors used by the scheme
    Colors
    {
        // base colors
        "White"				"255 255 255 255"
        "WhiteHO"           "255 255 255 128"
        "OffWhite"			"221 221 221 255"
        "DullWhite"			"190 190 190 255"
        "Light Gray"		"211 211 211 20"
        "Gray" 				"128 128 128 150"
        "Dark Gray" 		"64 64 64 200"
        "Black"                "0 0 0 255"
        "BlackHO"           "0 0 0 128"
        "TransparentBlack"    "0 0 0 196"
        "TransparentLightBlack"    "0 0 0 90"

        "Blank"                "0 0 0 0"
    
        "Red"                "192 28 0 140"
        "Orange"			"255 155 0 255"
        
        "SteamLightGreen"	"157 194 80 255"
        "AchievementsLightGrey"		"79 79 79 255"
        "AchievementsDarkGrey"		"55 55 55 255"
        "AchievementsInactiveFG"	"130 130 130 255"

        "MomentumRed"	    "255 106 106 255"
        "MomentumGreen"	    "152 255 153 255"
        "MomentumBlue"	    "24 150 211 255"
        "MomentumBlueHO"    "24 150 211 128"
        "MomentumLightBlue"	"76 139 180 255"

        "MomGreydientStep1" "32 32 32 255"
        "MomGreydientStep2" "42 42 42 255"
        "MomGreydientStep3" "54 54 54 255"
        "MomGreydientStep4" "65 65 65 255"
        "MomGreydientStep5" "79 79 79 255"
        "MomGreydientStep6" "95 95 95 255"
        "MomGreydientStep7" "129 129 129 255"
        "MomGreydientStep8" "200 200 200 255"
        
        "MomGreydientHOStep1" "32 32 32 127"
        "MomGreydientHOStep2" "42 42 42 127"
        "MomGreydientHOStep3" "54 54 54 127"
        "MomGreydientHOStep4" "65 65 65 127"
        "MomGreydientHOStep5" "79 79 79 127"
        "MomGreydientHOStep6" "95 95 95 127"
        "MomGreydientHOStep7" "129 129 129 127"
        "MomGreydientHOStep8" "200 200 200 127"
    }

    ///////////////////// BASE SETTINGS ////////////////////////
    //
    // default settings for all panels
    // controls use these to determine their settings
    BaseSettings
    {
        "MOM.Compare.Gain" "MomentumBlue"
        "MOM.Compare.Loss" "MomentumRed"
        "MOM.Compare.Tie" "MOM.Panel.Fg"

        "MOM.Panel.Fg"				"WhiteHO"
        "MOM.Panel.Bg"				"MomGreydientStep3"

        // vgui_controls color specifications
        Border.Bright					"200 200 200 255"	// the lit side of a control
        Border.Dark						"40 40 40 100"		// the dark/unlit side of a control
        Border.Selection				"0 0 0 76"			// the additional border color for displaying the default/selected button
        Border.DarkSolid				"MomGreydientStep1"
        Border.Subtle					"80 80 80 255"

        Button.TextColor				"White"
        Button.BgColor					"MomGreydientStep4"
        Button.ArmedTextColor			"MomentumBlue"
        Button.ArmedBgColor				"MomGreydientStep4"
        Button.SelectedTextColor		"White"
        Button.SelectedBgColor			"MomentumBlue"
        Button.DepressedTextColor		"White"
        Button.DepressedBgColor			"MomentumBlue"
        Button.FocusBorderColor			"82 82 82 0"

        CheckButton.TextColor			"White"
        CheckButton.SelectedTextColor	"White"
        CheckButton.BgColor				"MomGreydientStep5"
        CheckButton.Border1  			"MomGreydientStep1" 	// the left checkbutton border color
        CheckButton.Border2  			"MomGreydientStep1"		// the right checkbutton border color
        CheckButton.Check				"MomentumBlue"	// color of the check itself
        CheckButton.HighlightFgColor	"OffWhite"
        CheckButton.ArmedBgColor		"Blank"
        CheckButton.DepressedBgColor	"Blank"
        CheckButton.DisabledFgColor     "MomGreydientStep5"
        CheckButton.DisabledBgColor	   	"MomGreydientStep3"

        ComboBoxButton.ArrowColor		"MomentumBlue"
        ComboBoxButton.ArmedArrowColor	"MomGreydientStep8"
        ComboBoxButton.BgColor			"MomGreydientStep3"
        ComboBoxButton.DisabledBgColor	"MomGreydientHOStep3"

        Frame.TitleTextInsetX			8
        Frame.ClientInsetX				8
        Frame.ClientInsetY				8
        Frame.BgColor					"MomGreydientStep3"
        Frame.OutOfFocusBgColor			"MomGreydientHOStep3"
        Frame.FocusTransitionEffectTime	"0.3"					// time it takes for a window to fade in/out on focus/out of focus
        Frame.TransitionEffectTime		"0.3"					// time it takes for a window to fade in/out on open/close
        Frame.AutoSnapRange				"0"
        FrameGrip.Color1				"200 200 200 196"
        FrameGrip.Color2				"0 0 0 196"
        FrameTitleButton.FgColor		"MomentumRed"
        FrameTitleButton.BgColor		"Blank"
        FrameTitleButton.DisabledFgColor	"MomentumRed"
        FrameTitleButton.DisabledBgColor	"Blank"
        FrameSystemButton.FgColor		"Blank"
        FrameSystemButton.BgColor		"Blank"
        FrameSystemButton.Icon			""
        FrameSystemButton.DisabledIcon	""
        FrameTitleBar.Font				"FrameBar"		
        FrameTitleBar.TextColor			"255 255 255 204"
        FrameTitleBar.BgColor			"Blank"
        FrameTitleBar.DisabledTextColor	"255 255 255 91"
        FrameTitleBar.DisabledBgColor	"Blank"

        GraphPanel.FgColor				"White"
        GraphPanel.BgColor				"TransparentBlack"

        InlineEditPanel.Color           "MomentumBlue"

        Label.TextDullColor				"DullWhite"
        Label.TextColor					"White"
        Label.TextBrightColor			"White"
        Label.SelectedTextColor			"White"
        Label.BgColor					"Blank"
        Label.DisabledFgColor1			"Blank"
        Label.DisabledFgColor2			"MomGreydientStep7"

        ListPanel.TextColor					"White"
        ListPanel.TextBgColor				"Blank"
        ListPanel.BgColor					"MomGreydientStep4"
        ListPanel.SelectedTextColor			"White"
        ListPanel.SelectedBgColor			"MomentumBlue"
        ListPanel.OutOfFocusSelectedTextColor	"Gray"
        ListPanel.SelectedOutOfFocusBgColor	"MomentumBlueHO"
        ListPanel.EmptyListInfoTextColor	"White"

        Menu.TextColor					"White"
        Menu.BgColor					"MomGreydientStep5"
        Menu.ArmedTextColor				"White"
        Menu.ArmedBgColor				"MomentumBlue"
        Menu.TextInset					"6"

        Panel.FgColor					"White"
        Panel.BgColor					"MomGreydientStep3"

        ProgressBar.FgColor				"MomentumBlue"
        ProgressBar.BgColor				"MomGreydientStep1"
        ProgressBar.ProgressTextColor   "White"
        ProgressBar.ProgressTextFont    "DefaultSmall"

        PropertyDialog.ButtonFont   "Default"

        PropertySheet.TextColor			"MomGreydientStep8"
        PropertySheet.SelectedTextColor	"White"
        PropertySheet.SelectedBgColor	"MomentumBlue"
        PropertySheet.TransitionEffectTime	"0.25"	// time to change from one tab to another
        PropertySheet.BgColor "0 0 0 255"
        PropertySheet.TabFontSmall "DefaultSmall"

        RadioButton.TextColor			"White"
        RadioButton.SelectedTextColor	"MomentumBlue"

        RichText.TextColor				"White"
        RichText.BgColor				"MomGreydientStep4"
        RichText.SelectedTextColor		"MomentumBlue"
        RichText.SelectedBgColor		"Light Gray"//"MOM.Panel.Bg"

        ScrollBar.Wide					"10"

        ScrollBarButton.FgColor				"MomGreydientStep6"
        ScrollBarButton.BgColor				"MomGreydientStep2"
        ScrollBarButton.ArmedFgColor		"MomGreydientStep8"
        ScrollBarButton.ArmedBgColor		"MomGreydientStep6"
        ScrollBarButton.DepressedFgColor	"MomGreydientStep1"
        ScrollBarButton.DepressedBgColor	"MomGreydientStep2"

        ScrollBarSlider.FgColor				"MomGreydientStep6"	// nob color
        ScrollBarSlider.BgColor				"MomGreydientStep2"	// slider background color

        SectionedListPanel.HeaderTextColor	"MomGreydientStep8"
        SectionedListPanel.HeaderBgColor	"Blank"
        SectionedListPanel.DividerColor		"MomGreydientStep1"
        SectionedListPanel.TextColor		"White"
        SectionedListPanel.BrightTextColor	"White"
        SectionedListPanel.BgColor			"Blank"
        SectionedListPanel.SelectedTextColor			"White"
        SectionedListPanel.SelectedBgColor				"MomentumBlue"
        SectionedListPanel.OutOfFocusSelectedTextColor	"White"
        SectionedListPanel.OutOfFocusSelectedBgColor	"MomentumBlueHO"

        Slider.NobColor				"MomGreydientStep4"
        Slider.TextColor			"White"
        Slider.BgColor              "Blank"
        Slider.TrackColor			"MomGreydientStep2"
        Slider.DisabledTextColor1	"MomGreydientHOStep8"
        Slider.DisabledTextColor2	"MomGreydientHOStep8"

        TextEntry.TextColor			"White"
        TextEntry.BgColor			"MomGreydientStep5"
        TextEntry.CursorColor		"MomGreydientStep2"
        TextEntry.DisabledTextColor	"MomGreydientHOStep8"
        TextEntry.DisabledBgColor	"TransparentBlack"
        TextEntry.SelectedTextColor	"White"
        TextEntry.SelectedBgColor	"MomentumBlue"
        TextEntry.OutOfFocusSelectedBgColor	"MomentumBlueHO"
        TextEntry.FocusEdgeColor	"0 0 0 196"
        TextEntry.Border            "TextEntryBorder"

        ToggleButton.SelectedTextColor	"White"

        Tooltip.TextColor			"White"
        Tooltip.BgColor				"MomGreydientStep3"
        Tooltip.TextFont            "DefaultSmall"

        TreeView.BgColor			"Light Gray"

        WizardSubPanel.BgColor		"Blank"
    }

    //////////////////////// FONTS /////////////////////////////
    //
    // describes all the fonts
    Fonts
    {
        // fonts are used in order that they are listed
        // fonts listed later in the order will only be used if they fulfill a range not already filled
        // if a font fails to load then the subsequent fonts will replace
        // fonts are used in order that they are listed
        "DebugFixed"
        {
            "1"
            {
                "name"		"Courier New"
                "tall"		"14"
                "weight"	"400"
                "antialias" "1"
            }
        }
        // fonts are used in order that they are listed
        "DebugFixedSmall"
        {
            "1"
            {
                "name"		"Courier New"
                "tall"		"12"
                "weight"	"400"
                "antialias" "1"
            }
        }
        "DefaultFixedOutline"
        {
            "1"
            {
                "name"		 "Lucida Console" [$WINDOWS]
                "name"		 "Lucida Console" [$X360]
                "name"		 "Lucida Console" [$OSX]
                "name"		 "Verdana" [$LINUX]
                "tall"		"14" [$LINUX]
                "tall"		 "10"
                "tall_lodef" "15"
                "tall_hidef" "20"
                "weight"	 "0"
                "outline"	 "1"
            }
        }
        "Default"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"12"
                "weight"	"400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "DefaultBold"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana Bold" [$LINUX]
                "tall"		"16"
                "weight"	"1000"
                "antialias" "1"
            }
        }
        "DefaultBoldLarge"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana Bold" [$LINUX]
                "tall"		"18"
                "weight"	"1000"
                "antialias" "1"
            }
        }
        "DefaultBoldSmall"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana Bold" [$LINUX]
                "tall"		"14"
                "weight"	"1000"
                "antialias" "1"
            }
        }
        "DefaultUnderline"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"16"
                "weight"	"500"
                "underline" "1"
            }
        }
        "DefaultSmall"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"11"
                "weight"	"400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "DefaultSmallDropShadow"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"13"
                "weight"	"0"
                "dropshadow" "1"
                "antialias" "1"
            }
        }
        "Default6"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"6"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default8"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"8"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default10"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"10"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default12"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"12"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default14"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"14"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default16"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"16"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default18"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"18"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default20"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"20"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default22"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"22"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Default24"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"24"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "DefaultVerySmall"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"11"
                "weight" "400"
                "antialias" "1"
                "dropshadow" "1"
            }
        }

        "DefaultLarge"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"18"
                "weight"	"600"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "DefaultLargeDropShadow"
        {
            "1"
            {
                "name"		"Noto Sans"
                "tall"		"18"
                "weight"	"0"
                "dropshadow" "1"
                "antialias" "1"
            }
        }
        "DefaultVeryLarge"
        {
            "1"
            {
                "name" "Noto Sans"
                "tall" "22"
                "antialias" "1"
            }
        }

        "Titling14"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "14"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling16"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "16"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling18"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "18"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling20"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "20"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling22"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "22"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling24"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "24"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling30"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "30"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "Titling40"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "40"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        
        "UiBold"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"12"
                "weight"	"1000"
            }
        }
        
        "LoadingLarge"
        {
            "1"
            {
                "name"      "Bebas Neue"
                "tall"      "28"
                "weight"    "500"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "LoadingSmall"
        {
            "1"
            {
                "name"      "Bebas Neue"
                "tall"      "20"
                "weight"    "500"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "LoadingTip"
        {
            "1"
            {
                "name" "Noto Sans"
                "tall" "14"
                "weight" "500"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        "MenuLarge"
        {
            "1"
            {
                "name"		"Verdana" 
                "tall"		"16" 
                "weight"	"1000"
                "antialias" "1"
            }
        }
        "AchievementTitleFont"
        {
            "1"
            {
                "name"		"Verdana"
                "tall"		"20"
                "weight"	"1200"
                "antialias" "1"
                "outline" "1"
            }
        }
        
        "AchievementTitleFontSmaller"
        {
            "1"
            {
                "name"		"Verdana"
                "tall"		"18"
                "weight"	"1200"
                "antialias" "1"
                //"outline" "1"
            }
        }
        
        
        "AchievementDescriptionFont"
        {
            "1"
            {
                "name"		"Verdana"
                "tall"		"15"
                "weight"	"1200"
                "antialias" "1"
                "outline" "1"
                "yres"		"0 480"
            }
            "2"
            {
                "name"		"Verdana"
                "tall"		"20"
                "weight"	"1200"
                "antialias" "1"
                "outline" "1"
                "yres"	 "481 10000"
            }
        }

        // this is the symbol font
        "Marlett"
        {
            "1"
            {
                "name"		"Marlett"
                "tall"		"10"
                "weight"	"0"
                "symbol"	"1"
            }
        }

        "Trebuchet24"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"24"
                "weight"	"900"
            }
        }

        "Trebuchet20"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"20"
                "weight"	"900"
            }
        }

        "Trebuchet18"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"18"
                "weight"	"900"
            }
        }

        CenterPrintText
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"28"
				"weight"	"900"
				"antialias" "1"
                "dropshadow" "1"
			}
		}

        // HUD numbers
        // We use multiple fonts to 'pulse' them in the HUD, hence the need for many of near size
        "HUDNumber"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"40"
                "weight"	"900"
            }
        }
        "HUDNumber1"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"41"
                "weight"	"900"
            }
        }
        "HUDNumber2"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"42"
                "weight"	"900"
            }
        }
        "HUDNumber3"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"43"
                "weight"	"900"
            }
        }
        "HUDNumber4"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"44"
                "weight"	"900"
            }
        }
        "HUDNumber5"
        {
            "1"
            {
                "name"		"Trebuchet MS"
                "tall"		"45"
                "weight"	"900"
            }
        }
        "DefaultFixed"
        {
            "1"
            {
                "name"		 "Lucida Console" [$WINDOWS]
                "name"		 "Lucida Console" [$X360]
                "name"		 "Verdana" [$LINUX]
                "tall"		"11" [$LINUX]
                "tall"		"10"
                "weight"	"0"
            }
        }

        "DefaultFixedDropShadow"
        {
            "1"
            {
                "name"		 "Lucida Console" [$WINDOWS]
                "name"		 "Lucida Console" [$X360]
                "name"		 "Lucida Console" [$OSX]
                "name"		 "Courier" [$LINUX]
                "tall"		"14" [$LINUX]
                "tall"		"10"
                "weight"	"0"
                "dropshadow" "1"
            }
        }

        "CloseCaption_Normal"
        {
            "1"
            {
                "name"		"Tahoma" [!$POSIX]
                "name"		"Verdana" [$POSIX]
                "tall"		"16"
                "weight"	"500"
            }
        }
        "CloseCaption_Italic"
        {
            "1"
            {
                "name"		"Tahoma"
                "tall"		"16"
                "weight"	"500"
                "italic"	"1"
            }
        }
        "CloseCaption_Bold"
        {
            "1"
            {
                "name"		"Tahoma" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "tall"		"16"
                "weight"	"900"
            }
        }
        "CloseCaption_BoldItalic"
        {
            "1"
            {
                "name"		"Tahoma" [!$POSIX]
                "name"		"Verdana Bold Italic" [$POSIX]
                "tall"		"16"
                "weight"	"900"
                "italic"	"1"
            }
        }

        "FrameBar"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "16"
                "antialias" "1"
                "dropshadow" "1"
            }
        }

        TitleFont
        {
            "1"
            {
                "name"		"HalfLife2"
                "tall"		"72"
                "weight"	"400"
                "antialias"	"1"
                "custom"	"1"
            }
        }

        TitleFont2
        {
            "1"
            {
                "name"		"HalfLife2"
                "tall"		"120"
                "weight"	"400"
                "antialias"	"1"
                "custom"	"1"
            }
        }
        
        StatsTitle
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "weight"		"2000"
                "tall"			"20"
                "antialias"		"1"
            }
        }
        
        StatsText
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "weight"		"2000"
                "tall"			"18"
                "antialias"		"1"
            }
        }
        
        AchievementItemTitle
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "weight"		"1500"
                "tall"			"16" [!$POSIX]
                "tall"			"18" [$POSIX]
                "antialias"		"1"
            }
        }

        AchievementItemDate
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "weight"		"1500"
                "tall"			"16"
                "antialias"		"1"
            }
        }

        
        StatsPageText
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "weight"		"1500"
                "tall"			"14" [!$POSIX]
                "tall"			"16" [$POSIX]
                "antialias"		"1"
            }
        }
        
        AchievementItemTitleLarge
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana Bold" [$POSIX]
                "weight"		"1500"
                "tall"			"18" [!$POSIX]
                "tall"			"19" [$POSIX]
                "antialias"		"1"
            }
        }
        
        AchievementItemDescription
        {
            "1"
            {
                "name"		"Arial" [!$POSIX]
                "name"		"Verdana" [$POSIX]
                "weight"		"1000"
                "tall"			"14" [!$POSIX]
                "tall"			"15" [$POSIX]
                "antialias"		"1"
            }
        }

        
        "ServerBrowserTitle"
        {
            "1"
            {
                "name"		"Tahoma" [!$POSIX]
                "name"		"Verdana" [$POSIX]
                "tall"		"35"
                "tall_lodef"	"40"
                "weight"	"500"
                "additive"	"0"
                "antialias" "1"
            }
        }

        "ServerBrowserSmall"
        {
            "1"
            {
                "name"		"Tahoma"
                "tall"		"16"
                "weight"	"0"
                "range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
                "yres"	"480 599"
            }
            "2"
            {
                "name"		"Tahoma"
                "tall"		"16"
                "weight"	"0"
                "range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
                "yres"	"600 767"
            }
            "3"
            {
                "name"		"Tahoma"
                "tall"		"16"
                "weight"	"0"
                "range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
                "yres"	"768 1023"
                "antialias"	"1"
            }
            "4"
            {
                "name"		"Tahoma"
                "tall"		"19"
                "weight"	"0"
                "range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
                "yres"	"1024 1199"
                "antialias"	"1"
            }
            "5"
            {
                "name"		"Tahoma"
                "tall"		"19"
                "weight"	"0"
                "range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
                "yres"	"1200 6000"
                "antialias"	"1"
            }
        }
    
    }

    //
    //////////////////// BORDERS //////////////////////////////
    //
    // describes all the border types
    Borders
    {
        BaseBorder		SubtleBorder
        ButtonBorder	FrameBorder
        ComboBoxBorder	DepressedBorder
        MenuBorder		FrameBorder
        BrowserBorder	DepressedBorder
        PropertySheetBorder	FrameBorder
        ButtonKeyFocusBorder FrameBorder // this is the border used for default buttons (the button that gets pressed when you hit enter)
        ButtonDepressedBorder FrameBorder
        CheckButtonBorder FrameBorder
        ListPanelColumnButtonBorder FrameBorder
        PropertySheetBorder FrameBorder

        FrameBorder
        {
            // rounded corners for frames
            //"backgroundtype" "2"

            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }

        DividerBorder
        {
            // rounded corners for frames
            //"backgroundtype" "2"

            Left
            {
                "1"
                {
                    "color" "MomGreydientStep3"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "MomGreydientStep3"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "MomGreydientStep3"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "MomGreydientStep3"
                    "offset" "0 0"
                }
            }
        }

        SubtleBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }
        }

        DepressedBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }
        RaisedBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.Subtle"
                    "offset" "0 0"
                }
            }
        }
        
        TitleButtonBorder
        {
            "backgroundtype" "0"
            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }

        TitleButtonDisabledBorder
        {
            "backgroundtype" "0"
        }

        TitleButtonDepressedBorder
        {
            "backgroundtype" "0"
        }

        ScrollBarButtonBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }
        
        ScrollBarButtonDepressedBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }

        TextEntryBorder
        {
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }

        TabBorder
        {
            "inset" "0 0 1 1"
            Left
            {
                "1"
                {
                    "color" "Border.Bright"
                    "offset" "0 1"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.Dark"
                    "offset" "1 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.Bright"
                    "offset" "0 0"
                }
            }
        }

        TabActiveBorder
        {
            "inset" "1 1 1 1"
            Left
            {
                "1"
                {
                    "color" "MomentumBlue"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "MomentumBlue"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "MomentumBlue"
                    "offset" "0 0"
                }
            }

        }


        ToolTipBorder
        {
            "inset" "0 0 1 0"
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Top
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }

            Bottom
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 0"
                }
            }
        }
    }

    //////////////////////// CUSTOM FONT FILES /////////////////////////////
    //
    // specifies all the custom (non-system) font files that need to be loaded to service the above described fonts
    CustomFontFiles
    {
        "1"		"resource/HALFLIFE2.ttf"
        "2"		"resource/font/HL2EP2.ttf"
        "3"     "resource/marlett.ttf"
        "1"     "resource/font/SourceCodePro-Regular.ttf"
        "1"     "resource/font/NotoSans-Regular.ttf"
        "1"     "resource/font/BebasNeue-Regular.ttf"
    }
}
