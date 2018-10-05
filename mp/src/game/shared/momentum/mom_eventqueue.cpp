#include "cbase.h"
#include "mom_eventqueue.h"

//-----------------------------------------------------------------------------
//			CEventQueue implementation
//
// Purpose: holds and executes a global prioritized queue of entity actions
//-----------------------------------------------------------------------------
DEFINE_FIXEDSIZE_ALLOCATOR( EventQueuePrioritizedEvent_t, 128, CUtlMemoryPool::GROW_SLOW );

CEventQueue g_EventQueue;

CEventQueue::CEventQueue()
{
	m_Events.m_iFireTick = INT_MIN;
	m_Events.m_pNext = nullptr;

	Init();
}

CEventQueue::~CEventQueue()
{
	Clear();
}

// Robin: Left here for backwards compatability.
class CEventQueueSaveLoadProxy : public CBaseEntity
{
	DECLARE_CLASS( CEventQueueSaveLoadProxy, CBaseEntity );

	int Save( ISave &save )
	{
		if ( !BaseClass::Save(save) )
			return 0;

		// save out the message queue
		return g_EventQueue.Save( save );
	}


	int Restore( IRestore &restore )
	{
		if ( !BaseClass::Restore(restore) )
			return 0;

		// restore the event queue
		int iReturn = g_EventQueue.Restore( restore );

		// Now remove myself, because the CEventQueue_SaveRestoreBlockHandler
		// will handle future saves.
#ifdef GAME_DLL
		// MOM_TODO: Does client need to do this?
		UTIL_Remove( this );
#endif
		return iReturn;
	}
};

LINK_ENTITY_TO_CLASS(event_queue_saveload_proxy, CEventQueueSaveLoadProxy);

//-----------------------------------------------------------------------------
// EVENT QUEUE SAVE / RESTORE
//-----------------------------------------------------------------------------
static short EVENTQUEUE_SAVE_RESTORE_VERSION = 1;

class CEventQueue_SaveRestoreBlockHandler : public CDefSaveRestoreBlockHandler
{
public:
	const char *GetBlockName()
	{
		return "EventQueue";
	}

	//---------------------------------

	void Save( ISave *pSave )
	{
		g_EventQueue.Save( *pSave );
	}

	//---------------------------------

	void WriteSaveHeaders( ISave *pSave )
	{
		pSave->WriteShort( &EVENTQUEUE_SAVE_RESTORE_VERSION );
	}

	//---------------------------------

	void ReadRestoreHeaders( IRestore *pRestore )
	{
		// No reason why any future version shouldn't try to retain backward compatability. The default here is to not do so.
		short version;
		pRestore->ReadShort( &version );
		m_fDoLoad = ( version == EVENTQUEUE_SAVE_RESTORE_VERSION );
	}

	//---------------------------------

	void Restore( IRestore *pRestore, bool createPlayers )
	{
		if ( m_fDoLoad )
		{
			g_EventQueue.Restore( *pRestore );
		}
	}

private:
	bool m_fDoLoad;
};

//-----------------------------------------------------------------------------

CEventQueue_SaveRestoreBlockHandler g_EventQueue_SaveRestoreBlockHandler;

//-------------------------------------

ISaveRestoreBlockHandler *GetEventQueueSaveRestoreBlockHandler()
{
	return &g_EventQueue_SaveRestoreBlockHandler;
}


void CEventQueue::Init( void )
{
	Clear();
}

void CEventQueue::Clear( void )
{
	// delete all the events in the queue
	EventQueuePrioritizedEvent_t *pe = m_Events.m_pNext;

	while ( pe != NULL )
	{
		EventQueuePrioritizedEvent_t *next = pe->m_pNext;
		delete pe;
		pe = next;
	}

	m_Events.m_pNext = NULL;
}

