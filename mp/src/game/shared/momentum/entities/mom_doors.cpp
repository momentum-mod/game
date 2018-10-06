#include "cbase.h"
#include "mom_doors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( func_door_rotating, CRotDoor );

#ifdef CLIENT_DLL // Client prediction and recv table
BEGIN_PREDICTION_DATA(CRotDoor)
END_PREDICTION_DATA();

#undef CRotDoor // Undefine so we can type the real server class name for recv table

IMPLEMENT_CLIENTCLASS_DT(C_RotDoor, DT_RotDoor, CRotDoor)
END_RECV_TABLE();
#define CRotDoor C_RotDoor // Redefine for rest of the code
#else // Server save data and send table
IMPLEMENT_SERVERCLASS_ST(CRotDoor, DT_RotDoor)
END_SEND_TABLE();

BEGIN_DATADESC(CRotDoor)
	DEFINE_KEYFIELD(m_bSolidBsp, FIELD_BOOLEAN, "solidbsp"),
END_DATADESC();
#endif


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRotDoor::Spawn(void)
{
	BaseClass::Spawn();

	// set the axis of rotation
	CBaseToggle::AxisDir();

	// check for clockwise rotation
	if (HasSpawnFlags(SF_DOOR_ROTATE_BACKWARDS))
		m_vecMoveAng = m_vecMoveAng * -1;

	//m_flWait			= 2; who the hell did this? (sjb)
	m_vecAngle1 = GetLocalAngles();
	m_vecAngle2 = GetLocalAngles() + m_vecMoveAng * m_flMoveDistance;

	ASSERTSZ(m_vecAngle1 != m_vecAngle2, "rotating door start/end positions are equal\n");

	// Starting open allows a func_door to be lighted in the closed position but
	// spawn in the open position
	//
	// SF_DOOR_START_OPEN_OBSOLETE is an old broken way of spawning open that has
	// been deprecated.
	if (HasSpawnFlags(SF_DOOR_START_OPEN_OBSOLETE))
	{
		// swap pos1 and pos2, put door at pos2, invert movement direction
		QAngle vecNewAngles = m_vecAngle2;
		m_vecAngle2 = m_vecAngle1;
		m_vecAngle1 = vecNewAngles;
		m_vecMoveAng = -m_vecMoveAng;

		// We've already had our physics setup in BaseClass::Spawn, so teleport to our
		// current position. If we don't do this, our vphysics shadow will not update.
		Teleport(NULL, &m_vecAngle1, NULL);

		m_toggle_state = TS_AT_BOTTOM;
	}
	else if (m_eSpawnPosition == FUNC_DOOR_SPAWN_OPEN)
	{
		// We've already had our physics setup in BaseClass::Spawn, so teleport to our
		// current position. If we don't do this, our vphysics shadow will not update.
		Teleport(NULL, &m_vecAngle2, NULL);
		m_toggle_state = TS_AT_TOP;
	}
	else
	{
		m_toggle_state = TS_AT_BOTTOM;
	}

#ifdef HL1_DLL
	SetSolid(SOLID_VPHYSICS);
#endif

	// Slam the object back to solid - if we really want it to be solid.
	if (m_bSolidBsp)
	{
		SetSolid(SOLID_BSP);
	}
}

//-----------------------------------------------------------------------------

bool CRotDoor::CreateVPhysics()
{
	if (!IsSolidFlagSet(FSOLID_NOT_SOLID))
	{
		VPhysicsInitShadow(false, false);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
// This is ONLY used by the node graph to test movement through a door
void CRotDoor::SetToggleState(int state)
{
	if (state == TS_AT_TOP)
		SetLocalAngles(m_vecAngle2);
	else
		SetLocalAngles(m_vecAngle1);
}
