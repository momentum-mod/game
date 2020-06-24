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
    // color details
    // this is a list of all the colors used by the scheme
    Colors
    {
        Console.TextColor			"White"
        Console.DevTextColor		"MomentumBlue"
        Console.UserInputColor  "MomentumBlue"
    }

    ///////////////////// BASE SETTINGS ////////////////////////
    //
    // default settings for all panels
    // controls use these to determine their settings
    BaseSettings
    {
        RichText.Border "FrameBorder"
    }

    //////////////////////// FONTS /////////////////////////////
    //
    // describes all the fonts
    Fonts
    {
        "ConsoleText"
        {
            "1"
            {
                "name"		 "Source Code Pro"
                "tall"		"10"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
        
        "ConsoleEntryTextFont"
        {
            "1"
            {
                "name" "Source Code Pro"
                "tall" "14"
                "antialias" "1"
                "dropshadow" "1"
            }
        }

        "ConsoleCompletionListFont"
        {
            "1"
            {
                "name" "Source Code Pro"
                "tall" "14"
                "antialias" "1"
                "dropshadow" "1"
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
    }

    //
    //////////////////// BORDERS //////////////////////////////
    //
    // describes all the border types
    Borders
    {
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
                    "color" "0 0 0 0"
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
}