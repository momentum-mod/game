//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements visual effects entities: sprites, beams, bubbles, etc.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "EnvMessage.h"
#include "engine/IEngineSound.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "Color.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( env_message, CMessage );

BEGIN_DATADESC( CMessage )

	DEFINE_KEYFIELD( m_iszMessage, FIELD_STRING, "message" ),
	DEFINE_KEYFIELD( m_sNoise, FIELD_SOUNDNAME, "messagesound" ),
	DEFINE_KEYFIELD( m_MessageAttenuation, FIELD_INTEGER, "messageattenuation" ),
	DEFINE_KEYFIELD( m_MessageVolume, FIELD_FLOAT, "messagevolume" ),

	DEFINE_FIELD( m_Radius, FIELD_FLOAT ),

	DEFINE_INPUTFUNC( FIELD_VOID, "ShowMessage", InputShowMessage ),

	DEFINE_OUTPUT(m_OnShowMessage, "OnShowMessage"),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMessage::Spawn( void )
{
	Precache();

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );

	switch( m_MessageAttenuation )
	{
	case 1: // Medium radius
		m_Radius = ATTN_STATIC;
		break;
	
	case 2:	// Large radius
		m_Radius = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		m_Radius = ATTN_NONE;
		break;
	
	default:
	case 0: // Small radius
		m_Radius = SNDLVL_IDLE;
		break;
	}
	m_MessageAttenuation = 0;

	// Remap volume from [0,10] to [0,1].
	m_MessageVolume *= 0.1;

	// No volume, use normal
	if ( m_MessageVolume <= 0 )
	{
		m_MessageVolume = 1.0;
	}
}


void CMessage::Precache( void )
{
	if ( m_sNoise != NULL_STRING )
	{
		PrecacheScriptSound( STRING(m_sNoise) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for showing the message and/or playing the sound.
//-----------------------------------------------------------------------------
void CMessage::InputShowMessage( inputdata_t &inputdata )
{
	CBaseEntity *pPlayer = NULL;

	if ( m_spawnflags & SF_MESSAGE_ALL )
	{
		UTIL_ShowMessageAll( STRING( m_iszMessage ) );
	}
	else
	{
		if ( inputdata.pActivator && inputdata.pActivator->IsPlayer() )
		{
			pPlayer = inputdata.pActivator;
		}
		else
		{
			pPlayer = (gpGlobals->maxClients > 1) ? NULL : UTIL_GetLocalPlayer();
		}

		if ( pPlayer && pPlayer->IsPlayer() )
		{
			UTIL_ShowMessage( STRING( m_iszMessage ), ToBasePlayer( pPlayer ) );
		}
	}

	if ( m_sNoise != NULL_STRING )
	{
		CPASAttenuationFilter filter( this );
		
		EmitSound_t ep;
		ep.m_nChannel = CHAN_BODY;
		ep.m_pSoundName = (char*)STRING(m_sNoise);
		ep.m_flVolume = m_MessageVolume;
		ep.m_SoundLevel = ATTN_TO_SNDLVL( m_Radius );

		EmitSound( filter, entindex(), ep );
	}

	if ( m_spawnflags & SF_MESSAGE_ONCE )
	{
		UTIL_Remove( this );
	}

	m_OnShowMessage.FireOutput( inputdata.pActivator, this );
}


void CMessage::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	inputdata_t inputdata;

	inputdata.pActivator	= NULL;
	inputdata.pCaller		= NULL;

	InputShowMessage( inputdata );
}
