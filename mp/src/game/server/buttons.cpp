//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements buttons.
//
//=============================================================================

#include "cbase.h"
#include "mom_basedoor.h"
#include "ndebugoverlay.h"
#include "spark.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "tier1/strtools.h"
#include "buttons.h"
#include "mom_eventqueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//
// Rotating button (aka "lever")
//
LINK_ENTITY_TO_CLASS( func_rot_button, CRotButton );


void CRotButton::Spawn( void )
{
	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	if ( m_sounds )
	{
		m_sNoise = MakeButtonSound( m_sounds );
		PrecacheScriptSound(m_sNoise.ToCStr());
	}
	else
	{
		m_sNoise = NULL_STRING;
	}

	// set the axis of rotation
	CBaseToggle::AxisDir();

	// check for clockwise rotation
	if ( HasSpawnFlags( SF_DOOR_ROTATE_BACKWARDS) )
	{
		m_vecMoveAng = m_vecMoveAng * -1;
	}

	SetMoveType( MOVETYPE_PUSH );
	
#ifdef HL1_DLL
	SetSolid( SOLID_BSP );
#else
	SetSolid( SOLID_VPHYSICS );
#endif
	if ( HasSpawnFlags( SF_ROTBUTTON_NOTSOLID ) )
	{
		AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	SetModel( STRING( GetModelName() ) );
	
	if (m_flSpeed == 0)
		m_flSpeed = 40;

	if (m_flWait == 0)
		m_flWait = 1;

	if (m_iHealth > 0)
	{
		m_takedamage = DAMAGE_YES;
	}

	m_toggle_state = TS_AT_BOTTOM;
	m_vecAngle1	= GetLocalAngles();
	m_vecAngle2	= GetLocalAngles() + m_vecMoveAng * m_flMoveDistance;
	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating button start/end positions are equal\n");

	m_fStayPushed = (m_flWait == -1 ? TRUE : FALSE);
	m_fRotating = TRUE;

	SetUse(&CRotButton::ButtonUse);

	//
	// If touching activates the button, set its touch function.
	//
	if (!HasSpawnFlags(SF_BUTTON_TOUCH_ACTIVATES))
	{
		SetTouch ( NULL );
	}
	else
	{
		SetTouch( &CRotButton::ButtonTouch );
	}

	CreateVPhysics();
}


bool CRotButton::CreateVPhysics( void )
{
	VPhysicsInitShadow( false, false );
	return true;
}


//-----------------------------------------------------------------------------
// CMomentaryRotButton spawnflags
//-----------------------------------------------------------------------------
#define SF_MOMENTARY_DOOR			1
#define SF_MOMENTARY_NOT_USABLE		2
#define SF_MOMENTARY_AUTO_RETURN	16


BEGIN_DATADESC( CMomentaryRotButton )

	DEFINE_FIELD( m_lastUsed, FIELD_INTEGER ),
	DEFINE_FIELD( m_start, FIELD_VECTOR ),
	DEFINE_FIELD( m_end, FIELD_VECTOR ),
	DEFINE_FIELD( m_IdealYaw, FIELD_FLOAT ),
	DEFINE_FIELD( m_sNoise, FIELD_SOUNDNAME ),
	DEFINE_FIELD( m_bUpdateTarget, FIELD_BOOLEAN ),

	DEFINE_KEYFIELD( m_direction, FIELD_INTEGER, "StartDirection" ),
	DEFINE_KEYFIELD( m_returnSpeed, FIELD_FLOAT, "returnspeed" ),
	DEFINE_KEYFIELD( m_flStartPosition, FIELD_FLOAT, "StartPosition"),
	DEFINE_KEYFIELD( m_bSolidBsp, FIELD_BOOLEAN, "solidbsp" ),

	// Function Pointers
	DEFINE_FUNCTION( UseMoveDone ),
	DEFINE_FUNCTION( ReturnMoveDone ),
	DEFINE_FUNCTION( SetPositionMoveDone ),
	DEFINE_FUNCTION( UpdateThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetPosition", InputSetPosition ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetPositionImmediately", InputSetPositionImmediately ),
	DEFINE_INPUTFUNC( FIELD_VOID, "_DisableUpdateTarget", InputDisableUpdateTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID, "_EnableUpdateTarget", InputEnableUpdateTarget ),

	// Outputs
	DEFINE_OUTPUT( m_Position, "Position" ),
	DEFINE_OUTPUT( m_OnUnpressed, "OnUnpressed" ),
	DEFINE_OUTPUT( m_OnFullyClosed, "OnFullyClosed" ),
	DEFINE_OUTPUT( m_OnFullyOpen, "OnFullyOpen" ),
	DEFINE_OUTPUT( m_OnReachedPosition, "OnReachedPosition" ),

	DEFINE_INPUTFUNC( FIELD_VOID,	"Enable",	InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"Disable",	InputDisable ),
	DEFINE_FIELD( m_bDisabled, FIELD_BOOLEAN )

END_DATADESC()


LINK_ENTITY_TO_CLASS( momentary_rot_button, CMomentaryRotButton );


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::Spawn( void )
{
	CBaseToggle::AxisDir();

	m_bUpdateTarget = true;

	if ( m_flSpeed == 0 )
	{
		m_flSpeed = 100;
	}

	// Clamp start position and issue bounds warning
	if (m_flStartPosition < 0.0f || m_flStartPosition > 1.0f)
	{
		Warning("WARNING: Momentary door (%s) start position not between 0 and 1.  Clamping.\n",GetDebugName());
		m_flStartPosition = clamp(m_IdealYaw, 0.f, 1.f);
	}

	// Check direction fields (for backward compatibility)
	if (m_direction != 1 && m_direction != -1)
	{
		m_direction = 1;
	}

	if (m_flMoveDistance < 0)
	{
		m_vecMoveAng = m_vecMoveAng * -1;
		m_flMoveDistance = -m_flMoveDistance;
	}

	m_start = GetLocalAngles() - m_vecMoveAng * m_flMoveDistance * m_flStartPosition;
	m_end	= GetLocalAngles() + m_vecMoveAng * m_flMoveDistance * (1-m_flStartPosition);

	m_IdealYaw			= m_flStartPosition;

	// Force start direction at end points
	if (m_flStartPosition == 0.0)
	{
		m_direction = -1;
	}
	else if (m_flStartPosition == 1.0)
	{
		m_direction = 1;
	}

	if (HasSpawnFlags(SF_BUTTON_LOCKED))
	{
		m_bLocked = true;
	}

	if ( HasSpawnFlags( SF_BUTTON_USE_ACTIVATES ) )
	{
		if ( m_sounds )
		{
			m_sNoise = MakeButtonSound( m_sounds );
			PrecacheScriptSound(m_sNoise.ToCStr());
		}
		else
		{
			m_sNoise = NULL_STRING;
		}

		m_lastUsed	= 0;
		UpdateTarget(0,this);
	}

#ifdef HL1_DLL
	SetSolid( SOLID_BSP );
#else
	SetSolid( SOLID_VPHYSICS );
#endif
	if (HasSpawnFlags(SF_ROTBUTTON_NOTSOLID))
	{
		AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	SetMoveType( MOVETYPE_PUSH );
	SetModel( STRING( GetModelName() ) );
	
	CreateVPhysics();

	// Slam the object back to solid - if we really want it to be solid.
	if ( m_bSolidBsp )
	{
		SetSolid( SOLID_BSP );
	}

	m_bDisabled = false;
}

int	CMomentaryRotButton::ObjectCaps( void ) 
{ 
	int flags = BaseClass::ObjectCaps();
	if (!HasSpawnFlags(SF_BUTTON_USE_ACTIVATES))
	{
		return flags;
	}
	else
	{	
		return (flags | FCAP_CONTINUOUS_USE | FCAP_USE_IN_RADIUS);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CMomentaryRotButton::CreateVPhysics( void )
{
	VPhysicsInitShadow( false, false );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMomentaryRotButton::PlaySound( void )
{
	if ( m_sNoise == NULL_STRING )
		return;

	CPASAttenuationFilter filter( this );

	EmitSound_t ep;
	ep.m_nChannel = CHAN_VOICE;
	ep.m_pSoundName = (char*)STRING(m_sNoise);
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NORM;

	EmitSound( filter, entindex(), ep );
}


//-----------------------------------------------------------------------------
// Purpose: Returns a given angular position as a value along our motion from 0 to 1.
// Input  : vecAngles - 
//-----------------------------------------------------------------------------
float CMomentaryRotButton::GetPos( const QAngle &vecAngles )
{
	float flScale = 1;
	if (( m_vecMoveAng[0] < 0 ) || ( m_vecMoveAng[1] < 0 ) || ( m_vecMoveAng[2] < 0 ))
	{
		flScale = -1;
	}

	float flPos = flScale * CBaseToggle::AxisDelta( m_spawnflags, vecAngles, m_start ) / m_flMoveDistance;
	return( clamp( flPos, 0.f, 1.f ));
}


//------------------------------------------------------------------------------
// Purpose :
// Input   : flPosition 
//------------------------------------------------------------------------------
void CMomentaryRotButton::InputSetPosition( inputdata_t &inputdata )
{
	m_IdealYaw = clamp( inputdata.value.Float(), 0.f, 1.f );

	float flCurPos = GetPos( GetLocalAngles() );
	if ( flCurPos < m_IdealYaw )
	{
		// Moving forward (from start to end).
		SetLocalAngularVelocity( m_flSpeed * m_vecMoveAng );
		m_direction = 1;
	}
	else if ( flCurPos > m_IdealYaw )
	{
		// Moving backward (from end to start).
		SetLocalAngularVelocity( -m_flSpeed * m_vecMoveAng );
		m_direction = -1;
	}
	else
	{
		// We're there already; nothing to do.
		SetLocalAngularVelocity( vec3_angle );
		return;
	}

	SetMoveDone( &CMomentaryRotButton::SetPositionMoveDone );

	SetThink( &CMomentaryRotButton::UpdateThink );
	SetNextThink( gpGlobals->curtime );

	//
	// Think again in 0.1 seconds or the time that it will take us to reach our movement goal,
	// whichever is the shorter interval. This prevents us from overshooting and stuttering when we
	// are told to change position in very small increments.
	//
	QAngle vecNewAngles = m_start + m_vecMoveAng * ( m_IdealYaw * m_flMoveDistance );
	float flAngleDelta = fabs( AxisDelta( m_spawnflags, vecNewAngles, GetLocalAngles() ));
	float dt = flAngleDelta / m_flSpeed;
	if ( dt < TICK_INTERVAL )
	{
		dt = TICK_INTERVAL;
		float speed = flAngleDelta / TICK_INTERVAL;
		SetLocalAngularVelocity( speed * m_vecMoveAng * m_direction );
	}
	dt = clamp( dt, TICK_INTERVAL, TICK_INTERVAL * 6);

	SetMoveDoneTime( dt );
}


//------------------------------------------------------------------------------
// Purpose :
// Input   : flPosition 
//------------------------------------------------------------------------------
void CMomentaryRotButton::InputSetPositionImmediately( inputdata_t &inputdata )
{
	m_IdealYaw = clamp( inputdata.value.Float(), 0.f, 1.f );
	SetLocalAngles( m_start + m_vecMoveAng * ( m_IdealYaw * m_flMoveDistance ) );
}


//------------------------------------------------------------------------------
// Purpose: Turns off target updates so that we can change the wheel's position
//			without changing the target's position. Used for jiggling when locked.
//------------------------------------------------------------------------------
void CMomentaryRotButton::InputDisableUpdateTarget( inputdata_t &inputdata )
{
	m_bUpdateTarget = false;
}


//------------------------------------------------------------------------------
// Purpose: Turns target updates back on (after jiggling).
//------------------------------------------------------------------------------
void CMomentaryRotButton::InputEnableUpdateTarget( inputdata_t &inputdata )
{
	m_bUpdateTarget = true;
}


//-----------------------------------------------------------------------------
// Purpose: Locks the button. If locked, the button will play the locked sound
//			when the player tries to use it.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::Lock()
{
	BaseClass::Lock();

	SetLocalAngularVelocity( vec3_angle );
	SetMoveDoneTime( -1 );
	SetMoveDone( NULL );

	SetNextThink( TICK_NEVER_THINK );
	SetThink( NULL );
}


//-----------------------------------------------------------------------------
// Purpose: Unlocks the button, making it able to be pressed again.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::Unlock()
{
	BaseClass::Unlock();

	SetMoveDone( &CMomentaryRotButton::ReturnMoveDone );

	// Delay before autoreturn.
	SetMoveDoneTime( 0.1f );
}


//-----------------------------------------------------------------------------
// Purpose: Fires the appropriate outputs at the extremes of motion.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::OutputMovementComplete( void )
{
	if (m_IdealYaw == 1.0)
	{
		m_OnFullyClosed.FireOutput(this, this);
	}
	else if (m_IdealYaw == 0.0)
	{
		m_OnFullyOpen.FireOutput(this, this);
	}

	m_OnReachedPosition.FireOutput( this, this );
}


//------------------------------------------------------------------------------
// Purpose: MoveDone function for the SetPosition input handler. Tracks our
//			progress toward a movement goal and updates our outputs.
//------------------------------------------------------------------------------
void CMomentaryRotButton::SetPositionMoveDone(void)
{
	float flCurPos = GetPos( GetLocalAngles() );

	if ((( flCurPos >= m_IdealYaw ) && ( m_direction == 1 )) ||
		(( flCurPos <= m_IdealYaw ) && ( m_direction == -1 )))
	{
		//
		// We reached or surpassed our movement goal.
		//
		SetLocalAngularVelocity( vec3_angle );
		// BUGBUG: Won't this get the player stuck?
		SetLocalAngles( m_start + m_vecMoveAng * ( m_IdealYaw * m_flMoveDistance ) );
		SetNextThink( TICK_NEVER_THINK );
		SetMoveDoneTime( -1 );
		UpdateTarget( m_IdealYaw, this );
		OutputMovementComplete();
		return;
	}

	// TODO: change this to use a Think function like ReturnThink.
	QAngle vecNewAngles = m_start + m_vecMoveAng * ( m_IdealYaw * m_flMoveDistance );
	float flAngleDelta = fabs( AxisDelta( m_spawnflags, vecNewAngles, GetLocalAngles() ));
	float dt = flAngleDelta / m_flSpeed;
	if ( dt < TICK_INTERVAL )
	{
		dt = TICK_INTERVAL;
		float speed = flAngleDelta / TICK_INTERVAL;
		SetLocalAngularVelocity( speed * m_vecMoveAng * m_direction );
	}
	dt = clamp( dt, TICK_INTERVAL, TICK_INTERVAL * 6);

	SetMoveDoneTime( dt );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CMomentaryRotButton::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( m_bDisabled == true )
		return;

	if (m_bLocked)
	{
		if ( OnUseLocked( pActivator ) && HasSpawnFlags( SF_BUTTON_JIGGLE_ON_USE_LOCKED ) )
		{
			// Jiggle two degrees.
			float flDist = 2.0 / m_flMoveDistance;

			// Must be first!
			g_EventQueue.AddEvent( this, "_DisableUpdateTarget", 0, this, this );

			variant_t value;
			value.SetFloat( flDist );
			g_EventQueue.AddEvent( this, "SetPosition", value, 0.01, this, this );

			value.SetFloat( 0.0 );
			g_EventQueue.AddEvent( this, "SetPosition", value, 0.1, this, this );

			value.SetFloat( 0.5 * flDist );
			g_EventQueue.AddEvent( this, "SetPosition", value, 0.2, this, this );

			value.SetFloat( 0.0 );
			g_EventQueue.AddEvent( this, "SetPosition", value, 0.3, this, this );

			// Must be last! And must be late enough to cover the settling time.
			g_EventQueue.AddEvent( this, "_EnableUpdateTarget", 0.5, this, this );
		}

		return;
	}

	//
	// Reverse our direction and play movement sound every time the player
	// pauses between uses.
	//
	bool bPlaySound = false;
	
	if ( !m_lastUsed )
	{
		bPlaySound = true;
		m_direction = -m_direction;

		//Alert that we've been pressed
		m_OnPressed.FireOutput( m_hActivator, this );
	}

	m_lastUsed = 1;

	float flPos = GetPos( GetLocalAngles() );
	UpdateSelf( flPos, bPlaySound );

	//
	// Think every frame while we are moving.
	// HACK: Don't reset the think time if we already have a pending think.
	// This works around an issue with host_thread_mode > 0 when the player's
	// clock runs ahead of the server.
	//
	if ( !m_pfnThink )
	{
		SetThink( &CMomentaryRotButton::UpdateThink );
		SetNextThink( gpGlobals->curtime );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Handles changing direction at the extremes of our range of motion
//			and updating our avelocity while being used by the player.
// Input  : value - Number from 0 to 1 indicating our desired position within
//				our range of motion, 0 = start, 1 = end.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::UpdateSelf( float value, bool bPlaySound )
{
	//
	// Set our move clock to 0.1 seconds in the future so we stop spinning unless we are
	// used again before then.
	//
	SetMoveDoneTime( 0.1 );

	//
	// If we hit the end, zero our avelocity and snap to the end angles.
	//
	if ( m_direction > 0 && value >= 1.0 )
	{
		SetLocalAngularVelocity( vec3_angle );
		SetLocalAngles( m_end );

		m_OnFullyClosed.FireOutput(this, this);
		return;
	}
	//
	// If we returned to the start, zero our avelocity and snap to the start angles.
	//
	else if ( m_direction < 0 && value <= 0 )
	{
		SetLocalAngularVelocity( vec3_angle );
		SetLocalAngles( m_start );

		m_OnFullyOpen.FireOutput(this, this);
		return;
	}
	
	if ( bPlaySound )
	{
		PlaySound();
	}

	SetLocalAngularVelocity( ( m_direction * m_flSpeed ) * m_vecMoveAng );
	SetMoveDone( &CMomentaryRotButton::UseMoveDone );
}


//-----------------------------------------------------------------------------
// Purpose: Updates the value of our position, firing any targets.
// Input  : value - New position, from 0 - 1.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::UpdateTarget( float value, CBaseEntity *pActivator )
{
	if ( !m_bUpdateTarget )
		return;

	if (m_Position.Get() != value)
	{
		m_Position.Set(value, pActivator, this);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Handles the end of motion caused by player use.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::UseMoveDone( void )
{
	SetLocalAngularVelocity( vec3_angle );

	// Make sure our targets stop where we stopped.
	float flPos = GetPos( GetLocalAngles() );
	UpdateTarget( flPos, this );

	// Alert that we've been unpressed
	m_OnUnpressed.FireOutput( m_hActivator, this );

	m_lastUsed = 0;

	if ( !HasSpawnFlags( SF_BUTTON_TOGGLE ) && m_returnSpeed > 0 )
	{
		SetMoveDone( &CMomentaryRotButton::ReturnMoveDone );
		m_direction = -1;

		// Delay before autoreturn.
		SetMoveDoneTime( 0.1f );
	}
	else
	{
		SetThink( NULL );
		SetMoveDone( NULL );
	}
}


//-----------------------------------------------------------------------------
// Purpose: MoveDone function for rotating back to the start position.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::ReturnMoveDone( void )
{
	float value = GetPos( GetLocalAngles() );
	if ( value <= 0 )
	{
		//
		// Got back to the start, stop spinning.
		//
		SetLocalAngularVelocity( vec3_angle );
		SetLocalAngles( m_start );

		UpdateTarget( 0, NULL );

		SetMoveDoneTime( -1 );
		SetMoveDone( NULL );

		SetNextThink( TICK_NEVER_THINK );
		SetThink( NULL );
	}
	else
	{
		SetLocalAngularVelocity( -m_returnSpeed * m_vecMoveAng );
		SetMoveDoneTime( 0.1f );

		SetThink( &CMomentaryRotButton::UpdateThink );
		SetNextThink( gpGlobals->curtime + 0.01f );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Think function for updating target as we move.
//-----------------------------------------------------------------------------
void CMomentaryRotButton::UpdateThink( void )
{
	float value = GetPos( GetLocalAngles() );
	UpdateTarget( value, NULL );
	SetNextThink( gpGlobals->curtime );
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CMomentaryRotButton::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[255];
		
		Q_snprintf(tempstr,sizeof(tempstr),"QAngle: %.2f %.2f %.2f", GetLocalAngles()[0], GetLocalAngles()[1], GetLocalAngles()[2]);
		EntityText(text_offset,tempstr,0);
		text_offset++;

		Q_snprintf(tempstr,sizeof(tempstr),"AVelocity: %.2f %.2f %.2f", GetLocalAngularVelocity()[0], GetLocalAngularVelocity()[1], GetLocalAngularVelocity()[2]);
		EntityText(text_offset,tempstr,0);
		text_offset++;

		Q_snprintf(tempstr,sizeof(tempstr),"Target Pos:   %3.3f",m_IdealYaw);
		EntityText(text_offset,tempstr,0);
		text_offset++;

		float flCurPos = GetPos(GetLocalAngles());
		Q_snprintf(tempstr,sizeof(tempstr),"Current Pos:   %3.3f",flCurPos);
		EntityText(text_offset,tempstr,0);
		text_offset++;

		Q_snprintf(tempstr,sizeof(tempstr),"Direction: %s",(m_direction == 1) ? "Forward" : "Backward");
		EntityText(text_offset,tempstr,0);
		text_offset++;

	}
	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Input hander that starts the spawner
//-----------------------------------------------------------------------------
void CMomentaryRotButton::InputEnable( inputdata_t &inputdata )
{
	Enable();
}


//-----------------------------------------------------------------------------
// Purpose: Input hander that stops the spawner
//-----------------------------------------------------------------------------
void CMomentaryRotButton::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

//-----------------------------------------------------------------------------
// Purpose: Start the spawner
//-----------------------------------------------------------------------------
void CMomentaryRotButton::Enable( void )
{
	m_bDisabled = false;
}


//-----------------------------------------------------------------------------
// Purpose: Stop the spawner
//-----------------------------------------------------------------------------
void CMomentaryRotButton::Disable( void )
{
	m_bDisabled = true;
}