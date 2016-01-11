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
	#define CWeaponM3 C_WeaponM3
#else
	#include "momentum/te_shotgun_shot.h"
#endif


class CWeaponM3 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponM3, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponM3();

	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_M3; }


private:

	CWeaponM3( const CWeaponM3 & );

	float m_flPumpTime;
	CNetworkVar( int, m_fInSpecialReload );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM3, DT_WeaponM3 )

BEGIN_NETWORK_TABLE( CWeaponM3, DT_WeaponM3 )

	#ifdef CLIENT_DLL
		RecvPropInt( RECVINFO( m_fInSpecialReload ) )
	#else
		SendPropInt( SENDINFO( m_fInSpecialReload ), 2, SPROP_UNSIGNED )
	#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponM3 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_m3, CWeaponM3 );
PRECACHE_WEAPON_REGISTER( weapon_m3 );



CWeaponM3::CWeaponM3()
{
	m_flPumpTime = 0;
}

void CWeaponM3::PrimaryAttack()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}

	// Out of ammo?
	if ( m_iClip1 <= 0 )
	{
		Reload();
		if ( m_iClip1 == 0 )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		}

		return;
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	m_iClip1--;
	pPlayer->DoMuzzleFlash();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Dispatch the FX right away with full accuracy.
	FX_FireBullets( 
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(), 
		pPlayer->EyeAngles() + 2.0f * pPlayer->GetPunchAngle(), 
		GetWeaponID(),
		Primary_Mode,
		CBaseEntity::GetPredictionRandomSeed() & 255, // wrap it for network traffic so it's the same between client and server
		0.0675 );

	if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	if (m_iClip1 != 0)
		m_flPumpTime = gpGlobals->curtime + 0.5;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.875;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.875;
	if (m_iClip1 != 0)
		SetWeaponIdleTime( gpGlobals->curtime + 2.5 );
	else
		SetWeaponIdleTime( gpGlobals->curtime + 0.875 );
	m_fInSpecialReload = 0;

	// Update punch angles.
	QAngle angle = pPlayer->GetPunchAngle();

	if ( pPlayer->GetFlags() & FL_ONGROUND )
	{
		angle.x -= SharedRandomInt( "M3PunchAngleGround", 4, 6 );
	}
	else
	{
		angle.x -= SharedRandomInt( "M3PunchAngleAir", 8, 11 );
	}

	pPlayer->SetPunchAngle( angle );
}


bool CWeaponM3::Reload()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if (pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 || m_iClip1 == GetMaxClip1())
		return true;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return true;
		
	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0)
	{
		pPlayer->SetAnimation( PLAYER_RELOAD );

		SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );
		m_fInSpecialReload = 1;
		pPlayer->m_flNextAttack = gpGlobals->curtime + 0.5;
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
		SetWeaponIdleTime( gpGlobals->curtime + 0.5 );

#ifdef GAME_DLL
		//pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_START );
#endif

		return true;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > gpGlobals->curtime)
			return true;
		// was waiting for gun to move to side
		m_fInSpecialReload = 2;

		SendWeaponAnim( ACT_VM_RELOAD );
		SetWeaponIdleTime( gpGlobals->curtime + 0.5 );
#ifdef GAME_DLL
		if ( m_iClip1 == 7 )
		{
			//pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_END );
		}
		else
		{
			//pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD_LOOP );
		}
#endif
	}
	else
	{
		// Add them to the clip
		m_iClip1 += 1;
		
#ifdef GAME_DLL
		SendReloadEvents();
#endif
		
        CMomentumPlayer *pPlayer = GetPlayerOwner();

		if ( pPlayer )
			 pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );

		m_fInSpecialReload = 1;
	}

	return true;
}


void CWeaponM3::WeaponIdle()
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	if (m_flPumpTime && m_flPumpTime < gpGlobals->curtime)
	{
		// play pumping sound
		m_flPumpTime = 0;
	}

	if (m_flTimeWeaponIdle < gpGlobals->curtime)
	{
		if (m_iClip1 == 0 && m_fInSpecialReload == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ))
		{
			Reload( );
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip1 != 8 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ))
			{
				Reload( );
			}
			else
			{
				// reload debounce has timed out
				//MIKETODO: shotgun anims
				SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );
				
				// play cocking sound
				m_fInSpecialReload = 0;
				SetWeaponIdleTime( gpGlobals->curtime + 1.5 );
			}
		}
		else
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}
