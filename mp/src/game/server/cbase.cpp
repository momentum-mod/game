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
#include "mom_eventqueue.h"
#include "mom_entityinput.h"
#include "mom_entityoutput.h"
#include "mempool.h"
#include "tier1/strtools.h"
#include "datacache/imdlcache.h"
#include "env_debughistory.h"

#include "tier0/vprof.h"

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

