#ifndef _C_MOM_DOORS_H_
#define _C_MOM_DOORS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "c_mom_basetoggle.h"

// doors
#define SF_DOOR_ROTATE_YAW			0		// yaw by default
#define	SF_DOOR_START_OPEN_OBSOLETE	1
#define SF_DOOR_ROTATE_BACKWARDS	2
#define SF_DOOR_NONSOLID_TO_PLAYER	4
#define SF_DOOR_PASSABLE			8
#define SF_DOOR_ONEWAY				16
#define	SF_DOOR_NO_AUTO_RETURN		32
#define SF_DOOR_ROTATE_ROLL			64
#define SF_DOOR_ROTATE_PITCH		128
#define SF_DOOR_PUSE				256	// door can be opened by player's use button.
#define SF_DOOR_NONPCS				512	// NPC can't open
#define SF_DOOR_PTOUCH				1024 // player touch opens
#define SF_DOOR_LOCKED				2048	// Door is initially locked
#define SF_DOOR_SILENT				4096	// Door plays no audible sound, and does not alert NPCs when opened
#define	SF_DOOR_USE_CLOSES			8192	// Door can be +used to close before its autoreturn delay has expired.
#define SF_DOOR_SILENT_TO_NPCS		16384	// Does not alert NPC's when opened.
#define SF_DOOR_IGNORE_USE			32768	// Completely ignores player +use commands.
#define SF_DOOR_NEW_USE_RULES		65536	// For func_door entities, behave more like prop_door_rotating with respect to +USE (changelist 242482)

class C_BaseDoor : public C_BaseToggle
{
	DECLARE_CLASS(C_BaseDoor, C_BaseToggle);
	DECLARE_CLIENTCLASS();

public:
	C_BaseDoor() : m_bIsBhopBlock(false), m_flWaveHeight(0.f) {};

	bool m_bIsBhopBlock;
	float m_flWaveHeight;
private:
};

#endif