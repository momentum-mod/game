#base "SourceScheme.res"

"Scheme"
{
	"Colors"
	{

	}

	"BaseSettings"
	{
        //If this is 1, the logo is text, found in gameui2_english.txt
        "MainMenu.Logo.Text"                            "0"
        //Note: The following is only used if the above is "0"
        "MainMenu.Logo.Image"                           "menu/MomentumTitle"//This is based in materials/vgui !!
        "MainMenu.Logo.Image.Width"                     "400"
        "MainMenu.Logo.Image.Height"                    "100"
        
        //Note: These are used for position if the logo is not attached to the menu
		"MainMenu.Logo.OffsetX"							"10"
		"MainMenu.Logo.OffsetY"							"0"

        //Makes the Logo (text OR image) stick to the menu
		"MainMenu.Logo.AttachToMenu"					"0"

		"MainMenu.Buttons.OffsetX"						"16"
		"MainMenu.Buttons.OffsetY"						"16"
		"MainMenu.Buttons.Space"						"4"
	
		"MainMenu.Button.Width.Out"						"180"
		"MainMenu.Button.Width.Over"					"180"
		"MainMenu.Button.Width.Pressed"					"180"
		"MainMenu.Button.Width.Released"				"180"

		"MainMenu.Button.Height.Out"					"28"
		"MainMenu.Button.Height.Over"					"28"
		"MainMenu.Button.Height.Pressed"				"28"
		"MainMenu.Button.Height.Released"				"28"

		"MainMenu.Button.Text.OffsetX"					"8"
		"MainMenu.Button.Text.OffsetY"					"1"
        
        // Length (in seconds) of the animations
        "MainMenu.Button.Animation.Width"				"0.15"
		"MainMenu.Button.Animation.Height"				"0.25"
		"MainMenu.Button.Animation.Background"			"0.2"
		"MainMenu.Button.Animation.Text"				"0.2"
		"MainMenu.Button.Animation.Description"			"0.3"
        
        // Color settings
        "MainMenu.Logo.Left"							"255 255 255 255"
		"MainMenu.Logo.Right"							"51 122 183 255"
		
        "MainMenu.Button.Background.Out"                "0 0 0 128"
        "MainMenu.Button.Background.Over"                "MomentumBlue"
        "MainMenu.Button.Background.Pressed"            "20 20 20 255"
        "MainMenu.Button.Background.Released"            "255 255 255 150"

        "MainMenu.Button.Background.Outline.Out"            "0 0 0 0"
        "MainMenu.Button.Background.Outline.Over"        "255 255 255 2"
        "MainMenu.Button.Background.Outline.Pressed"        "255 255 255 2"
        "MainMenu.Button.Background.Outline.Released"        "255 255 255 2"

        "MainMenu.Button.Text.Out"                        "White"
        "MainMenu.Button.Text.Over"                       "White"
        "MainMenu.Button.Text.Pressed"                    "White"
        "MainMenu.Button.Text.Released"                    "White"

        // Sound settings
        "MainMenu.Sound.Open" "ui/menu_open.wav"
        "MainMenu.Sound.Close" "ui/menu_close.wav"
        
        "MainMenu.Button.Sound.Armed" "ui/button_over.wav"
        "MainMenu.Button.Sound.Depressed" "ui/button_click.wav"
        "MainMenu.Button.Sound.Released" "ui/button_release.wav"
	}
	
	"Fonts"
	{
		"MainMenu.Logo.Font"
		{
			"settings"
			{
				"name"			"Bebas Neue"
				"tall"			"65"
				"weight"		"400"
				"antialias"		"1"
			}
		}
		
		"MainMenu.Button.Text.Font"
		{
			"settings"
			{
				"name"			"Bebas Neue"
				"tall"			"22"
				"antialias"		"1"
                "dropshadow" "1"
			}
		}
        
        "MainMenu.VersionLabel.Font"
        {
            "settings"
            {
                "name" "Bebas Neue"
                "tall" "20"
                "antialias" "1"
                "dropshadow" "1"
            }
        }
	}

	"CustomFontFiles"
	{
	}
}