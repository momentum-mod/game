#ifndef _MOM_BUTTONS_H_
#define _MOM_BUTTONS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basebutton.h"

#ifdef CLIENT_DLL
#define CRotButton C_RotButton
#define CMomentaryRotButton C_MomentaryRotButton
#else
#endif

class CRotButton : public CBaseButton
{
public:
	DECLARE_CLASS(CBaseButton, CBaseToggle);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	bool CreateVPhysics(void);

#ifdef GAME_DLL // Server specific things
public:
	virtual void Spawn(void);
	DECLARE_DATADESC();
#endif
};

//-----------------------------------------------------------------------------
// CMomentaryRotButton spawnflags
//-----------------------------------------------------------------------------
#define SF_MOMENTARY_DOOR			1
#define SF_MOMENTARY_NOT_USABLE		2
#define SF_MOMENTARY_AUTO_RETURN	16

class CMomentaryRotButton : public CRotButton
{
public:
	DECLARE_CLASS(CMomentaryRotButton, CRotButton);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	bool	CreateVPhysics(void);
	void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void	UseMoveDone(void);
	void	ReturnMoveDone(void);
	void	OutputMovementComplete(void);
	void	SetPositionMoveDone(void);
	void	UpdateSelf(float value, bool bPlaySound);

	void	PlaySound(void);
	void	UpdateTarget(float value, CBaseEntity *pActivator);

	float GetPos(const QAngle &vecAngles);

	virtual void Lock();
	virtual void Unlock();

	// Input handlers
	void InputSetPosition(inputdata_t &inputdata);
	void InputSetPositionImmediately(inputdata_t &inputdata);
	void InputDisableUpdateTarget(inputdata_t &inputdata);
	void InputEnableUpdateTarget(inputdata_t &inputdata);

	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);

	virtual void Enable(void);
	virtual void Disable(void);

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
	void UpdateThink(void);

#ifdef GAME_DLL // Server specific things
public:
	static CMomentaryRotButton *Instance(edict_t *pent) { return (CMomentaryRotButton *)GetContainingEntity(pent); }
	virtual void Spawn(void);
	virtual int ObjectCaps(void);
	int		DrawDebugTextOverlays(void);
	DECLARE_DATADESC();
#endif
};

#endif