//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Team spawnpoint handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "entitylist.h"
#include "entityoutput.h"
#include "player.h"
#include "eventqueue.h"
#include "gamerules.h"
#include "team_spawnpoint.h"
#include "team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( info_player_teamspawn, CTeamSpawnPoint );

BEGIN_DATADESC( CTeamSpawnPoint )

	// keys
	DEFINE_KEYFIELD( m_iDisabled, FIELD_INTEGER, "StartDisabled" ),

	// input functions
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	// outputs
	DEFINE_OUTPUT( m_OnPlayerSpawn, "OnPlayerSpawn" ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Attach this spawnpoint to it's team
//-----------------------------------------------------------------------------
void CTeamSpawnPoint::Activate( void )
{
	BaseClass::Activate();
	if ( GetTeamNumber() > 0 && GetTeamNumber() <= MAX_TEAMS )
	{
		GetGlobalTeam( GetTeamNumber() )->AddSpawnpoint( this );
	}
	else
	{
		Warning( "info_player_teamspawn with invalid team number: %d\n", GetTeamNumber() );
		UTIL_Remove( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Is this spawnpoint ready for a player to spawn in?
//-----------------------------------------------------------------------------
bool CTeamSpawnPoint::IsValid( CBasePlayer *pPlayer )
{
	CBaseEntity *ent = NULL;
	for ( CEntitySphereQuery sphere( GetAbsOrigin(), 128 ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
	{
		// if ent is a client, don't spawn on 'em
		CBaseEntity *plent = ent;
		if ( plent && plent->IsPlayer() && plent != pPlayer )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeamSpawnPoint::InputEnable( inputdata_t &inputdata )
{
	m_iDisabled = FALSE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTeamSpawnPoint::InputDisable( inputdata_t &inputdata )
{
	m_iDisabled = TRUE;
}
