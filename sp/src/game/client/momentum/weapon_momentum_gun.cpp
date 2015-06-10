#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "c_basehlcombatweapon.h"

#ifdef CLIENT_DLL
//#include "c_hl2mp_player.h"
#else
#include "hl2_player.h"
#endif

//#include "weapon_hl2mpbasehlmpcombatweapon.h"

//modify this to alter the rate of fire
#define ROF 0.075f //RPS, 60 Sec / 800 Rounds = 0.075f

//The gun will fire up to this number of bullets while you hold the fire button. 
//If you set it to 1 the gun will be semi auto. If you set it to 3 the gun will fire three round bursts
#define BURST 500

#ifdef CLIENT_DLL
#define CWeaponMomentumGun C_WeaponMomentumGun
#endif

//-----------------------------------------------------------------------------
// CWeaponMomentumGun
//-----------------------------------------------------------------------------

class CWeaponMomentumGun : public C_BaseHLCombatWeapon
{
public:

	DECLARE_CLASS(CWeaponMomentumGun, C_BaseHLCombatWeapon);
	CWeaponMomentumGun(void);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	void Precache(void);
	void ItemPostFrame(void);
	void ItemPreFrame(void);
	void ItemBusyFrame(void);
	void PrimaryAttack(void);
	void AddViewKick(void);
	void DryFire(void);
	bool Holster(CBaseCombatWeapon *pSwitchingTo = NULL); // Required so that you un-zoom when switching weapons
	Activity GetPrimaryAttackActivity(void);

	virtual bool Reload(void);
	virtual const Vector& GetBulletSpread(void);

	int GetMinBurst() { return 2; }
	int GetMaxBurst() { return 5; }
	float GetFireRate(void) { return ROF; }

	//modify this part to control the general accuracy of the gun



	void ToggleZoom(void);
	void CheckZoomToggle(void);

	DECLARE_ACTTABLE();

private:
	CNetworkVar(int, m_iBurst);
	CNetworkVar(bool, m_bInZoom);
	CNetworkVar(float, m_flAttackEnds);
	CNetworkVar(int, m_iStance);

private:
	CWeaponMomentumGun(const CWeaponMomentumGun &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponMomentumGun, DT_WeaponMomentumGun)

BEGIN_NETWORK_TABLE(CWeaponMomentumGun, DT_WeaponMomentumGun)
#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_iBurst)),
RecvPropBool(RECVINFO(m_bInZoom)),
RecvPropTime(RECVINFO(m_flAttackEnds)),
RecvPropInt(RECVINFO(m_iStance)),
#else
SendPropInt(SENDINFO(m_iBurst)),
SendPropBool(SENDINFO(m_bInZoom)),
SendPropTime(SENDINFO(m_flAttackEnds)),
SendPropInt(SENDINFO(m_iStance)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponMomentumGun)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_momentum_gun, CWeaponMomentumGun);
PRECACHE_WEAPON_REGISTER(weapon_momentum_gun);

acttable_t CWeaponMomentumGun::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_MP_CROUCH_IDLE, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_MP_CROUCHWALK, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_MP_RELOAD_STAND, ACT_HL2MP_GESTURE_RELOAD_AR2, false },
	{ ACT_MP_RELOAD_CROUCH, ACT_HL2MP_GESTURE_RELOAD_AR2, false },
	{ ACT_MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
};

