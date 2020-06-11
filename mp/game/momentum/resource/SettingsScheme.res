#base "SourceScheme.res"
Scheme
{
    Colors
    {

    }

    BaseSettings
    {
        CheckButton.CheckFont   "CheckMarlett"
        CheckButton.TextInset   "6"
        CheckButton.CheckInset  "0"

        ComboBoxButton.Font     "ComboMarlett"
 
        Frame.OutOfFocusBgColor			"MomGreydientStep3"
        
        SectionedListPanel.DividerColor "MomGreydientStep3"
        SectionedListPanel.Font         "Default12"
        SectionedListPanel.HeaderFont   "Default12"
    }

    Fonts
    {
        CheckMarlett
        {
            "1"
            {
                "name" "Marlett"
                "tall" "18"
                "weight" "0"
                "symbol"	"1"
                "dropshadow" "1"
            }
        }

        ComboMarlett
        {
            "1"
            {
                "name" "Marlett"
                "tall" "10"
                "weight" "0"
                "symbol"	"1"
                "dropshadow" "1"
            }
        }

        FrameBarBigBoi
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "30"
                "antialias" "1"
                "dropshadow" "1"
            }
        }

        Subheader
        {
            "1"
            {
                "name" "Bebas Neue"
                "tall" "22"
                "antialias" "1"
                "dropshadow" "1"
            }
        }

        Default
        {
            "1"
            {
                "tall" "14"
            }
        }
    }

    Borders
    {
        CurrentSettingsBorder
        {
            // rounded corners for frames
            //"backgroundtype" "2"

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