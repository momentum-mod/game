//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*
Entity Data Descriptions

Each entity has an array which defines it's data in way that is useful for
entity communication, parsing initial values from the map, and save/restore.

each entity has to have the following line in it's class declaration:

	DECLARE_DATADESC();

this line defines that it has an m_DataDesc[] array, and declares functions through
which various subsystems can iterate the data.

In it's implementation, each entity has to have:

	typedescription_t CBaseEntity::m_DataDesc[] = { ... }
	
in which all it's data is defined (see below), followed by:


which implements the functions necessary for iterating through an entities data desc.

There are several types of data:
	
	FIELD		: this is a variable which gets saved & loaded from disk
	KEY			: this variable can be read in from the map file
	GLOBAL		: a global field is actually local; it is saved/restored, but is actually
				  unique to the entity on each level.
	CUSTOM		: the save/restore parsing functions are described by the user.
	ARRAY		: an array of values
	OUTPUT		: a variable or event that can be connected to other entities (see below)
	INPUTFUNC	: maps a string input to a function pointer. Outputs connected to this input
				  will call the notify function when fired.
	INPUT		: maps a string input to a member variable. Outputs connected to this input
				  will update the input data value when fired.
	INPUTNOTIFY	: maps a string input to a member variable/function pointer combo. Outputs
				  connected to this input will update the data value and call the notify
				  function when fired.

some of these can overlap.  all the data descriptions usable are:

	DEFINE_FIELD(		name,	fieldtype )
	DEFINE_KEYFIELD(	name,	fieldtype,	mapname )
	DEFINE_KEYFIELD_NOTSAVED(	name,	fieldtype,	mapname )
	DEFINE_ARRAY(		name,	fieldtype,	count )
	DEFINE_GLOBAL_FIELD(name,	fieldtype )
	DEFINE_CUSTOM_FIELD(name,	datafuncs,	mapname )
	DEFINE_GLOBAL_KEYFIELD(name,	fieldtype,	mapname )

where:
	type is the name of the class (eg. CBaseEntity)
	name is the name of the variable in the class (eg. m_iHealth)
	fieldtype is the type of data (FIELD_STRING, FIELD_INTEGER, etc)
	mapname is the string by which this variable is associated with map file data
	count is the number of items in the array
	datafuncs is a struct containing function pointers for a custom-defined save/restore/parse

OUTPUTS:

 	DEFINE_OUTPUT(		outputvar,	outputname )

	This maps the string 'outputname' to the COutput-derived member variable outputvar.  In the VMF
	file these outputs can be hooked up to inputs (see above).  Whenever the internal state
	of an entity changes it will often fire off outputs so that map makers can hook up behaviors.
	e.g.  A door entity would have OnDoorOpen, OnDoorClose, OnTouched, etc outputs.
*/


#include "cbase.h"
#include "entitylist.h"
#include "mapentities_shared.h"
#include "isaverestore.h"
#include "eventqueue.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "mempool.h"
#include "tier1/strtools.h"
#include "datacache/imdlcache.h"
#include "env_debughistory.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ISaveRestoreOps *variantFuncs;	// function pointer set for save/restoring variants

BEGIN_SIMPLE_DATADESC( CEventAction )
	DEFINE_FIELD( m_iTarget, FIELD_STRING ),
	DEFINE_FIELD( m_iTargetInput, FIELD_STRING ),
	DEFINE_FIELD( m_iParameter, FIELD_STRING ),
	DEFINE_FIELD( m_flDelay, FIELD_FLOAT ),
	DEFINE_FIELD( m_nTimesToFire, FIELD_INTEGER ),
	DEFINE_FIELD( m_iIDStamp, FIELD_INTEGER ),

	// This is dealt with by the Restore method
	// DEFINE_FIELD( m_pNext, CEventAction ),
END_DATADESC()


// ID Stamp used to uniquely identify every output
int CEventAction::s_iNextIDStamp = 0;


//-----------------------------------------------------------------------------
// Purpose: Creates an event action and assigns it an unique ID stamp.
// Input  : ActionData - the map file data block describing the event action.
//-----------------------------------------------------------------------------
CEventAction::CEventAction( const char *ActionData )
{
	m_pNext = NULL;
	m_iIDStamp = ++s_iNextIDStamp;

	m_flDelay = 0;
	m_iTarget = NULL_STRING;
	m_iParameter = NULL_STRING;
	m_iTargetInput = NULL_STRING;
	m_nTimesToFire = EVENT_FIRE_ALWAYS;

	if (ActionData == NULL)
		return;

	char szToken[256];

	//
	// Parse the target name.
	//
	const char *psz = nexttoken(szToken, ActionData, ',', sizeof(szToken));
	if (szToken[0] != '\0')
	{
		m_iTarget = AllocPooledString(szToken);
	}

	//
	// Parse the input name.
	//
	psz = nexttoken(szToken, psz, ',', sizeof(szToken));
	if (szToken[0] != '\0')
	{
		m_iTargetInput = AllocPooledString(szToken);
	}
	else
	{
		m_iTargetInput = AllocPooledString("Use");
	}

	//
	// Parse the parameter override.
	//
	psz = nexttoken(szToken, psz, ',', sizeof(szToken));
	if (szToken[0] != '\0')
	{
		m_iParameter = AllocPooledString(szToken);
	}

	//
	// Parse the delay.
	//
	psz = nexttoken(szToken, psz, ',', sizeof(szToken));
	if (szToken[0] != '\0')
	{
		m_flDelay = atof(szToken);
	}

	//
	// Parse the number of times to fire.
	//
	nexttoken(szToken, psz, ',', sizeof(szToken));
	if (szToken[0] != '\0')
	{
		m_nTimesToFire = atoi(szToken);
		if (m_nTimesToFire == 0)
		{
			m_nTimesToFire = EVENT_FIRE_ALWAYS;
		}
	}
}


// this memory pool stores blocks around the size of CEventAction/inputitem_t structs
// can be used for other blocks; will error if to big a block is tried to be allocated
CUtlMemoryPool g_EntityListPool( MAX(sizeof(CEventAction),sizeof(CMultiInputVar::inputitem_t)), 512, CUtlMemoryPool::GROW_FAST, "g_EntityListPool" );