IMPLEMENT_ACTTABLE(CWeaponMomentumGun);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponMomentumGun::CWeaponMomentumGun(void)
{
	m_iBurst = BURST;
	m_iStance = 10;
	m_fMinRange1 = 1;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 1;
	m_fMaxRange2 = 200;
	m_bFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: Required for caching the entity during loading
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::Precache(void)
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: The gun is empty, plays a clicking noise with a dryfire anim
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::DryFire(void)
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: This happens if you click and hold the primary fire button
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::PrimaryAttack(void)
{
	//do we have any bullets left from the current burst cycle? 
	if (m_iBurst != 0)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
		{
			return;
		}

		WeaponSound(SINGLE);
		pPlayer->DoMuzzleFlash();

		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		pPlayer->SetAnimation(PLAYER_ATTACK1);
		//ToHL2Player(pPlayer)->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

		// Each time the player fires the gun, reset the view punch.  
		if (pOwner)
		{
			pOwner->ViewPunchReset();
		}

		BaseClass::PrimaryAttack();

		// We fired one shot, decrease the number of bullets available for this burst cycle 
		m_iBurst--;
		m_flNextPrimaryAttack = gpGlobals->curtime + ROF;
		m_flAttackEnds = gpGlobals->curtime + SequenceDuration();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::ItemPreFrame(void)
{
	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	if (m_bInReload)
	{
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
	{
		return;
	}

	if (pOwner->m_nButtons & IN_ATTACK)
	{
		if (m_flAttackEnds<gpGlobals->curtime)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
	else
	{
		//The firing cycle ended. Reset the burst counter to the max value
		m_iBurst = BURST;
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime) && (m_iClip1 <= 0))
		{
			DryFire();
		}
	}
	CheckZoomToggle();
	//check the character's current stance for the accuracy calculation
}

//-----------------------------------------------------------------------------
// Purpose: If we have bullets left then play the attack anim otherwise idle
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponMomentumGun::GetPrimaryAttackActivity(void)
{
	if (m_iBurst != 0)
	{
		return ACT_VM_PRIMARYATTACK;
	}
	else
	{
		return ACT_VM_IDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: The gun is being reloaded 
//-----------------------------------------------------------------------------
bool CWeaponMomentumGun::Reload(void)
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
		//ToBaseCombatCharacter(GetOwner())->DoAnimationEvent(PLAYERANIMEVENT_RELOAD);
		//reset the burst counter to the default
		m_iBurst = BURST;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: Put away the gun and disable zoom if needed
//----------------------------------------------------------------------------- 
bool CWeaponMomentumGun::Holster(CBaseCombatWeapon *pSwitchingTo /* = NULL */)
{
	if (m_bInZoom)
	{
		ToggleZoom();
	}
	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: Calculate the viewkick
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
	{
		return;
	}

	int iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
	RandomSeed(iSeed);

	QAngle viewPunch;

	viewPunch.x = random->RandomFloat(0.25f, 0.5f);
	viewPunch.y = random->RandomFloat(-.6f, .6f);
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}


//-----------------------------------------------------------------------------
// Purpose: Toggle the zoom by changing the client's FOV
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
	{
		return;
	}

#ifndef CLIENT_DLL
	if (m_bInZoom)
	{
		// Narrowing the Field Of View here is what gives us the zoomed effect
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;

			// Send a message to hide the scope
			/* CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(0);
			MessageEnd();*/
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 45, 0.1f))
		{
			m_bInZoom = true;

			// Send a message to Show the scope
			/* CSingleUserRecipientFilter filter(pPlayer);
			UserMessageBegin(filter, "ShowScope");
			WRITE_BYTE(1);
			MessageEnd();*/
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the zoom if the Sec attack button was pressed
//-----------------------------------------------------------------------------
void CWeaponMomentumGun::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer && (pPlayer->m_afButtonPressed & IN_ATTACK2))
	{
		ToggleZoom();
	}
}

const Vector& CWeaponMomentumGun::GetBulletSpread(void)
{
	static Vector cone;
	cone = VECTOR_CONE_1DEGREES;
	//TODO override this with CSS weapon pickups ingame


	//This part simlates recoil. Each successive shot will have increased spread.
	if (m_iBurst != BURST)
	{
		for (int i = m_iBurst; i < BURST; i++)
		{
			cone += VECTOR_CONE_1DEGREES;
		}
	}

	//This part is the zoom modifier. If in zoom, lower the bullet spread.
	if (m_bInZoom)
	{
		cone -= VECTOR_CONE_1DEGREES;
	}

	return cone;
}