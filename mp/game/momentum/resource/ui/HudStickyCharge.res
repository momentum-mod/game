"Resource/ui/HudStickyCharge.res"
{
    "HudStickyCharge"
    {
        "ControlName" "CHudStickyCharge"
        "fieldName" "HudStickyCharge"
        "visible"		"1"
        "enabled"		"1"
        "xpos"          "c-60"
        "ypos"          "c+90"
        "wide"			"120"
        "tall"          "15"
        "paintbackground" "0"
        "bgcolor_override" "0 0 0 0"
    }

    "ChargeMeter"
    {	
        "ControlName"	"ContinuousProgressBar"
        "fieldName"		"ChargeMeter"
        "font"			"DefaultVerySmall"
        "xpos"			"0"
        "ypos"			"0"
        "wide"			"120"
        "tall"			"9"				
        "autoResize"	"0"
        "visible"		"1"
        "enabled"		"1"
        "textAlignment"	"center"
        "dulltext"		"0"
        "brighttext"	"0"
        "bgcolor_override" "0 0 0 100"
    }
    "ChargeMeterLabel"
    {
        "ControlName"       "Label"
        "fieldName"         "ChargeMeterLabel"
        "xpos"              "0"
        "ypos"              "1"
        "wide"              "120"
        "visible"           "1"
        "enabled"           "1"
        "tabPosition"       "0"
        "labelText"         "CHARGE"
        "textAlignment"     "center"
        "dulltext"          "0"
        "brighttext"        "0"
        "pin_to_sibling" "ChargeMeter"
        "pin_to_sibling_corner" "0"
        "pin_corner_to_sibling" "0"
        "font"              "HudNumbersExtremelySmall"
        "fgcolor_override" 	"150 150 150 255"
        "auto_tall_tocontents" "1"
    }
}
