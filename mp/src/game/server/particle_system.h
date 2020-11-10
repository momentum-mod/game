//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
// Purpose: An entity that spawns and controls a particle system
//-----------------------------------------------------------------------------
class CParticleSystem : public CBaseEntity
{
	DECLARE_CLASS( CParticleSystem, CBaseEntity );
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CParticleSystem();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void Activate( void );
	virtual void Think( void );
	virtual int  UpdateTransmitState(void);

	void		StartParticleSystem( void );
	void		StopParticleSystem( void );

	void		InputStart( inputdata_t &inputdata );
	void		InputStop( inputdata_t &inputdata );
	void		InputDestroyImmediately( inputdata_t &inputdata );

	enum { kMAXCONTROLPOINTS = 63 }; ///< actually one less than the total number of cpoints since 0 is assumed to be me

protected:

	/// Load up and resolve the entities that are supposed to be the control points 
	void ReadControlPointEnts( void );

	bool				m_bStartActive;
	string_t			m_iszEffectName;
	
	CNetworkVar( bool,	m_bActive );
	CNetworkVar( bool, m_bDestroyImmediately );
	CNetworkVar( int,	m_iEffectIndex )
	CNetworkVar( float,	m_flStartTime );	// Time at which this effect was started.  This is used after restoring an active effect.
	CNetworkVar( float,	m_flDuration );

	string_t			m_iszControlPointNames[kMAXCONTROLPOINTS];
	CNetworkArray( EHANDLE, m_hControlPointEnts, kMAXCONTROLPOINTS );
	CNetworkArray( unsigned char, m_iControlPointParents, kMAXCONTROLPOINTS );
	CNetworkVar( bool,	m_bWeatherEffect );
	CNetworkVar( bool,	m_bAttachToPlayer );
};
