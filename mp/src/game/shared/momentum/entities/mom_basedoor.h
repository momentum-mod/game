#ifndef _MOM_BASEDOOR_H_
#define _MOM_BASEDOOR_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basetoggle.h"
#include "mom_basebutton.h"
#include "mom_entityoutput.h"

#include "engine/IEngineSound.h"

#ifdef CLIENT_DLL
#define CBaseDoor C_BaseDoor
#else
#include "locksounds.h"
#include "physics_npc_solver.h"
#endif

#define DOOR_SENTENCEWAIT	6
#define DOOR_SOUNDWAIT		1
#define BUTTON_SOUNDWAIT	0.5

#define CLOSE_AREAPORTAL_THINK_CONTEXT "CloseAreaportalThink"

enum FuncDoorSpawnPos_t
{
	FUNC_DOOR_SPAWN_CLOSED = 0,
	FUNC_DOOR_SPAWN_OPEN,
};

// Since I'm here, might as well explain how these work.  Base.fgd is the file that connects
// flags to entities.  It is full of lines with this number, a label, and a default value.
// Voila, dynamicly generated checkboxes on the Flags tab of Entity Properties.

class CBaseDoor : public CBaseToggle
{
public:
	DECLARE_CLASS(CBaseDoor, CBaseToggle);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	CBaseDoor() : m_bIsBhopBlock(false) {};

	virtual void Spawn(void);
	virtual void Activate(void);

	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void StartBlocked(CBaseEntity *pOther);
	virtual void Blocked(CBaseEntity *pOther);
	virtual void EndBlocked(void);

	bool CreateVPhysics(void);

	// This is ONLY used by the node graph to test movement through a door
	void InputSetToggleState(inputdata_t &inputdata);
	virtual void SetToggleState(int state);

	virtual bool IsRotatingDoor() { return false; }
	virtual bool ShouldSavePhysics();
	// used to selectivly override defaults
	void DoorTouch(CBaseEntity *pOther);

	// local functions
	int DoorActivate(void);
	void DoorGoUp(void);
	void DoorGoDown(void);
	void DoorHitTop(void);
	void DoorHitBottom(void);
	void UpdateAreaPortals(bool isOpen);
	void Unlock(void);
	void Lock(void);
	int GetDoorMovementGroup(CBaseDoor *pDoorList[], int listMax);

	// Input handlers
	void InputClose(inputdata_t &inputdata);
	void InputLock(inputdata_t &inputdata);
	void InputOpen(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);
	void InputUnlock(inputdata_t &inputdata);
	void InputSetSpeed(inputdata_t &inputdata);

	Vector m_vecMoveDir;		// The direction of motion for linear moving doors.

	byte	m_bLockedSentence;
	byte	m_bUnlockedSentence;

	bool	m_bForceClosed;			// If set, always close, even if we're blocked.
	bool	m_bDoorGroup;
	bool	m_bLocked;				// Whether the door is locked
	bool	m_bIgnoreDebris;
	bool	m_bIgnoreNonPlayerEntsOnBlock;	// Non-player entities should never block.  This variable needs more letters.

	FuncDoorSpawnPos_t m_eSpawnPosition;

	float	m_flBlockDamage;		// Damage inflicted when blocked.
	string_t	m_NoiseMoving;		//Start/Looping sound
	string_t	m_NoiseArrived;		//End sound
	string_t	m_NoiseMovingClosed;		//Start/Looping sound
	string_t	m_NoiseArrivedClosed;		//End sound
	string_t	m_ChainTarget;		///< Entity name to pass Touch and Use events to

	CNetworkVar(unsigned int, m_iChainTargetCRC);
	CNetworkVar(float, m_flWaveHeight);

	// Outputs
	COutputEvent m_OnBlockedClosing;		// Triggered when the door becomes blocked while closing.
	COutputEvent m_OnBlockedOpening;		// Triggered when the door becomes blocked while opening.
	COutputEvent m_OnUnblockedClosing;		// Triggered when the door becomes unblocked while closing.
	COutputEvent m_OnUnblockedOpening;		// Triggered when the door becomes unblocked while opening.
	COutputEvent m_OnFullyClosed;			// Triggered when the door reaches the fully closed position.
	COutputEvent m_OnFullyOpen;				// Triggered when the door reaches the fully open position.
	COutputEvent m_OnClose;					// Triggered when the door is told to close.
	COutputEvent m_OnOpen;					// Triggered when the door is told to open.
	COutputEvent m_OnLockedUse;				// Triggered when the user tries to open a locked door.

	void			StartMovingSound(void);
	virtual void	StopMovingSound(void);
	void			MovingSoundThink(void);
#ifdef HL1_DLL
	bool		PassesBlockTouchFilter(CBaseEntity *pOther);
	string_t	m_iBlockFilterName;
	EHANDLE		m_hBlockFilter;
#endif

	bool		ShouldLoopMoveSound(void) { return m_bLoopMoveSound; }
	bool		m_bLoopMoveSound;			// Move sound loops until stopped
	bool		m_bIsBhopBlock;

private:
	void ChainUse(void);	///< Chains +use on through to m_ChainTarget
	void ChainTouch(CBaseEntity *pOther);	///< Chains touch on through to m_ChainTarget
	void SetChaining(bool chaining) { m_isChaining = chaining; }	///< Latch to prevent recursion
	bool m_isChaining;

	void CloseAreaPortalsThink(void);	///< Delays turning off area portals when closing doors to prevent visual artifacts

#ifdef GAME_DLL // Server specific things
public:
	virtual bool ShouldBlockNav() const OVERRIDE { return false; }
	virtual int UpdateTransmitState();
	virtual void Precache(void);
	virtual bool KeyValue(const char *szKeyName, const char *szValue);
	virtual int	ObjectCaps(void)
	{
		int flags = BaseClass::ObjectCaps();
		if (HasSpawnFlags(SF_DOOR_PUSE))
			return flags | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS;

		return flags;
	};

	locksound_t m_ls;			// door lock sounds

	DECLARE_DATADESC();
#endif
};

#endif