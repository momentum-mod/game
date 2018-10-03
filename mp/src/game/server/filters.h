//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Filters are outboard entities that hold a set of rules that other
//			entities can use to determine behaviors.
//			
//			For example, triggers can use an activator filter to determine who
//			activates them. NPCs and breakables can use a damage filter to
//			determine what can damage them.
//
//			Current filter criteria are:
//
//				Activator name
//				Activator class
//				Activator team
//				Damage type (for damage filters only)
//
//			More than one filter can be combined to create a more complex boolean
//			expression by using filter_multi.
//
//=============================================================================//

#ifndef FILTERS_H
#define FILTERS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_entityoutput.h"

// ###################################################################
//	> BaseFilter
// ###################################################################
class CBaseFilter : public CBaseEntity
{
	DECLARE_CLASS( CBaseFilter, CBaseEntity );
	DECLARE_NETWORKCLASS();

public:

	DECLARE_DATADESC();

	bool PassesFilter( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilter( const CTakeDamageInfo &info );

	virtual int UpdateTransmitState();

	bool m_bNegated;

	// Inputs
	void InputTestActivator( inputdata_t &inputdata );

	// Outputs
	COutputEvent	m_OnPass;		// Fired when filter is passed
	COutputEvent	m_OnFail;		// Fired when filter is failed

protected:

	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);
};

#endif // FILTERS_H
