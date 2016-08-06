//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CS_WEAPON_PARSE_H
#define CS_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"

//--------------------------------------------------------------------------------------------------------
enum CSWeaponType
{
	WEAPONTYPE_KNIFE=0,	
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};


//--------------------------------------------------------------------------------------------------------
enum CSWeaponID
{
	WEAPON_NONE = 0,

	WEAPON_PISTOL,
    WEAPON_RIFLE,
    WEAPON_SHOTGUN,
    WEAPON_SMG,
    WEAPON_SNIPER,
    WEAPON_LMG,
    WEAPON_GRENADE,
	WEAPON_KNIFE,

	WEAPON_MAX,		// number of weapons weapon index
};

//--------------------------------------------------------------------------------------------------------
const char * WeaponClassAsString( CSWeaponType weaponType );


//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromString( const char * weaponType );

//--------------------------------------------------------------------------------------------------------
enum CSWeaponID;


//--------------------------------------------------------------------------------------------------------
const char * WeaponClassAsString( CSWeaponType weaponType );


//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromString( const char * weaponType );


//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromWeaponID( CSWeaponID weaponID );


//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromWeaponID( CSWeaponID weaponID );


//--------------------------------------------------------------------------------------------------------
class CCSWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CCSWeaponInfo, FileWeaponInfo_t );
	
	CCSWeaponInfo();

    void Parse( KeyValues *pKeyValuesData, const char *szWeaponName ) override;

public:
	CSWeaponType m_WeaponType;

	int	  m_iCrosshairMinDistance;
	int	  m_iCrosshairDeltaDistance;

	char m_szAddonModel[MAX_WEAPON_STRING];		// If this is set, it is used as the addon model. Otherwise, szWorldModel is used.
	char m_szDroppedModel[MAX_WEAPON_STRING];	// Alternate dropped model, if different from the szWorldModel the player holds
	char m_szSilencerModel[MAX_WEAPON_STRING];	// Alternate model with silencer attached

	int	  m_iMuzzleFlashStyle;
	float m_flMuzzleScale;
	
	// Parameters for FX_FireBullets:
	int		m_iPenetration;
	int		m_iDamage;
	float	m_flRange;
	float	m_flRangeModifier;
	int		m_iBullets;

};


#endif // CS_WEAPON_PARSE_H
