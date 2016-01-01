## ALPHA
- [ ] ClientScoreboardDialog.cpp (Client)
    - [ ] Get online data from the API
    - [x] Add friends leaderboard list
    - [x] Add format for online & friends leaderboard lists
    - [ ] Fill the lists with API data
    - [ ] Update rank for event runtime_posted
    - [x] Discuss update interval time
    - [ ] Consider adding a "100 tick" column
    - [x] Localize rank tokens
    - [ ] Make FindItemIDForPlayerIndex(int) return an ItemID for another person's time
    - [ ] Sort function for online times
    - [x] Fix bugs: Lines being chopped down & mapsummary not being set
    - [ ] Discuss columns widths
    
- [ ] MenuMapSelection.cpp (Client)
    - [ ] Parse data from API, compare to already existing/downloaded maps
    - [ ] Download a selected map and its zone file
    
- [ ] The "mom"-ification and refactoring
    - [ ] Use mom_player and mom_gamerules instead of generic ones (Server)
    - [ ] Remove any and all unnecessary HL2/generic code that doesn't pertain to the mod

- [x] Creation of a shared (Client/Server) utils class with useful methods/data (gamemode, tickrate etc)
    - [ ] Create global enumeration for gamemodes
    - [ ] Store current gamemdoe on a global variable

- [ ] weapon_momentum_gun (Client/Server)
    - [ ] Import CS:S weapon entities over as proxy weapons to change the gun's behavior
    - [ ] Make the gun toggleable (the player spawns with it, presses button to use/hide it)

- [ ] timer.cpp (Client) 
    - [ ] Play effects (animations) for run states
    - [x] Move to bottom center (above speedometer)
    - [x] Utilize the .res file variables for position/color/etc
    - [x] Feed real data for the hud
    - [x] Have more info (checkpoints, current stage/total stages, etc)
    - [x] Only display relevant info
    - [ ] Act accordingly to gamemode
    - [x] Implement Hud Messaging system to interact with Timer.cpp (server)
    - [x] Localization
    - [ ] Discuss bufsize for strings taking intoa count localizations
    
- [ ] hud_cp_menu.cpp (Client) 
    - [x] Make creating a checkpoint stop your timer
    - [ ] Make checkpoints available for output to files
    - [ ] Consider local timer for routing
    - [ ] Consider KZ game mode basically requiring checkpoints
    - [ ] Extract underlying menu class and make hud_cp_menu create one
    - [x] Use .res files

- [ ] TimeTriggers.cpp (Server)
    - [ ] Tweak limit speed method
    - [x] Implement Hud Messaging system to interact with timer.cpp (client)

- [ ] Timer.cpp (Server)
    - [ ] Add hash checking
    - [x] (FIXED) GetCPCount seems to return wrongly
    - [ ] Are command flags needed?
    
- [ ] In-game mapzone editor (Server/Client) allows for creation of zone files (on older CS maps) without using Hammer

- [ ] mapzones.cpp (Server)
    - [ ] Add support for trigger_momentum_teleport and trigger_momentum_userinput
    - [ ] Make sure it works correctly after that before releasing Alpha!
    
## BETA
- [ ] Implement CEF
    - [ ] Create custom HTML HUD
    - [ ] Create custom HTML menu
    - [ ] Incorporate (precompiled/source?) into project
    
- [ ] Replays
    - [ ] Use ghostingmod as an example of how to make/read the files
    - [ ] Make map selection/leaderboards be able to download replays

- [ ] Spectate system (like replays but streamed online)  
    - [ ] Use ghostingmod (server) as an example  
    - [ ] Racing system (like spectating but you can move around)
        - [ ] Synchronize start, notified ends
        - [ ] Allow disqualifications, drop outs, disconnections

- [ ] Global chat
    - [ ] Simple general and map chat via IRC or something
    
- [ ] func_shootboost: (Potential entity that handles shootboosts (not needed))

- [ ] weapon_momentum_gun (Client/Server)
    - [ ] "Wire-frame" models for each gun override
    - [ ] Edited gun sounds that reference original sounds?

## BETA+ (Official Release)
- [ ] Get greenlit

- [ ] Timer.cpp (Server)
    - [ ] Include the extra security measures
