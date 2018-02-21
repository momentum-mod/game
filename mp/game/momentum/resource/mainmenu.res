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
        "EngineCommand" "ShowMapSelectionPanel"
		"priority"		"99"
		"specifics"		"shared"
	}
	
    // Declared in code
    // "Spectate"
    // {
        // "text" "#GameUI2_Spectate"
        // "description" "#GameUI2_SpectateDescription"
        // "EngineCommand" "mom_spectate"
        // "priority" "90"
        // "specifics" "ingame"
    // }
    
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
        "EngineCommand" "mom_settings_show"
        "priority" "60"
        "specifics" "shared"
    }
    
    "QuitToMenu"
    {
        "text" "#GameUI2_Quit2Menu"
        "description" "#GameUI2_Quit2MenuDescription"
        "EngineCommand" "disconnect"
        "priority" "50"
        "specifics" "ingame"
    }
    
    "Credits"
    {
        "text" "#GameUI2_Credits"
        "description" "#GameUI2_CreditsDescription"
        "EngineCommand" "map credits"
        "priority" "50"
        "specifics" "mainmenu"
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