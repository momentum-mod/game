//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "entityblocker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( entity_blocker, CEntityBlocker );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&mins - 
//			&maxs - 
//			NULL - 
// Output : CEntityBlocker
//-----------------------------------------------------------------------------
CEntityBlocker *CEntityBlocker::Create( const Vector &origin, const Vector &mins, const Vector &maxs, CBaseEntity *pOwner, bool bBlockPhysics )
{
	CEntityBlocker *pBlocker = (CEntityBlocker *) CBaseEntity::Create( "entity_blocker", origin, vec3_angle, pOwner );
	
	if ( pBlocker != NULL )
	{
		pBlocker->SetSize( mins, maxs );
		if ( bBlockPhysics )
		{
			pBlocker->VPhysicsInitStatic();
		}
	}

	return pBlocker;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEntityBlocker::Spawn( void )
{
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_CUSTOMRAYTEST );
}

//-----------------------------------------------------------------------------
// Purpose: Entity blockers don't block tracelines so they don't screw up weapon fire, etc
//-----------------------------------------------------------------------------
bool CEntityBlocker::TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace )
{
	return false;
}