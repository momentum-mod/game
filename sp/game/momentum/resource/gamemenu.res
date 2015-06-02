"GameMenu"
{
	"1"
	{
		"label" "#GameUI_GameMenu_ResumeGame"
		"command" "ResumeGame"
		"InGameOrder" "10"
		"OnlyInGame" "1"
	}
	"2"
	{
		"label" "#GameUI_GameMenu_CreateServer"
		"command" "OpenCreateMultiplayerGameDialog"
		"InGameOrder" "30"
	}
	"3"
	{
		"label" "#GameUI_GameMenu_ActivateVR"
		"command" "engine vr_activate"
		"InGameOrder" "70"
		"OnlyWhenVREnabled" "1"
		"OnlyWhenVRInactive" "1"
	}
	"4"
	{
		"label" "#GameUI_GameMenu_DeactivateVR"
		"command" "engine vr_deactivate"
		"InGameOrder" "70"
		"OnlyWhenVREnabled" "1"
		"OnlyWhenVRActive" "1"
	}
	"5"
	{
		"label" "#GameUI_GameMenu_Options"
		"command" "OpenOptionsDialog"
		"InGameOrder" "90"
	}
	"6"
	{
		"label" "#GameUI_GameMenu_Quit"
		"command" "QuitNoConfirm"
		"InGameOrder" "100"
	}
}

