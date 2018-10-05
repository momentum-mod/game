//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef BUTTONS_H
#define BUTTONS_H
#ifdef _WIN32
#pragma once
#endif

#include "mom_basebutton.h"

//
// Rotating button (aka "lever")
//
class CRotButton : public CBaseButton
{
public:
	DECLARE_CLASS( CRotButton, CBaseButton );

	void Spawn( void );
	bool CreateVPhysics( void );

};


class CMomentaryRotButton : public CRotButton
{
	DECLARE_CLASS( CMomentaryRotButton, CRotButton );

public:
	void	Spawn ( void );
	bool	CreateVPhysics( void );
	virtual int	ObjectCaps( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	UseMoveDone( void );
	void	ReturnMoveDone( void );
	void	OutputMovementComplete(void);
	void	SetPositionMoveDone(void);
	void	UpdateSelf( float value, bool bPlaySound );

	void	PlaySound( void );
	void	UpdateTarget( float value, CBaseEntity *pActivator );

	int		DrawDebugTextOverlays(void);

	static CMomentaryRotButton *Instance( edict_t *pent ) { return (CMomentaryRotButton *)GetContainingEntity(pent); }

	float GetPos(const QAngle &vecAngles);

	DECLARE_DATADESC();

	virtual void Lock();
	virtual void Unlock();

	// Input handlers
	void InputSetPosition( inputdata_t &inputdata );
	void InputSetPositionImmediately( inputdata_t &inputdata );
	void InputDisableUpdateTarget( inputdata_t &inputdata );
	void InputEnableUpdateTarget( inputdata_t &inputdata );

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

	virtual void Enable( void );
	virtual void Disable( void );

	bool	m_bDisabled;

	COutputFloat m_Position;
	COutputEvent m_OnUnpressed;
	COutputEvent m_OnFullyOpen;
	COutputEvent m_OnFullyClosed;
	COutputEvent m_OnReachedPosition;

	int			m_lastUsed;
	QAngle		m_start;
	QAngle		m_end;
	float		m_IdealYaw;
	string_t	m_sNoise;

	bool		m_bUpdateTarget;		// Used when jiggling so that we don't jiggle the target (door, etc)

	int			m_direction;
	float		m_returnSpeed;
	float		m_flStartPosition;

protected:

	void UpdateThink( void );
};


#endif // BUTTONS_H
