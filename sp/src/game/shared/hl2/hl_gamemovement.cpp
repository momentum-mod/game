//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Special handling for hl2 usable ladders
//
//=============================================================================//
#include "cbase.h"
#include "hl_gamemovement.h"
#include "in_buttons.h"
#include "utlrbtree.h"
#include "hl2_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHL2GameMovement::CHL2GameMovement()
{
}

//-----------------------------------------------------------------------------
// Purpose: Debounce the USE button
//-----------------------------------------------------------------------------
void CHL2GameMovement::SwallowUseKey()
{
	mv->m_nOldButtons |= IN_USE;
	player->m_afButtonPressed &= ~IN_USE;

	GetHL2Player()->m_bPlayUseDenySound = false;
}

#if !defined( CLIENT_DLL )
// This is a simple helper class to reserver a player sized hull at a spot, owned by the current player so that nothing
//  can move into this spot and cause us to get stuck when we get there
class CReservePlayerSpot : public CBaseEntity
{
	DECLARE_CLASS(CReservePlayerSpot, CBaseEntity)
public:
	static CReservePlayerSpot *ReserveSpot(CBasePlayer *owner, const Vector& org, const Vector& mins, const Vector& maxs, bool& validspot);

	virtual void Spawn();
};

CReservePlayerSpot *CReservePlayerSpot::ReserveSpot(
	CBasePlayer *owner, const Vector& org, const Vector& mins, const Vector& maxs, bool& validspot)
{
	CReservePlayerSpot *spot = (CReservePlayerSpot *)CreateEntityByName("reserved_spot");
	Assert(spot);

	spot->SetAbsOrigin(org);
	UTIL_SetSize(spot, mins, maxs);
	spot->SetOwnerEntity(owner);
	spot->Spawn();

	// See if spot is valid
	trace_t tr;
	UTIL_TraceHull(
		org,
		org,
		mins,
		maxs,
		MASK_PLAYERSOLID,
		owner,
		COLLISION_GROUP_PLAYER_MOVEMENT,
		&tr);

	validspot = !tr.startsolid;

	if (!validspot)
	{
		Vector org2 = org + Vector(0, 0, 1);

		// See if spot is valid
		trace_t tr;
		UTIL_TraceHull(
			org2,
			org2,
			mins,
			maxs,
			MASK_PLAYERSOLID,
			owner,
			COLLISION_GROUP_PLAYER_MOVEMENT,
			&tr);
		validspot = !tr.startsolid;
	}

	return spot;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CReservePlayerSpot::Spawn()
{
	BaseClass::Spawn();

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_NONE);
	// Make entity invisible
	AddEffects(EF_NODRAW);
}

LINK_ENTITY_TO_CLASS(reserved_spot, CReservePlayerSpot);

#endif

bool CHL2GameMovement::OnLadder(trace_t &trace)
{
	if (trace.plane.normal.z == 1.0f)
		return false;

	return BaseClass::OnLadder(trace);
}

static bool NearbyDismountLessFunc(const NearbyDismount_t& lhs, const NearbyDismount_t& rhs)
{
	return lhs.distSqr < rhs.distSqr;
}

void CHL2GameMovement::WalkMove() {
	BaseClass::WalkMove();
	CheckForLadders(player->GetGroundEntity() != NULL);
}

void CHL2GameMovement::AirMove() {
	BaseClass::AirMove();
	CheckForLadders(false);
}

