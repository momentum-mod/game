//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Resource collection entity
//
// $NoKeywords: $
//=============================================================================//

#ifndef SKYCAMERA_H
#define SKYCAMERA_H

#ifdef _WIN32
#pragma once
#endif

class CSkyCamera;

#define SF_SKY_MASTER (1 << 0)
#define SF_SKY_START_UPDATING (1 << 1)

//=============================================================================
//
// Sky Camera Class
// Now derived directly from CBaseEntity for parenting and angles! (please don't break anything)
// 
//=============================================================================
class CSkyCamera : public CBaseEntity
{
	DECLARE_CLASS( CSkyCamera, CBaseEntity );

public:

	DECLARE_DATADESC();
	CSkyCamera();
	~CSkyCamera();
	virtual void Spawn( void );
	virtual void Activate();

	bool AcceptInput( const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID );

	int UpdateTransmitState() { return HasSpawnFlags( SF_SKY_START_UPDATING ) ? SetTransmitState( FL_EDICT_ALWAYS ) : BaseClass::UpdateTransmitState(); }

	void SetCameraEntityMode();
	void SetCameraPositionMode();

	bool DoUpdate( bool bUpdateData = false );
	void UpdateThink();

	void InputForceUpdate( inputdata_t &inputdata );
	void InputStartUpdating( inputdata_t &inputdata );
	void InputStopUpdating( inputdata_t &inputdata );

	void InputActivateSkybox( inputdata_t &inputdata );
	void InputDeactivateSkybox( inputdata_t &inputdata );

	void InputSetFogStartDist( inputdata_t &data );
	void InputSetFogEndDist( inputdata_t &data );
	void InputTurnOnFog( inputdata_t &data );
	void InputTurnOffFog( inputdata_t &data );
	void InputSetFogColor( inputdata_t &data );
	void InputSetFogColorSecondary( inputdata_t &data );
	void InputSetFogMaxDensity( inputdata_t &inputdata );
	void InputCopyFogController( inputdata_t &inputdata );
	void InputCopyFogControllerWithScale( inputdata_t &inputdata );

	void InputSetFarZ( inputdata_t &data );

	void InputSetSkyColor( inputdata_t &inputdata );

    void InputSetScale( inputdata_t& inputdata );

public:
	sky3dparams_t	m_skyboxData;
	bool			m_bUseAngles;
	// Uses angles for actual skybox
	bool			m_bUseAnglesForSky;
	CSkyCamera		*m_pNext;
};


//-----------------------------------------------------------------------------
// Retrieves the current skycamera
//-----------------------------------------------------------------------------
CSkyCamera*		GetCurrentSkyCamera();
CSkyCamera*		GetSkyCameraList();


#endif // SKYCAMERA_H
