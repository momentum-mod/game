## ALPHA
- [ ] ClientScoreboardDialog.cpp (Client)
    - [ ] Get online data from the API
    - [ ] Add friends leaderboard list
    - [ ] Add format for online & friends leaderboard lists
    - [ ] Fill the lists with API data
    - [ ] Update rank for event runtime_posted
    - [ ] Discuss update interval time
    - [ ] Consider adding a "100 tick" column
    - [x] Localize rank tokens
    - [ ] Make FindItemIDForPlayerIndex(int) return an ItemID for another person's time
    - [ ] Sort function for online times
    
- [ ] MenuMapSelection.cpp (Client)
    - [ ] Parse data from API, compare to already existing/downloaded maps
    - [ ] Download a selected map and its zone file
    
- [ ] The "mom"-ification and refactoring
    - [ ] Use mom_player and mom_gamerules instead of generic ones (Server)
    - [ ] Remove any and all unnecessary HL2/generic code that doesn't pertain to the mod

- [ ] Creation of a shared (Client/Server) utils class with useful methods/data (gamemode, tickrate etc)

- [ ] weapon_momentum_gun (Client/Server)
    - [ ] Import CS:S weapon entities over as proxy weapons to change the gun's behavior
    - [ ] Make the gun toggleable (the player spawns with it, presses button to use/hide it)

- [ ] timer.cpp (Client) 
    - [ ] Play effects (animations) for run states
    - [ ] Move to bottom center (above speedometer)
    - [ ] Utilize the .res file variables for position/color/etc
    - [ ] Have more info (checkpoints, current stage/total stages, etc)
    
- [ ] hud_cp_menu.cpp (Client) 
    - [x] Make creating a checkpoint stop your timer
    - [ ] Make checkpoints available for output to files
    - [ ] Consider local timer for routing
    - [ ] Consider KZ game mode basically requiring checkpoints
    - [ ] Extract underlying menu class and make hud_cp_menu create one

- [ ] TimeTriggers.cpp (Server)
    - [ ] Tweak limit speed method
    
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