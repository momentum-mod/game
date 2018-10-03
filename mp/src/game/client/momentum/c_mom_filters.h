#ifndef _C_MOM_FILTERS_H_
#define _C_MOM_FILTERS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "checksum_crc.h"

#include "mom_entityoutput.h"

#define MAX_FILTERS 5

class C_BaseFilter : public C_BaseEntity
{
	DECLARE_CLASS(C_BaseFilter, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	
public:
	C_BaseFilter() {};

	bool PassesFilter( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilter( const CTakeDamageInfo &info );

	bool m_bNegated;

	// Inputs
	void InputTestActivator( inputdata_t &inputdata );

	// Outputs
	COutputEvent	m_OnPass;		// Fired when filter is passed
	COutputEvent	m_OnFail;		// Fired when filter is failed

protected:

	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	//virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);
};

#endif