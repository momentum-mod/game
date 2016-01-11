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

	#define CWeaponXM1014 C_WeaponXM1014

#else

	#include "momentum/te_shotgun_shot.h"

#endif


class CWeaponXM1014 : public CWeaponCSBase
{
public:
	DECLARE_CLASS( CWeaponXM1014, CWeaponCSBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponXM1014();

	virtual void Spawn();
	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual void WeaponIdle();

	virtual CSWeaponID GetWeaponID( void ) const		{ return WEAPON_XM1014; }


private:

	CWeaponXM1014( const CWeaponXM1014 & );

	float m_flPumpTime;
	CNetworkVar( int, m_fInSpecialReload );

};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponXM1014, DT_WeaponXM1014 )

BEGIN_NETWORK_TABLE( CWeaponXM1014, DT_WeaponXM1014 )
	#ifdef CLIENT_DLL
		RecvPropInt( RECVINFO( m_fInSpecialReload ) )
	#else
		SendPropInt( SENDINFO( m_fInSpecialReload ), 2, SPROP_UNSIGNED )
	#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponXM1014 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_xm1014, CWeaponXM1014 );
PRECACHE_WEAPON_REGISTER( weapon_xm1014 );



CWeaponXM1014::CWeaponXM1014()
{
	m_flPumpTime = 0;
}

void CWeaponXM1014::Spawn()
{
	//m_iDefaultAmmo = M3_DEFAULT_GIVE;
	//FallInit();// get ready to fall
	BaseClass::Spawn();
}


void CWeaponXM1014::PrimaryAttack()
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

	if (m_iClip1 <= 0)
	{
		Reload();
		
		if (m_iClip1 == 0)
		{
			PlayEmptySound( );
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
		}

		return;
	}

	 SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	 
	//pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	//pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

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
		0.0725 // flSpread
		);

	if (!m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	if (m_iClip1 != 0)
		m_flPumpTime = gpGlobals->curtime + 0.5;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.25;
	if (m_iClip1 != 0)
		SetWeaponIdleTime( gpGlobals->curtime + 2.5 );
	else
		SetWeaponIdleTime( gpGlobals->curtime + 0.25 );
	m_fInSpecialReload = 0;

	// Update punch angles.
	QAngle angle = pPlayer->GetPunchAngle();

	if ( pPlayer->GetFlags() & FL_ONGROUND )
	{
		angle.x -= SharedRandomInt( "XM1014PunchAngleGround", 3, 5 );
	}
	else
	{
		angle.x -= SharedRandomInt( "XM1014PunchAngleAir", 7, 10 );
	}

	pPlayer->SetPunchAngle( angle );
}


bool CWeaponXM1014::Reload()
{
	CMomentumPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	if (pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 || m_iClip1 == GetMaxClip1())
		return true;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return true;
	
	//MIKETODO: shotgun reloading (wait until we get content)
	
	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0)
	{
		pPlayer->SetAnimation( PLAYER_RELOAD );

		SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );
		m_fInSpecialReload = 1;
		pPlayer->m_flNextAttack = gpGlobals->curtime + 0.5;
		SetWeaponIdleTime( gpGlobals->curtime + 0.5 );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;

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
		if ( m_iClip1 == 6 )
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


void CWeaponXM1014::WeaponIdle()
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
			if (m_iClip1 != 7 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ))
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
