#include "cbase.h"
#include "mom_entityoutput.h"

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
// Input  : ActionData - the map file data block descibing the event action.
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
				gpGlobals->curtime,
				pCaller ? STRING(pCaller->m_iClassname) : "NULL",
				pCaller ? pCaller->GetDebugName() : "NULL",
				STRING(ev->m_iTarget),
				STRING(ev->m_iTargetInput),
				ev->m_flDelay,
				STRING(ev->m_iParameter) );

			DevMsg( 2, "%s", szBuffer );
#ifdef GAME_DLL
			ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
#endif
		}
		else
		{
			char szBuffer[256];
			Q_snprintf( szBuffer,
				sizeof(szBuffer),
				"(%0.2f) output: (%s,%s) -> (%s,%s)(%s)\n",
				gpGlobals->curtime,
				pCaller ? STRING(pCaller->m_iClassname) : "NULL",
				pCaller ? pCaller->GetDebugName() : "NULL", STRING(ev->m_iTarget),
				STRING(ev->m_iTargetInput),
				STRING(ev->m_iParameter) );

			DevMsg( 2, "%s", szBuffer );
#ifdef GAME_DLL
			ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
#endif
		}
#ifdef GAME_DLL
		if ( pCaller && pCaller->m_debugOverlays & OVERLAY_MESSAGE_BIT)
		{
			pCaller->DrawOutputOverlay(ev);
		}
#endif

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
				Q_snprintf( szBuffer,
				sizeof(szBuffer),
				"Removing from action list: (%s,%s) -> (%s,%s)\n",
				pCaller ? STRING(pCaller->m_iClassname) : "NULL",
				pCaller ? pCaller->GetDebugName() : "NULL",
				STRING(ev->m_iTarget), STRING(ev->m_iTargetInput) );
				DevMsg( 2, "%s", szBuffer );
#ifdef GAME_DLL
				ADD_DEBUG_HISTORY( HISTORY_ENTITY_IO, szBuffer );
#endif
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