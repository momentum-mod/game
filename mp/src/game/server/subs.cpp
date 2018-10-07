//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Frequently used global functions.
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "mom_basedoor.h"
#include "entitylist.h"
#include "globals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CNullEntity : public CBaseEntity
{
public:
	DECLARE_CLASS( CNullEntity, CBaseEntity );

	void Spawn( void );
};

// Null Entity, remove on startup
void CNullEntity::Spawn( void )
{
	UTIL_Remove( this );
}
LINK_ENTITY_TO_CLASS(info_null,CNullEntity);

class CBaseDMStart : public CPointEntity
{
public:
	DECLARE_CLASS( CBaseDMStart, CPointEntity );

	bool IsTriggered( CBaseEntity *pEntity );

	DECLARE_DATADESC();

	string_t m_Master;

private:
};

BEGIN_DATADESC( CBaseDMStart )

	DEFINE_KEYFIELD( m_Master, FIELD_STRING, "master" ),

END_DATADESC()


// These are the new entry points to entities. 
LINK_ENTITY_TO_CLASS(info_player_deathmatch,CBaseDMStart);
LINK_ENTITY_TO_CLASS(info_player_start,CPointEntity);
LINK_ENTITY_TO_CLASS(info_landmark,CPointEntity);

bool CBaseDMStart::IsTriggered( CBaseEntity *pEntity )
{
	bool master = UTIL_IsMasterTriggered( m_Master, pEntity );

	return master;
}


// Convenient way to delay removing oneself
void CBaseEntity::SUB_Remove( void )
{
	if (m_iHealth > 0)
	{
		// this situation can screw up NPCs who can't tell their entity pointers are invalid.
		m_iHealth = 0;
		DevWarning( 2, "SUB_Remove called on entity with health > 0\n");
	}

	UTIL_Remove( this );
}


// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity::SUB_DoNothing( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Finds all active entities with the given targetname and calls their
//			'Use' function.
// Input  : targetName - Target name to search for.
//			pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pTarget = NULL;
	if ( !targetName || !targetName[0] )
		return;

	DevMsg( 2, "Firing: (%s)\n", targetName );

	for (;;)
	{
		CBaseEntity *pSearchingEntity = pActivator;
		pTarget = gEntList.FindEntityByName( pTarget, targetName, pSearchingEntity, pActivator, pCaller );
		if ( !pTarget )
			break;

		if (!pTarget->IsMarkedForDeletion() )	// Don't use dying ents
		{
			DevMsg( 2, "[%03d] Found: %s, firing (%s)\n", gpGlobals->tickcount%1000, pTarget->GetDebugName(), targetName );
			pTarget->Use( pActivator, pCaller, useType, value );
		}
	}
}
