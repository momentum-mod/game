#ifndef _C_MOM_BASETOGGLE_H_
#define _C_MOM_BASETOGGLE_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"

enum
{
	SF_TRIGGER_ALLOW_CLIENTS				= 0x01,		// Players can fire this trigger
	SF_TRIGGER_ALLOW_NPCS					= 0x02,		// NPCS can fire this trigger
	SF_TRIGGER_ALLOW_PUSHABLES				= 0x04,		// Pushables can fire this trigger
	SF_TRIGGER_ALLOW_PHYSICS				= 0x08,		// Physics objects can fire this trigger
	SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS		= 0x10,		// *if* NPCs can fire this trigger, this flag means only player allies do so
	SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES		= 0x20,		// *if* Players can fire this trigger, this flag means only players inside vehicles can 
	SF_TRIGGER_ALLOW_ALL					= 0x40,		// Everything can fire this trigger EXCEPT DEBRIS!
	SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES	= 0x200,	// *if* Players can fire this trigger, this flag means only players outside vehicles can 
	SF_TRIG_PUSH_ONCE						= 0x80,		// trigger_push removes itself after firing once
	SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER	= 0x100,	// if pushed object is player on a ladder, then this disengages them from the ladder (HL2only)
	SF_TRIG_TOUCH_DEBRIS 					= 0x400,	// Will touch physics debris objects
	SF_TRIGGER_ONLY_NPCS_IN_VEHICLES		= 0X800,	// *if* NPCs can fire this trigger, only NPCs in vehicles do so (respects player ally flag too)
	SF_TRIGGER_DISALLOW_BOTS                = 0x1000,   // Bots are not allowed to fire this trigger
};

class C_BaseToggle : public C_BaseEntity
{
	DECLARE_CLASS(C_BaseToggle, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:
	C_BaseToggle() : m_flWait(0.f){};

	int GetSpawnFlags(void) const;
	void AddSpawnFlags(int nFlags);
	void RemoveSpawnFlags(int nFlags);
	void ClearSpawnFlags(void);
	bool HasSpawnFlags(int nFlags) const;

	float m_flWait;
	CNetworkVar(int, m_iSpawnFlags);
	CNetworkVector(m_vecPosition1);
	CNetworkVector(m_vecPosition2);
	EHANDLE m_hActivator;
};

inline int C_BaseToggle::GetSpawnFlags(void) const { return m_iSpawnFlags; }
inline void C_BaseToggle::AddSpawnFlags(int nFlags) { m_iSpawnFlags |= nFlags; }
inline void C_BaseToggle::RemoveSpawnFlags(int nFlags) { m_iSpawnFlags &= ~nFlags; }
inline void C_BaseToggle::ClearSpawnFlags(void) { m_iSpawnFlags = 0; }
inline bool C_BaseToggle::HasSpawnFlags(int nFlags) const { return (m_iSpawnFlags & nFlags) != 0; }

#endif