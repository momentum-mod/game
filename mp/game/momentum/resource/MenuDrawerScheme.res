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
    }

    ///////////////////// BASE SETTINGS ////////////////////////
    //
    // default settings for all panels
    // controls use these to determine their settings
    BaseSettings
    {
        Button.BgColor					"MomGreydientStep3"
        Button.ArmedTextColor			"MomentumBlue"
        Button.ArmedBgColor				"MomGreydientStep3"

        ListPanelHeader.BgColor "MomGreydientStep2"
        ListPanelHeader.Font "Default12"

        ListPanel.BgColor "MomGreydientStep2"
        ListPanel.AlternatingColors "1"
        ListPanel.AlternatingColor1 "MomGreydientStep4"
        ListPanel.AlternatingColor2 "MomGreydientStep3"

        PropertySheet.TextColor			"WhiteLO"
        PropertySheet.SelectedTextColor	"White"
        PropertySheet.ActiveTabBgColor "MomGreydientStep3"
        PropertySheet.InactiveTabBgColor "MomGreydientStep2"
        PropertySheet.TransitionEffectTime	"0"	// time to change from one tab to another
        PropertySheet.TabFont "Titling14"
    }

    //////////////////////// FONTS /////////////////////////////
    //
    // describes all the fonts
    Fonts
    {
        "MapListFont"
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "20"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
    }

    //
    //////////////////// BORDERS //////////////////////////////
    //
    // describes all the border types
    Borders
    {
        BrowserBorder	DepressedBorder

        GalleryNavButton.DefaultBorder RaisedBorder
        GalleryNavButton.KeyFocusBorder RaisedBorder
        GalleryNavButton.DepressedBorder RaisedBorder

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

        DrawerHandleBorder
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
    }
}
