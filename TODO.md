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
    - [x] Discuss columns widths
    - [ ] De-hardcode the font used on size checking
    - [ ] Find where to place friends leaderboards
    - [ ] A lot of variables are not necessary. Ensure which are and remove the rest
    - [ ] Use GetTextSize instead of calculating it per character
    
- [ ] MenuMapSelection.cpp (Client)
    - [ ] Parse data from API, compare to already existing/downloaded maps
    - [ ] Download a selected map and its zone file
    
- [ ] The "mom"-ification and refactoring
    - [ ] mom_player (Server/Client)
         - [ ] Copy valuable snippets from HL2/CS Player classes
         - [ ] Clean up the EntSelectSpawnPoint() method
    - [ ] mom_gamerules (Shared)
         - [ ] Follow the hl2_gamerules.cpp file for creation
    - [x] mom_gamemovement.cpp (Shared)
        - [x] Implement rampboost fix by TotallyMehis
    - [ ] mom_usermessages.cpp (Shared)
        - [ ] Remove the usermessages that aren't necessary
    - [ ] mom_client.cpp (Server)
        - [ ] Precache all necessary sounds/models for the mod
    - [ ] VPC Scripts
        - [ ] Add the new mom files to the proper VPC scripts
        - [ ] Verify & remove the files we deleted from the appropriate VPC scripts
    - [ ] Remove any and all unnecessary HL2/generic code that doesn't pertain to the mod
        - [ ] Remove all ifdef (SIXENSE) code segments
        - [ ] Remove all ifdef (_XBOX/X360 etc) code segments
        - [ ] Remove all ifdef (TF/PORTAL/DOD) code segments
        - [ ] Remove all ifdef (HL2_EPISODIC) code segments
        - [ ] Remove any unused files

- [ ] Creation of a shared (Client/Server) utils class with useful methods/data (gamemode, tickrate etc)
    - [ ] Create global enumeration for gamemodes
    - [ ] Store current gamemode on a global variable

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
    - [x] Discuss bufsize for strings taking intoa count localizations
    
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
    - [ ] Add the option to define what angles should the player have after being teleported

- [ ] Timer.cpp (Server)
    - [ ] Add hash checking
    - [x] GetCPCount seems to return wrongly
    - [x] Are command flags needed?
    
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
