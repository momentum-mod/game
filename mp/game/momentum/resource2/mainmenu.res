"MainMenu"
{	
	"ResumeGame"
	{
		"text"			"#GameUI2_ResumeGame"
		"description"	"#GameUI2_ResumeGameDescription"
		"command"		"ResumeGame"
		"priority"		"100"
		"specifics"		"ingame"
	}
	
	"SelectMap"
	{
		"text"			"#GameUI2_SelectMap"
		"description"	"#GameUI2_SelectMapDescription"
		"command"		"engine ToggleMapSelectionPanel"
		"priority"		"99"
		"specifics"		"mainmenu"
	}
	
	"Options"
	{
		"text"			"#GameUI2_Options"
		"description"	"#GameUI2_OptionsDescription"
		"command"		"OpenOptionsDialog"
		"priority"		"70"
		"specifics"		"shared"
	}
    
    "MomentumSettings"
    {
        "text" "#GameUI2_MomSettings"
        "description" "#GameUI2_MomSettingsDescription"
        "command" "engine mom_settings_show"
        "priority" "60"
        "specifics" "shared"
    }
    
    "QuitToMenu"
    {
        "text" "#GameUI2_Quit2Menu"
        "description" "#GameUI2_Quit2MenuDescription"
        "command" "engine disconnect"
        "priority" "50"
        "specifics" "ingame"
    }
    
    "Credits"
    {
        "text" "#GameUI2_Credits"
        "description" "#GameUI2_CreditsDescription"
        "command" "engine progress_enable \n map credits"
        "priority" "50"
        "specifics" "mainmenu"
    }
    
    "Blank"
    {
        "text" ""
        "description" ""
        "command" ""
        "priority" "20"
        "specifics" "shared"
        "blank" "1"
    }

	"Quit"
	{
		"text"			"#GameUI2_Quit"
		"description"	"#GameUI2_QuitDescription"
		"command"		"QuitNoConfirm"
		"priority"		"10"
		"specifics"		"shared"
	}
}