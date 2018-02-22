"Scheme"
{
	"BaseSettings"
	{
        //If this is 1, the logo is text, found in gameui2_english.txt
        "MainMenu.Logo.Text"                            "0"
        //Note: The following is only used if the above is "0"
        "MainMenu.Logo.Image"                           "menu/logo"//This is based in materials/vgui !!
        "MainMenu.Logo.Image.Width"                     "350"
        "MainMenu.Logo.Image.Height"                    "75"
        
        //Note: These are used for position if the logo is not attached to the menu
		"MainMenu.Logo.OffsetX"							"2"
		"MainMenu.Logo.OffsetY"							"2"

        //Makes the Logo (text OR image) stick to the menu
		"MainMenu.Logo.AttachToMenu"					"0"
	
		//"MainMenu.Logo.PlayerCount"						"1"

		"MainMenu.Buttons.OffsetX"						"32"
		"MainMenu.Buttons.OffsetY"						"32"
		"MainMenu.Buttons.Space"						"0"
	
		"MainMenu.Button.Width.Out"						"384"
		"MainMenu.Button.Width.Over"					"384"
		"MainMenu.Button.Width.Pressed"					"384"
		"MainMenu.Button.Width.Released"				"384"

		"MainMenu.Button.Height.Out"					"74"
		"MainMenu.Button.Height.Over"					"74"
		"MainMenu.Button.Height.Pressed"				"74"
		"MainMenu.Button.Height.Released"				"74"

		"MainMenu.Button.Text.OffsetX"					"10"
		"MainMenu.Button.Text.OffsetY"					"0"

		"MainMenu.Button.Description.OffsetX"			"1"
		"MainMenu.Button.Description.OffsetY"			"-9"

		"MainMenu.Button.Description.Hide.Out"			"1"
		"MainMenu.Button.Description.Hide.Over"			"0"
		"MainMenu.Button.Description.Hide.Pressed"		"0"
		"MainMenu.Button.Description.Hide.Released"		"1"

		"MainMenu.Button.Background.Blur.Out"			"0"
		"MainMenu.Button.Background.Blur.Over"			"0"
		"MainMenu.Button.Background.Blur.Pressed"		"0"
		"MainMenu.Button.Background.Blur.Released"		"0"
        
        "MainMenu.Button.Animation.Width"				"0.15"
		"MainMenu.Button.Animation.Height"				"0.25"
		"MainMenu.Button.Animation.Background"			"0.2"
		"MainMenu.Button.Animation.Text"				"0.2"
		"MainMenu.Button.Animation.Description"			"0.3"
        
        //Sounds
        "MainMenu.Sound.Open" "ui/menu_open.wav"
        "MainMenu.Sound.Close" "ui/menu_close.wav"
        
        "MainMenu.Button.Sound.Armed" "ui/button_over.wav"
        "MainMenu.Button.Sound.Depressed" "ui/button_click.wav"
        "MainMenu.Button.Sound.Released" "ui/button_release.wav"
	}

	"Colors"
	{
        "White" "255 255 255 255"
        "Light Blue" "51 122 183 255"
        "Dark Blue" "22 69 119 255"
    
		"MainMenu.Logo.Left"							"255 255 255 255"
		"MainMenu.Logo.Right"							"51 122 183 255"//"255 217 73 255"

		"MainMenu.Logo.PlayerCount.Color"				"255 255 255 255"
		
		"MainMenu.Button.Background.Out"				"0 0 0 0"
		"MainMenu.Button.Background.Over"				"0 0 0 150"
		"MainMenu.Button.Background.Pressed"			"20 20 20 255"
		"MainMenu.Button.Background.Released"			"0 0 0 150"

		"MainMenu.Button.Background.Outline.Out"		"0 0 0 0"
		"MainMenu.Button.Background.Outline.Over"		"255 255 255 2"
		"MainMenu.Button.Background.Outline.Pressed"	"255 255 255 2"
		"MainMenu.Button.Background.Outline.Released"	"255 255 255 2"

		"MainMenu.Button.Text.Out"						"255 255 255 255"
		"MainMenu.Button.Text.Over"						"51 122 183 255"//"255 217 73 255"
		"MainMenu.Button.Text.Pressed"					"22 69 119 255"
		"MainMenu.Button.Text.Released"					"51 122 183 255"

		"MainMenu.Button.Description.Out"				"0 0 0 0"
		"MainMenu.Button.Description.Over"				"180 180 180 200"
		"MainMenu.Button.Description.Pressed"			"180 180 180 200"
		"MainMenu.Button.Description.Released"			"180 180 180 200"
	}
	
	"Fonts"
	{
		"MainMenu.Logo.Font"
		{
			"settings"
			{
				"name"			"BigNoodleTitling" //"Facile Sans"
				"tall"			"65"
				"weight"		"400"
				"antialias"		"1"
			}
		}
		
		"MainMenu.Button.Text.Font"
		{
			"settings"
			{
				"name"			"BigNoodleTitling" //"Nokia Pure Text Regular" // Roboto Medium
				"tall"			"25"
				"weight"		"50"
				"antialias"		"1"
			}
		}

		"MainMenu.Button.Description.Font"
		{
			"settings"
			{
				"name"			"Noto Sans"
				"tall"			"16"
				"weight"		"400"
				"antialias"		"1"
			}
		}
		"MainMenu.Logo.PlayerCount.Font"
		{
			"settings"
			{
				"name"			"BigNoodleTitling" //"Facile Sans"
				"tall"			"18"
				"weight"		"300"
				"antialias"		"1"
				"dropshadow"	"1"
			}
		}
        
        "MainMenu.VersionLabel.Font"
        {
            "settings"
            {
                "name" "BigNoodleTitling"
                "tall" "20"
                "weight" "300"
                "antialias" "1"
            }
        }
	}

	"CustomFontFiles"
	{
		"file"	"resource/font/NotoSans-Regular.ttf" // Noto Sans
        "file" "resource/font/BigNoodleTitling.ttf"
	}
}