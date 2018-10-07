#ifndef _MOM_DOORS_H_
#define _MOM_DOORS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basedoor.h"

#ifdef CLIENT_DLL
#define CRotDoor C_RotDoor
#else
#endif

/*func_door_rotating

TOGGLE causes the door to wait in both the start and end states for  
a trigger event.

START_OPEN causes the door to move to its destination when spawned,  
and operate in reverse.  It is used to temporarily or permanently  
close off an area when triggered (not usefull for touch or  
takedamage doors).

You need to have an origin brush as part of this entity.  The  
center of that brush will be
the point around which it is rotated. It will rotate around the Z  
axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"distance" is how many degrees the door will be rotated.
"speed" determines how fast the door moves; default value is 100.

REVERSE will cause the door to rotate in the opposite direction.

"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote  
button or trigger field activates the door.
"health"	if set, door must be shot open
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"dmg"		damage to inflict when blocked (2 default)
*/

//==================================================
// CRotDoor 
//==================================================
class CRotDoor : public CBaseDoor
{
public:
	DECLARE_CLASS(CRotDoor, CBaseDoor);
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL // Client specific things
	DECLARE_PREDICTABLE();
#endif

	// Shared things
	bool CreateVPhysics();
	// This is ONLY used by the node graph to test movement through a door
	virtual void SetToggleState(int state);
	virtual bool IsRotatingDoor() { return true; }

	bool m_bSolidBsp;

#ifdef GAME_DLL // Server specific things
public:
	virtual void Spawn(void);
	DECLARE_DATADESC();
#endif
};

#endif