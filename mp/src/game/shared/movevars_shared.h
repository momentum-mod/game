//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef MOVEVARS_SHARED_H
#define MOVEVARS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "convar.h"

float GetCurrentGravity( void );

extern ConVar sv_gravity;
extern ConVar sv_stopspeed;
extern ConVar sv_noclipaccelerate;
extern ConVar sv_noclipspeed;
extern ConVar sv_noclipspeed_vertical;
extern ConVar sv_noclipspeed_duck_multiplier;
extern ConVar sv_noclipspeed_sprint_multiplier;

extern ConVar sv_jumpsound;

extern ConVar sv_maxspeed;
extern ConVar sv_accelerate;
extern ConVar sv_duckscale;

extern ConVar sv_maxairspeed;
extern ConVar sv_airaccelerate;
extern ConVar sv_airdecelerate;

extern ConVar sv_maxairstrafespeed;
extern ConVar sv_airstrafeaccelerate;
extern ConVar sv_rampslide;
extern ConVar sv_rampslide_gravity;
extern ConVar sv_rampslide_speed;
extern ConVar sv_rampslide_jumps;

extern ConVar sv_jumpstyle;
extern ConVar sv_strafestyle;
extern ConVar sv_slidestyle;
extern ConVar sv_slickstyle;
extern ConVar sv_jump_buffertime;
extern ConVar sv_flatknockback;
extern ConVar sv_df_rocket_addspeed;
extern ConVar sv_snapvelocity;
extern ConVar sv_scalecmd_fix;
extern ConVar sv_autojump;
extern ConVar sv_autojump_boost;
extern ConVar sv_crouchjump;
extern ConVar sv_crouchslide;
extern ConVar sv_wallcliptime;
extern ConVar sv_aircontrol_enable;
extern ConVar sv_aircontrol;
extern ConVar sv_aircontrolpower;

extern ConVar sv_wadescale;
extern ConVar sv_swimscale;

extern ConVar sv_wateraccelerate;
extern ConVar sv_waterfriction;
extern ConVar sv_footsteps;
extern ConVar sv_rollspeed;
extern ConVar sv_rollangle;
extern ConVar sv_friction;
extern ConVar sv_crouchfriction;
extern ConVar sv_bounce;
extern ConVar sv_maxvelocity;
extern ConVar sv_stepsize;
extern ConVar sv_skyname;
extern ConVar sv_backspeed;
extern ConVar sv_waterdist;
extern ConVar sv_specaccelerate;
extern ConVar sv_specspeed;
extern ConVar sv_specnoclip;
extern ConVar sv_swimsound;

// Momentum convars
extern ConVar sv_considered_on_ground;
extern ConVar sv_duck_collision_fix;
extern ConVar sv_ground_trigger_fix;
extern ConVar sv_edge_fix;
// - - Parkour
extern ConVar sv_wallrun_anticipation;
extern ConVar sv_wallrun_time;
extern ConVar sv_wallrun_speed;
extern ConVar sv_wallrun_accel;
extern ConVar sv_wallrun_boost;
extern ConVar sv_wallrun_jump_boost;
extern ConVar sv_wallrun_jump_push;
extern ConVar sv_wallrun_feet_z;
extern ConVar sv_airjump_delta;
extern ConVar sv_wallrun_roll;
extern ConVar sv_wallrun_min_rise;
extern ConVar sv_wallrun_max_rise;
extern ConVar sv_wallrun_scramble_z;
extern ConVar sv_wallrun_lookahead;
extern ConVar sv_wallrun_inness;
extern ConVar sv_wallrun_outness;
extern ConVar sv_wallrun_look_delay;
extern ConVar sv_wallrun_lookness;
extern ConVar sv_wallrun_stick_angle;
extern ConVar sv_wallrun_corner_stick_angle;
extern ConVar sv_slide_speed_boost;
extern ConVar sv_coyote_time;
extern ConVar sv_slide_time;
extern ConVar sv_slide_lock;

#endif // MOVEVARS_SHARED_H
