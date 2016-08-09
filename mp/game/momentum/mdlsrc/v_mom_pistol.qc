$modelname "weapons\v_mom_pistol.mdl"
$cdmaterials "models\weapons\w_models\w_mom_pistol"

$scale 0.3
$body studio "v_mom_pistol.dmx"
$surfaceprop "metal"
$origin 2.6 6 3 5

//MOM_TODO: Change this to be a draw (pulling gun out) animation
$sequence draw "v_mom_pistol.dmx" fps 300 snap activity ACT_VM_DRAW 1
$sequence idle "anims\idle.dmx" fps 24 loop snap activity ACT_VM_IDLE 1
$sequence fire1 "anims\fire1.dmx" fps 45 snap activity ACT_VM_PRIMARYATTACK 1
//MOM_TODO: Change this to be a fire2 (burst fire) animation
$sequence fire2 "anims\fire1.dmx" fps 24 snap activity ACT_VM_SECONDARYATTACK 1
$sequence switch "anims\switchmode.dmx" fps 60 snap activity ACT_VM_SECONDARYATTACK_SPECIAL 1