#include "tier0/memdbgoff.h"

void *CEventAction::operator new( size_t stAllocateBlock )
{
	return g_EntityListPool.Alloc( stAllocateBlock );
}

void *CEventAction::operator new( size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine )
{
	return g_EntityListPool.Alloc( stAllocateBlock );
}

void CEventAction::operator delete( void *pMem )
{
	g_EntityListPool.Free( pMem );
}

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Returns the highest-valued delay in our list of event actions.
//-----------------------------------------------------------------------------
float CBaseEntityOutput::GetMaxDelay(void)
{
	float flMaxDelay = 0;
	CEventAction *ev = m_ActionList;

	while (ev != NULL)
	{
		if (ev->m_flDelay > flMaxDelay)
		{
			flMaxDelay = ev->m_flDelay;
		}
		ev = ev->m_pNext;
	}

	return(flMaxDelay);
}


//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CBaseEntityOutput::~CBaseEntityOutput()
{
	CEventAction *ev = m_ActionList;
	while (ev != NULL)
	{
		CEventAction *pNext = ev->m_pNext;	
		delete ev;
		ev = pNext;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Fires the event, causing a sequence of action to occur in other ents.
// Input  : pActivator - Entity that initiated this sequence of actions.
//			pCaller - Entity that is actually causing the event.
//-----------------------------------------------------------------------------
void CBaseEntityOutput::FireOutput(variant_t Value, CBaseEntity *pActivator, CBaseEntity *pCaller, float fDelay)
{
	//
	// Iterate through all eventactions and fire them off.
	//
	CEventAction *ev = m_ActionList;
	CEventAction *prev = NULL;
	
	while (ev != NULL)
	{
		if (ev->m_iParameter == NULL_STRING)
		{
			//
			// Post the event with the default parameter.
			//
			g_EventQueue.AddEvent( STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), Value, ev->m_flDelay + fDelay, pActivator, pCaller, ev->m_iIDStamp );
		}
		else
		{
			//
			// Post the event with a parameter override.
			//
			variant_t ValueOverride;
			ValueOverride.SetString( ev->m_iParameter );
			g_EventQueue.AddEvent( STRING(ev->m_iTarget), STRING(ev->m_iTargetInput), ValueOverride, ev->m_flDelay, pActivator, pCaller, ev->m_iIDStamp );
		}

		if ( ev->m_flDelay )
		{
			char szBuffer[256];
			Q_snprintf( szBuffer,
						sizeof(szBuffer),
						"(%0.2f) output: (%s,%s) -> (%s,%s,%.1f)(%s)\n",
#ifdef TF_DLL
						engine->GetServerTime(),
#else
						gpGlobals->curtime,
#endif
						pCaller ? STRING(pCaller->m_iClassname) : "NULL",
						pCaller ? STRING(pCaller->GetEntityName()) : "NULL",
						STRING(ev->m_iTarget),
						STRING(ev->m_iTargetInput),
						ev->m_flDelay,
						STRING(ev->m_iParameter) );

			DevMsg( 2, "%s", szBuffer );
			ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
		}
		else
		{
			char szBuffer[256];
			Q_snprintf( szBuffer,
						sizeof(szBuffer),
						"(%0.2f) output: (%s,%s) -> (%s,%s)(%s)\n",
#ifdef TF_DLL
						engine->GetServerTime(),
#else
						gpGlobals->curtime,
#endif
						pCaller ? STRING(pCaller->m_iClassname) : "NULL",
						pCaller ? STRING(pCaller->GetEntityName()) : "NULL", STRING(ev->m_iTarget),
						STRING(ev->m_iTargetInput),
						STRING(ev->m_iParameter) );

			DevMsg( 2, "%s", szBuffer );
			ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
		}

		if ( pCaller && pCaller->m_debugOverlays & OVERLAY_MESSAGE_BIT)
		{
			pCaller->DrawOutputOverlay(ev);
		}

		//
		// Remove the event action from the list if it was set to be fired a finite
		// number of times (and has been).
		//
		bool bRemove = false;
		if (ev->m_nTimesToFire != EVENT_FIRE_ALWAYS)
		{
			ev->m_nTimesToFire--;
			if (ev->m_nTimesToFire == 0)
			{
				char szBuffer[256];
				Q_snprintf( szBuffer, sizeof(szBuffer), "Removing from action list: (%s,%s) -> (%s,%s)\n", pCaller ? STRING(pCaller->m_iClassname) : "NULL", pCaller ? STRING(pCaller->GetEntityName()) : "NULL", STRING(ev->m_iTarget), STRING(ev->m_iTargetInput));
				DevMsg( 2, "%s", szBuffer );
				ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
				bRemove = true;
			}
		}

		if (!bRemove)
		{
			prev = ev;
			ev = ev->m_pNext;
		}
		else
		{
			if (prev != NULL)
			{
				prev->m_pNext = ev->m_pNext;
			}
			else
			{
				m_ActionList = ev->m_pNext;
			}

			CEventAction *next = ev->m_pNext;
			delete ev;
			ev = next;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Parameterless firing of an event
// Input  : pActivator - 
//			pCaller - 
//-----------------------------------------------------------------------------
void COutputEvent::FireOutput(CBaseEntity *pActivator, CBaseEntity *pCaller, float fDelay)
{
	variant_t Val;
	Val.Set( FIELD_VOID, NULL );
	CBaseEntityOutput::FireOutput(Val, pActivator, pCaller, fDelay);
}


void CBaseEntityOutput::ParseEventAction( const char *EventData )
{
	AddEventAction( new CEventAction( EventData ) );
}

void CBaseEntityOutput::AddEventAction( CEventAction *pEventAction )
{
	pEventAction->m_pNext = m_ActionList;
	m_ActionList = pEventAction;
}


// save data description for the event queue
BEGIN_SIMPLE_DATADESC( CBaseEntityOutput )

	DEFINE_CUSTOM_FIELD( m_Value, variantFuncs ),

	// This is saved manually by CBaseEntityOutput::Save
	// DEFINE_FIELD( m_ActionList, CEventAction ),
END_DATADESC()


int CBaseEntityOutput::Save( ISave &save )
{
	// save that value out to disk, so we know how many to restore
	if ( !save.WriteFields( "Value", this, NULL, m_DataMap.dataDesc, m_DataMap.dataNumFields ) )
		return 0;

	for ( CEventAction *ev = m_ActionList; ev != NULL; ev = ev->m_pNext )
	{
		if ( !save.WriteFields( "EntityOutput", ev, NULL, ev->m_DataMap.dataDesc, ev->m_DataMap.dataNumFields ) )
			return 0;
	}

	return 1;
}

int CBaseEntityOutput::Restore( IRestore &restore, int elementCount )
{
	// load the number of items saved
	if ( !restore.ReadFields( "Value", this, NULL, m_DataMap.dataDesc, m_DataMap.dataNumFields ) )
		return 0;

	m_ActionList = NULL;

	// read in all the fields
	CEventAction *lastEv = NULL;
	for ( int i = 0; i < elementCount; i++ )
	{
		CEventAction *ev = new CEventAction(NULL);

		if ( !restore.ReadFields( "EntityOutput", ev, NULL, ev->m_DataMap.dataDesc, ev->m_DataMap.dataNumFields ) )
			return 0;

		// add it to the list in the same order it was saved in
		if ( lastEv )
		{
			lastEv->m_pNext = ev;
		}
		else
		{
			m_ActionList = ev;
		}
		ev->m_pNext = NULL;
		lastEv = ev;
	}

	return 1;
}

int CBaseEntityOutput::NumberOfElements( void )
{
	int count = 0;
	for ( CEventAction *ev = m_ActionList; ev != NULL; ev = ev->m_pNext )
	{
		count++;
	}
	return count;
}

/// Delete every single action in the action list. 
void CBaseEntityOutput::DeleteAllElements( void ) 
{
	// walk front to back, deleting as we go. We needn't fix up pointers because
	// EVERYTHING will die.

	CEventAction *pNext = m_ActionList;
	// wipe out the head
	m_ActionList = NULL;
	while (pNext)
	{
		CEventAction *strikeThis = pNext;
		pNext = pNext->m_pNext;
		delete strikeThis;
	}

}

/// EVENTS save/restore parsing wrapper

class CEventsSaveDataOps : public ISaveRestoreOps
{
	virtual void Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
	{
		AssertMsg( fieldInfo.pTypeDesc->fieldSize == 1, "CEventsSaveDataOps does not support arrays");

		CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
		const int fieldSize = fieldInfo.pTypeDesc->fieldSize;
 		for ( int i = 0; i < fieldSize; i++, ev++ )
		{
			// save out the number of fields
			int numElements = ev->NumberOfElements();
			pSave->WriteInt( &numElements, 1 );

			// save the event data
			ev->Save( *pSave );
		}
	}

	virtual void Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
	{
		AssertMsg( fieldInfo.pTypeDesc->fieldSize == 1, "CEventsSaveDataOps does not support arrays");

		CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
		const int fieldSize = fieldInfo.pTypeDesc->fieldSize;
		for ( int i = 0; i < fieldSize; i++, ev++ )
		{
			int nElements = pRestore->ReadInt();
			
			Assert( nElements < 100 );

			ev->Restore( *pRestore, nElements );
		}
	}

	virtual bool IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		AssertMsg( fieldInfo.pTypeDesc->fieldSize == 1, "CEventsSaveDataOps does not support arrays");
		
		// check all the elements of the array (usually only 1)
		CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
		const int fieldSize = fieldInfo.pTypeDesc->fieldSize;
		for ( int i = 0; i < fieldSize; i++, ev++ )
		{
			// It's not empty if it has events or if it has a non-void variant value
			if (( ev->NumberOfElements() != 0 ) || ( ev->ValueFieldType() != FIELD_VOID ))
				return 0;
		}

		// variant has no data
		return 1;
	}

	virtual void MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		// Don't no how to. This is okay, since objects of this type
		// are always born clean before restore, and not reused
	}

	virtual bool Parse( const SaveRestoreFieldInfo_t &fieldInfo, char const* szValue )
	{
		CBaseEntityOutput *ev = (CBaseEntityOutput*)fieldInfo.pField;
		ev->ParseEventAction( szValue );
		return true;
	}
};

CEventsSaveDataOps g_EventsSaveDataOps;
ISaveRestoreOps *eventFuncs = &g_EventsSaveDataOps;

//-----------------------------------------------------------------------------
//			CMultiInputVar implementation
//
// Purpose: holds a list of inputs and their ID tags
//			used for entities that hold inputs from a set of other entities
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Purpose: destructor, frees the data list
//-----------------------------------------------------------------------------
CMultiInputVar::~CMultiInputVar()
{
	if ( m_InputList )
	{
		while ( m_InputList->next != NULL )
		{
			inputitem_t *input = m_InputList->next;
			m_InputList->next = input->next;
			delete input;
		}
		delete m_InputList;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the data set with a new value
// Input  : newVal - the new value to add to or update in the list
//			outputID - the source of the value
//-----------------------------------------------------------------------------
void CMultiInputVar::AddValue( variant_t newVal, int outputID )
{
	// see if it's already in the list
	inputitem_t *inp;
	for ( inp = m_InputList; inp != NULL; inp = inp->next )
	{
		// already in list, so just update this link
		if ( inp->outputID == outputID )
		{
			inp->value = newVal;
			return;
		}
	}

	// add to start of list
	inp = new inputitem_t;
	inp->value = newVal;
	inp->outputID = outputID;
	if ( !m_InputList )
	{
		m_InputList = inp;
		inp->next = NULL;
	}
	else
	{
		inp->next = m_InputList;
		m_InputList = inp;
	}
}


#include "tier0/memdbgoff.h"

//-----------------------------------------------------------------------------
// Purpose: allocates memory from the entitylist pool
//-----------------------------------------------------------------------------
void *CMultiInputVar::inputitem_t::operator new( size_t stAllocateBlock )
{
	return g_EntityListPool.Alloc( stAllocateBlock );
}

void *CMultiInputVar::inputitem_t::operator new( size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine )
{
	return g_EntityListPool.Alloc( stAllocateBlock );
}

//-----------------------------------------------------------------------------
// Purpose: frees memory from the entitylist pool
//-----------------------------------------------------------------------------
void CMultiInputVar::inputitem_t::operator delete( void *pMem )
{
	g_EntityListPool.Free( pMem );
}

#include "tier0/memdbgon.h"


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

// Robin: Left here for backwards compatibility.
class CEventQueueSaveLoadProxy : public CLogicalEntity
{
	DECLARE_CLASS( CEventQueueSaveLoadProxy, CLogicalEntity );

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
		UTIL_Remove( this );

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
		// No reason why any future version shouldn't try to retain backward compatibility. The default here is to not do so.
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
#ifdef TF_DLL
		engine->GetServerTime()
#else
		gpGlobals->curtime
#endif
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
#ifdef TF_DLL
	newEvent->m_flFireTime = engine->GetServerTime() + fireDelay;	// priority key in the priority queue
#else
	newEvent->m_iFireTick = gpGlobals->tickcount + round(fireDelay * (1.0f / gpGlobals->interval_per_tick));	// priority key in the priority queue
#endif
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
#ifdef TF_DLL
	newEvent->m_flFireTime = engine->GetServerTime() + fireDelay;	// primary priority key in the priority queue
#else
	newEvent->m_iFireTick = gpGlobals->tickcount + round(fireDelay * (1.0f / gpGlobals->interval_per_tick));	// primary priority key in the priority queue
#endif
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
	if (!CBaseEntity::Debug_ShouldStep())
	{
		return;
	}

	EventQueuePrioritizedEvent_t *pe = m_Events.m_pNext;

#ifdef TF_DLL
	while ( pe != NULL && pe->m_flFireTime <= engine->GetServerTime() )
#else
	while ( pe != NULL && pe->m_iFireTick <= gpGlobals->tickcount )
#endif
	{
		MDLCACHE_CRITICAL_SECTION();

		bool targetFound = false;

		// find the targets
		if ( pe->m_iTarget != NULL_STRING )
		{
			// In the context the event, the searching entity is also the caller
			CBaseEntity *pSearchingEntity = pe->m_pCaller;
			CBaseEntity *target = NULL;
			while ( 1 )
			{
				target = gEntList.FindEntityByName( target, pe->m_iTarget, pSearchingEntity, pe->m_pActivator, pe->m_pCaller );
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
					target = gEntList.FindEntityByClassname( target, STRING(pe->m_iTarget) );
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
				pName = STRING(pe->m_pCaller->GetEntityName());
			}
			
			char szBuffer[256];
			Q_snprintf( szBuffer, sizeof(szBuffer), "unhandled input: (%s) -> (%s), from (%s,%s); target entity not found\n", STRING(pe->m_iTargetInput), STRING(pe->m_iTarget), pClass, pName );
			DevMsg( 2, "%s", szBuffer );
			ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
		}

		// remove the event from the list (remembering that the queue may have been added to)
		RemoveEvent( pe );
		delete pe;

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

		// restart the list (to catch any new items have probably been added to the queue)
		pe = m_Events.m_pNext;	
	}
}

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
			if (!stricmp(STRING(pCur->m_pCaller->GetEntityName()), STRING(pCaller->GetEntityName())) &&
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

bool CEventQueue::EventAffectsEntity(EventQueuePrioritizedEvent_t* event, CBaseEntity* pTarget)
{
	CBaseEntity* pSearchingEntity = event->m_pCaller;
	if (event->m_pActivator == pTarget ||
		event->m_pEntTarget == pTarget ||
		pTarget == gEntList.FindEntityByName(nullptr, event->m_iTarget, pSearchingEntity, event->m_pActivator, event->m_pCaller))
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Removes all pending events of the specified type from the I/O queue of the specified target
// Input  : *pTarget - 
//			*sInputName - NULL for any input, or a specified one
//
//			TODO: This is only as reliable as callers are in passing the correct
//				  caller pointer when they fire the outputs. Make more foolproof.
//-----------------------------------------------------------------------------
void CEventQueue::CancelEventOn( CBaseEntity *pTarget, const char *sInputName )
{
	if (!pTarget)
		return;

	EventQueuePrioritizedEvent_t *pCur = m_Events.m_pNext;
	const size_t inputNameSize = sInputName ? strlen(sInputName) : 0;
	while (pCur != nullptr)
	{
		bool bDelete = false;
		if (EventAffectsEntity(pCur, pTarget))
		{
			if (!sInputName || !Q_strncmp(STRING(pCur->m_iTargetInput), sInputName, inputNameSize))
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
	const size_t inputNameSize = sInputName ? strlen(sInputName) : 0;
	while (pCur != nullptr)
	{
		if (EventAffectsEntity(pCur, pTarget))
		{
			if (!sInputName || !Q_strncmp(STRING(pCur->m_iTargetInput), sInputName, inputNameSize))
				return true;
		}

		pCur = pCur->m_pNext;
	}

	return false;
}

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

void CEventQueue::SaveAll( CEventQueueState& state )
{
	state.m_vecEvents.RemoveAll();
	for ( EventQueuePrioritizedEvent_t* pe = m_Events.m_pNext; pe != nullptr; pe = pe->m_pNext )
	{
		state.m_vecEvents.AddToTail();

		CEventQueueEvent &e = state.m_vecEvents.Tail();
		e.FromPrioritizedEvent( pe, nullptr );
	}
}

void CEventQueue::RestoreAll( const CEventQueueState &state )
{
	// Remove all the current events in preparation for restoring previous state
	Clear();

	FOR_EACH_VEC( state.m_vecEvents, i )
	{
		EventQueuePrioritizedEvent_t *newEvent = new EventQueuePrioritizedEvent_t;
		state.m_vecEvents[i].ToPrioritizedEvent( newEvent, nullptr );
		AddEvent( newEvent );
	}
}

void CEventQueue::SaveForTarget( CBaseEntity *pTarget, CEventQueueState &state )
{
	state.m_vecEvents.RemoveAll();
	for ( EventQueuePrioritizedEvent_t *pe = m_Events.m_pNext; pe != nullptr; pe = pe->m_pNext )
	{
		// Only add the event if it affects the entity of interest
		if (EventAffectsEntity(pe, pTarget))
		{
			state.m_vecEvents.AddToTail();

			CEventQueueEvent &e = state.m_vecEvents.Tail();
			e.FromPrioritizedEvent( pe, pTarget );
		}
	}
}

void CEventQueue::RestoreForTarget( CBaseEntity *pTarget, const CEventQueueState &state )
{
	// Remove all the current events which have correct target ent in preparation for restoring previous state
	// delete all the events in the queue
	CancelEventOn( pTarget, nullptr );
	
	FOR_EACH_VEC( state.m_vecEvents, i )
	{
		EventQueuePrioritizedEvent_t *newEvent = new EventQueuePrioritizedEvent_t;
		state.m_vecEvents[i].ToPrioritizedEvent( newEvent, pTarget );
		AddEvent( newEvent );
	}
}

static CBaseEntity *GetEntityFromName( string_t name )
{
	if ( name == NULL_STRING )
	{
		return nullptr;
	}
	return gEntList.FindEntityByName( nullptr, name );
}

void CEventQueueEvent::FromPrioritizedEvent( const EventQueuePrioritizedEvent_t *pe, CBaseEntity *pAbstractedEntity )
{
	// Convert from absolute tick count to an offset so its correct regardless of current tickcount
	m_iFireDelayTicks = pe->m_iFireTick - gpGlobals->tickcount;
	m_iOutputID = pe->m_iOutputID;
	m_iTarget = pe->m_iTarget;
	m_iTargetInput = pe->m_iTargetInput;
	m_szActivator = pe->m_pActivator ? pe->m_pActivator->GetEntityName() : NULL_STRING;
	m_iCaller = pe->m_pCaller ? pe->m_pCaller->entindex() : 0;
	m_szEntTarget = pe->m_pEntTarget ? pe->m_pEntTarget->GetEntityName() : NULL_STRING;
	m_VariantValue = pe->m_VariantValue;

	if (pAbstractedEntity)
	{
		m_bAbstractTarget = (pe->m_pEntTarget == pAbstractedEntity);
		m_bAbstractActivator = (pe->m_pActivator == pAbstractedEntity);
		m_bAbstractCaller = (pe->m_pCaller == pAbstractedEntity);
	}
	else
	{
		m_bAbstractTarget = false;
		m_bAbstractActivator = false;
		m_bAbstractCaller = false;
	}
}

void CEventQueueEvent::ToPrioritizedEvent(EventQueuePrioritizedEvent_t* pe, CBaseEntity *pAbstractedEntity ) const
{
	// Convert from tick offset to absolute tick count
	pe->m_iFireTick = m_iFireDelayTicks + gpGlobals->tickcount;
	pe->m_iOutputID = m_iOutputID;
	pe->m_iTarget = m_iTarget;
	pe->m_iTargetInput = m_iTargetInput;

	if ( pAbstractedEntity && m_bAbstractActivator )
	{
		pe->m_pActivator = pAbstractedEntity;
	}
	else
	{
		pe->m_pActivator = GetEntityFromName( m_szActivator );
	}

	if ( pAbstractedEntity && m_bAbstractCaller )
	{
		pe->m_pCaller = pAbstractedEntity;
	}
	else
	{
		pe->m_pCaller = UTIL_EntityByIndex( m_iCaller );
	}

	if ( pAbstractedEntity && m_bAbstractTarget )
	{
		pe->m_pEntTarget = pAbstractedEntity;
	}
	else
	{
		pe->m_pEntTarget = GetEntityFromName( m_szEntTarget );
	}

	pe->m_VariantValue = m_VariantValue;
}

void CEventQueueEvent::LoadFromKeyValues( KeyValues *kv )
{
	m_iFireDelayTicks = kv->GetInt("fire_delay_ticks");
	m_iTarget = MAKE_STRING( kv->GetString("target") );
	m_iTargetInput = MAKE_STRING( kv->GetString( "target_input" ) );
	m_szActivator = MAKE_STRING( kv->GetString( "activator" ) );
	m_iCaller = kv->GetInt( "caller" );
	m_iOutputID = kv->GetInt( "output_id" );
	m_szEntTarget = MAKE_STRING( kv->GetString( "ent_target" ) );

	fieldtype_t fieldtype = (fieldtype_t)kv->GetInt( "variant_field_type" );
	m_VariantValue.SetString( MAKE_STRING( kv->GetString( "variant_value" ) ) );
	m_VariantValue.Convert( fieldtype );

	m_bAbstractTarget = kv->GetBool( "abstract_target" );
	m_bAbstractActivator = kv->GetBool( "abstract_activator" );
	m_bAbstractCaller = kv->GetBool( "abstract_caller" );
}

void CEventQueueEvent::SaveToKeyValues( KeyValues *kv ) const
{
	if ( m_VariantValue.FieldType() == FIELD_CLASSPTR )
	{
		DevWarning( "Tried to save invalid variant type\n" );
		return;
	}

	kv->SetInt( "fire_delay_ticks", m_iFireDelayTicks );
	kv->SetString( "target", STRING( m_iTarget ) );
	kv->SetString( "target_input", STRING( m_iTargetInput ) );
	kv->SetString( "activator", STRING( m_szActivator ) );
	kv->SetInt( "caller", m_iCaller );
	kv->SetInt( "output_id", m_iOutputID );
	kv->SetString( "ent_target", STRING( m_szEntTarget ) );
	kv->SetInt( "variant_field_type", m_VariantValue.FieldType() );
	kv->SetString( "variant_value", m_VariantValue.String() );
	kv->SetBool( "abstract_target", m_bAbstractTarget );
	kv->SetBool( "abstract_activator", m_bAbstractActivator );
	kv->SetBool( "abstract_caller", m_bAbstractCaller );
}

void CEventQueueState::LoadFromKeyValues( KeyValues *kv )
{
	m_vecEvents.RemoveAll();

	KeyValues *sub = kv->FindKey("events");
	if (!sub)
		return;

	KeyValues *s = sub->GetFirstSubKey();
	if (!s)
		return;

	do
	{
		m_vecEvents.AddToTail();
		m_vecEvents.Tail().LoadFromKeyValues( s );
	} while ((s = s->GetNextKey()) != nullptr);
}

void CEventQueueState::SaveToKeyValues( KeyValues *kv ) const
{
	if ( m_vecEvents.IsEmpty() )
	{
		return;
	}

	KeyValues *events = new KeyValues( "events" );
	FOR_EACH_VEC( m_vecEvents, i )
	{
		char name[8];
		Q_snprintf( name, sizeof( name ), "%i", i );

		KeyValues* eventkv = new KeyValues( name );
		m_vecEvents[i].SaveToKeyValues( eventkv );
		events->AddSubKey( eventkv );
	}

	kv->AddSubKey( events );
}

////////////////////////// variant_t implementation //////////////////////////

// BUGBUG: Add support for function pointer save/restore to variants
// BUGBUG: Must pass datamap_t to read/write fields 
void variant_t::Set( fieldtype_t ftype, void *data )
{
	fieldType = ftype;

	switch ( ftype )
	{
	case FIELD_BOOLEAN:		bVal = *((bool *)data);				break;
	case FIELD_CHARACTER:	iVal = *((char *)data);				break;
	case FIELD_SHORT:		iVal = *((short *)data);			break;
	case FIELD_INTEGER:		iVal = *((int *)data);				break;
	case FIELD_STRING:		iszVal = *((string_t *)data);		break;
	case FIELD_FLOAT:		flVal = *((float *)data);			break;
	case FIELD_COLOR32:		rgbaVal = *((color32 *)data);		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		vecVal[0] = ((float *)data)[0];
		vecVal[1] = ((float *)data)[1];
		vecVal[2] = ((float *)data)[2];
		break;
	}

	case FIELD_EHANDLE:		eVal = *((EHANDLE *)data);			break;
	case FIELD_CLASSPTR:	eVal = *((CBaseEntity **)data);		break;
	case FIELD_VOID:		
	default:
		iVal = 0; fieldType = FIELD_VOID;	
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Copies the value in the variant into a block of memory
// Input  : *data - the block to write into
//-----------------------------------------------------------------------------
void variant_t::SetOther( void *data )
{
	switch ( fieldType )
	{
	case FIELD_BOOLEAN:		*((bool *)data) = bVal != 0;		break;
	case FIELD_CHARACTER:	*((char *)data) = iVal;				break;
	case FIELD_SHORT:		*((short *)data) = iVal;			break;
	case FIELD_INTEGER:		*((int *)data) = iVal;				break;
	case FIELD_STRING:		*((string_t *)data) = iszVal;		break;
	case FIELD_FLOAT:		*((float *)data) = flVal;			break;
	case FIELD_COLOR32:		*((color32 *)data) = rgbaVal;		break;

	case FIELD_VECTOR:
	case FIELD_POSITION_VECTOR:
	{
		((float *)data)[0] = vecVal[0];
		((float *)data)[1] = vecVal[1];
		((float *)data)[2] = vecVal[2];
		break;
	}

	case FIELD_EHANDLE:		*((EHANDLE *)data) = eVal;			break;
	case FIELD_CLASSPTR:	*((CBaseEntity **)data) = eVal;		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Converts the variant to a new type. This function defines which I/O
//			types can be automatically converted between. Connections that require
//			an unsupported conversion will cause an error message at runtime.
// Input  : newType - the type to convert to
// Output : Returns true on success, false if the conversion is not legal
//-----------------------------------------------------------------------------
bool variant_t::Convert( fieldtype_t newType )
{
	if ( newType == fieldType )
	{
		return true;
	}

	//
	// Converting to a null value is easy.
	//
	if ( newType == FIELD_VOID )
	{
		Set( FIELD_VOID, NULL );
		return true;
	}

	//
	// FIELD_INPUT accepts the variant type directly.
	//
	if ( newType == FIELD_INPUT )
	{
		return true;
	}

	switch ( fieldType )
	{
		case FIELD_INTEGER:
		{
			switch ( newType )
			{
				case FIELD_FLOAT:
				{
					SetFloat( (float) iVal );
					return true;
				}

				case FIELD_BOOLEAN:
				{
					SetBool( iVal != 0 );
					return true;
				}
			}
			break;
		}

		case FIELD_FLOAT:
		{
			switch ( newType )
			{
				case FIELD_INTEGER:
				{
					SetInt( (int) flVal );
					return true;
				}

				case FIELD_BOOLEAN:
				{
					SetBool( flVal != 0 );
					return true;
				}
			}
			break;
		}

		//
		// Everyone must convert from FIELD_STRING if possible, since
		// parameter overrides are always passed as strings.
		//
		case FIELD_STRING:
		{
			switch ( newType )
			{
				case FIELD_INTEGER:
				{
					if (iszVal != NULL_STRING)
					{
						SetInt(atoi(STRING(iszVal)));
					}
					else
					{
						SetInt(0);
					}
					return true;
				}

				case FIELD_FLOAT:
				{
					if (iszVal != NULL_STRING)
					{
						SetFloat(atof(STRING(iszVal)));
					}
					else
					{
						SetFloat(0);
					}
					return true;
				}

				case FIELD_BOOLEAN:
				{
					if (iszVal != NULL_STRING)
					{
						SetBool( atoi(STRING(iszVal)) != 0 );
					}
					else
					{
						SetBool(false);
					}
					return true;
				}

				case FIELD_VECTOR:
				{
					Vector tmpVec = vec3_origin;
					if (sscanf(STRING(iszVal), "[%f %f %f]", &tmpVec[0], &tmpVec[1], &tmpVec[2]) == 0)
					{
						// Try sucking out 3 floats with no []s
						sscanf(STRING(iszVal), "%f %f %f", &tmpVec[0], &tmpVec[1], &tmpVec[2]);
					}
					SetVector3D( tmpVec );
					return true;
				}

				case FIELD_COLOR32:
				{
					int nRed = 0;
					int nGreen = 0;
					int nBlue = 0;
					int nAlpha = 255;

					sscanf(STRING(iszVal), "%d %d %d %d", &nRed, &nGreen, &nBlue, &nAlpha);
					SetColor32( nRed, nGreen, nBlue, nAlpha );
					return true;
				}

				case FIELD_EHANDLE:
				{
					// convert the string to an entity by locating it by classname
					CBaseEntity *ent = NULL;
					if ( iszVal != NULL_STRING )
					{
						// FIXME: do we need to pass an activator in here?
						ent = gEntList.FindEntityByName( NULL, iszVal );
					}
					SetEntity( ent );
					return true;
				}
			}
		
			break;
		}

		case FIELD_EHANDLE:
		{
			switch ( newType )
			{
				case FIELD_STRING:
				{
					// take the entities targetname as the string
					string_t iszStr = NULL_STRING;
					if ( eVal != NULL )
					{
						SetString( eVal->GetEntityName() );
					}
					return true;
				}
			}
			break;
		}
	}

	// invalid conversion
	return false;
}

//-----------------------------------------------------------------------------
// Only for when something like !activator needs to become a FIELD_EHANDLE, or when that's a possibility.
//-----------------------------------------------------------------------------
bool variant_t::Convert(fieldtype_t newType, CBaseEntity* pSelf, CBaseEntity* pActivator, CBaseEntity* pCaller)
{
	// Support for turning !activator, !caller, and !self into a FIELD_EHANDLE.
	// Extremely necessary.
	if (newType == FIELD_EHANDLE)
	{
		if (newType == fieldType)
			return true;

		CBaseEntity* ent = NULL;
		if (iszVal != NULL_STRING)
		{
			ent = gEntList.FindEntityGeneric(NULL, STRING(iszVal), pSelf, pActivator, pCaller);
		}
		SetEntity(ent);
		return true;
	}

	return Convert(newType);
}

//-----------------------------------------------------------------------------
// Purpose: All types must be able to display as strings for debugging purposes.
// Output : Returns a pointer to the string that represents this value.
//
//			NOTE: The returned pointer should not be stored by the caller as
//				  subsequent calls to this function will overwrite the contents
//				  of the buffer!
//-----------------------------------------------------------------------------
const char *variant_t::ToString( void ) const
{
	COMPILE_TIME_ASSERT( sizeof(string_t) == sizeof(int) );

	static char szBuf[512];

	switch (fieldType)
	{
	case FIELD_STRING:
		{
			return(STRING(iszVal));
		}

	case FIELD_BOOLEAN:
		{
			if (bVal == 0)
			{
				Q_strncpy(szBuf, "false",sizeof(szBuf));
			}
			else
			{
				Q_strncpy(szBuf, "true",sizeof(szBuf));
			}
			return(szBuf);
		}

	case FIELD_INTEGER:
		{
			Q_snprintf( szBuf, sizeof( szBuf ), "%i", iVal );
			return(szBuf);
		}

	case FIELD_FLOAT:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "%g", flVal);
			return(szBuf);
		}

	case FIELD_COLOR32:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "%d %d %d %d", (int)rgbaVal.r, (int)rgbaVal.g, (int)rgbaVal.b, (int)rgbaVal.a);
			return(szBuf);
		}

	case FIELD_VECTOR:
		{
			Q_snprintf(szBuf,sizeof(szBuf), "[%g %g %g]", (double)vecVal[0], (double)vecVal[1], (double)vecVal[2]);
			return(szBuf);
		}

	case FIELD_VOID:
		{
			szBuf[0] = '\0';
			return(szBuf);
		}

	case FIELD_EHANDLE:
		{
			const char *pszName = (Entity()) ? STRING(Entity()->GetEntityName()) : "<<null entity>>";
			Q_strncpy( szBuf, pszName, 512 );
			return (szBuf);
		}
	}

	return("No conversion to string");
}

#define classNameTypedef variant_t // to satisfy DEFINE... macros

typedescription_t variant_t::m_SaveBool[] =
{
	DEFINE_FIELD( bVal, FIELD_BOOLEAN ),
};
typedescription_t variant_t::m_SaveInt[] =
{
	DEFINE_FIELD( iVal, FIELD_INTEGER ),
};
typedescription_t variant_t::m_SaveFloat[] =
{
	DEFINE_FIELD( flVal, FIELD_FLOAT ),
};
typedescription_t variant_t::m_SaveEHandle[] =
{
	DEFINE_FIELD( eVal, FIELD_EHANDLE ),
};
typedescription_t variant_t::m_SaveString[] =
{
	DEFINE_FIELD( iszVal, FIELD_STRING ),
};
typedescription_t variant_t::m_SaveColor[] =
{
	DEFINE_FIELD( rgbaVal, FIELD_COLOR32 ),
};

#undef classNameTypedef

//
// Struct for saving and restoring vector variants, since they are
// stored as float[3] and we want to take advantage of position vector
// fixup across level transitions.
//
#define classNameTypedef variant_savevector_t // to satisfy DEFINE... macros

struct variant_savevector_t
{
	Vector vecSave;
};
typedescription_t variant_t::m_SaveVector[] =
{
	// Just here to shut up ClassCheck
//	DEFINE_ARRAY( vecVal, FIELD_FLOAT, 3 ),

	DEFINE_FIELD( vecSave, FIELD_VECTOR ),
};
typedescription_t variant_t::m_SavePositionVector[] =
{
	DEFINE_FIELD( vecSave, FIELD_POSITION_VECTOR ),
};
#undef classNameTypedef

#define classNameTypedef variant_savevmatrix_t // to satisfy DEFINE... macros
struct variant_savevmatrix_t
{
	VMatrix matSave;
};
typedescription_t variant_t::m_SaveVMatrix[] =
{
	DEFINE_FIELD( matSave, FIELD_VMATRIX ),
};
typedescription_t variant_t::m_SaveVMatrixWorldspace[] =
{
	DEFINE_FIELD( matSave, FIELD_VMATRIX_WORLDSPACE ),
};
#undef classNameTypedef

#define classNameTypedef variant_savevmatrix3x4_t // to satisfy DEFINE... macros
struct variant_savevmatrix3x4_t
{
	matrix3x4_t matSave;
};
typedescription_t variant_t::m_SaveMatrix3x4Worldspace[] =
{
	DEFINE_FIELD( matSave, FIELD_MATRIX3X4_WORLDSPACE ),
};
#undef classNameTypedef

class CVariantSaveDataOps : public CDefSaveRestoreOps
{
	// saves the entire array of variables
	virtual void Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
	{
		variant_t *var = (variant_t*)fieldInfo.pField;

		int type = var->FieldType();
		pSave->WriteInt( &type, 1 );

		switch ( var->FieldType() )
		{
		case FIELD_VOID:
			break;
		case FIELD_BOOLEAN:	
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveBool, 1 );
			break;
		case FIELD_INTEGER:	
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveInt, 1 );
			break;
		case FIELD_FLOAT:
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveFloat, 1 );
			break;
		case FIELD_EHANDLE:
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveEHandle, 1 );
			break;
		case FIELD_STRING:
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveString, 1 );
			break;
		case FIELD_COLOR32:
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveColor, 1 );
			break;
		case FIELD_VECTOR:
		{
			variant_savevector_t Temp;
			var->Vector3D(Temp.vecSave);
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SaveVector, 1 );
			break;
		}

		case FIELD_POSITION_VECTOR:
		{
			variant_savevector_t Temp;
			var->Vector3D(Temp.vecSave);
			pSave->WriteFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SavePositionVector, 1 );
			break;
		}

		default:
			Warning( "Bad type %d in saved variant_t\n", var->FieldType() );
			Assert(0);
		}
	}

	// restores a single instance of the variable
	virtual void Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
	{
		variant_t *var = (variant_t*)fieldInfo.pField;

		*var = variant_t();

		var->fieldType = (_fieldtypes)pRestore->ReadInt();

		switch ( var->fieldType )
		{
		case FIELD_VOID:
			break;
		case FIELD_BOOLEAN:	
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveBool, 1 );
			break;
		case FIELD_INTEGER:	
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveInt, 1 );
			break;
		case FIELD_FLOAT:
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveFloat, 1 );
			break;
		case FIELD_EHANDLE:
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveEHandle, 1 );
			break;
		case FIELD_STRING:
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveString, 1 );
			break;
		case FIELD_COLOR32:
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, var, NULL, variant_t::m_SaveColor, 1 );
			break;
		case FIELD_VECTOR:
		{
			variant_savevector_t Temp;
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SaveVector, 1 );
			var->SetVector3D(Temp.vecSave);
			break;
		}
		case FIELD_POSITION_VECTOR:
		{
			variant_savevector_t Temp;
			pRestore->ReadFields( fieldInfo.pTypeDesc->fieldName, &Temp, NULL, variant_t::m_SavePositionVector, 1 );
			var->SetPositionVector3D(Temp.vecSave);
			break;
		}
		default:
			Warning( "Bad type %d in saved variant_t\n", var->FieldType() );
			Assert(0);
			break;
		}
	}


	virtual bool IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		// check all the elements of the array (usually only 1)
		variant_t *var = (variant_t*)fieldInfo.pField;
		for ( int i = 0; i < fieldInfo.pTypeDesc->fieldSize; i++, var++ )
		{
			if ( var->FieldType() != FIELD_VOID )
				return 0;
		}

		// variant has no data
		return 1;
	}

	virtual void MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		// Don't no how to. This is okay, since objects of this type
		// are always born clean before restore, and not reused
	}
};

