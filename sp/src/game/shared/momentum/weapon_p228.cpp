//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_csbase.h"
#include "fx_cs_shared.h"
#include "mom_player_shared.h"

#if defined( CLIENT_DLL )
	#define CWeaponP228 C_WeaponP228
#endif


class CWeaponP228 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponP228, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponP228();

	virtual void Spawn();

	virtual void PrimaryAttack();
	virtual bool Deploy();

	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_P228; }


private:
	
	CWeaponP228( const CWeaponP228 & );
	void P228Fire( float flSpread );

	float m_flLastFire;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponP228, DT_WeaponP228 )

BEGIN_NETWORK_TABLE( CWeaponP228, DT_WeaponP228 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponP228 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_p228, CWeaponP228 );
PRECACHE_WEAPON_REGISTER( weapon_p228 );



CWeaponP228::CWeaponP228()
{
	m_flLastFire = gpGlobals->curtime;
}


void CWeaponP228::Spawn( )
{
	m_flAccuracy = 0.9;
	
	BaseClass::Spawn();
}


bool CWeaponP228::Deploy( )
{
	m_flAccuracy = 0.9;

	return BaseClass::Deploy();
}

void CWeaponP228::PrimaryAttack( void )
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if ( !FBitSet( pPlayer->GetFlags(), FL_ONGROUND ) )
		P228Fire( 1.5f * (1 - m_flAccuracy) );
	else if (pPlayer->GetAbsVelocity().Length2D() > 5)
		P228Fire( 0.255f * (1 - m_flAccuracy) );
	else if ( FBitSet( pPlayer->GetFlags(), FL_DUCKING ) )
		P228Fire( 0.075f * (1 - m_flAccuracy) );
	else
		P228Fire( 0.15f * (1 - m_flAccuracy) );
}

void CWeaponP228::P228Fire( float flSpread )
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	pPlayer->m_iShotsFired++;

	if (pPlayer->m_iShotsFired > 1)
		return;

	// Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
	m_flAccuracy -= (0.3)*(0.325 - (gpGlobals->curtime - m_flLastFire));

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;
	else if (m_flAccuracy < 0.6)
		m_flAccuracy = 0.6;

	m_flLastFire = gpGlobals->curtime;
	
	if (m_iClip1 <= 0)
	{
		if (m_bFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		}

		return;
	}

	m_iClip1--;
	
	 pPlayer->DoMuzzleFlash();
	//SetPlayerShieldAnim();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
	// Aiming
	FX_FireBullets(
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(),
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		flSpread );
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetCSWpnData().m_flCycleTime;

	if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + 2 );

	//ResetPlayerShieldAnim();

	QAngle angle = pPlayer->GetPunchAngle();
	angle.x -= 2;
	pPlayer->SetPunchAngle( angle );
}


bool CWeaponP228::Reload()
{
	if ( !DefaultPistolReload() )
		return false;

	m_flAccuracy = 0.9;
	return true;
}

void CWeaponP228::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	// only idle if the slid isn't back
	if (m_iClip1 != 0)
	{	
		SetWeaponIdleTime( gpGlobals->curtime + 3.0 ) ;
		SendWeaponAnim( ACT_VM_IDLE );
	}
}