void CHL2GameMovement::PlayerMove()
{
	CHL2_Player *m_pCSPlayer = GetHL2Player();

	BaseClass::PlayerMove();

	if (m_pCSPlayer->IsAlive())
	{
		// Check if our eye height is too close to the ceiling and lower it.
		// This is needed because we have taller models with the old collision bounds.

		const float eyeClearance = 12.0f; // eye pos must be this far below the ceiling

		Vector offset = player->GetViewOffset();

		Vector vHullMin = GetPlayerMins(player->m_Local.m_bDucked);
		vHullMin.z = 0.0f;
		Vector vHullMax = GetPlayerMaxs(player->m_Local.m_bDucked);

		Vector start = player->GetAbsOrigin();
		start.z += vHullMax.z;
		Vector end = start;
		end.z += eyeClearance - vHullMax.z;
		end.z += player->m_Local.m_bDucked ? VEC_DUCK_VIEW.z : VEC_VIEW.z;

		vHullMax.z = 0.0f;

		Vector fudge(1, 1, 0);
		vHullMin += fudge;
		vHullMax -= fudge;

		trace_t trace;
		Ray_t ray;
		ray.Init(start, end, vHullMin, vHullMax);
		UTIL_TraceRay(ray, PlayerSolidMask(), mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

		if (trace.fraction < 1.0f)
		{
			float est = start.z + trace.fraction * (end.z - start.z) - player->GetAbsOrigin().z - eyeClearance;
			if ((player->GetFlags() & FL_DUCKING) == 0 && !player->m_Local.m_bDucking && !player->m_Local.m_bDucked)
			{
				offset.z = est;
			}
			else
			{
				offset.z = min(est, offset.z);
			}
			player->SetViewOffset(offset);
		}
		else
		{
			if ((player->GetFlags() & FL_DUCKING) == 0 && !player->m_Local.m_bDucking && !player->m_Local.m_bDucked)
			{
				player->SetViewOffset(VEC_VIEW);
			}
			else if (player->m_Local.m_bDucked && !player->m_Local.m_bDucking)
			{
				player->SetViewOffset(VEC_DUCK_VIEW);
			}
		}
	}
}

bool CHL2GameMovement::CheckJumpButton()
{
	CHL2_Player *m_pCSPlayer = GetHL2Player();

	if (m_pCSPlayer->pl.deadflag)
	{
		mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released
		return false;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if (m_pCSPlayer->m_flWaterJumpTime)
	{
		m_pCSPlayer->m_flWaterJumpTime -= gpGlobals->frametime;
		if (m_pCSPlayer->m_flWaterJumpTime < 0)
			m_pCSPlayer->m_flWaterJumpTime = 0;

		return false;
	}

	// If we are in the water most of the way...
	if (m_pCSPlayer->GetWaterLevel() >= 2)
	{
		// swimming, not jumping
		SetGroundEntity(NULL);

		if (m_pCSPlayer->GetWaterType() == CONTENTS_WATER)    // We move up a certain amount
			mv->m_vecVelocity[2] = 100;
		else if (m_pCSPlayer->GetWaterType() == CONTENTS_SLIME)
			mv->m_vecVelocity[2] = 80;

		// play swiming sound
		if (m_pCSPlayer->m_flSwimSoundTime <= 0)
		{
			// Don't play sound again for 1 second
			m_pCSPlayer->m_flSwimSoundTime = 1000;
			PlaySwimSound();
		}

		return false;
	}

	// No more effect
	if (m_pCSPlayer->GetGroundEntity() == NULL)
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;		// in air, so no effect
	}

	//if (mv->m_nOldButtons & IN_JUMP)
	//	return false;		// don't pogo stick

	// In the air now.
	SetGroundEntity(NULL);

	m_pCSPlayer->PlayStepSound((Vector &)mv->GetAbsOrigin(), player->m_pSurfaceData, 1.0, true);

	//MoveHelper()->PlayerSetAnimation( PLAYER_JUMP );
	//m_pCSPlayer->DoAnimationEvent(PLAYERANIMEVENT_JUMP);

	float flGroundFactor = 1.0f;
	if (player->m_pSurfaceData)
	{
		flGroundFactor = player->m_pSurfaceData->game.jumpFactor;
	}

	// if we weren't ducking, bots and hostages do a crouchjump programatically
	if ((!player || player->IsBot()) && !(mv->m_nButtons & IN_DUCK))
	{
		//m_pCSPlayer->m_duckUntilOnGround = true;
		FinishDuck();
	}

	// Acclerate upward
	// If we are ducking...
	float startz = mv->m_vecVelocity[2];
	if ((m_pCSPlayer->m_Local.m_bDucking) || (m_pCSPlayer->GetFlags() & FL_DUCKING))
	{
		mv->m_vecVelocity[2] = flGroundFactor * sqrt(2 * 800 * 57.0);  // 2 * gravity * height
	}
	else
	{
		mv->m_vecVelocity[2] += flGroundFactor * sqrt(2 * 800 * 57.0);  // 2 * gravity * height
	}

	FinishGravity();

	mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
	mv->m_outStepHeight += 0.1f;

	// Flag that we jumped.
	mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released
	return true;

}

bool CHL2GameMovement::CanUnduck()
{
	trace_t trace;
	Vector newOrigin;

	VectorCopy(mv->GetAbsOrigin(), newOrigin);

	if (player->GetGroundEntity() != NULL)
	{
		newOrigin += VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
	}
	else
	{
		// If in air an letting go of croush, make sure we can offset origin to make
		//  up for uncrouching
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

		newOrigin += -0.5f * (hullSizeNormal - hullSizeCrouch);
	}

	UTIL_TraceHull(mv->GetAbsOrigin(), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX, PlayerSolidMask(), player, COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

	if (trace.startsolid || (trace.fraction != 1.0f))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Stop ducking
//-----------------------------------------------------------------------------
void CHL2GameMovement::FinishUnDuck(void)
{
	trace_t trace;
	Vector newOrigin;

	VectorCopy(mv->GetAbsOrigin(), newOrigin);

	if (player->GetGroundEntity() != NULL)
	{
		newOrigin += VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
	}
	else
	{
		// If in air an letting go of croush, make sure we can offset origin to make
		//  up for uncrouching
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

		Vector viewDelta = -0.5f * (hullSizeNormal - hullSizeCrouch);

		VectorAdd(newOrigin, viewDelta, newOrigin);
	}

	player->m_Local.m_bDucked = false;
	player->RemoveFlag(FL_DUCKING);
	player->m_Local.m_bDucking = false;
	player->SetViewOffset(GetPlayerViewOffset(false));
	player->m_Local.m_flDucktime = 0;

	mv->SetAbsOrigin(newOrigin);

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Finish ducking
//-----------------------------------------------------------------------------
void CHL2GameMovement::FinishDuck(void)
{
	Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
	Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

	Vector viewDelta = 0.5f * (hullSizeNormal - hullSizeCrouch);

	player->SetViewOffset(GetPlayerViewOffset(true));
	player->AddFlag(FL_DUCKING);
	player->m_Local.m_bDucking = false;

	if (!player->m_Local.m_bDucked)
	{

		Vector org = mv->GetAbsOrigin();

		if (player->GetGroundEntity() != NULL)
		{
			org -= VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
		}
		else
		{
			org += viewDelta;
		}
		mv->SetAbsOrigin(org);

		player->m_Local.m_bDucked = true;
	}

	// See if we are stuck?
	FixPlayerCrouchStuck(true);

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

//-----------------------------------------------------------------------------
// Purpose: See if duck button is pressed and do the appropriate things
//-----------------------------------------------------------------------------
void CHL2GameMovement::Duck(void)
{
	int buttonsChanged = (mv->m_nOldButtons ^ mv->m_nButtons);	// These buttons have changed this frame
	int buttonsPressed = buttonsChanged & mv->m_nButtons;			// The changed ones still down are "pressed"
	int buttonsReleased = buttonsChanged & mv->m_nOldButtons;		// The changed ones which were previously down are "released"

	if (mv->m_nButtons & IN_DUCK)
	{
		mv->m_nOldButtons |= IN_DUCK;
	}
	else
	{
		mv->m_nOldButtons &= ~IN_DUCK;
	}

	if (IsDead())
	{
		// Unduck
		if (player->GetFlags() & FL_DUCKING)
		{
			FinishUnDuck();
		}
		return;
	}

	HandleDuckingSpeedCrop();

	// Holding duck, in process of ducking or fully ducked?
	if ((mv->m_nButtons & IN_DUCK) || (player->m_Local.m_bDucking) || (player->GetFlags() & FL_DUCKING))
	{
		if (mv->m_nButtons & IN_DUCK)
		{
			bool alreadyDucked = (player->GetFlags() & FL_DUCKING) ? true : false;

			if ((buttonsPressed & IN_DUCK) && !(player->GetFlags() & FL_DUCKING))
			{
				// Use 1 second so super long jump will work
				player->m_Local.m_flDucktime = 1000;
				player->m_Local.m_bDucking = true;
			}

			float duckmilliseconds = max(0.0f, 1000.0f - (float)player->m_Local.m_flDucktime);
			float duckseconds = duckmilliseconds / 1000.0f;

			//time = max( 0.0, ( 1.0 - (float)player->m_Local.m_flDucktime / 1000.0 ) );

			if (player->m_Local.m_bDucking)
			{
				// Finish ducking immediately if duck time is over or not on ground
				if ((duckseconds > TIME_TO_DUCK) ||
					(player->GetGroundEntity() == NULL) ||
					alreadyDucked)
				{
					FinishDuck();
				}
				else
				{
					// Calc parametric time
					float duckFraction = SimpleSpline(duckseconds / TIME_TO_DUCK);
					SetDuckedEyeOffset(duckFraction);
				}
			}
		}
		else
		{
			// Try to unduck unless automovement is not allowed
			// NOTE: When not onground, you can always unduck
			if (player->m_Local.m_bAllowAutoMovement || player->GetGroundEntity() == NULL)
			{
				if ((buttonsReleased & IN_DUCK) && (player->GetFlags() & FL_DUCKING))
				{
					// Use 1 second so super long jump will work
					player->m_Local.m_flDucktime = 1000;
					player->m_Local.m_bDucking = true;  // or unducking
				}

				float duckmilliseconds = max(0.0f, 1000.0f - (float)player->m_Local.m_flDucktime);
				float duckseconds = duckmilliseconds / 1000.0f;

				if (CanUnduck())
				{
					if (player->m_Local.m_bDucking ||
						player->m_Local.m_bDucked) // or unducking
					{
						// Finish ducking immediately if duck time is over or not on ground
						if ((duckseconds > TIME_TO_UNDUCK) ||
							(player->GetGroundEntity() == NULL))
						{
							FinishUnDuck();
						}
						else
						{
							// Calc parametric time
							float duckFraction = SimpleSpline(1.0f - (duckseconds / TIME_TO_UNDUCK));
							SetDuckedEyeOffset(duckFraction);
						}
					}
				}
				else
				{
					// Still under something where we can't unduck, so make sure we reset this timer so
					//  that we'll unduck once we exit the tunnel, etc.
					player->m_Local.m_flDucktime = 1000;
				}
			}
		}
	}
}

void CHL2GameMovement::HandleDuckingSpeedCrop()
{
	if (!m_iSpeedCropped && (player->GetFlags() & FL_DUCKING))
	{
		float frac = 0.33333333f;
		mv->m_flForwardMove *= frac;
		mv->m_flSideMove *= frac;
		mv->m_flUpMove *= frac;
		m_iSpeedCropped = true;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
/**
* In CS, crouching up ladders goes slowly and doesn't make a sound.
*/
float CHL2GameMovement::ClimbSpeed(void) const
{
	if (mv->m_nButtons & IN_DUCK)
	{
		return BaseClass::ClimbSpeed() * DuckSpeedMultiplier;
	}
	else
	{
		return BaseClass::ClimbSpeed();
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
/**
* In CS, strafing on ladders goes slowly.
*/
float CHL2GameMovement::LadderLateralMultiplier(void) const
{
	if (mv->m_nButtons & IN_DUCK)
	{
		return 1.0f;
	}
	else
	{
		return 0.5f;
	}
}

void CHL2GameMovement::CheckParameters(void)
{
	BaseClass::CheckParameters();
}

//-------------------------------------------------------------------------------------------------------------------------------
/**
* Looks behind and beneath the player in the air, in case he skips out over the top of a ladder.  If the
* trace hits a ladder, the player is snapped to the ladder.
*/
void CHL2GameMovement::CheckForLadders(bool wasOnGround)
{
#ifndef CLIENT_DLL
	CHL2_Player* m_pCSPlayer = GetHL2Player();
	if (!wasOnGround)
	{
		// If we're higher than the last place we were on the ground, bail - obviously we're not dropping
		// past a ladder we might want to grab.
		if (mv->GetAbsOrigin().z > m_pCSPlayer->m_lastStandingPos.z)
			return;

		Vector dir = -m_pCSPlayer->m_lastStandingPos + mv->GetAbsOrigin();
		if (!dir.x && !dir.y)
		{
			// If we're dropping straight down, we don't know which way to look for a ladder.  Oh well.
			return;
		}

		dir.z = 0.0f;
		float dist = dir.NormalizeInPlace();
		if (dist > 64.0f)
		{
			// Don't grab ladders too far behind us.
			return;
		}

		trace_t trace;

		TracePlayerBBox(
			mv->GetAbsOrigin(),
			m_pCSPlayer->m_lastStandingPos - dir*(5 + dist),
			(PlayerSolidMask() & (~CONTENTS_PLAYERCLIP)), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

		if (trace.fraction != 1.0f && OnLadder(trace) && trace.plane.normal.z != 1.0f)
		{
			if (m_pCSPlayer->CanGrabLadder(trace.endpos, trace.plane.normal))
			{
				player->SetMoveType(MOVETYPE_LADDER);
				player->SetMoveCollide(MOVECOLLIDE_DEFAULT);

				player->SetLadderNormal(trace.plane.normal);
				mv->m_vecVelocity.Init();

				// The ladder check ignored playerclips, to fix a bug exposed by de_train, where a clipbrush is
				// flush with a ladder.  This causes the above tracehull to fail unless we ignore playerclips.
				// However, we have to check for playerclips before we snap to that pos, so we don't warp a
				// player into a clipbrush.
				TracePlayerBBox(
					mv->GetAbsOrigin(),
					m_pCSPlayer->m_lastStandingPos - dir*(5 + dist),
					PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

				mv->SetAbsOrigin(trace.endpos);
			}
		}
	}
	else
	{
		m_pCSPlayer->m_lastStandingPos = mv->GetAbsOrigin();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2GameMovement::LadderMove(void)
{
	bool isOnLadder = BaseClass::LadderMove();
#ifndef CLIENT_DLL
	CHL2_Player *m_pCSPlayer = GetHL2Player();
	if (isOnLadder && m_pCSPlayer)
	{

		m_pCSPlayer->SurpressLadderChecks(mv->GetAbsOrigin(), m_pCSPlayer->m_vecLadderNormal);
	}
#endif
	return isOnLadder;

}

void CHL2GameMovement::SetGroundEntity(trace_t *pm)
{
	CBaseEntity *newGround = pm ? pm->m_pEnt : NULL;

	//Adrian: Special case for combine balls.
	if (newGround && newGround->GetCollisionGroup() == HL2COLLISION_GROUP_COMBINE_BALL_NPC)
	{
		return;
	}

	BaseClass::SetGroundEntity(pm);
}

bool CHL2GameMovement::CanAccelerate()
{
	BaseClass::CanAccelerate();

	return true;
}


#ifndef PORTAL	// Portal inherits from this but needs to declare it's own global interface
// Expose our interface.
static CHL2GameMovement g_GameMovement;
IGameMovement *g_pGameMovement = (IGameMovement *)&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT, g_GameMovement);
#endif