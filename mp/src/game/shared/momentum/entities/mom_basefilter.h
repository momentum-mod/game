#ifndef _MOM_BASEFILTER_H_
#define _MOM_BASEFILTER_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_entityoutput.h"

#include "takedamageinfo.h"

#ifdef CLIENT_DLL
#define CBaseFilter C_BaseFilter
#endif

#define MAX_FILTERS 5

#define SF_FILTER_ENEMY_NO_LOSE_AQUIRED	(1<<0)

enum filter_t
{
	FILTER_AND,
	FILTER_OR,
};

class CBaseFilter : public CBaseEntity
{
public:
	DECLARE_CLASS(CBaseFilter, CBaseEntity);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things

	bool PassesFilter( CBaseEntity *pCaller, CBaseEntity *pEntity );
	bool PassesDamageFilter( const CTakeDamageInfo &info );

	// Inputs
	void InputTestActivator( inputdata_t &inputdata );

	// Outputs
	COutputEvent	m_OnPass;		// Fired when filter is passed
	COutputEvent	m_OnFail;		// Fired when filter is failed

	// Vars
	CNetworkVar(bool, m_bNegated);

protected:
	virtual bool PassesFilterImpl( CBaseEntity *pCaller, CBaseEntity *pEntity );
	virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);

#ifdef GAME_DLL // Server specific things
	virtual int UpdateTransmitState();

	DECLARE_DATADESC();
#endif
};

#endif