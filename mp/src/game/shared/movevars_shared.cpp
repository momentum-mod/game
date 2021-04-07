//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//


#include "cbase.h"
#include "movevars_shared.h"
#include "mom_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

float GetCurrentGravity( void )
{
#if defined( TF_CLIENT_DLL ) || defined( TF_DLL )
	if ( TFGameRules() )
	{
		return ( sv_gravity.GetFloat() * TFGameRules()->GetGravityMultiplier() );
	}
#endif 
	return sv_gravity.GetFloat();
}

inline void UpdatePhysicsGravity(const float gravity)
{
    if (physenv)
        physenv->SetGravity(Vector(0,0,-gravity));
}

#ifdef CLIENT_DLL
class CGravityChange : public CGameEventListener, public CAutoGameSystem
{
public:
    bool Init() OVERRIDE
    {
        ListenForGameEvent("gravity_change");
        return true;
    }
    void FireGameEvent(IGameEvent *event) OVERRIDE
    {
        UpdatePhysicsGravity(event->GetFloat("newgravity"));
    }
};
static CGravityChange s_GravityChange;
#else
static void GravityChanged_Callback(IConVar *var, const char *pOldString, float)
{
    ConVarRef grav(var);
    UpdatePhysicsGravity(grav.GetFloat());
    if (gpGlobals->mapname != NULL_STRING)
    {
        IGameEvent *event = gameeventmanager->CreateEvent("gravity_change");
        if (event)
        {
            event->SetFloat("newgravity", grav.GetFloat());
            gameeventmanager->FireEvent(event);
        }
    }
}
#endif

ConVar	sv_gravity("sv_gravity", "800", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING, "World gravity."
#ifdef GAME_DLL
    , GravityChanged_Callback
#endif
    );

