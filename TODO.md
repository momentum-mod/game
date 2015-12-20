- [ ] Custom .FGD creation
    -  [x] filter_activator_checkpoint
    -  [x] trigger_checkpoint:
        -  [x] Hitting this trigger will set your checkpoint to this checkpoint IF you are on a lower checkpoint
        -  [x] Reset any onehop can jump here things
    -  [x] trigger_teleport_checkpoint
    -  [ ] func_shootboost: (Potential entity not needed that handles shootboosts but is an idea)
    -  [x] trigger_onehop_reset
    -  [x] trigger_onehop:
        -  [x] **flags**: something about allowing to hop on it again after hitting another.
    -  [ ] trigger_multihop

-  [ ] timer.cpp (Client) 
    -  [ ] Play effects for run states
    -  [ ] Add more HUD message states
    -  [ ] Better method to avoid sv_cheats 1 runs

-  [ ] Timer.cpp (Server)
    -  [X] Play sound on menu invalid input
    -  [ ] ~~Create a decal to show where the checkpoint has been created~~
    -  [x] Set the former Onehop hopped status to false if the flag is set to that
    -  [x] Avoid crash if the entity no longer exists

 -  [ ] hud_cp_menu.cpp (Client) 
    -  [ ] ~~Make creating a checkpoint stop your timer~~
    -  [ ] Make checkpoints available for output to files
    -  [x] Play sound for menu input
    -  [ ] Consider local timer
    -  [ ] Consider KZ

-  [ ] TimeTriggers.cpp (Server)
    -  [ ] Limit speed inside Start Trigger
    -  [ ] Output to fire on the player (!activator) that resets checkpoint like ResetCheckpoint or something.
    -  [x] OneHops needs polishment and getting finished
        -  [ ] ~~Add teleport after delay m_fMaxHoldSeconds has passed to Onehops~~

-  [x] mapzones.cpp (Server)
    -  [x] Remove goto usage
    -  [x] Add the newly created trigger types

-  [x] game_sounds_manifest.txt 
    -  [x] Create it (So we can add our own sounds)