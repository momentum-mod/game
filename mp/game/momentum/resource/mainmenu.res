"MainMenu"
{
    "ResumeGame"
    {
        "text"      "#GameUI2_ResumeGame"
        "command"   "ResumeGame"
        "priority"  "100"
        "specifics" "ingame"
        "enabled"       "1"
    }

    "SelectMap"
    {
        "text"          "#GameUI2_SelectMap"
        "EngineCommand" "ShowMapSelectionPanel"
        "priority"      "99"
        "specifics"     "ingame"
        "enabled"       "0"
    }

    "Play"
    {
        "text"          "#GameUI2_Play"
        "EngineCommand" "ShowMapSelectionPanel"
        "priority"      "98"
        "specifics"     "mainmenu"
        "enabled"       "0"
    }
    
    "Console"
    {
        "text"          "Show Console"
        "EngineCommand" "showconsole"
        "priority"      "97"
        "specifics"     "shared"
        "enabled"       "1"
    }
    
    "Settings"
    {
        "text" "#GameUI2_Settings"
        "EngineCommand" "mom_settings_show"
        "priority" "60"
        "specifics" "shared"
        "enabled"       "1"
    }
    
    "QuitToMenu"
    {
        "text" "#GameUI2_Quit2Menu"
        "EngineCommand" "disconnect"
        "priority" "50"
        "specifics" "ingame"
        "enabled"       "1"
    }
    
    "Credits"
    {
        "text" "#GameUI2_Credits"
        "EngineCommand" "mom_credits_show"
        "priority" "50"
        "specifics" "mainmenu"
        "enabled"       "1"
    }

    "Quit"
    {
        "text"          "#GameUI2_Quit"
        "command"       "QuitNoConfirm"
        "priority"      "10"
        "specifics"     "shared"
        "enabled"       "1"
    }
}
