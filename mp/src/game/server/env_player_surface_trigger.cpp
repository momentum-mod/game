//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "decals.h"
#include "env_player_surface_trigger.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( env_player_surface_trigger, CEnvPlayerSurfaceTrigger );

BEGIN_DATADESC( CEnvPlayerSurfaceTrigger )
	DEFINE_KEYFIELD( m_iTargetGameMaterial, FIELD_INTEGER, "gamematerial" ),
	DEFINE_FIELD( m_iCurrentGameMaterial, FIELD_INTEGER ),
	DEFINE_FIELD( m_bDisabled, FIELD_BOOLEAN ),

	DEFINE_THINKFUNC( UpdateMaterialThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),

	// Outputs
	DEFINE_OUTPUT(m_OnSurfaceChangedToTarget, "OnSurfaceChangedToTarget"),
	DEFINE_OUTPUT(m_OnSurfaceChangedFromTarget, "OnSurfaceChangedFromTarget"),
END_DATADESC()

// Global list of surface triggers
CUtlVector< CHandle<CEnvPlayerSurfaceTrigger> >	g_PlayerSurfaceTriggers;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEnvPlayerSurfaceTrigger::~CEnvPlayerSurfaceTrigger( void )
{
	g_PlayerSurfaceTriggers.FindAndRemove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::Spawn( void )
{
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );

	m_iCurrentGameMaterial = 0;
	m_bDisabled = false;

	g_PlayerSurfaceTriggers.AddToTail( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::OnRestore( void )
{
	BaseClass::OnRestore();

	g_PlayerSurfaceTriggers.AddToTail( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::SetPlayerSurface( CBasePlayer *pPlayer, const trace_t &tr )
{
    int iCount = g_PlayerSurfaceTriggers.Count();

    if (iCount > 0)
    {
        IPhysicsSurfaceProps *pPhysprops = MoveHelper()->GetSurfaceProps();
        surfacedata_t *pSurfaceProp = pPhysprops->GetSurfaceData(tr.surface.surfaceProps);
        char cCurrGameMaterial = pSurfaceProp->game.material;

        // Loop through the surface triggers and tell them all about the change
        for (int i = 0; i < iCount; i++)
        {
            g_PlayerSurfaceTriggers[i]->PlayerSurfaceChanged(pPlayer, cCurrGameMaterial);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::PlayerSurfaceChanged( CBasePlayer *pPlayer, char gameMaterial )
{
	if ( m_bDisabled )
		return;

    if (pPlayer->m_chPreviousTextureType == gameMaterial)
        return;

    pPlayer->m_chPreviousTextureType = gameMaterial;

	// Fire the output if we've changed, but only if it involves the target material
	if ( gameMaterial != (char)m_iCurrentGameMaterial &&
	     ( gameMaterial == m_iTargetGameMaterial || m_iCurrentGameMaterial == m_iTargetGameMaterial ) )
	{
		DevMsg( 2, "Player changed material to %d (was %d)\n", gameMaterial, m_iCurrentGameMaterial );

		m_iCurrentGameMaterial = (int)gameMaterial;

		SetThink( &CEnvPlayerSurfaceTrigger::UpdateMaterialThink );
		SetNextThink( gpGlobals->curtime );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function to fire outputs. Done this way so that sv_alternate ticks
//			doesn't allow multiple surface changes in the same tick to fire outputs.
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::UpdateMaterialThink( void )
{
	if ( m_iCurrentGameMaterial == m_iTargetGameMaterial )
	{
		m_OnSurfaceChangedToTarget.FireOutput( NULL, this );
	}
	else 
	{
		m_OnSurfaceChangedFromTarget.FireOutput( NULL, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::InputDisable( inputdata_t &inputdata )
{
	m_bDisabled = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPlayerSurfaceTrigger::InputEnable( inputdata_t &inputdata )
{
	m_bDisabled = false;
}
