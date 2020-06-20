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
#base "SourceScheme.res"
Scheme
{
	//////////////////////// COLORS ///////////////////////////
	Colors
	{
		"MOM.Speedometer.Normal"	"200 200 200 255"
		"MOM.Speedometer.Increase"	"24 150 211 255"
		"MOM.Speedometer.Decrease"	"255 106 106 255"
	}
	
	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		"FgColor"			"255 255 255 255"
		"BgColor"			"0 0 0 76"
		
		"BrightFg"		"128 255 255 255"

		"DamagedBg"			"180 0 0 200"
		"DamagedFg"			"180 0 0 230"
		"BrightDamagedFg"		"255 0 0 255"

		// weapon selection colors
		"SelectionNumberFg"		"255 255 255 255"
		"SelectionTextFg"		"255 255 255 255"
		"SelectionEmptyBoxBg" 	"32 0 0 128"
		"SelectionBoxBg" 		"0 0 0 80"
		"SelectionSelectedBoxBg" "0 0 0 128"
		
		"ZoomReticleColor"	"255 0 0 255"

		// HL1-style HUD colors
		"Yellowish"			"0 128 255 255"
		"Normal"			"0 80 255 255"
		"Caution"			"255 48 0 255"

		"ViewportBG"		"Blank"
		"team0"			"204 204 204 255" // Spectators
		"team1"			"255 64 64 255" // CT's
		"team2"			"153 204 255 255" // T's

		"MapDescriptionText"	"White" // the text used in the map description window
		"CT_Blue"			"153 204 255 255"
		"T_Red"				"255 64 64 255"
		"Hostage_Yellow"	"Panel.FgColor"
		"HudIcon_Green"		"0 160 0 255"
		"HudIcon_Red"		"160 0 0 255"

		// CHudMenu/CHudStaticMenu
		"ItemColor"		"MomentumBlue"
		"MenuColor"		"233 208 173 255"
		"MenuBoxBg"		"0 0 0 100"

		// Hint message colors
		"HintMessageFg"			"255 255 255 255"
		"HintMessageBg" 		"0 0 0 60"

        "HudMessage.Font" "Default14"
	}
	
	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	Fonts
	{
		MomentumIcons
		{
			"1"
			{
				"name"		"MomIcons"
				"tall"		"12"
				"weight"	"0"
				"antialias" "1"
				"custom"	"1"
                "dropshadow" "1"
			}
		}
		WeaponIcons
		{
			"1"
			{
				"name"		"MomIcons"
				"tall"		"64"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		WeaponIconsSelected
		{
			"1"
			{
				"name"		"MomIcons"
				"tall"		"64"
				"weight"	"0"
				"antialias" "1"
				"blur"		"5"
				"scanlines"	"2"
				"additive"	"1"
				"custom"	"1"
			}
		}
        WeaponIconsSmall
		{
			"1"
			{
				"name"		"MomIcons"
				"tall"		"32"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		Crosshairs
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"40"
				"weight"	"0"
				"antialias" "0"
				"additive"	"1"
				"custom"	"1"
				"yres"		"1 10000"
			}
		}
		QuickInfo
		{
			"1"
			{
				"name"		"HL2cross"
				"tall"		"28"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		SquadIcons
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"32"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		HudNumbers
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"32"
				"weight"	"400"
				"antialias" "1"
                "dropshadow" "1"
			}
		}
		HudNumbersGlow
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"32"
				"weight"	"0"
				"blur"		"4"
				"scanlines" "2"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudNumbersSmall
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"16"
				"weight"	"700"
				"antialias" "1"
                "dropshadow" "1"
			}
		}
		HudNumbersSmallBold
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"16"
				"weight"	"700"
				"antialias" "1"
                "dropshadow" "1"
			}
		}
		HudNumbersVerySmall
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
                "dropshadow" "1"
			}
		}
		HudNumbersExtremelySmall
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"10"
				"weight"	"700"
				"antialias" "1"
                "dropshadow" "1"
			}
		}
		HudNumbersSmallGlow
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"16"
				"weight"	"700"
				"blur"		"2" //4
				"scanlines" "1" //2
				"additive"	"1"
				"antialias" "1"
			}
		}
		HudSelectionNumbers
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"11"
				"weight"	"700"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudHintTextLarge
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"14"
				"weight"	"700"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudHintTextSmall
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"11"
				"weight"	"400"
				"antialias" "1"
				"additive"	"1"
			}
		}
		HudSelectionText
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"10"
				"weight"	"700"
				"antialias" "1"
			}
		}
		BudgetLabel
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"400"
				"outline"	"1"
			}
		}
		DebugOverlay
		{
			"1"
			{
				"name"		"Courier New"
				"tall"		"14"
				"weight"	"400"
				"outline"	"1"
			}
		}
		"CloseCaption_Normal"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"26"
				"weight"	"500"
			}
		}
		"CloseCaption_Italic"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"26"
				"weight"	"500"
				"italic"	"1"
			}
		}
		"CloseCaption_Bold"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"26"
				"weight"	"900"
			}
		}
		"CloseCaption_BoldItalic"
		{
			"1"
			{
				"name"		"Tahoma"
				"tall"		"26"
				"weight"	"900"
				"italic"	"1"
			}
		}

		"Marlett"
		{
			"1"
			{
				"tall"		"14"
			}
		}

		"Trebuchet24"
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"22"
				"weight"	"700"
				"range"		"0x0000 0x007F"	//	Basic Latin
				"antialias" "1"
				"additive"	"1"
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
		
		ClientVoteSystem
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"25"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}

        ClientTitleFont
		{
			"1"
			{
				"name"		"Bebas Neue"
				"tall"		"50"
				"weight"	"0"
				"antialias" "1"
				"blur"		"2"
				"scanlines"	"2"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsLogo
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"128"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsText
		{
			"1"
			{
				"name"		"Trebuchet MS"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"additive"	"1"
			}
		}
		CreditsOutroLogos
		{
			"1"
			{
				"name"		"HalfLife2"
				"tall"		"48"
				"weight"	"0"
				"antialias" "1"
				"additive"	"1"
				"custom"	"1"
			}
		}
		CreditsOutroText
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"10"
				"weight"	"900"
				"antialias" "1"
			}
		}
        CreditsOutroTextItalic
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"11"
				"weight"	"700"
                "italic" "1"
				"antialias" "1"
			}
		}
        CreditsOutroTextBold
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"13"
				"weight"	"700"
				"antialias" "1"
			}
		}
		CreditsOutroTextLarge
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"16"
				"weight"	"900"
				"antialias" "1"
			}
		}
		CreditsOutroTextExtraLarge
		{
			"1"
			{
				"name"		"Noto Sans"
				"tall"		"48"
				"weight"	"400"
				"antialias" "1"
			}
		}
	}

		//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
		
	}

	
	//////////////////////// CUSTOM FONT FILES /////////////////////////////
	//
	// specifies all the custom (non-system) font files that need to be loaded to service the above described fonts
	CustomFontFiles
	{
		"1"		"resource/HALFLIFE2.ttf"
		"1"		"resource/font/MomIcons.ttf"
		"3"		"resource/HL2crosshairs.ttf"
	}

}
