#ifndef _MOM_FILTERS_H_
#define _MOM_FILTERS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basefilter.h"

#ifdef CLIENT_DLL
#define CFilterMultiple C_FilterMultiple
#define CFilterName C_FilterName
#define CFilterTeam C_FilterTeam
#define CFilterClass C_FilterClass
#define CFilterMassGreater C_FilterMassGreater
#define CFilterDamageType C_FilterDamageType
#define CFilterEnemy C_FilterEnemy
#else
#include "ai_basenpc.h"
#include "ai_squad.h"
#endif

// ###################################################################
//	> FilterMultiple
//
//   Allows one to filter through mutiple filters
// ###################################################################
class CFilterMultiple : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterMultiple, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Spawn(void);
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);
	virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);
	virtual void Activate(void);

	filter_t	m_nFilterType;
	string_t	m_iFilterName[MAX_FILTERS];
	EHANDLE		m_hFilter[MAX_FILTERS];

	CNetworkArray(unsigned int, m_iFilterNameArrayCRC, MAX_FILTERS);
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// ###################################################################
//	> FilterName
// ###################################################################
class CFilterName : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterName, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);

	string_t m_iFilterName;
	CNetworkVar(unsigned int, m_iFilterNameCRC);
#ifdef GAME_DLL // Server specific things
public:
	virtual void Spawn(void);
	DECLARE_DATADESC();
#endif
};

// ###################################################################
//	> FilterTeam
// ###################################################################
class CFilterTeam : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterTeam, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);

	int		m_iFilterTeam;
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// ###################################################################
//	> FilterClass
// ###################################################################
class CFilterClass : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterClass, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Spawn(void);
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);

	string_t m_iFilterClass;
	CNetworkVar(unsigned int, m_iFilterClassCRC);
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// ###################################################################
//	> FilterMassGreater
// ###################################################################
class CFilterMassGreater : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterMassGreater, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);

	CNetworkVar(float, m_fFilterMass);
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

#if defined(CLIENT_DLL) && defined(GAME_DLL)
// ###################################################################
//	> FilterDamageType
// ###################################################################
class CFilterDamageType : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterDamageType, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);
	virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);

	CNetworkVar(int, m_iDamageType);
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};

// ###################################################################
//	> FilterEnemy
// ###################################################################
class CFilterEnemy : public CBaseFilter
{
public:
	DECLARE_CLASS(CFilterEnemy, CBaseFilter);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual bool PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity);
	virtual bool PassesDamageFilterImpl(const CTakeDamageInfo &info);

private:
	bool	PassesNameFilter(CBaseEntity *pCaller);
	bool	PassesProximityFilter(CBaseEntity *pCaller, CBaseEntity *pEnemy);
	bool	PassesMobbedFilter(CBaseEntity *pCaller, CBaseEntity *pEnemy);

	string_t	m_iszEnemyName;				// Name or classname
	float		m_flRadius;					// Radius (enemies are acquired at this range)
	float		m_flOuterRadius;			// Outer radius (enemies are LOST at this range)
	int		m_nMaxSquadmatesPerEnemy;	// Maximum number of squadmates who may share the same enemy
	string_t	m_iszPlayerName;			// "!player"
#ifdef GAME_DLL // Server specific things
public:
	DECLARE_DATADESC();
#endif
};
#endif

#endif