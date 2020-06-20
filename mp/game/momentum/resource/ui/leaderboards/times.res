"resource/ui/leaderboards/times.res"
{
	"OnlineTimesStatus"
	{
		"ControlName"	"Label"
		"fieldName"		"OnlineTimesStatus"
		"xpos"			"0"
		"ypos"			"-110"
        "zpos" "20"
		"wide"			"0"
		"tall"			"0"
		"visible"		"0"
		"enabled"		"1"
		"labelText"		"#MOM_API_WaitingForResponse"
		"textAlignment"	"center"
		"dulltext"		"0"
		"brighttext"	"1"
		"font"			"Titling30"
		"wrap"			"0"
		"centerwrap"	"0"
		"textinsetx"	"0"
		"textinsety"	"0"
		"auto_wide_tocontents" "1"
        "auto_tall_tocontents" "1"
        "proportionalToParent" "1"
        "pin_to_sibling" "Top10Leaderboards"
        "pin_to_sibling_corner" "6"
        "pin_corner_to_sibling" "6"
	}

    "LocalLeaderboardsButton"
    {
        "ControlName" "Button"
        "fieldName" "LocalLeaderboardsButton"
        "xpos" "3"
        "ypos" "0"
        "wide"		"88"
		"tall"		"16"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"0"
		"tabPosition"		"0"
		"labelText"		"#MOM_Leaderboards_Local"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
        "command" "ShowLocal"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"Font"			"Titling18"
        "actionsignallevel" "1"
        "mouseinputenabled" "1"
    }
	"GlobalLeaderboardsButton"
	{
		"ControlName"		"Button"
		"fieldName"		"GlobalLeaderboardsButton"
		"xpos"		"3"
		"ypos"		"0"
		"wide"		"88"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"2"
		"RoundedCorners"		"15"
        "pin_to_sibling" "LocalLeaderboardsButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"1"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Leaderboards_Global"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
        "command" "ShowGlobal"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"Font"			"Titling18"
        "actionsignallevel" "1"
        "mouseinputenabled" "1"
	}
	"GlobalTop10Button"
	{
		"ControlName"		"Button"
		"fieldName"		"GlobalTop10Button"
		"xpos"		"6"
		"ypos"		"0"
		"wide"		"88"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"2"
		"RoundedCorners"		"15"
        "pin_to_sibling" "GlobalLeaderboardsButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"1"
		"visible"		"0"
		"enabled"		"0"
		"tabPosition"		"0"
		"labelText"		"#MOM_Leaderboards_Top10"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
        "command" "GlobalTypeTop10"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"Font"			"Titling18"
        "actionsignallevel" "1"
        "mouseinputenabled" "1"
	}
	"GlobalAroundButton"
	{
		"ControlName"		"Button"
		"fieldName"		"GlobalAroundButton"
		"xpos"		"3"
		"ypos"		"0"
		"wide"		"88"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"2"
		"RoundedCorners"		"15"
        "pin_to_sibling" "GlobalTop10Button"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"1"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Leaderboards_Around"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
        "command" "GlobalTypeAround"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"Font"			"Titling18"
        "actionsignallevel" "1"
        "mouseinputenabled" "1"
	}
	"FriendsLeaderboardsButton"
	{
		"ControlName"		"Button"
		"fieldName"		"FriendsLeaderboardsButton"
		"xpos"		"3"
		"ypos"		"0"
		"wide"		"88"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"2"
		"RoundedCorners"		"15"
        "pin_to_sibling" "GlobalAroundButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"1"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Leaderboards_Friends"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
        "command" "ShowFriends"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"Font"			"Titling18"
        "actionsignallevel" "1"
        "mouseinputenabled" "1"
	}
    "FilterButton"
    {
        "ControlName" "ToggleButton"
        "fieldName" "FilterButton"
        "xpos"		"435"
		"ypos"		"2"
		"wide"		"90"
		"tall"		"16"
		"autoResize"		"0"
		"pinCorner"		"2"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"0"
		"enabled"		"0"
		"tabPosition"		"0"
		"labelText"		"#MOM_Leaderboards_Filter"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
        "command" "ShowFilter"
		"centerwrap"		"0"
		"textinsetx"		"6"
		"textinsety"		"0"
		"auto_wide_tocontents"		"1"
		"use_proportional_insets"		"0"
		"Default"		"0"
		"Font"			"Titling18"
        "actionsignallevel" "1"
        "mouseinputenabled" "1"
    }

    "Top10Leaderboards"
	{
		"ControlName"	"SectionedListPanel"
		"fieldName"		"Top10Leaderboards"
        "proportionalToParent" "1"
		"xpos"			"0"
		"ypos"			"3"
		"wide"			"468"
		"tall"			"240"
        "pin_to_sibling" "LocalLeaderboardsButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"	"2"
        "actionsignallevel" "1"
        "paintborder" "1"
        "keyboardinputenabled" "1"
        "mouseinputenabled" "1"
        "vertical_scrollbar" "0" // MOM_TODO: Do we want the player to be able to explore more?
        //"linespacing" "32"
	}
    "AroundLeaderboards"
	{
		"ControlName"	"SectionedListPanel"
		"fieldName"		"AroundLeaderboards"
		"xpos"			"0"
		"ypos"			"3"
		"wide"			"468"
		"tall"			"240"
        "pin_to_sibling" "LocalLeaderboardsButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"	"2"
        "actionsignallevel" "1"
        "paintborder" "1"
        "keyboardinputenabled" "1"
        "mouseinputenabled" "1"
        "vertical_scrollbar" "0" // MOM_TODO: Do we want the player to be able to explore more?
        //"linespacing" "32"
	}
	"LocalLeaderboards"
	{
		"ControlName"	"SectionedListPanel"
		"fieldName"		"LocalLeaderboards"
		"xpos"			"0"
		"ypos"			"3"
		"wide"			"468"
		"tall"			"240"
        "pin_to_sibling" "LocalLeaderboardsButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"3"
        "actionsignallevel" "1"
        "paintborder" "1"
        "keyboardinputenabled" "1"
        "mouseinputenabled" "1"
        "vertical_scrollbar" "0" // MOM_TODO: Do we want the player to be able to explore past runs?
	}	
	"FriendsLeaderboards"
	{
		"ControlName"	"SectionedListPanel"
		"fieldName"		"FriendsLeaderboards"
		"xpos"			"0"
		"ypos"			"3"
		"wide"			"468"
		"tall"			"240"
        "pin_to_sibling" "LocalLeaderboardsButton"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"2"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"	"2"
        "actionsignallevel" "1"
        "paintborder" "1"
        "keyboardinputenabled" "1"
        "mouseinputenabled" "1"
        "vertical_scrollbar" "0" // MOM_TODO: Do we want the player to be able to explore more?
        //"linespacing" "32"
	}
}