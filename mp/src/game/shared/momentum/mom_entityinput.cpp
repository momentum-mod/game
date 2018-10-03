#include "cbase.h"
#include "mom_entityinput.h"

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