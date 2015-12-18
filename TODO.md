[-] Custom .FGD creation
    [X] filter_activator_checkpoint
    
    [-] trigger_checkpoint:
    
        [ ] Hitting this trigger will set your checkpoint to this checkpoint IF you are on a lower checkpoint
        
        [X] Reset any onehop can jump here things
        
    [X] trigger_teleport_checkpoint
 
    [ ] func_shootboost:
        
        [ ] Potential entity not needed that handles shootboosts but is an idea

    [ ] trigger_onehop_reset
    
    [-] trigger_onehop:
    
        [ ] flags: something about allowing to hop on it again after hitting another.
 
    [ ] trigger_multihop
 
## timer.cpp (Client) 
[-] Play effects for run states

[ ] Add more HUD message states

[ ] Better method to avoid sv_cheats 1 runs


## Timer.cpp (Server)
[X] Play sound on invalid menu input

[ ] Create a decal to show where the checkpoint has been created

[ ] Set the former Onehop hopped status to false if the flag is set to that


## hud_cp_menu.cpp (Client) 
[ ] Make creating a checkpoint stop your timer

[ ] Make checkpoints available for output to files

[ ] Avoid crash if the entity no longer exists

[X] Play sound for menu input

[ ] Consider local timer

[ ] Consider KZ


## TimeTriggers.cpp (Server)
[ ] Limit speed inside Start Trigger

[ ] Output to fire on the player (!activator) that resets checkpoint like ResetCheckpoint or something.

[ ] OneHops needs polishment and getting finished

    [ ] Add teleport after delay m_fMaxHoldSeconds has passed to Onehops

## mapzones.cpp (Server)
[ ] Remove goto usage

[ ] Add the newly created trigger types

## game_sounds_manifest.txt 
[X] Create it (So we can add our own sounds)