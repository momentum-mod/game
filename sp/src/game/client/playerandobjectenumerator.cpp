//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "playerandobjectenumerator.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Enumator class for ragdolls being affected by explosive forces
CPlayerAndObjectEnumerator::CPlayerAndObjectEnumerator( float radius )
{
	m_flRadiusSquared		= radius * radius;
	m_Objects.RemoveAll();
	m_pLocal = C_BasePlayer::GetLocalPlayer();
}

int	CPlayerAndObjectEnumerator::GetObjectCount()
{
	return m_Objects.Size();
}

C_BaseEntity *CPlayerAndObjectEnumerator::GetObject( int index )
{
	if ( index < 0 || index >= GetObjectCount() )
		return NULL;

	return m_Objects[ index ];
}

// Actual work code
IterationRetval_t CPlayerAndObjectEnumerator::EnumElement( IHandleEntity *pHandleEntity )
{
	if ( !m_pLocal )
		return ITERATION_STOP;

	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
	if ( pEnt == NULL )
		return ITERATION_CONTINUE;

	if ( pEnt == m_pLocal )
		return ITERATION_CONTINUE;

	if ( !pEnt->IsPlayer() &&
		 !pEnt->IsNPC() )
	{
		return ITERATION_CONTINUE;
	}

	if ( pEnt->IsNPC() )
	{
		C_AI_BaseNPC *pNPC = (C_AI_BaseNPC *)pEnt;

		if ( !pNPC->ShouldAvoidObstacle() )
			 return ITERATION_CONTINUE;
	}

	// Ignore vehicles, since they have vcollide collisions that's push me away
	if ( pEnt->GetCollisionGroup() == COLLISION_GROUP_VEHICLE )
		return ITERATION_CONTINUE;

	Vector	deltaPos = pEnt->GetAbsOrigin() - m_pLocal->GetAbsOrigin();
	if ( deltaPos.LengthSqr() > m_flRadiusSquared )
		return ITERATION_CONTINUE;

	CHandle< C_BaseEntity > h;
	h = pEnt;
	m_Objects.AddToTail( h );

	return ITERATION_CONTINUE;
}