void CEventQueue::Dump( void )
{
	EventQueuePrioritizedEvent_t *pe = m_Events.m_pNext;

	Msg("Dumping event queue. Current time is: %.2f\n",
		gpGlobals->curtime
	);

	while ( pe != NULL )
	{
		EventQueuePrioritizedEvent_t *next = pe->m_pNext;

		Msg("   (%d) Target: '%s', Input: '%s', Parameter '%s'. Activator: '%s', Caller '%s'.  \n", 
			pe->m_iFireTick,
			STRING(pe->m_iTarget), 
			STRING(pe->m_iTargetInput), 
			pe->m_VariantValue.String(),
			pe->m_pActivator ? pe->m_pActivator->GetDebugName() : "None", 
			pe->m_pCaller ? pe->m_pCaller->GetDebugName() : "None"  );

		pe = next;
	}

	Msg("Finished dump.\n");
}


//-----------------------------------------------------------------------------
// Purpose: adds the action into the correct spot in the priority queue, targeting entity via string name
//-----------------------------------------------------------------------------
void CEventQueue::AddEvent( const char *target, const char *targetInput, variant_t Value, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID )
{
	// build the new event
	EventQueuePrioritizedEvent_t *newEvent = new EventQueuePrioritizedEvent_t;
	newEvent->m_iFireTick = gpGlobals->tickcount + round(fireDelay * (1.0f / gpGlobals->interval_per_tick));	// priority key in the priority queue
	newEvent->m_iTarget = MAKE_STRING( target );
	newEvent->m_pEntTarget = NULL;
	newEvent->m_iTargetInput = MAKE_STRING( targetInput );
	newEvent->m_pActivator = pActivator;
	newEvent->m_pCaller = pCaller;
	newEvent->m_VariantValue = Value;
	newEvent->m_iOutputID = outputID;

	AddEvent( newEvent );
}

//-----------------------------------------------------------------------------
// Purpose: adds the action into the correct spot in the priority queue, targeting entity via pointer
//-----------------------------------------------------------------------------
void CEventQueue::AddEvent( CBaseEntity *target, const char *targetInput, variant_t Value, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID )
{
	// build the new event
	EventQueuePrioritizedEvent_t *newEvent = new EventQueuePrioritizedEvent_t;
	newEvent->m_iFireTick = gpGlobals->tickcount + round(fireDelay * (1.0f / gpGlobals->interval_per_tick));	// primary priority key in the priority queue
	newEvent->m_iTarget = NULL_STRING;
	newEvent->m_pEntTarget = target;
	newEvent->m_iTargetInput = MAKE_STRING( targetInput );
	newEvent->m_pActivator = pActivator;
	newEvent->m_pCaller = pCaller;
	newEvent->m_VariantValue = Value;
	newEvent->m_iOutputID = outputID;

	AddEvent( newEvent );
}

void CEventQueue::AddEvent( CBaseEntity *target, const char *action, float fireDelay, CBaseEntity *pActivator, CBaseEntity *pCaller, int outputID )
{
	variant_t Value;
	Value.Set( FIELD_VOID, NULL );
	AddEvent( target, action, Value, fireDelay, pActivator, pCaller, outputID );
}


//-----------------------------------------------------------------------------
// Purpose: private function, adds an event into the list
// Input  : *newEvent - the (already built) event to add
//-----------------------------------------------------------------------------
void CEventQueue::AddEvent( EventQueuePrioritizedEvent_t *newEvent )
{
	// loop through the actions looking for a place to insert
	EventQueuePrioritizedEvent_t *pe;
	for ( pe = &m_Events; pe->m_pNext != NULL; pe = pe->m_pNext )
	{
		if ( pe->m_pNext->m_iFireTick > newEvent->m_iFireTick )
		{
			break;
		}
	}

	Assert( pe );

	// insert
	newEvent->m_pNext = pe->m_pNext;
	newEvent->m_pPrev = pe;
	pe->m_pNext = newEvent;
	if ( newEvent->m_pNext )
	{
		newEvent->m_pNext->m_pPrev = newEvent;
	}
}

