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
ConVar	sv_specaccelerate( "sv_specaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_specspeed	( "sv_specspeed", "3", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_specnoclip	( "sv_specnoclip", "1", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);

ConVar	sv_maxspeed		( "sv_maxspeed", "320",  FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);

ConVar	sv_accelerate	( "sv_accelerate", "5", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING);

ConVar	sv_airaccelerate(  "sv_airaccelerate", "150", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_MAPPING );    
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