$modelname "weapons\v_mom_paintgun.mdl"
$cdmaterials "models\weapons\w_models\w_mom_paintgun"
$surfaceprop "weapon"

$origin 4.2 10 3 0
$attachment "controlpanel0_ur" "Screen_UpperRight" 0.0 0.0 0.0 rotate 0 0 -20
$attachment "controlpanel0_ll" "Screen_LowerLeft" 0.0 0.0 0.00 rotate 0 0 -20

$scale 3.0
$body studio "v_mom_paintgun.dmx"


//MOM_TODO: Change this to be a draw (pulling gun out) animation
//$sequence draw "reference.dmx" fps 300 snap activity ACT_VM_DRAW 1
$sequence idle "v_mom_paintgun.dmx" fps 24 loop snap activity ACT_VM_IDLE 1
//$sequence fire1 "anims\fire1.dmx" fps 60 snap activity ACT_VM_PRIMARYATTACK 1
//$sequence fire2 "anims\fire1.dmx" fps 36 snap activity ACT_VM_SECONDARYATTACK 1
//$sequence switch "anims\switchmode.dmx" fps 72 snap activity ACT_VM_SECONDARYATTACK_SPECIAL 1