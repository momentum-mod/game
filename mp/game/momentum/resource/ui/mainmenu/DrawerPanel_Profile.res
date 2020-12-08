"resource/ui/mainmenu/DrawerPanel_Profile.res"
{
    "DrawerPanel_Profile"
    {
        "ControlName"		"PropertyPage"
        "fieldName"		"DrawerPanel_Profile"
        "proportionalToParent" "1"
        // pos and size are set in code
        "autoResize"	"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"0"
        "tabPosition"		"0"
        "PaintBackgroundType" "2"
    }

    "UserComponent"
    {
        "ControlName"		"Panel"
		"fieldName"		"UserComponent"
		"proportionalToParent"		"1"
        "xpos" "8"
        "ypos" "8"
        "wide" "f20"
        "tall" "36"
        "paintborder" "1"
        "border" "FrameBorder"
    }

    "StatsAndActivity"
    {
        "ControlName"		"PropertySheet"
		"fieldName"		"StatsAndActivity"
		"proportionalToParent"		"1"
		"xpos"		"0"
		"ypos"		"8"
		"zpos"		"0"
		"wide"		"f20"
		"tall"		"f60"
		"autoResize"		"0"
		"pinCorner"		"0"
		"PinnedCornerOffsetX"		"0"
		"PinnedCornerOffsetY"		"0"
		"UnpinnedCornerOffsetX"		"0"
		"UnpinnedCornerOffsetY"		"0"
		"RoundedCorners"		"15"
        "pin_to_sibling" "UserComponent"
        "pin_to_sibling_corner" "2"
        "pin_corner_to_sibling" "0"
		"actionsignallevel"		"-1"
		"visible"		"1"
		"enabled"		"1"
		"paintbackground"		"1"
		"mouseinputenabled"		"1"
		"keyboardinputenabled"		"1"
		"IgnoreScheme"		"0"
		"tabPosition"		"1"
        // "bgcolor_override" "MomGreydientStep1"
        "tabheight" "16"
        "tabwidth" "54"
        "tabxfittotext" "0"
        "tabskv"
        {
            "textinsety" "4"
            "textAlignment" "center"
            "font" "Titling14"
        }
    }
}