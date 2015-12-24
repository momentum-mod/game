## BETA
- [ ] Implement CEF
- [ ] Make checkpoints use target_destination instead of string in Hammer, for an easier use

## ALPHA
- [ ] func_shootboost: (Potential entity not needed that handles shootboosts but is an idea)

- [ ] timer.cpp (Client) 
    -  [ ] Play effects for run states
    -  [x] Add more HUD message states
    
- [ ] hud_cp_menu.cpp (Client) 
    -  [ ] ~~Make creating a checkpoint stop your timer~~
    -  [ ] Make checkpoints available for output to files
    -  [ ] Consider local timer
    -  [ ] Consider KZ

- [x] TimeTriggers.cpp (Server)
    -  [x] Limit speed inside Start Trigger
    -  [x] Output to fire on the player (!player) that resets checkpoint (ResetCheckpoint)
