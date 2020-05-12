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
        // base colors
        "White"				"255 255 255 255"
        "Light Gray"		"211 211 211 20"
        "Gray" 				"128 128 128 150"
        "Dark Gray" 		"64 64 64 200"
        
        "Red"                "192 28 0 140"
        "Black"                "0 0 0 196"
        "TransparentBlack"    "0 0 0 196"
        "TransparentLightBlack"    "0 0 0 90"
    
        "Blank"                "0 0 0 0"

        "MOM.Panel.Fg"				"255 255 255 125"
        "MOM.Panel.Bg"				"211 211 211 50"

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
        // vgui_controls color specifications
        Frame.TitleTextInsetX			8
        Frame.ClientInsetX				8
        Frame.ClientInsetY				2
        Frame.FocusTransitionEffectTime	"0.3"					// time it takes for a window to fade in/out on focus/out of focus
        Frame.TransitionEffectTime		"0.3"					// time it takes for a window to fade in/out on open/close
        Frame.AutoSnapRange				"0"
        Menu.TextInset					"6"

        ScrollBar.Wide					"10"
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
                "tall"		"10"
                "weight"	"500"
                "antialias" "1"
            }
        }
        // fonts are used in order that they are listed
        "DebugFixedSmall"
        {
            "1"
            {
                "name"		"Courier New"
                "tall"		"7"
                "weight"	"500"
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
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"16"
                "weight"	"500"
                "antialias" "1"
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
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"12" [!$LINUX]
                "tall"		"16" [$LINUX]
                "weight"	"0"
                "antialias" "1"
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
        "DefaultVerySmall"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"11"
                "weight"	"0"
                "antialias" "1"
            }
        }

        "DefaultLarge"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"18"
                "weight"	"0"
                "antialias" "1"
            }
        }

        "DefaultLargeDropShadow"
        {
            "1"
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
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
                "name" "Tahoma" [!$LINUX]
                "name" "Verdana" [$LINUX]
                "tall" "22"
                "antialias" "1"
            }
        }
        
        "UiBold"
        {
            "1"	[$WIN32]
            {
                "name"		"Tahoma" [!$LINUX]
                "name"		"Verdana" [$LINUX]
                "tall"		"12"
                "weight"	"1000"
            }
            "1"	[$X360]
            {
                "name"		"Tahoma"
                "tall"		"24"
                "weight"	"2000"
                "outline"	"1"
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
        
        "ConsoleText"
        {
            "1"
            {
                "name"		 "Source Code Pro"
                "tall"		"10"
                "antialias" "1"
            }
        }
        
        "ConsoleEntryTextFont"
        {
            "1"
            {
                "name" "Source Code Pro"
                "tall" "14"
                "antialias" "1"
            }
        }

        "ConsoleCompletionListFont"
        {
            "1"
            {
                "name" "Source Code Pro"
                "tall" "14"
                "antialias" "1"
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
    }

    //
    //////////////////// BORDERS //////////////////////////////
    //
    // describes all the border types
    Borders
    {
        BaseBorder		FrameBorder
        ButtonBorder	RaisedBorder
        ComboBoxBorder	DepressedBorder
        MenuBorder		SubtleBorder
        BrowserBorder	DepressedBorder
        PropertySheetBorder	RaisedBorder

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

        TabBorder
        {
            "inset" "1 1 1 1"

            render 
            {
                "0" "fill( x0, y0, x1, y1, Black )"
            }

            render_bg 
            {
                "0" "fill( x0, y0, x1, y1, Orange )"
            }

        }

        TabActiveBorder
        {
            "inset" "1 1 1 1"
            Left
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

            Right
            {
                "1"
                {
                    "color" "Border.Subtle"
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

        // this is the border used for default buttons (the button that gets pressed when you hit enter)
        ButtonKeyFocusBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.Selection"
                    "offset" "0 0"
                }
            }
            Top
            {
                "1"
                {
                    "color" "Border.Selection"
                    "offset" "0 0"
                }
            }
            Right
            {
                "1"
                {
                    "color" "Border.Selection"
                    "offset" "0 0"
                }
            }
            Bottom
            {
                "1"
                {
                    "color" "Border.Selection"
                    "offset" "0 0"
                }
            }
        }

        ButtonDepressedBorder
        {
            "inset" "0 0 0 0"
            Left
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "0 1"
                }
            }

            Right
            {
                "1"
                {
                    "color" "Border.DarkSolid"
                    "offset" "1 0"
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
}