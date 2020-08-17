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

#include "baseentity.h"
#include "entityoutput.h"

// ###################################################################
//	> BaseFilter
// ###################################################################
class CBaseFilter : public CLogicalEntity
{
	DECLARE_CLASS( CBaseFilter, CLogicalEntity );

public:

	DECLARE_DATADESC();

	bool PassesFilter( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilter( const CTakeDamageInfo &info );

	bool m_bNegated;

	// Inputs
	void InputTestActivator( inputdata_t &inputdata );
	void InputTestEntity( inputdata_t &inputdata );
	virtual void InputSetField( inputdata_t &inputdata );

	bool m_bPassCallerWhenTested;

	// Outputs
	COutputEvent	m_OnPass;		// Fired when filter is passed
	COutputEvent	m_OnFail;		// Fired when filter is failed

protected:

	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);
};

//=========================================================
// Trace filter that uses a filter entity.
// If the regular trace filter stuff tells this trace to hit an entity, it will go through a filter entity.
// If the entity passes the filter, the trace will go through.
// This can be negated with m_bHitIfPassed, meaning entities that pass will be hit.
// Use m_bFilterExclusive to make the filter the sole factor in hitting an entity.
//=========================================================
class CTraceFilterEntityFilter : public CTraceFilterSimple
{
public:
	CTraceFilterEntityFilter( const IHandleEntity *passentity, int collisionGroup ) : CTraceFilterSimple( passentity, collisionGroup ) {}
	CTraceFilterEntityFilter( int collisionGroup ) : CTraceFilterSimple( NULL, collisionGroup ) {}

	bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		bool base = CTraceFilterSimple::ShouldHitEntity( pHandleEntity, contentsMask );
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if (m_bFilterExclusive && m_pFilter)
			return m_pFilter->PassesFilter(m_pCaller, pEntity) ? m_bHitIfPassed : !m_bHitIfPassed;
		else if (m_pFilter && (base ? !m_bHitIfPassed : m_bHitIfPassed))
		{
			return m_bHitIfPassed ? m_pFilter->PassesFilter(m_pCaller, pEntity) : !m_pFilter->PassesFilter(m_pCaller, pEntity);
		}

		return base;
	}

	CBaseFilter *m_pFilter;
	CBaseEntity *m_pCaller;

	bool m_bHitIfPassed;
	bool m_bFilterExclusive;

};

#endif // FILTERS_H
