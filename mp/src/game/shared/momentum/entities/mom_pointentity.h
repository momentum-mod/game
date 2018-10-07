#ifndef _MOM_POINTENTITY_H_
#define _MOM_POINTENTITY_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef CLIENT_DLL
#define CPointEntity C_PointEntity
#else
#endif

class CPointEntity : public CBaseEntity
{
public:
	DECLARE_CLASS(CPointEntity, CBaseEntity);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	virtual void Spawn(void);

#ifdef GAME_DLL // Server specific things
public:
	virtual int ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual bool KeyValue(const char *szKeyName, const char *szValue);
	DECLARE_DATADESC();
#endif
};

#endif