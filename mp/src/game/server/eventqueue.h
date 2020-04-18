//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A global class that holds a prioritized queue of entity I/O events.
//			Events can be posted with a nonzero delay, which determines how long
//			they are held before being dispatched to their recipients.
//
//			The queue is serviced once per server frame.
//
//=============================================================================//

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H
#ifdef _WIN32
#pragma once
#endif

#include "mempool.h"

struct EventQueuePrioritizedEvent_t
{
	int m_iFireTick;
	string_t m_iTarget;
	string_t m_iTargetInput;
	EHANDLE m_pActivator;
	EHANDLE m_pCaller;
	int m_iOutputID;
	EHANDLE m_pEntTarget;  // a pointer to the entity to target; overrides m_iTarget

	variant_t m_VariantValue;	// variable-type parameter

	EventQueuePrioritizedEvent_t *m_pNext;
	EventQueuePrioritizedEvent_t *m_pPrev;

	DECLARE_SIMPLE_DATADESC();

	DECLARE_FIXEDSIZE_ALLOCATOR( PrioritizedEvent_t );
};

class CEventQueueEvent
{
public:
	void FromPrioritizedEvent( const EventQueuePrioritizedEvent_t *pe, CBaseEntity *pAbstractedEntity );
	void ToPrioritizedEvent( EventQueuePrioritizedEvent_t *pe, CBaseEntity *pAbstractedEntity ) const;
	void LoadFromKeyValues( KeyValues* kv );
	void SaveToKeyValues( KeyValues* kv ) const;
public:
	int m_iFireDelayTicks;
	string_t m_iTarget;
	string_t m_iTargetInput;
	string_t m_szActivator;
	int m_iCaller; // ent idx of caller
	int m_iOutputID;
	string_t m_szEntTarget; // name of entity to target; overrides m_iTarget

	variant_t m_VariantValue; // variable-type parameter

	// This event gets saved with respect to a certain entity we are interested in (probably a player).
	// If any of the target, activator, or caller are that entity, keep a record of that so we can
	// set those to the entity we end up restoring this event for later.
	bool m_bAbstractTarget;
	bool m_bAbstractActivator;
	bool m_bAbstractCaller;
};

class CEventQueueState
{
public:
	void LoadFromKeyValues( KeyValues* kv );
	void SaveToKeyValues( KeyValues* kv ) const;
public:
	CUtlVector<CEventQueueEvent> m_vecEvents;
};

class CEventQueue
{
public:
	// pushes an event into the queue, targeting a string name (m_iName), or directly by a pointer
	void AddEvent( const char *target, const char *action, variant_t Value, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID = 0 );
	void AddEvent( CBaseEntity *target, const char *action, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID = 0 );
	void AddEvent( CBaseEntity *target, const char *action, variant_t Value, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID = 0 );

	void CancelEvents( CBaseEntity *pCaller );
	void CancelEventOn( CBaseEntity *pTarget, const char *sInputName );
	bool HasEventPending( CBaseEntity *pTarget, const char *sInputName );
	bool EventAffectsEntity( EventQueuePrioritizedEvent_t* event, CBaseEntity* pTarget );

	// services the queue, firing off any events who's time hath come
	void ServiceEvents( void );

	// debugging
	void ValidateQueue( void );

	// serialization
	int Save( ISave &save );
	int Restore( IRestore &restore );

	void SaveAll( CEventQueueState &state );
	void RestoreAll( const CEventQueueState& state );
	void SaveForTarget( CBaseEntity *pTarget, CEventQueueState &state );
	void RestoreForTarget( CBaseEntity *pTarget, const CEventQueueState& state );

	CEventQueue();
	~CEventQueue();

	void Init( void );
	void Clear( void ); // resets the list

	void Dump( void );

private:

	void AddEvent( EventQueuePrioritizedEvent_t *event );
	void RemoveEvent( EventQueuePrioritizedEvent_t *pe );

	DECLARE_SIMPLE_DATADESC();
	EventQueuePrioritizedEvent_t m_Events;
	int m_iListCount;
};

extern CEventQueue g_EventQueue;


#endif // EVENTQUEUE_H