void CEventQueue::RemoveEvent( EventQueuePrioritizedEvent_t *pe )
{
	Assert( pe->m_pPrev );
	pe->m_pPrev->m_pNext = pe->m_pNext;
	if ( pe->m_pNext )
	{
		pe->m_pNext->m_pPrev = pe->m_pPrev;
	}
}


//-----------------------------------------------------------------------------
// Purpose: fires off any events in the queue who's fire time is (or before) the present time
//-----------------------------------------------------------------------------
void CEventQueue::ServiceEvents( void )
{
#ifdef CLIENT_DLL
	//Msg("[Client] CEventQueue::ServiceEvent\n");
#else
	//Msg("[Server] CEventQueue::ServiceEvent\n");
#endif
#ifdef GAME_DLL
	if (!CBaseEntity::Debug_ShouldStep())
	{
		return;
	}
#endif

	EventQueuePrioritizedEvent_t *pe = m_Events.m_pNext;

	while ( pe != NULL && pe->m_iFireTick <= gpGlobals->tickcount )
	{
		MDLCACHE_CRITICAL_SECTION();

		bool targetFound = false;

		// find the targets
		if ( pe->m_iTarget != NULL_STRING )
		{
			// In the context the event, the searching entity is also the caller
#ifdef GAME_DLL
			// MOM_TODO: Client needs to do this
			CBaseEntity *pSearchingEntity = pe->m_pCaller;
#endif
			CBaseEntity *target = NULL;
			while ( 1 )
			{
#ifdef GAME_DLL
				// MOM_TODO: Client needs to do this
				target = gEntList.FindEntityByName( target, pe->m_iTarget, pSearchingEntity, pe->m_pActivator, pe->m_pCaller );
#endif
				if ( !target )
					break;

				// pump the action into the target
				target->AcceptInput( STRING(pe->m_iTargetInput), pe->m_pActivator, pe->m_pCaller, pe->m_VariantValue, pe->m_iOutputID );
				targetFound = true;
			}
		}

		// direct pointer
		if ( pe->m_pEntTarget != NULL )
		{
			pe->m_pEntTarget->AcceptInput( STRING(pe->m_iTargetInput), pe->m_pActivator, pe->m_pCaller, pe->m_VariantValue, pe->m_iOutputID );
			targetFound = true;
		}

		if ( !targetFound )
		{
			// See if we can find a target if we treat the target as a classname
			if ( pe->m_iTarget != NULL_STRING )
			{
				CBaseEntity *target = NULL;
				while ( 1 )
				{
#ifdef GAME_DLL
					// MOM_TODO: Client needs to do this
					target = gEntList.FindEntityByClassname( target, STRING(pe->m_iTarget) );
#endif
					if ( !target )
						break;

					// pump the action into the target
					target->AcceptInput( STRING(pe->m_iTargetInput), pe->m_pActivator, pe->m_pCaller, pe->m_VariantValue, pe->m_iOutputID );
					targetFound = true;
				}
			}
		}

		if ( !targetFound )
		{
			const char *pClass ="", *pName = "";

			// might be NULL
			if ( pe->m_pCaller )
			{
				pClass = STRING(pe->m_pCaller->m_iClassname);
				pName = pe->m_pCaller->GetDebugName();
			}

			char szBuffer[256];
			Q_snprintf( szBuffer, sizeof(szBuffer), "unhandled input: (%s) -> (%s), from (%s,%s); target entity not found\n", STRING(pe->m_iTargetInput), STRING(pe->m_iTarget), pClass, pName );
			DevMsg( 2, "%s", szBuffer );
#ifdef GAME_DLL
			ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
#endif
		}

		// remove the event from the list (remembering that the queue may have been added to)
		RemoveEvent( pe );
		delete pe;

#ifdef GAME_DLL
		//
		// If we are in debug mode, exit the loop if we have fired the correct number of events.
		//
		if (CBaseEntity::Debug_IsPaused())
		{
			if (!CBaseEntity::Debug_Step())
			{
				break;
			}
		}
#endif

		// restart the list (to catch any new items have probably been added to the queue)
		pe = m_Events.m_pNext;	
	}
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Dumps the contents of the Entity I/O event queue to the console.
//-----------------------------------------------------------------------------
void CC_DumpEventQueue()
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	g_EventQueue.Dump();
}
static ConCommand dumpeventqueue( "dumpeventqueue", CC_DumpEventQueue, "Dump the contents of the Entity I/O event queue to the console." );
#endif

