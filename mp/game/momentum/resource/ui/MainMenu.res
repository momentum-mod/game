"resource/ui/MainMenu.res"
{
    "CMOMMenuOverride"
    {
        "controlName" "EditablePanel"
        "fieldName" "CMOMMenuOverride"
        "xpos" "0"
        "ypos" "0"
        //wide and tall are set in code
    }
    
    "GameLogo"
    {
        "controlName" "ImagePanel"
        "fieldName" "GameLogo"
        "xpos" "cs-0.5"//DO NOT TOUCH! This auto-centers it based on width.
        "ypos" "20"
        //"proportionalToParent" "1"
        "wide" "500"
        "tall" "125"
        "image" "logo"
        "scaleImage" "1"
    }
    
    "MenuButtonsBackground"
    {
        "controlName" "EditablePanel"
        "fieldName" "MenuButtonsBackground"
        "xpos" "c-125"
        "ypos" "c-20"
        "enabled" "1"
        "visible" "1"
        "wide" "250"
        "tall" "300"
        "PaintBackgroundType" "2"
        "bgcolor_override" "MOM.Panel.Bg"
        "RoundedCorners" "15"
    }
    
    "TestLabel"
    {
        "controlName" "Label"
        "fieldName" "TestLabel"
        "xpos" "cs-0.5"
        "ypos" "c0"
        "enabled" "1"
        "visible" "1"
        "wide" "200"
        "tall" "25"
        //"proportionalToParent" "1"
        "labelText" "#MOM_MapSelector_MainMenuOption"
        "font" "MainMenuButton"
        "auto_wide_tocontents" "0"
        "textAlignment" "center"
    }

}