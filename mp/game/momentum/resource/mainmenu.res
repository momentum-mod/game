"MainMenu"
{
    "ResumeGame"
    {
        "text"      "#GameUI2_ResumeGame"
        "command"   "ResumeGame"
        "priority"  "100"
        "specifics" "ingame"
    }

    "SelectMap"
    {
        "text"          "#GameUI2_SelectMap"
        "EngineCommand" "ShowMapSelectionPanel"
        "priority"      "99"
        "specifics"     "ingame"
    }

    "Play"
    {
        "text"          "#GameUI2_Play"
        "EngineCommand" "ShowMapSelectionPanel"
        "priority"      "98"
        "specifics"     "mainmenu"
    }
    
    "Settings"
    {
        "text" "#GameUI2_Settings"
        "EngineCommand" "mom_settings_show"
        "priority" "60"
        "specifics" "shared"
    }
    
    "QuitToMenu"
    {
        "text" "#GameUI2_Quit2Menu"
        "EngineCommand" "disconnect"
        "priority" "50"
        "specifics" "ingame"
    }
    
    "Credits"
    {
        "text" "#GameUI2_Credits"
        "EngineCommand" "mom_credits_show"
        "priority" "50"
        "specifics" "mainmenu"
    }

    "Quit"
    {
        "text"          "#GameUI2_Quit"
        "command"       "QuitNoConfirm"
        "priority"      "10"
        "specifics"     "shared"
    }
}