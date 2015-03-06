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

static ConVar sv_autoladderdismount("sv_autoladderdismount", "1", FCVAR_REPLICATED, "Automatically dismount from ladders when you reach the end (don't have to +USE).");
static ConVar sv_ladderautomountdot("sv_ladderautomountdot", "0.4", FCVAR_REPLICATED, "When auto-mounting a ladder by looking up its axis, this is the tolerance for looking now directly along the ladder axis.");

static ConVar sv_ladder_useonly("sv_ladder_useonly", "0", FCVAR_REPLICATED, "If set, ladders can only be mounted by pressing +USE");

#define USE_DISMOUNT_SPEED 100

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHL2GameMovement::CHL2GameMovement()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : int
//-----------------------------------------------------------------------------
int CHL2GameMovement::GetCheckInterval(IntervalType_t type)
{
	// HL2 ladders need to check every frame!!!
	if (type == LADDER)
		return 1;

	return BaseClass::GetCheckInterval(type);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2GameMovement::IsForceMoveActive()
{
	LadderMove_t *lm = GetLadderMove();
	return lm->m_bForceLadderMove;
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
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : mounting - 
//			transit_speed - 
//			goalpos - 
//			*ladder - 
//-----------------------------------------------------------------------------
void CHL2GameMovement::StartForcedMove(bool mounting, float transit_speed, const Vector& goalpos, CFuncLadder *ladder)
{
	LadderMove_t* lm = GetLadderMove();
	Assert(lm);
	// Already active, just ignore
	if (lm->m_bForceLadderMove)
	{
		return;
	}

#if !defined( CLIENT_DLL )
	if (ladder)
	{
		ladder->PlayerGotOn(GetHL2Player());

		// If the Ladder only wants to be there for automount checking, abort now
		if (ladder->DontGetOnLadder())
			return;
	}

	// Reserve goal slot here
	bool valid = false;
	lm->m_hReservedSpot = CReservePlayerSpot::ReserveSpot(
		player,
		goalpos,
		GetPlayerMins((player->GetFlags() & FL_DUCKING) ? true : false),
		GetPlayerMaxs((player->GetFlags() & FL_DUCKING) ? true : false),
		valid);
	if (!valid)
	{
		// FIXME:  Play a deny sound?
		if (lm->m_hReservedSpot)
		{
			UTIL_Remove(lm->m_hReservedSpot);
			lm->m_hReservedSpot = NULL;
		}
		return;
	}
#endif

	// Use current player origin as start and new origin as dest
	lm->m_vecGoalPosition = goalpos;
	lm->m_vecStartPosition = mv->GetAbsOrigin();

	// Figure out how long it will take to make the gap based on transit_speed
	Vector delta = lm->m_vecGoalPosition - lm->m_vecStartPosition;

	float distance = delta.Length();

	Assert(transit_speed > 0.001f);

	// Compute time required to move that distance
	float transit_time = distance / transit_speed;
	if (transit_time < 0.001f)
	{
		transit_time = 0.001f;
	}

	lm->m_bForceLadderMove = true;
	lm->m_bForceMount = mounting;

	lm->m_flStartTime = gpGlobals->curtime;
	lm->m_flArrivalTime = lm->m_flStartTime + transit_time;

	lm->m_hForceLadder = ladder;

	// Don't get stuck during this traversal since we'll just be slamming the player origin
	player->SetMoveType(MOVETYPE_NONE);
	player->SetMoveCollide(MOVECOLLIDE_DEFAULT);
	player->SetSolid(SOLID_NONE);
	SetLadder(ladder);

	// Debounce the use key
	SwallowUseKey();
}

//-----------------------------------------------------------------------------
// Purpose: Returns false when finished
//-----------------------------------------------------------------------------
bool CHL2GameMovement::ContinueForcedMove()
{
	LadderMove_t* lm = GetLadderMove();
	Assert(lm);
	Assert(lm->m_bForceLadderMove);

	// Suppress regular motion
	mv->m_flForwardMove = 0.0f;
	mv->m_flSideMove = 0.0f;
	mv->m_flUpMove = 0.0f;

	// How far along are we
	float frac = (gpGlobals->curtime - lm->m_flStartTime) / (lm->m_flArrivalTime - lm->m_flStartTime);
	if (frac > 1.0f)
	{
		lm->m_bForceLadderMove = false;
#if !defined( CLIENT_DLL )
		// Remove "reservation entity"
		if (lm->m_hReservedSpot)
		{
			UTIL_Remove(lm->m_hReservedSpot);
			lm->m_hReservedSpot = NULL;
		}
#endif
	}

	frac = clamp(frac, 0.0f, 1.0f);

	// Move origin part of the way
	Vector delta = lm->m_vecGoalPosition - lm->m_vecStartPosition;

	// Compute interpolated position
	Vector org;
	VectorMA(lm->m_vecStartPosition, frac, delta, org);
	mv->SetAbsOrigin(org);

	// If finished moving, reset player to correct movetype (or put them on the ladder)
	if (!lm->m_bForceLadderMove)
	{
		player->SetSolid(SOLID_BBOX);
		player->SetMoveType(MOVETYPE_WALK);

		if (lm->m_bForceMount && lm->m_hForceLadder != NULL)
		{
			player->SetMoveType(MOVETYPE_LADDER);
			SetLadder(lm->m_hForceLadder);
		}

		// Zero out any velocity
		mv->m_vecVelocity.Init();
	}

	// Stil active
	return lm->m_bForceLadderMove;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the player is on a ladder
// Input  : &trace - ignored
//-----------------------------------------------------------------------------
bool CHL2GameMovement::OnLadder(trace_t &trace)
{
	return (GetLadder() != NULL) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : ladders - 
//			maxdist - 
//			**ppLadder - 
//			ladderOrigin - 
//-----------------------------------------------------------------------------
void CHL2GameMovement::Findladder(float maxdist, CFuncLadder **ppLadder, Vector& ladderOrigin, const CFuncLadder *skipLadder)
{
	CFuncLadder *bestLadder = NULL;
	float bestDist = MAX_COORD_INTEGER;
	Vector bestOrigin;

	bestOrigin.Init();

	float maxdistSqr = maxdist * maxdist;


	int c = CFuncLadder::GetLadderCount();
	for (int i = 0; i < c; i++)
	{
		CFuncLadder *ladder = CFuncLadder::GetLadder(i);

		if (!ladder->IsEnabled())
			continue;

		if (skipLadder && ladder == skipLadder)
			continue;

		Vector topPosition;
		Vector bottomPosition;

		ladder->GetTopPosition(topPosition);
		ladder->GetBottomPosition(bottomPosition);

		Vector closest;
		CalcClosestPointOnLineSegment(mv->GetAbsOrigin(), bottomPosition, topPosition, closest, NULL);

		float distSqr = (closest - mv->GetAbsOrigin()).LengthSqr();

		// Too far away
		if (distSqr > maxdistSqr)
		{
			continue;
		}

		// Need to trace to see if it's clear
		trace_t tr;

		UTIL_TraceLine(mv->GetAbsOrigin(), closest,
			MASK_PLAYERSOLID,
			player,
			COLLISION_GROUP_NONE,
			&tr);

		if (tr.fraction != 1.0f &&
			tr.m_pEnt &&
			tr.m_pEnt != ladder)
		{
			// Try a trace stepped up from the ground a bit, in case there's something at ground level blocking us.
			float sizez = GetPlayerMaxs().z - GetPlayerMins().z;

			UTIL_TraceLine(mv->GetAbsOrigin() + Vector(0, 0, sizez * 0.5f), closest,
				MASK_PLAYERSOLID,
				player,
				COLLISION_GROUP_NONE,
				&tr);

			if (tr.fraction != 1.0f &&
				tr.m_pEnt &&
				tr.m_pEnt != ladder &&
				!tr.m_pEnt->IsSolidFlagSet(FSOLID_TRIGGER))
			{
				continue;
			}
		}

		// See if this is the best one so far
		if (distSqr < bestDist)
		{
			bestDist = distSqr;
			bestLadder = ladder;
			bestOrigin = closest;
		}
	}

	// Return best ladder spot
	*ppLadder = bestLadder;
	ladderOrigin = bestOrigin;

}

static bool NearbyDismountLessFunc(const NearbyDismount_t& lhs, const NearbyDismount_t& rhs)
{
	return lhs.distSqr < rhs.distSqr;
}

void CHL2GameMovement::GetSortedDismountNodeList(const Vector &org, float radius, CFuncLadder *ladder, CUtlRBTree< NearbyDismount_t, int >& list)
{
	float radiusSqr = radius * radius;

	int i;
	int c = ladder->GetDismountCount();
	for (i = 0; i < c; i++)
	{
		CInfoLadderDismount *spot = ladder->GetDismount(i);
		if (!spot)
			continue;

		float distSqr = (spot->GetAbsOrigin() - org).LengthSqr();
		if (distSqr > radiusSqr)
			continue;

		NearbyDismount_t nd;
		nd.dismount = spot;
		nd.distSqr = distSqr;

		list.Insert(nd);
	}
}

void CHL2GameMovement::PlayerMove()
{
	CHL2_Player *m_pCSPlayer = GetHL2Player();

	BaseClass::PlayerMove();

	/*if (FBitSet(m_pCSPlayer->GetFlags(), FL_ONGROUND))
	{
		if (m_pCSPlayer->m_flVelocityModifier < 1.0)
		{
			m_pCSPlayer->m_flVelocityModifier += gpGlobals->frametime / 3.0f;
		}

		if (m_pCSPlayer->m_flVelocityModifier > 1.0)
			m_pCSPlayer->m_flVelocityModifier = 1.0;
	}*/

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
			//else if (m_pCSPlayer->m_duckUntilOnGround)
			//{
			//	// Duck Hull, but we're in the air.  Calculate where the view would be.
			//	Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
			//	Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

			//	// We've got the duck hull, pulled up to the top of where the player should be
			//	Vector lowerClearance = hullSizeNormal - hullSizeCrouch;
			//	Vector duckEyeHeight = GetPlayerViewOffset(false) - lowerClearance;
			//	player->SetViewOffset(duckEyeHeight);
			//}
			else if (player->m_Local.m_bDucked && !player->m_Local.m_bDucking)
			{
				player->SetViewOffset(VEC_DUCK_VIEW);
			}
		}
	}
}

bool CHL2GameMovement::CheckJumpButton()
{
	//CHL2_Player *m_pHL2Player = GetHL2Player();
	CHL2_Player *m_pCSPlayer = GetHL2Player();

	//Assert(m_pHL2Player);

	//if (m_pHL2Player->pl.deadflag)
	//{
	//	mv->m_nOldButtons |= IN_JUMP;   // don't jump again until released
	//	return false;
	//}

	//// See if we are waterjumping.  If so, decrement count and return.
	//if (m_pHL2Player->m_flWaterJumpTime)
	//{
	//	m_pHL2Player->m_flWaterJumpTime -= gpGlobals->frametime;
	//	if (m_pHL2Player->m_flWaterJumpTime < 0)
	//		m_pHL2Player->m_flWaterJumpTime = 0;

	//	return false;
	//}

	//// If we are in the water most of the way...
	//if (m_pHL2Player->GetWaterLevel() >= 2)
	//{
	//	// swimming, not jumping
	//	SetGroundEntity(NULL);

	//	if (m_pHL2Player->GetWaterType() == CONTENTS_WATER)    // We move up a certain amount
	//		mv->m_vecVelocity[2] = 100;
	//	else if (m_pHL2Player->GetWaterType() == CONTENTS_SLIME)
	//		mv->m_vecVelocity[2] = 80;

	//	// play swiming sound
	//	if (m_pHL2Player->m_flSwimSoundTime <= 0)
	//	{
	//		// Don't play sound again for 1 second
	//		m_pHL2Player->m_flSwimSoundTime = 1000;
	//		PlaySwimSound();
	//	}

	//	return false;
	//}

	//// No more effect
	//if (m_pHL2Player->GetGroundEntity() == NULL)
	//{
	//	mv->m_nOldButtons |= IN_JUMP;
	//	return false;           // in air, so no effect
	//}

	////	if (mv->m_nOldButtons & IN_JUMP)
	////		return false;           // don't pogo stick

	//// In the air now.
	//SetGroundEntity(NULL);

	//m_pHL2Player->PlayStepSound(mv->m_vecAbsOrigin, player->GetSurfaceData(), 1.0, true);

	//MoveHelper()->PlayerSetAnimation(PLAYER_JUMP);

	//float flGroundFactor = 1.0f;
	//if (player->GetSurfaceData())
	//{
	//	flGroundFactor = 1.0;//player->GetSurfaceData()->game.jumpFactor;
	//}

	//// Acclerate upward
	//// If we are ducking...
	//float startz = mv->m_vecVelocity[2];
	//mv->m_vecVelocity[2] += flGroundFactor * sqrt(2 * 800 * 45.0);  // 2 * gravity * height
	//FinishGravity();

	//mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
	//mv->m_outStepHeight += 0.1f;

	//// Flag that we jumped.
	//mv->m_nOldButtons |= IN_JUMP;   // don't jump again until released
	//return true;
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
	if (/*m_pCSPlayer->m_duckUntilOnGround || */(m_pCSPlayer->m_Local.m_bDucking) || (m_pCSPlayer->GetFlags() & FL_DUCKING))
	{
		// d = 0.5 * g * t^2		- distance traveled with linear accel
		// t = sqrt(2.0 * 45 / g)	- how long to fall 45 units
		// v = g * t				- velocity at the end (just invert it to jump up that high)
		// v = g * sqrt(2.0 * 45 / g )
		// v^2 = g * g * 2.0 * 45 / g
		// v = sqrt( g * 2.0 * 45 )

		mv->m_vecVelocity[2] = flGroundFactor * sqrt(2 * 800 * 57.0);  // 2 * gravity * height
	}
	else
	{
		mv->m_vecVelocity[2] += flGroundFactor * sqrt(2 * 800 * 57.0);  // 2 * gravity * height
	}

	/*if (m_pCSPlayer->m_flStamina > 0)
	{
	float flRatio;

	flRatio = (STAMINA_MAX - ((m_pCSPlayer->m_flStamina / 1000.0) * STAMINA_RECOVER_RATE)) / STAMINA_MAX;

	mv->m_vecVelocity[2] *= flRatio;
	}

	m_pCSPlayer->m_flStamina = (STAMINA_COST_JUMP / STAMINA_RECOVER_RATE) * 1000.0;*/

	FinishGravity();

	mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
	mv->m_outStepHeight += 0.1f;

#ifndef CLIENT_DLL
	// allow bots to react
	IGameEvent * event = gameeventmanager->CreateEvent("player_jump");
	if (event)
	{
		event->SetInt("userid", m_pCSPlayer->GetUserID());
		gameeventmanager->FireEvent(event);
	}
#endif

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

void CHL2GameMovement::CheckParameters(void)
{
	if (mv->m_nButtons & IN_SPEED)
	{
		mv->m_flClientMaxSpeed = 100;
	}
	else
	{
		mv->m_flClientMaxSpeed = mv->m_flMaxSpeed;
	}

	BaseClass::CheckParameters();
}

//-----------------------------------------------------------------------------
// Purpose: 
//			*ladder - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2GameMovement::ExitLadderViaDismountNode(CFuncLadder *ladder, bool strict, bool useAlternate)
{
	// Find the best ladder exit node
	float bestDot = -99999.0f;
	float bestDistance = 99999.0f;
	Vector bestDest;
	bool found = false;

	// For 'alternate' dismount
	bool foundAlternate = false;
	Vector alternateDest;
	float alternateDist = 99999.0f;

	CUtlRBTree< NearbyDismount_t, int >	nearbyDismounts(0, 0, NearbyDismountLessFunc);

	GetSortedDismountNodeList(mv->GetAbsOrigin(), 100.0f, ladder, nearbyDismounts);

	int i;

	for (i = nearbyDismounts.FirstInorder(); i != nearbyDismounts.InvalidIndex(); i = nearbyDismounts.NextInorder(i))
	{
		CInfoLadderDismount *spot = nearbyDismounts[i].dismount;
		if (!spot)
		{
			Assert(!"What happened to the spot!!!");
			continue;
		}

		// See if it's valid to put the player there...
		Vector org = spot->GetAbsOrigin() + Vector(0, 0, 1);

		trace_t tr;
		UTIL_TraceHull(
			org,
			org,
			GetPlayerMins((player->GetFlags() & FL_DUCKING) ? true : false),
			GetPlayerMaxs((player->GetFlags() & FL_DUCKING) ? true : false),
			MASK_PLAYERSOLID,
			player,
			COLLISION_GROUP_PLAYER_MOVEMENT,
			&tr);

		// Nope...
		if (tr.startsolid)
		{
			continue;
		}

		// Find the best dot product
		Vector vecToSpot = org - (mv->GetAbsOrigin() + player->GetViewOffset());
		vecToSpot.z = 0.0f;
		float d = VectorNormalize(vecToSpot);

		float dot = vecToSpot.Dot(m_vecForward);

		// We're not facing at it...ignore
		if (dot < 0.5f)
		{
			if (useAlternate && d < alternateDist)
			{
				alternateDest = org;
				alternateDist = d;
				foundAlternate = true;
			}

			continue;
		}

		if (dot > bestDot)
		{
			bestDest = org;
			bestDistance = d;
			bestDot = dot;
			found = true;
		}
	}

	if (found)
	{
		// Require a more specific 
		if (strict &&
			((bestDot < 0.7f) || (bestDistance > 40.0f)))
		{
			return false;
		}

		StartForcedMove(false, player->MaxSpeed(), bestDest, NULL);
		return true;
	}

	if (useAlternate)
	{
		// Desperate. Don't refuse to let a person off of a ladder if it can be helped. Use the
		// alternate dismount if there is one.
		if (foundAlternate && alternateDist <= 60.0f)
		{
			StartForcedMove(false, player->MaxSpeed(), alternateDest, NULL);
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bOnLadder - 
//-----------------------------------------------------------------------------
void CHL2GameMovement::FullLadderMove()
{
#if !defined( CLIENT_DLL )
	CFuncLadder *ladder = GetLadder();
	Assert(ladder);
	if (!ladder)
	{
		return;
	}

	CheckWater();

	// Was jump button pressed?  If so, don't do anything here
	if (mv->m_nButtons & IN_JUMP)
	{
		CheckJumpButton();
		return;
	}
	else
	{
		mv->m_nOldButtons &= ~IN_JUMP;
	}

	player->SetGroundEntity( NULL );

	// Remember old positions in case we cancel this movement
	Vector oldVelocity	= mv->m_vecVelocity;
	Vector oldOrigin	= mv->GetAbsOrigin();

	Vector topPosition;
	Vector bottomPosition;

	ladder->GetTopPosition(topPosition);
	ladder->GetBottomPosition(bottomPosition);

	// Compute parametric distance along ladder vector...
	float oldt;
	CalcDistanceSqrToLine(mv->GetAbsOrigin(), topPosition, bottomPosition, &oldt);

	// Perform the move accounting for any base velocity.
	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
	TryPlayerMove();
	VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

	// Pressed buttons are "changed(xor)" and'ed with the mask of currently held buttons
	int buttonsChanged = (mv->m_nOldButtons ^ mv->m_nButtons);	// These buttons have changed this frame
	int buttonsPressed = buttonsChanged & mv->m_nButtons;
	bool pressed_use = (buttonsPressed & IN_USE) ? true : false;
	bool pressing_forward_or_side = mv->m_flForwardMove != 0.0f || mv->m_flSideMove != 0.0f;

	Vector ladderVec = topPosition - bottomPosition;
	float LadderLength = VectorNormalize(ladderVec);
	// This test is not perfect by any means, but should help a bit
	bool moving_along_ladder = false;
	if (pressing_forward_or_side)
	{
		float fwdDot = m_vecForward.Dot(ladderVec);
		if (fabs(fwdDot) > 0.9f)
		{
			moving_along_ladder = true;
		}
	}

	// Compute parametric distance along ladder vector...
	float newt;
	CalcDistanceSqrToLine(mv->GetAbsOrigin(), topPosition, bottomPosition, &newt);

	// Fudge of 2 units
	float tolerance = 1.0f / LadderLength;

	bool wouldleaveladder = false;
	// Moving pPast top or bottom?
	if (newt < -tolerance)
	{
		wouldleaveladder = newt < oldt;
	}
	else if (newt > (1.0f + tolerance))
	{
		wouldleaveladder = newt > oldt;
	}

	// See if we are near the top or bottom but not moving
	float dist1sqr, dist2sqr;

	dist1sqr = (topPosition - mv->GetAbsOrigin()).LengthSqr();
	dist2sqr = (bottomPosition - mv->GetAbsOrigin()).LengthSqr();

	float dist = MIN(dist1sqr, dist2sqr);
	bool neardismountnode = (dist < 16.0f * 16.0f) ? true : false;
	float ladderUnitsPerTick = (MAX_CLIMB_SPEED * gpGlobals->interval_per_tick);
	bool neardismountnode2 = (dist < ladderUnitsPerTick * ladderUnitsPerTick) ? true : false;

	// Really close to node, cvar is set, and pressing a key, then simulate a +USE
	bool auto_dismount_use = (neardismountnode2 &&
		sv_autoladderdismount.GetBool() &&
		pressing_forward_or_side &&
		!moving_along_ladder);

	bool fully_underwater = (player->GetWaterLevel() == WL_Eyes) ? true : false;

	// If the user manually pressed use or we're simulating it, then use_dismount will occur
	bool use_dismount = pressed_use || auto_dismount_use;

	if (fully_underwater && !use_dismount)
	{
		// If fully underwater, we require looking directly at a dismount node 
		///  to "float off" a ladder mid way...
		if (ExitLadderViaDismountNode(ladder, true))
		{
			// See if they +used a dismount point mid-span..
			return;
		}
	}

	// If the movement would leave the ladder and they're not automated or pressing use, disallow the movement
	if (!use_dismount)
	{
		if (wouldleaveladder)
		{
			// Don't let them leave the ladder if they were on it
			mv->m_vecVelocity = oldVelocity;
			mv->SetAbsOrigin(oldOrigin);
		}
		return;
	}

	// If the move would not leave the ladder and we're near close to the end, then just accept the move
	if (!wouldleaveladder && !neardismountnode)
	{
		// Otherwise, if the move would leave the ladder, disallow it.
		if (pressed_use)
		{
			if (ExitLadderViaDismountNode(ladder, false, IsX360()))
			{
				// See if they +used a dismount point mid-span..
				return;
			}

			player->SetMoveType(MOVETYPE_WALK);
			player->SetMoveCollide(MOVECOLLIDE_DEFAULT);
			SetLadder(NULL);
			GetHL2Player()->m_bPlayUseDenySound = false;

			// Dismount with a bit of velocity in facing direction
			VectorScale(m_vecForward, USE_DISMOUNT_SPEED, mv->m_vecVelocity);
			mv->m_vecVelocity.z = 50;
		}
		return;
	}

	// Debounce the use key
	if (pressed_use)
	{
		SwallowUseKey();
	}

	// Try auto exit, if possible
	if (ExitLadderViaDismountNode(ladder, false, pressed_use))
	{
		return;
	}

	if (wouldleaveladder)
	{
		// Otherwise, if the move would leave the ladder, disallow it.
		if (pressed_use)
		{
			player->SetMoveType(MOVETYPE_WALK);
			player->SetMoveCollide(MOVECOLLIDE_DEFAULT);
			SetLadder(NULL);

			// Dismount with a bit of velocity in facing direction
			VectorScale(m_vecForward, USE_DISMOUNT_SPEED, mv->m_vecVelocity);
			mv->m_vecVelocity.z = 50;
		}
		else
		{
			mv->m_vecVelocity = oldVelocity;
			mv->SetAbsOrigin(oldOrigin);
		}
	}
#endif
}

bool CHL2GameMovement::CheckLadderAutoMountEndPoint(CFuncLadder *ladder, const Vector& bestOrigin)
{
	// See if we're really near an endpoint
	if (!ladder)
		return false;

	Vector top, bottom;
	ladder->GetTopPosition(top);
	ladder->GetBottomPosition(bottom);

	float d1, d2;

	d1 = (top - mv->GetAbsOrigin()).LengthSqr();
	d2 = (bottom - mv->GetAbsOrigin()).LengthSqr();

	if (d1 > 16 * 16 && d2 > 16 * 16)
		return false;

	Vector ladderAxis;

	if (d1 < 16 * 16)
	{
		// Close to top
		ladderAxis = bottom - top;
	}
	else
	{
		ladderAxis = top - bottom;
	}

	VectorNormalize(ladderAxis);

	if (ladderAxis.Dot(m_vecForward) > sv_ladderautomountdot.GetFloat())
	{
		StartForcedMove(true, player->MaxSpeed(), bestOrigin, ladder);
		return true;
	}

	return false;
}

bool CHL2GameMovement::CheckLadderAutoMountCone(CFuncLadder *ladder, const Vector& bestOrigin, float maxAngleDelta, float maxDistToLadder)
{
	// Never 'back' onto ladders or stafe onto ladders
	if (ladder != NULL &&
		(mv->m_flForwardMove > 0.0f))
	{
		Vector top, bottom;
		ladder->GetTopPosition(top);
		ladder->GetBottomPosition(bottom);

		Vector ladderAxis = top - bottom;
		VectorNormalize(ladderAxis);

		Vector probe = mv->GetAbsOrigin();

		Vector closest;
		CalcClosestPointOnLineSegment(probe, bottom, top, closest, NULL);

		Vector vecToLadder = closest - probe;

		float dist = VectorNormalize(vecToLadder);

		Vector flatLadder = vecToLadder;
		flatLadder.z = 0.0f;
		Vector flatForward = m_vecForward;
		flatForward.z = 0.0f;

		VectorNormalize(flatLadder);
		VectorNormalize(flatForward);

		float facingDot = flatForward.Dot(flatLadder);
		float angle = acos(facingDot) * 180 / M_PI;

		bool closetoladder = (dist != 0.0f && dist < maxDistToLadder) ? true : false;
		bool reallyclosetoladder = (dist != 0.0f && dist < 4.0f) ? true : false;

		bool facingladderaxis = (angle < maxAngleDelta) ? true : false;
		bool facingalongaxis = ((float)fabs(ladderAxis.Dot(m_vecForward)) > sv_ladderautomountdot.GetFloat()) ? true : false;
#if 0
		Msg("close %i length %.3f maxdist %.3f facing %.3f dot %.3f ang %.3f\n",
			closetoladder ? 1 : 0,
			dist,
			maxDistToLadder,
			(float)fabs(ladderAxis.Dot(m_vecForward)),
			facingDot,
			angle);
#endif

		// Tracker 21776:  Don't mount ladders this way if strafing
		bool strafing = (fabs(mv->m_flSideMove) < 1.0f) ? false : true;

		if (((facingDot > 0.0f && !strafing) || facingalongaxis) &&
			(facingladderaxis || reallyclosetoladder) &&
			closetoladder)
		{
			StartForcedMove(true, player->MaxSpeed(), bestOrigin, ladder);
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Must be facing toward ladder
// Input  : *ladder - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2GameMovement::LookingAtLadder(CFuncLadder *ladder)
{
	if (!ladder)
	{
		return false;
	}

	// Get ladder end points
	Vector top, bottom;
	ladder->GetTopPosition(top);
	ladder->GetBottomPosition(bottom);

	// Find closest point on ladder to player (could be an endpoint)
	Vector closest;
	CalcClosestPointOnLineSegment(mv->GetAbsOrigin(), bottom, top, closest, NULL);

	// Flatten our view direction to 2D
	Vector flatForward = m_vecForward;
	flatForward.z = 0.0f;

	// Because the ladder itself is not a solid, the player's origin may actually be 
	// permitted to pass it, and that will screw up our dot product.
	// So back up the player's origin a bit to do the facing calculation.
	Vector vecAdjustedOrigin = mv->GetAbsOrigin() - 8.0f * flatForward;

	// Figure out vector from player to closest point on ladder
	Vector vecToLadder = closest - vecAdjustedOrigin;

	// Flatten it to 2D
	Vector flatLadder = vecToLadder;
	flatLadder.z = 0.0f;

	// Normalize the vectors (unnecessary)
	VectorNormalize(flatLadder);
	VectorNormalize(flatForward);

	// Compute dot product to see if forward is in same direction as vec to ladder
	float facingDot = flatForward.Dot( flatLadder );

	float requiredDot = ( sv_ladder_useonly.GetBool() ) ? -0.99 : 0.0;

	// Facing same direction if dot > = requiredDot...
	bool facingladder = ( facingDot >= requiredDot );

	return facingladder;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &trace - 
//-----------------------------------------------------------------------------
bool CHL2GameMovement::CheckLadderAutoMount(CFuncLadder *ladder, const Vector& bestOrigin)
{
#if !defined( CLIENT_DLL )

	if (ladder != NULL)
	{
		StartForcedMove(true, player->MaxSpeed(), bestOrigin, ladder);
		return true;
	}

#endif
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2GameMovement::LadderMove(void)
{

	if (player->GetMoveType() == MOVETYPE_NOCLIP)
	{
		SetLadder(NULL);
		return false;
	}

	// If being forced to mount/dismount continue to act like we are on the ladder
	if ( IsForceMoveActive() && ContinueForcedMove() )
	{
		return true;
	}

	CFuncLadder *bestLadder = NULL;
	Vector bestOrigin(0, 0, 0);

	CFuncLadder *ladder = GetLadder();

	// Something 1) deactivated the ladder...  or 2) something external applied
	//  a force to us.  In either case  make the player fall, etc.
	if (ladder &&
		(!ladder->IsEnabled() ||
		(player->GetBaseVelocity().LengthSqr() > 1.0f)))
	{
		GetHL2Player()->ExitLadder();
		ladder = NULL;
	}

	if (!ladder)
	{
		Findladder(64.0f, &bestLadder, bestOrigin, NULL);
	}

#if !defined (CLIENT_DLL)
	if (!ladder && bestLadder && sv_ladder_useonly.GetBool())
	{
		GetHL2Player()->DisplayLadderHudHint();
	}
#endif

	int buttonsChanged = (mv->m_nOldButtons ^ mv->m_nButtons);	// These buttons have changed this frame
	int buttonsPressed = buttonsChanged & mv->m_nButtons;
	bool pressed_use = (buttonsPressed & IN_USE) ? true : false;

	// If I'm already moving on a ladder, use the previous ladder direction
	if (!ladder && !pressed_use)
	{
		// If flying through air, allow mounting ladders if we are facing < 15 degress from the ladder and we are close
		if (!ladder && !sv_ladder_useonly.GetBool())
		{
			// Tracker 6625:  Don't need to be leaping to auto mount using this method...
			// But if we are on the ground, then we must not be backing into the ladder (Tracker 12961)
			bool onground = player->GetGroundEntity() ? true : false;
			if (!onground || (mv->m_flForwardMove > 0.0f))
			{
				if (CheckLadderAutoMountCone(bestLadder, bestOrigin, 15.0f, 32.0f))
				{
					return true;
				}
			}

			// Pressing forward while looking at ladder and standing (or floating) near a mounting point
			if (mv->m_flForwardMove > 0.0f)
			{
				if (CheckLadderAutoMountEndPoint(bestLadder, bestOrigin))
				{
					return true;
				}
			}
		}

		return false;
	}

	if (!ladder &&
		LookingAtLadder(bestLadder) &&
		CheckLadderAutoMount(bestLadder, bestOrigin))
	{
		return true;
	}

	// Reassign the ladder
	ladder = GetLadder();
	if (!ladder)
	{
		return false;
	}

	// Don't play the deny sound
	if (pressed_use)
	{
		GetHL2Player()->m_bPlayUseDenySound = false;
	}

	// Make sure we are on the ladder
	player->SetMoveType(MOVETYPE_LADDER);
	player->SetMoveCollide(MOVECOLLIDE_DEFAULT);

	player->SetGravity(0.0f);

	float forwardSpeed = 0.0f;
	float rightSpeed = 0.0f;

	float speed = player->MaxSpeed();


	if (mv->m_nButtons & IN_BACK)
	{
		forwardSpeed -= speed;
	}

	if (mv->m_nButtons & IN_FORWARD)
	{
		forwardSpeed += speed;
	}

	if (mv->m_nButtons & IN_MOVELEFT)
	{
		rightSpeed -= speed;
	}

	if (mv->m_nButtons & IN_MOVERIGHT)
	{
		rightSpeed += speed;
	}

	if (mv->m_nButtons & IN_JUMP)
	{
		player->SetMoveType(MOVETYPE_WALK);
		// Remove from ladder
		SetLadder(NULL);

		// Jump in view direction
		Vector jumpDir = m_vecForward;

		// unless pressing backward or something like that
		if (mv->m_flForwardMove < 0.0f)
		{
			jumpDir = -jumpDir;
		}

		VectorNormalize(jumpDir);

		VectorScale(jumpDir, MAX_CLIMB_SPEED, mv->m_vecVelocity);
		// Tracker 13558:  Don't add any extra z velocity if facing downward at all
		if (m_vecForward.z >= 0.0f)
		{
			mv->m_vecVelocity.z = mv->m_vecVelocity.z + 50;
		}
		return false;
	}

	if (forwardSpeed != 0 || rightSpeed != 0)
	{
		// See if the player is looking toward the top or the bottom
		Vector velocity;

		VectorScale(m_vecForward, forwardSpeed, velocity);
		VectorMA(velocity, rightSpeed, m_vecRight, velocity);

		VectorNormalize(velocity);

		Vector ladderUp;
		ladder->ComputeLadderDir(ladderUp);
		VectorNormalize(ladderUp);

		Vector topPosition;
		Vector bottomPosition;

		ladder->GetTopPosition(topPosition);
		ladder->GetBottomPosition(bottomPosition);

		// Check to see if we've mounted the ladder in a bogus spot and, if so, just fall off the ladder...
		float dummyt = 0.0f;
		float distFromLadderSqr = CalcDistanceSqrToLine(mv->GetAbsOrigin(), topPosition, bottomPosition, &dummyt);
		if (distFromLadderSqr > 36.0f)
		{
			// Uh oh, we fell off zee ladder...
			player->SetMoveType(MOVETYPE_WALK);
			// Remove from ladder
			SetLadder(NULL);
			return false;
		}

		bool ishorizontal = fabs(topPosition.z - bottomPosition.z) < 64.0f ? true : false;

		float changeover = ishorizontal ? 0.0f : 0.3f;

		float factor = 1.0f;
		if (velocity.z >= 0)
		{
			float dotTop = ladderUp.Dot(velocity);
			if (dotTop < -changeover)
			{
				// Aimed at bottom
				factor = -1.0f;
			}
		}
		else
		{
			float dotBottom = -ladderUp.Dot(velocity);
			if (dotBottom > changeover)
			{
				factor = -1.0f;
			}
		}

		mv->m_vecVelocity = MAX_CLIMB_SPEED * factor * ladderUp;
	}
	else
	{
		mv->m_vecVelocity.Init();
	}

	return true;
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