//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements two types of doors: linear and rotating.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mom_basedoor.h"
#include "entitylist.h"
#include "physics.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
#include "physics_npc_solver.h"
#include "mom_basebutton.h"

#ifdef HL1_DLL
#include "mom_basefilter.h"
#endif

#ifdef CSTRIKE_DLL
#include "KeyValues.h"
#endif

#ifdef TF_DLL
#include "tf_gamerules.h"
#endif // TF_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*func_door_rotating

TOGGLE causes the door to wait in both the start and end states for  
a trigger event.

START_OPEN causes the door to move to its destination when spawned,  
and operate in reverse.  It is used to temporarily or permanently  
close off an area when triggered (not usefull for touch or  
takedamage doors).

You need to have an origin brush as part of this entity.  The  
center of that brush will be
the point around which it is rotated. It will rotate around the Z  
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote  
button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
*/

//==================================================
// CRotDoor 
//==================================================

class CRotDoor : public CBaseDoor
{
public:
	DECLARE_CLASS( CRotDoor, CBaseDoor );

	void Spawn( void );
	bool CreateVPhysics();
	// This is ONLY used by the node graph to test movement through a door
	virtual void SetToggleState( int state );
	virtual bool IsRotatingDoor() { return true; }

	bool m_bSolidBsp;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( func_door_rotating, CRotDoor );

BEGIN_DATADESC( CRotDoor )
	DEFINE_KEYFIELD( m_bSolidBsp, FIELD_BOOLEAN, "solidbsp" ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRotDoor::Spawn( void )
{
	BaseClass::Spawn();

	// set the axis of rotation
	CBaseToggle::AxisDir();

	// check for clockwise rotation
	if ( HasSpawnFlags(SF_DOOR_ROTATE_BACKWARDS) )
		m_vecMoveAng = m_vecMoveAng * -1;
	
	//m_flWait			= 2; who the hell did this? (sjb)
	m_vecAngle1	= GetLocalAngles();
	m_vecAngle2	= GetLocalAngles() + m_vecMoveAng * m_flMoveDistance;

	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating door start/end positions are equal\n");

	// Starting open allows a func_door to be lighted in the closed position but
	// spawn in the open position
	//
	// SF_DOOR_START_OPEN_OBSOLETE is an old broken way of spawning open that has
	// been deprecated.
	if ( HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE) )
	{	
		// swap pos1 and pos2, put door at pos2, invert movement direction
		QAngle vecNewAngles = m_vecAngle2;
		m_vecAngle2 = m_vecAngle1;
		m_vecAngle1 = vecNewAngles;
		m_vecMoveAng = -m_vecMoveAng;

		// We've already had our physics setup in BaseClass::Spawn, so teleport to our
		// current position. If we don't do this, our vphysics shadow will not update.
		Teleport( NULL, &m_vecAngle1, NULL );

		m_toggle_state = TS_AT_BOTTOM;
	}
	else if ( m_eSpawnPosition == FUNC_DOOR_SPAWN_OPEN )
	{	
		// We've already had our physics setup in BaseClass::Spawn, so teleport to our
		// current position. If we don't do this, our vphysics shadow will not update.
		Teleport( NULL, &m_vecAngle2, NULL );
		m_toggle_state = TS_AT_TOP;
	}
	else
	{
		m_toggle_state = TS_AT_BOTTOM;
	}

#ifdef HL1_DLL
	SetSolid( SOLID_VPHYSICS );
#endif
		
	// Slam the object back to solid - if we really want it to be solid.
	if ( m_bSolidBsp )
	{
		SetSolid( SOLID_BSP );
	}
}

//-----------------------------------------------------------------------------

bool CRotDoor::CreateVPhysics()
{
	if ( !IsSolidFlagSet( FSOLID_NOT_SOLID ) )
	{
		VPhysicsInitShadow( false, false );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
// This is ONLY used by the node graph to test movement through a door
void CRotDoor::SetToggleState( int state )
{
	if ( state == TS_AT_TOP )
		SetLocalAngles( m_vecAngle2 );
	else
		SetLocalAngles( m_vecAngle1 );
}
