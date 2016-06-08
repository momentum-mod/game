"resource/ui/MapFinishedDialog.res"
{
	"Replay_Icon"
	{
		"ControlName"	"ImagePanel"
		"fieldName"		"Replay_Icon"
		"xpos"			"180"
		"ypos"			"80"
		"wide"			"64"
		"tall"			"64"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"border"		""
		"image"			"replay_icon"
	}
    
    "Replay_Label"
	{
		"ControlName"	"Label"
		"fieldName"		"Replay_Label"
		"font"			"Default"//Set by "TextFont" in HudLayout.res
		"xpos"			"175"
		"ypos"			"110"
		"wide"			"100"
		"tall"			"0"//Set by font size
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"labelText"		"#MOM_PlayReplay"
		"textAlignment"	"west"
		"dulltext"		"0"
		"brighttext"	"0"
	}
	
	styles
	{
		status
		{
			bgcolor="PropertySheetBG"
			inset="8 0 0 0"
		}
	}
}
