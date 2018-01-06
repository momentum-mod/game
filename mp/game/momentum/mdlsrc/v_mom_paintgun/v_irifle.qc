/*
==============================================================================

IRifle

==============================================================================
*/

$modelname weapons/v_mom_paintgun.mdl
$cdmaterials models\Weapons\V_hand models\Weapons\V_irifle

// whole body
$body studio "IRifle_reference.smd"

$origin 0 0 67

//Muzzle flash
$attachment "muzzle" "Base" 0 0 20 rotate -90
// Control panel attachments
$attachment "controlpanel0_ur" "muzzle" -14.00 6.00 4.00 rotate -90 0 0
$attachment "controlpanel0_ll" "muzzle" -14.00 2.00 0.00 rotate -90 0 0

$poseparameter "VentPoses" 0 1 

$sequence IR_idle "Idle" loop fps 30 activity ACT_VM_IDLE 1 node Ready
$sequence IR_fire "Fire01" fps 30 snap activity ACT_VM_PRIMARYATTACK 1 { event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" } node Fire
$sequence fire2 "Fire02" snap fps 30 activity ACT_VM_RECOIL1 1 { event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" } node Fire
$sequence fire3 "Fire03" snap fps 30 activity ACT_VM_RECOIL2 1 { event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" } node Fire
$sequence fire4 "Fire04" snap fps 30 activity ACT_VM_RECOIL3 1 { event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" } node Fire
$sequence IR_fire2 "Alt_fire" fps 30 snap activity ACT_VM_SECONDARYATTACK 1 { event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }

$sequence IR_reload "Reload" fps 30 activity ACT_VM_RELOAD 1 node Ready {
{ event AE_CL_PLAYSOUND 1 "Weapon_AR2.Reload_Rotate" }
{ event AE_CL_PLAYSOUND 19 "Weapon_AR2.Reload_Push" }
}

$sequence IR_draw "Draw" fps 30 snap activity ACT_VM_DRAW 1 node Ready
$sequence IR_holster "Holster" fps 30 activity ACT_VM_HOLSTER 1 node Ready
$sequence idletolow "idle_to_low" fps 30 activity ACT_VM_LOWERED_TO_IDLE 1  transition Ready Low
$sequence lowtoidle "Low_to_idle" fps 30 activity ACT_VM_IDLE_TO_LOWERED 1 transition Low Ready
$sequence lowidle "Low_idle" fps 30 loop activity ACT_VM_IDLE_LOWERED 1 node Low
$sequence shake "Shake" fps 30 loop activity ACT_VM_FIDGET 1 node Fire

$animation a_vent_neutral "Vent_Poses" frame 0 0
$animation a_vent_open "Vent_Poses" frame 0 0 subtract a_vent_neutral 0 
$animation a_vent_closed "Vent_Poses" frame 1 1 subtract a_vent_neutral 0
$sequence VentPoses a_vent_open a_vent_closed blend "VentPoses" 1 0 delta autoplay