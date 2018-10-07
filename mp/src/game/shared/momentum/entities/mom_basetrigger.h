#ifndef _MOM_BASETRIGGER_H_
#define _MOM_BASETRIGGER_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basetoggle.h"
#include "mom_basefilter.h"
#include "mom_entityoutput.h"

#include "networkvar.h"
#include "checksum_crc.h"
#include "saverestore_utlvector.h"

#ifdef CLIENT_DLL
#define CBaseTrigger C_BaseTrigger
#else
#include "ai_basenpc.h"
#include "iservervehicle.h"
#endif

// Spawnflags
enum
{
	SF_TRIGGER_ALLOW_CLIENTS				= 0x01,		// Players can fire this trigger
	SF_TRIGGER_ALLOW_NPCS					= 0x02,		// NPCS can fire this trigger
	SF_TRIGGER_ALLOW_PUSHABLES				= 0x04,		// Pushables can fire this trigger
	SF_TRIGGER_ALLOW_PHYSICS				= 0x08,		// Physics objects can fire this trigger
	SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS		= 0x10,		// *if* NPCs can fire this trigger, this flag means only player allies do so
	SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES		= 0x20,		// *if* Players can fire this trigger, this flag means only players inside vehicles can 
	SF_TRIGGER_ALLOW_ALL					= 0x40,		// Everything can fire this trigger EXCEPT DEBRIS!
	SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES	= 0x200,	// *if* Players can fire this trigger, this flag means only players outside vehicles can 
	SF_TRIG_PUSH_ONCE						= 0x80,		// trigger_push removes itself after firing once
	SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER	= 0x100,	// if pushed object is player on a ladder, then this disengages them from the ladder (HL2only)
	SF_TRIG_TOUCH_DEBRIS 					= 0x400,	// Will touch physics debris objects
	SF_TRIGGER_ONLY_NPCS_IN_VEHICLES		= 0X800,	// *if* NPCs can fire this trigger, only NPCs in vehicles do so (respects player ally flag too)
	SF_TRIGGER_DISALLOW_BOTS                = 0x1000,   // Bots are not allowed to fire this trigger
};

class CBaseTrigger : public CBaseToggle
{
public:
	DECLARE_CLASS(CBaseTrigger, CBaseToggle);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	CBaseTrigger();

	virtual void Spawn(void);
	virtual void Activate(void);
	virtual void PostClientActive(void);
	virtual int UpdateTransmitState(void);

	void InitTrigger(void);

	void Enable(void);
	void Disable(void);
	void UpdateOnRemove(void);
	void TouchTest(void);

	// Input handlers
	virtual void InputEnable(inputdata_t &inputdata);
	virtual void InputDisable(inputdata_t &inputdata);
	virtual void InputToggle(inputdata_t &inputdata);
	virtual void InputTouchTest(inputdata_t &inputdata);

	virtual void InputStartTouch(inputdata_t &inputdata);
	virtual void InputEndTouch(inputdata_t &inputdata);

	virtual bool UsesFilter(void) { return (m_hFilter.Get() != NULL); }
	virtual bool PassesTriggerFilters(CBaseEntity *pOther);

	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

	virtual void OnStartTouch(CBaseEntity *pOther) {}
	virtual void OnEndTouch(CBaseEntity *pOther) {}

	virtual void StartTouchAll() {}
	virtual void EndTouchAll() {}

	bool IsTouching(CBaseEntity *pOther);

	CBaseEntity *GetTouchedEntityOfType(const char *sClassName);

	// by default, triggers don't deal with TraceAttack
	void TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType) {}

	bool PointIsWithin(const Vector &vecPoint);

	string_t	m_iFilterName;
	CHandle<class CBaseFilter>	m_hFilter;

	CNetworkVar(bool, m_bDisabled);
	CNetworkVar(unsigned int, m_iTargetCRC);
	CNetworkVar(unsigned int, m_iFilterCRC);

protected:
	// Outputs
	COutputEvent m_OnStartTouch;
	COutputEvent m_OnStartTouchAll;
	COutputEvent m_OnEndTouch;
	COutputEvent m_OnEndTouchAll;
	COutputEvent m_OnTouching;
	COutputEvent m_OnNotTouching;

	// Entities currently being touched by this trigger
	CUtlVector< EHANDLE >	m_hTouchingEntities;

#ifdef GAME_DLL // Server specific things
public:
	int	 DrawDebugTextOverlays(void);

	DECLARE_DATADESC();
#endif
};

#endif