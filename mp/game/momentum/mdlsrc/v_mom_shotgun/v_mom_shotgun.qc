$modelname "weapons\v_mom_shotgun.mdl"
$cdmaterials "models\weapons\w_models\w_mom_shotgun"

$scale 0.75
$body studio "reference.dmx"
$surfaceprop "weapon"
$origin 1.5 4.5 1 3

$sequence idle "reference.dmx" fps 1 loop snap activity ACT_VM_IDLE 1
//MOM_TODO: Change this to be a draw (pulling gun out) animation
//$sequence draw "reference.dmx" fps 300 snap activity ACT_VM_DRAW 1
//$sequence idle "anims\idle.dmx" fps 24 loop snap activity ACT_VM_IDLE 1
//$sequence fire1 "anims\fire1.dmx" fps 60 snap activity ACT_VM_PRIMARYATTACK 1
//MOM_TODO: Change this to be a fire2 (burst fire) animation
//$sequence fire2 "anims\fire1.dmx" fps 36 snap activity ACT_VM_SECONDARYATTACK 1
//$sequence switch "anims\switchmode.dmx" fps 72 snap activity ACT_VM_SECONDARYATTACK_SPECIAL 1