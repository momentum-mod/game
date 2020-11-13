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
	//////////////////////// COLORS ///////////////////////////
	Colors
	{
        // original alpha was 225
        "FirstPlace"    "240 210 147 50"
        "SecondPlace"   "175 175 175 50"
        "ThirdPlace"    "205 127 50 50"
        "ItemDefault"   "Blank"
	}
	
	///////////////////// BASE SETTINGS ////////////////////////
	//
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		"Panel.FgColor"			"255 255 255 255"
        "Panel.BgColor"         "MomGreydientHOStep3"

		"MapDescriptionText"	"White" // the text used in the map description window


        Button.TextColor				"White"
		Button.BgColor					"Blank"
		Button.ArmedTextColor			"White"
		Button.ArmedBgColor				"MomentumBlue"
		Button.DepressedTextColor		"White"
		Button.DepressedBgColor			"MomentumBlue"

        
		Label.DisabledFgColor2			"MomentumBlue"
	}
	
	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	Fonts
	{
		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"tall"		"14"
			}
		}
	}

		//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
		LeaderboardsPlayerBorder
		{
			"inset" "0 0 0 0"
			Left
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

			Top
			{
				"1"
				{
					"color" "MomentumBlue"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "MomentumBlue"
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
	}
}