//-----------------------------------------------------------------------------
// Purpose: Removes all pending events from the I/O queue that were added by the
//			given caller.
//
//			TODO: This is only as reliable as callers are in passing the correct
//				  caller pointer when they fire the outputs. Make more foolproof.
//-----------------------------------------------------------------------------
void CEventQueue::CancelEvents( CBaseEntity *pCaller )
{
	if (!pCaller)
		return;

	EventQueuePrioritizedEvent_t *pCur = m_Events.m_pNext;

	while (pCur != NULL)
	{
		bool bDelete = false;
		if (pCur->m_pCaller == pCaller)
		{
			// Pointers match; make sure everything else matches.
			if (!stricmp(pCur->m_pCaller->GetDebugName(), pCaller->GetDebugName()) &&
				!stricmp(pCur->m_pCaller->GetClassname(), pCaller->GetClassname()))
			{
				// Found a matching event; delete it from the queue.
				bDelete = true;
			}
		}

		EventQueuePrioritizedEvent_t *pCurSave = pCur;
		pCur = pCur->m_pNext;

		if (bDelete)
		{
			RemoveEvent( pCurSave );
			delete pCurSave;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Removes all pending events of the specified type from the I/O queue of the specified target
//
//			TODO: This is only as reliable as callers are in passing the correct
//				  caller pointer when they fire the outputs. Make more foolproof.
//-----------------------------------------------------------------------------
void CEventQueue::CancelEventOn( CBaseEntity *pTarget, const char *sInputName )
{
	if (!pTarget)
		return;

	EventQueuePrioritizedEvent_t *pCur = m_Events.m_pNext;
	const size_t inputNameSize = strlen(sInputName);
	while (pCur != nullptr)
	{
		bool bDelete = false;
		if (pCur->m_pEntTarget == pTarget)
		{
			if (!Q_strncmp(STRING(pCur->m_iTargetInput), sInputName, inputNameSize))
			{
				// Found a matching event; delete it from the queue.
				bDelete = true;
			}
		}

		EventQueuePrioritizedEvent_t *pCurSave = pCur;
		pCur = pCur->m_pNext;

		if (bDelete)
		{
			RemoveEvent( pCurSave );
			delete pCurSave;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the target has any pending inputs.
// Input  : *pTarget - 
//			*sInputName - NULL for any input, or a specified one
//-----------------------------------------------------------------------------
bool CEventQueue::HasEventPending( CBaseEntity *pTarget, const char *sInputName )
{
	if (!pTarget)
		return false;

	EventQueuePrioritizedEvent_t *pCur = m_Events.m_pNext;
	const size_t inputNameSize = strlen(sInputName);
	while (pCur != nullptr)
	{
		if (pCur->m_pEntTarget == pTarget)
		{
			if (!sInputName || !Q_strncmp(STRING(pCur->m_iTargetInput), sInputName, inputNameSize))
				return true;
		}

		pCur = pCur->m_pNext;
	}

	return false;
}

// MOM_TODO: This is only called from CServerGameDLL::GameFrame where should we add it to client?
void ServiceEventQueue( void )
{
	VPROF("ServiceEventQueue()");

	g_EventQueue.ServiceEvents();
}



// save data description for the event queue
BEGIN_SIMPLE_DATADESC( CEventQueue )
// These are saved explicitly in CEventQueue::Save below
// DEFINE_FIELD( m_Events, EventQueuePrioritizedEvent_t ),

DEFINE_FIELD( m_iListCount, FIELD_INTEGER ),	// this value is only used during save/restore
END_DATADESC()


// save data for a single event in the queue
BEGIN_SIMPLE_DATADESC( EventQueuePrioritizedEvent_t )
DEFINE_FIELD( m_iFireTick, FIELD_INTEGER ),
DEFINE_FIELD( m_iTarget, FIELD_STRING ),
DEFINE_FIELD( m_iTargetInput, FIELD_STRING ),
DEFINE_FIELD( m_pActivator, FIELD_EHANDLE ),
DEFINE_FIELD( m_pCaller, FIELD_EHANDLE ),
DEFINE_FIELD( m_pEntTarget, FIELD_EHANDLE ),
DEFINE_FIELD( m_iOutputID, FIELD_INTEGER ),
DEFINE_CUSTOM_FIELD( m_VariantValue, variantFuncs ),

//	DEFINE_FIELD( m_pNext, FIELD_??? ),
//	DEFINE_FIELD( m_pPrev, FIELD_??? ),
END_DATADESC()

int CEventQueue::Save( ISave &save )
{
	// count the number of items in the queue
	EventQueuePrioritizedEvent_t *pe;

	m_iListCount = 0;
	for ( pe = m_Events.m_pNext; pe != NULL; pe = pe->m_pNext )
	{
		m_iListCount++;
	}

	// save that value out to disk, so we know how many to restore
	if ( !save.WriteFields( "EventQueue", this, NULL, m_DataMap.dataDesc, m_DataMap.dataNumFields ) )
		return 0;

	// cycle through all the events, saving them all
	for ( pe = m_Events.m_pNext; pe != NULL; pe = pe->m_pNext )
	{
		if ( !save.WriteFields( "PEvent", pe, NULL, pe->m_DataMap.dataDesc, pe->m_DataMap.dataNumFields ) )
			return 0;
	}

	return 1;
}

int CEventQueue::Restore( IRestore &restore )
{
	// clear the event queue
	Clear();

	// rebuild the event queue by restoring all the queue items
	EventQueuePrioritizedEvent_t tmpEvent;

	// load the number of items saved
	if ( !restore.ReadFields( "EventQueue", this, NULL, m_DataMap.dataDesc, m_DataMap.dataNumFields ) )
		return 0;

	for ( int i = 0; i < m_iListCount; i++ )
	{
		if ( !restore.ReadFields( "PEvent", &tmpEvent, NULL, tmpEvent.m_DataMap.dataDesc, tmpEvent.m_DataMap.dataNumFields ) )
			return 0;

		// add the restored event into the list
		if ( tmpEvent.m_pEntTarget )
		{
			AddEvent( tmpEvent.m_pEntTarget,
				STRING(tmpEvent.m_iTargetInput),
				tmpEvent.m_VariantValue,
#ifdef TF_DLL
				tmpEvent.m_flFireTime - engine->GetServerTime(),
#else
				tmpEvent.m_iFireTick - gpGlobals->tickcount,
#endif
				tmpEvent.m_pActivator,
				tmpEvent.m_pCaller,
				tmpEvent.m_iOutputID );
		}
		else
		{
			AddEvent( STRING(tmpEvent.m_iTarget),
				STRING(tmpEvent.m_iTargetInput),
				tmpEvent.m_VariantValue,
#ifdef TF_DLL
				tmpEvent.m_flFireTime - engine->GetServerTime(),
#else
				tmpEvent.m_iFireTick - gpGlobals->tickcount,
#endif
				tmpEvent.m_pActivator,
				tmpEvent.m_pCaller,
				tmpEvent.m_iOutputID );
		}
	}

	return 1;
}
