//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "mom_player_shared.h"

#if defined( CLIENT_DLL )
	#define CWeaponAug C_WeaponAug
#endif


class CWeaponAug : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponAug, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponAug();

	virtual void SecondaryAttack();
	virtual void PrimaryAttack();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_AUG; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	void AUGFire( float flSpread, bool bZoomed );
	
	CWeaponAug( const CWeaponAug & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAug, DT_WeaponAug )

BEGIN_NETWORK_TABLE( CWeaponAug, DT_WeaponAug )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAug )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_aug, CWeaponAug );
PRECACHE_WEAPON_REGISTER( weapon_aug );



CWeaponAug::CWeaponAug()
{
}

void CWeaponAug::SecondaryAttack()
{
	#ifndef CLIENT_DLL
		CMomentumPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return;

		if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		{
			pPlayer->SetFOV( pPlayer, 55, 0.2f );
		}
		else if ( pPlayer->GetFOV() == 55 )
		{
			pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.15f );
		}
		else 
		{
			pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
		}
	#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}


void CWeaponAug::PrimaryAttack()
{
	CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	bool bZoomed = pPlayer->GetFOV() < pPlayer->GetDefaultFOV();

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		AUGFire( 0.035f + 0.4f * m_flAccuracy, bZoomed );
	
	else if ( pPlayer->GetAbsVelocity().Length2D() > 140 )
		AUGFire( 0.035f + 0.07f * m_flAccuracy, bZoomed );
	else
		AUGFire( 0.02f * m_flAccuracy, bZoomed );
}


void CWeaponAug::AUGFire( float flSpread, bool bZoomed )
{
	float flCycleTime = GetCSWpnData().m_flCycleTime;

	if ( bZoomed )
		flCycleTime = 0.135f;

	if ( !CSBaseGunFire( flSpread, flCycleTime, true ) )
		return;

	CMomentumPlayer *pPlayer = GetPlayerOwner();

	// CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
	if ( !pPlayer )
		return;

	if ( pPlayer->GetAbsVelocity().Length2D() > 5 )
		 pPlayer->KickBack ( 1, 0.45, 0.275, 0.05, 4, 2.5, 7 );
	
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack ( 1.25, 0.45, 0.22, 0.18, 5.5, 4, 5 );
	
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack ( 0.575, 0.325, 0.2, 0.011, 3.25, 2, 8 );
	
	else
		pPlayer->KickBack ( 0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8 );
}