#if defined( DOD_DLL ) || defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
ConVar	sv_stopspeed	( "sv_stopspeed","100", FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum stopping speed when on ground." );
#else
ConVar	sv_stopspeed	( "sv_stopspeed","75", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Minimum stopping speed when on ground." );
#endif // DOD_DLL || CSTRIKE_DLL

ConVar	sv_noclipaccelerate( "sv_noclipaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_noclipspeed	( "sv_noclipspeed", "14", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar sv_noclipspeed_vertical("sv_noclipspeed_vertical", "7", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar sv_noclipspeed_duck_multiplier("sv_noclipspeed_duck_multiplier", "0.3", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar sv_noclipspeed_sprint_multiplier("sv_noclipspeed_sprint_multiplier", "0.5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_specaccelerate( "sv_specaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_specspeed	( "sv_specspeed", "3", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_specnoclip	( "sv_specnoclip", "1", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);

ConVar	sv_maxspeed		( "sv_maxspeed", "320",  FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);

ConVar	sv_accelerate	( "sv_accelerate", "5", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);

ConVar	sv_maxairspeed(  "sv_maxairspeed", "320", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING );
ConVar	sv_airaccelerate(  "sv_airaccelerate", "150", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING );
ConVar	sv_maxairstrafespeed(  "sv_maxairstrafespeed", "30", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING );
ConVar	sv_airstrafeaccelerate(  "sv_airstrafeaccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING );


ConVar sv_aircontrol("sv_aircontrol", "150", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);
ConVar sv_aircontrolpower("sv_aircontrolpower", "2", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);

ConVar	sv_cpm_physics("sv_cpm_physics", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);
ConVar	sv_cpm_trimpmultiplier("sv_cpm_trimpmultiplier", "1.3", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);
ConVar sv_cpm_trimpslowdown("sv_cpm_trimpslowdown", "1.4", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);

ConVar	sv_wateraccelerate(  "sv_wateraccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);
ConVar	sv_waterfriction(  "sv_waterfriction", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);
ConVar	sv_footsteps	( "sv_footsteps", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Play footstep sound for players" );
ConVar	sv_rollspeed	( "sv_rollspeed", "200", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);
ConVar	sv_rollangle	( "sv_rollangle", "0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Max view roll angle");

ConVar sv_swimsound("sv_swimsound", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Play swim sound for players");

ConVar	sv_friction		( "sv_friction","4", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING, "World friction." );

#if defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
ConVar	sv_bounce		( "sv_bounce","0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Bounce multiplier for when physically simulated objects collide with other objects." );
ConVar	sv_maxvelocity	( "sv_maxvelocity","3500", FCVAR_REPLICATED, "Maximum speed any ballistically moving object is allowed to attain per axis." );
ConVar	sv_stepsize		( "sv_stepsize","18", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar	sv_backspeed	( "sv_backspeed", "0.6", FCVAR_ARCHIVE | FCVAR_REPLICATED, "How much to slow down backwards motion" );
ConVar  sv_waterdist	( "sv_waterdist","12", FCVAR_REPLICATED, "Vertical view fixup when eyes are near water plane." );
#else
ConVar	sv_bounce		( "sv_bounce","0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING, "Bounce multiplier for when physically simulated objects collide with other objects." );
ConVar	sv_maxvelocity	( "sv_maxvelocity","3500", FCVAR_REPLICATED | FCVAR_MAPPING, "Maximum speed any ballistically moving object is allowed to attain per axis." );
ConVar	sv_stepsize		( "sv_stepsize","18", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar	sv_backspeed	( "sv_backspeed", "0.6", FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "How much to slow down backwards motion" );
ConVar  sv_waterdist	( "sv_waterdist","12", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Vertical view fixup when eyes are near water plane." );
#endif // CSTRIKE_DLL

ConVar	sv_skyname		( "sv_skyname", "sky_urb01", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Current name of the skybox texture" );

// Momentum convars
MAKE_CONVAR(sv_considered_on_ground, "1.0", FCVAR_MAPPING, "Amount of units you have to be above the ground to be considered on ground.\n", 0.0f, 5.f);
MAKE_TOGGLE_CONVAR(sv_duck_collision_fix, "1", FCVAR_MAPPING, "Fixes headbugs by updating the collision box after duck code instead of at the end of the tick. 1 = ON, 0 = OFF.\n");
MAKE_TOGGLE_CONVAR(sv_ground_trigger_fix, "1", FCVAR_MAPPING, "Fixes being able to jump off the ground if grounded with a trigger under the player (bounces and jumpbugs). 1 = ON, 0 = OFF.\n");
MAKE_TOGGLE_CONVAR(sv_edge_fix, "1", FCVAR_MAPPING, "Makes edgebugs more consistent and allows for bunnyhopping instead of edgebugging. 1 = ON, 0 = OFF.\n");


#define DEFAULT_JUMP_HEIGHT_STRING "50.0"
#define DEFAULT_SLIDE_TIME_STRING "2000.0" 
#define DEFAULT_SLIDE_SPEED_BOOST_STRING "75.0"
#define DEFAULT_WALLRUN_TIME_STRING "2000.0"
#define DEFAULT_WALLRUN_SPEED_STRING "300.0"
#define DEFAULT_WALLRUN_BOOST_STRING "60.0"

ConVar sv_slide_time("sv_slide_time", DEFAULT_SLIDE_TIME_STRING, FCVAR_NOTIFY | FCVAR_REPLICATED, "Powerslide time.");
ConVar
sv_slide_speed_boost(
	"sv_slide_speed_boost",
	DEFAULT_SLIDE_SPEED_BOOST_STRING,
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Speed boost for powerslide.");
ConVar
sv_wallrun_time(
	"sv_wallrun_time",
	DEFAULT_WALLRUN_TIME_STRING,
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun max duration.");

ConVar
sv_airjump_delta(
	"sv_airjump_delta",
	"125.0",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Amount to change direction in airjump.");

ConVar
sv_wallrun_anticipation(
	"sv_wallrun_anticipation",
	"2",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"0 - none, 1 - Eye roll only, 2 - Full (bumps).");

ConVar
sv_wallrun_boost(
	"sv_wallrun_boost",
	DEFAULT_WALLRUN_BOOST_STRING,
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun speed boost.");
ConVar
sv_wallrun_jump_boost(
	"sv_wallrun_jump_boost",
	"0.15",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Fraction of wallrun speed to add to jump.");
ConVar
sv_wallrun_jump_push(
	"sv_wallrun_jump_push",
	"0.25",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Fraction of wall jump speed to go to pushing off wall.");
ConVar
sv_wallrun_speed(
	"sv_wallrun_speed",
	DEFAULT_WALLRUN_SPEED_STRING,
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun speed.");
ConVar
sv_wallrun_accel(
	"sv_wallrun_accel",
	"4.25",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun acceleration.");

ConVar
sv_wallrun_roll(
	"sv_wallrun_roll",
	"14.0",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun view roll angle.");

ConVar
sv_wallrun_max_rise(
	"sv_wallrun_max_rise",
	"25.0",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun upward limit.");

ConVar
sv_wallrun_min_rise(
	"sv_wallrun_min_rise",
	"-50.0",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Wallrun downward limit.");

ConVar
sv_wallrun_scramble_z(
	"sv_wallrun_scramble_z",
	"28.0",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Height we can climb to.");

ConVar
sv_wallrun_lookahead(
	"sv_wallrun_lookahead",
	"0.22",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Amount of time (in seconds) to lookahead for bumps or corners when wallrunning.");

ConVar
sv_wallrun_inness(
	"sv_wallrun_inness",
	"360",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Scaling factor for how much to steer inward toward the wall when wallrunning");
ConVar
sv_wallrun_outness(
	"sv_wallrun_outness",
	"300",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Scaling factor for how much to steer outward around obstacles when wallrunning");

ConVar
sv_wallrun_lookness(
	"sv_wallrun_lookness",
	"1",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Scaling factor for how much to adjust view to look where you're going when wallrunning");
ConVar
sv_wallrun_look_delay(
	"sv_wallrun_look_delay",
	"0.3",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"How long to wait before aligning the view with the move direction when wallrunning.");

ConVar
sv_wallrun_feet_z(
	"sv_wallrun_feet_z",
	"10",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Fudge for starting a wallrun with your feet below the bottom edge of the wall");

ConVar
sv_wallrun_stick_angle(
	"sv_wallrun_stick_angle",
	"45",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Min angle away from wall norm for player to wallrun");

ConVar
sv_wallrun_corner_stick_angle(
	"sv_wallrun_corner_stick_angle",
	"80",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"End wallrun at end of wall if facing within this angle of wall norm");

ConVar
sv_coyote_time(
	"sv_coyote_time",
	"0.2",
	FCVAR_NOTIFY | FCVAR_REPLICATED,
	"Time after leaving a surface that jumps are still allowed.");

ConVar
sv_slide_lock(
	"sv_slide_lock",
	"1",
	FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_ARCHIVE,
	"Locks your move direction when sliding");