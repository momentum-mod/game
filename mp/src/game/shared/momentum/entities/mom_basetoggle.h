#ifndef _MOM_BASETOGGLE_H_
#define _MOM_BASETOGGLE_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef CLIENT_DLL
#define CBaseToggle C_BaseToggle
#endif

class CBaseToggle : public CBaseEntity
{
public:
	DECLARE_CLASS(CBaseToggle, CBaseEntity);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things

#ifdef GAME_DLL // Server specific things
	DECLARE_DATADESC();
#endif
};

#endif