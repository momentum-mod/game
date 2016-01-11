//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "mom_player_shared.h"

#if defined( CLIENT_DLL )
	#define CWeaponSG552 C_WeaponSG552
#endif


class CWeaponSG552 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponSG552, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSG552();

	virtual void SecondaryAttack();
	virtual void PrimaryAttack();

	virtual float GetMaxSpeed() const;

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_SG552; }

#ifdef CLIENT_DLL
	virtual bool	HideViewModelWhenZoomed( void ) { return false; }
#endif

private:

	CWeaponSG552( const CWeaponSG552 & );

	void SG552Fire( float flSpread, bool bZoomed );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSG552, DT_WeaponSG552 )

BEGIN_NETWORK_TABLE( CWeaponSG552, DT_WeaponSG552 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSG552 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sg552, CWeaponSG552 );
PRECACHE_WEAPON_REGISTER( weapon_sg552 );



CWeaponSG552::CWeaponSG552()
{
}



void CWeaponSG552::SecondaryAttack()
{
	#ifndef CLIENT_DLL
		CMomentumPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return;

		if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV())
		{
			pPlayer->SetFOV( pPlayer, 55, 0.2f );
		}
		else if (pPlayer->GetFOV() == 55)
		{
			pPlayer->SetFOV( pPlayer, 0, 0.15f );
		}
		else 
		{
			//FIXME: This seems wrong
			pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV() );
		}
	#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}

void CWeaponSG552::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	bool bZoomed = pPlayer->GetFOV() < pPlayer->GetDefaultFOV();

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		SG552Fire( 0.035f + 0.45f * m_flAccuracy, bZoomed );
	else if (pPlayer->GetAbsVelocity().Length2D() > 140)
		SG552Fire( 0.035f + 0.075f * m_flAccuracy, bZoomed );
	else
		SG552Fire( 0.02f * m_flAccuracy, bZoomed );
}


void CWeaponSG552::SG552Fire( float flSpread, bool bZoomed )
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

	if (pPlayer->GetAbsVelocity().Length2D() > 5)
		pPlayer->KickBack (1, 0.45, 0.28, 0.04, 4.25, 2.5, 7);
	else if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		pPlayer->KickBack (1.25, 0.45, 0.22, 0.18, 6, 4, 5);
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		pPlayer->KickBack (0.6, 0.35, 0.2, 0.0125, 3.7, 2, 10);
	else
		pPlayer->KickBack (0.625, 0.375, 0.25, 0.0125, 4, 2.25, 9);
}


float CWeaponSG552::GetMaxSpeed() const
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

	if ( !pPlayer || pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		return BaseClass::GetMaxSpeed();
	else
		return 200; // zoomed in.
}	