CVariantSaveDataOps g_VariantSaveDataOps;
ISaveRestoreOps *variantFuncs = &g_VariantSaveDataOps;

/////////////////////// entitylist /////////////////////

CUtlMemoryPool g_EntListMemPool( sizeof(entitem_t), 256, CUtlMemoryPool::GROW_NONE, "g_EntListMemPool" );

#include "tier0/memdbgoff.h"

void *entitem_t::operator new( size_t stAllocateBlock )
{
	return g_EntListMemPool.Alloc( stAllocateBlock );
}

void *entitem_t::operator new( size_t stAllocateBlock, int nBlockUse, const char *pFileName, int nLine )
{
	return g_EntListMemPool.Alloc( stAllocateBlock );
}

void entitem_t::operator delete( void *pMem )
{
	g_EntListMemPool.Free( pMem );
}

#include "tier0/memdbgon.h"


CEntityList::CEntityList()
{
	m_pItemList = NULL;
	m_iNumItems = 0;
}

CEntityList::~CEntityList()
{
	// remove all items from the list
	entitem_t *next, *e = m_pItemList;
	while ( e != NULL )
	{
		next = e->pNext;
		delete e;
		e = next;
	}
	m_pItemList = NULL;
}

void CEntityList::AddEntity( CBaseEntity *pEnt )
{
	// check if it's already in the list; if not, add it
	entitem_t *e = m_pItemList;
	while ( e != NULL )
	{
		if ( e->hEnt == pEnt )
		{
			// it's already in the list
			return;
		}

		if ( e->pNext == NULL )
		{
			// we've hit the end of the list, so tack it on
			e->pNext = new entitem_t;
			e->pNext->hEnt = pEnt;
			e->pNext->pNext = NULL;
			m_iNumItems++;
			return;
		}

		e = e->pNext;
	}
	
	// empty list
	m_pItemList = new entitem_t;
	m_pItemList->hEnt = pEnt;
	m_pItemList->pNext = NULL;
	m_iNumItems = 1;
}

void CEntityList::DeleteEntity( CBaseEntity *pEnt )
{
	// find the entry in the list and delete it
	entitem_t *prev = NULL, *e = m_pItemList;
	while ( e != NULL )
	{
		// delete the link if it's the matching entity OR if the link is NULL
		if ( e->hEnt == pEnt || e->hEnt == NULL )
		{
			if ( prev )
			{
				prev->pNext = e->pNext;
			}
			else
			{
				m_pItemList = e->pNext;
			}

			delete e;
			m_iNumItems--;

			// REVISIT: Is this correct?  Is this just here to clean out dead EHANDLEs?
			// restart the loop
			e = m_pItemList;
			prev = NULL;
			continue;
		}

		prev = e;
		e = e->pNext;
	}
}

