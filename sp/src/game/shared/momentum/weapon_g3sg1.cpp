//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "mom_player_shared.h"

#if defined( CLIENT_DLL )
	#define CWeaponG3SG1 C_WeaponG3SG1
#else
	#include "KeyValues.h"
#endif


class CWeaponG3SG1 : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponG3SG1, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponG3SG1();

	virtual void Spawn();
	virtual void SecondaryAttack();
	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual bool Deploy();

	virtual float GetMaxSpeed();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_G3SG1; }

private:

	CWeaponG3SG1( const CWeaponG3SG1 & );

	void G3SG1Fire( float flSpread );


	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponG3SG1, DT_WeaponG3SG1 )

BEGIN_NETWORK_TABLE( CWeaponG3SG1, DT_WeaponG3SG1 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponG3SG1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_g3sg1, CWeaponG3SG1 );
PRECACHE_WEAPON_REGISTER( weapon_g3sg1 );



CWeaponG3SG1::CWeaponG3SG1()
{
	m_flLastFire = gpGlobals->curtime;
}

void CWeaponG3SG1::Spawn()
{
	BaseClass::Spawn();
	m_flAccuracy = 0.98;
}


void CWeaponG3SG1::SecondaryAttack()
{
	#ifndef CLIENT_DLL
		CMomentumPlayer *pPlayer = GetPlayerOwner();
		if ( !pPlayer )
			return;

		if ( pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		{
			pPlayer->SetFOV( pPlayer, 40, 0.3f );
		}
		else if (pPlayer->GetFOV() == 40)
		{
			pPlayer->SetFOV( pPlayer, 15, 0.05 );
		}
		else if (pPlayer->GetFOV() == 15)
		{
			pPlayer->SetFOV( pPlayer, pPlayer->GetDefaultFOV(), 0.1f );
		}

		//pPlayer->ResetMaxSpeed();
	#endif

#ifndef CLIENT_DLL
	// If this isn't guarded, the sound will be emitted twice, once by the server and once by the client.
	// Let the server play it since if only the client plays it, it's liable to get played twice cause of
	// a prediction error. joy.
	EmitSound( "Default.Zoom" ); // zoom sound

	// let the bots hear the rifle zoom
	IGameEvent * event = gameeventmanager->CreateEvent( "weapon_zoom" );
	if( event )
	{
		event->SetInt( "userid", pPlayer->GetUserID() );
		gameeventmanager->FireEvent( event );
	}
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;   
	m_zoomFullyActiveTime = gpGlobals->curtime + 0.3; // The worst zoom time from above.  
}

void CWeaponG3SG1::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;
	
	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		G3SG1Fire( 0.45 * (1 - m_flAccuracy) );
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		G3SG1Fire( 0.15 );
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		G3SG1Fire( 0.035 * (1 - m_flAccuracy) );
	else
		G3SG1Fire( 0.055 * (1 - m_flAccuracy) );
}

void CWeaponG3SG1::G3SG1Fire( float flSpread )
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// If we are not zoomed in, or we have very recently zoomed and are still transitioning, the bullet diverts more.
	if (pPlayer->GetFOV() == pPlayer->GetDefaultFOV() || (gpGlobals->curtime < m_zoomFullyActiveTime))
		flSpread += 0.025;

	// Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
	m_flAccuracy = 0.55 + (0.3) * (gpGlobals->curtime - m_flLastFire);	

	if (m_flAccuracy > 0.98)
		m_flAccuracy = 0.98;

	m_flLastFire = gpGlobals->curtime;

	if ( !CSBaseGunFire( flSpread, GetCSWpnData().m_flCycleTime, true ) )
		return;

	// Adjust the punch angle.
	QAngle angle = pPlayer->GetPunchAngle();
	angle.x -= SharedRandomFloat("G3SG1PunchAngleX", 0.75, 1.75 ) + ( angle.x / 4 );
	angle.y += SharedRandomFloat("G3SG1PunchAngleY", -0.75, 0.75 );
	pPlayer->SetPunchAngle( angle );
}


bool CWeaponG3SG1::Reload()
{
	bool ret = BaseClass::Reload();
	
	m_flAccuracy = 0.98;
	
	return ret;
}

bool CWeaponG3SG1::Deploy()
{
	bool ret = BaseClass::Deploy();
	
	m_flAccuracy = 0.98;
	
	return ret;
}

float CWeaponG3SG1::GetMaxSpeed()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer && pPlayer->GetFOV() == pPlayer->GetDefaultFOV() )
		return BaseClass::GetMaxSpeed();
	else
		return 150; // zoomed in
}
