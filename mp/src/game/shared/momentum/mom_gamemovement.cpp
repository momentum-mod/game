#include "cbase.h"

#include "in_buttons.h"
#include "mom_gamemovement.h"
#include "movevars_shared.h"
#include <rumble_shared.h>
#include <stdarg.h>

#include "tier0/memdbgon.h"

extern bool g_bMovementOptimizations;
// remove this eventually
ConVar sv_ramp_fix("sv_ramp_fix", "1");

#ifndef CLIENT_DLL
#include "env_player_surface_trigger.h"
static ConVar dispcoll_drawplane("dispcoll_drawplane", "0");
static MAKE_CONVAR(mom_punchangle_enable, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED,
                   "Toggle landing punchangle. 0 = OFF, 1 = ON\n", 0, 9999);
#endif

CMomentumGameMovement::CMomentumGameMovement() : m_flReflectNormal(NO_REFL_NORMAL_CHANGE), m_pPlayer(nullptr) {}

void CMomentumGameMovement::PlayerRoughLandingEffects(float fvol)
{
    if (fvol > 0.0)
    {
        //
        // Play landing sound right away.
        player->m_flStepSoundTime = 400;

        // Play step sound for current texture.
        player->PlayStepSound(const_cast<Vector &>(mv->GetAbsOrigin()), player->m_pSurfaceData, fvol, true);

#ifndef CLIENT_DLL
        //
        // Knock the screen around a little bit, temporary effect (IF ENABLED)
        //
        if (mom_punchangle_enable.GetBool())
        {
            player->m_Local.m_vecPunchAngle.Set(
                ROLL, player->m_Local.m_flFallVelocity * 0.013 * mom_punchangle_enable.GetInt());

            if (player->m_Local.m_vecPunchAngle[PITCH] > 8)
            {
                player->m_Local.m_vecPunchAngle.Set(PITCH, 8);
            }
        }

        player->RumbleEffect((fvol > 0.85f) ? (RUMBLE_FALL_LONG) : (RUMBLE_FALL_SHORT), 0, RUMBLE_FLAGS_NONE);
#endif
    }
}

void CMomentumGameMovement::DecayPunchAngle(void)
{
    float len;

    Vector vPunchAngle;

    vPunchAngle.x = m_pPlayer->m_Local.m_vecPunchAngle->x;
    vPunchAngle.y = m_pPlayer->m_Local.m_vecPunchAngle->y;
    vPunchAngle.z = m_pPlayer->m_Local.m_vecPunchAngle->z;

    len = VectorNormalize(vPunchAngle);
    len -= (10.0 + len * 0.5) * gpGlobals->frametime;
    len = max(len, 0.0);
    VectorScale(vPunchAngle, len, vPunchAngle);

    m_pPlayer->m_Local.m_vecPunchAngle.Set(0, vPunchAngle.x);
    m_pPlayer->m_Local.m_vecPunchAngle.Set(1, vPunchAngle.y);
    m_pPlayer->m_Local.m_vecPunchAngle.Set(2, vPunchAngle.z);
}

float CMomentumGameMovement::LadderLateralMultiplier(void) const { return mv->m_nButtons & IN_DUCK ? 1.0f : 0.5f; }

float CMomentumGameMovement::ClimbSpeed(void) const
{
    return (mv->m_nButtons & IN_DUCK ? BaseClass::ClimbSpeed() * DUCK_SPEED_MULTIPLIER : BaseClass::ClimbSpeed());
}

void CMomentumGameMovement::WalkMove()
{
    ConVarRef gm("mom_gamemode");
    if (gm.GetInt() == MOMGM_SCROLL)
    {
        if (m_pPlayer->m_flStamina > 0)
        {
            float flRatio;

            flRatio = (STAMINA_MAX - ((m_pPlayer->m_flStamina / 1000.0) * STAMINA_RECOVER_RATE)) / STAMINA_MAX;

            // This Goldsrc code was run with variable timesteps and it had framerate dependencies.
            // People looking at Goldsrc for reference are usually
            // (these days) measuring the stoppage at 60fps or greater, so we need
            // to account for the fact that Goldsrc was applying more stopping power
            // since it applied the slowdown across more frames.
            float flReferenceFrametime = 1.0f / 70.0f;
            float flFrametimeRatio = gpGlobals->frametime / flReferenceFrametime;

            flRatio = pow(flRatio, flFrametimeRatio);

            mv->m_vecVelocity.x *= flRatio;
            mv->m_vecVelocity.y *= flRatio;
        }
    }

    BaseClass::WalkMove();
    CheckForLadders(m_pPlayer->GetGroundEntity() != nullptr);
}

void CMomentumGameMovement::CheckForLadders(bool wasOnGround)
{
    if (!wasOnGround)
    {
        // If we're higher than the last place we were on the ground, bail - obviously we're not dropping
        // past a ladder we might want to grab.
        if (mv->GetAbsOrigin().z > m_pPlayer->m_lastStandingPos.z)
            return;

        Vector dir = -m_pPlayer->m_lastStandingPos + mv->GetAbsOrigin();
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

        TracePlayerBBox(mv->GetAbsOrigin(), m_pPlayer->m_lastStandingPos - dir * (5 + dist),
                        (PlayerSolidMask() & (~CONTENTS_PLAYERCLIP)), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

        if (trace.fraction != 1.0f && OnLadder(trace) && trace.plane.normal.z != 1.0f)
        {
            if (m_pPlayer->CanGrabLadder(trace.endpos, trace.plane.normal))
            {
                m_pPlayer->SetMoveType(MOVETYPE_LADDER);
                m_pPlayer->SetMoveCollide(MOVECOLLIDE_DEFAULT);

                m_pPlayer->SetLadderNormal(trace.plane.normal);
                mv->m_vecVelocity.Init();

                // The ladder check ignored playerclips, to fix a bug exposed by de_train, where a clipbrush is
                // flush with a ladder.  This causes the above tracehull to fail unless we ignore playerclips.
                // However, we have to check for playerclips before we snap to that pos, so we don't warp a
                // player into a clipbrush.
                TracePlayerBBox(mv->GetAbsOrigin(), m_pPlayer->m_lastStandingPos - dir * (5 + dist), PlayerSolidMask(),
                                COLLISION_GROUP_PLAYER_MOVEMENT, trace);

                mv->SetAbsOrigin(trace.endpos);
            }
        }
    }
    else
    {
        m_pPlayer->m_lastStandingPos = mv->GetAbsOrigin();
    }
}

bool CMomentumGameMovement::LadderMove(void)
{
    bool isOnLadder = BaseClass::LadderMove();
    if (isOnLadder && m_pPlayer)
    {
        m_pPlayer->SurpressLadderChecks(mv->GetAbsOrigin(), m_pPlayer->m_vecLadderNormal);
    }
    return isOnLadder;
}

bool CMomentumGameMovement::OnLadder(trace_t &trace)
{
    if (trace.plane.normal.z == 1.0f)
        return false;

    return BaseClass::OnLadder(trace);
}

void CMomentumGameMovement::HandleDuckingSpeedCrop()

{
    if (!m_iSpeedCropped & SPEED_CROPPED_DUCK)
    {
        if ((mv->m_nButtons & IN_DUCK) || (player->m_Local.m_bDucking) || (player->GetFlags() & FL_DUCKING))
        {
            mv->m_flForwardMove *= DUCK_SPEED_MULTIPLIER;
            mv->m_flSideMove *= DUCK_SPEED_MULTIPLIER;
            mv->m_flUpMove *= DUCK_SPEED_MULTIPLIER;
            m_iSpeedCropped |= SPEED_CROPPED_DUCK;
        }
    }
}

bool CMomentumGameMovement::CanUnduck()
{
    trace_t trace;
    Vector newOrigin;

    VectorCopy(mv->GetAbsOrigin(), newOrigin);

    if (player->GetGroundEntity() != nullptr || m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE)
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

    UTIL_TraceHull(mv->GetAbsOrigin(), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX, PlayerSolidMask(), player,
                   COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

    if (trace.startsolid || (trace.fraction != 1.0f))
        return false;

    return true;
}

void CMomentumGameMovement::Duck(void)
{
    int buttonsChanged = (mv->m_nOldButtons ^ mv->m_nButtons); // These buttons have changed this frame
    int buttonsPressed = buttonsChanged & mv->m_nButtons;      // The changed ones still down are "pressed"
    int buttonsReleased =
        buttonsChanged & mv->m_nOldButtons; // The changed ones which were previously down are "released"

    // Check to see if we are in the air.
    bool bInAir = player->GetGroundEntity() == nullptr && player->GetMoveType() != MOVETYPE_LADDER;

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

    if (m_pPlayer->m_duckUntilOnGround)
    {
        if (!bInAir)
        {
            m_pPlayer->m_duckUntilOnGround = false;
            if (CanUnduck())
            {
                FinishUnDuck();
            }
            return;
        }
        else
        {
            if (mv->m_vecVelocity.z > 0.0f)
                return;

            // Check if we can un-duck.  We want to unduck if we have space for the standing hull, and
            // if it is less than 2 inches off the ground.
            trace_t trace;
            Vector newOrigin;
            Vector groundCheck;

            VectorCopy(mv->GetAbsOrigin(), newOrigin);
            Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
            Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
            newOrigin -= (hullSizeNormal - hullSizeCrouch);
            groundCheck = newOrigin;
            groundCheck.z -= player->GetStepSize();

            UTIL_TraceHull(newOrigin, groundCheck, VEC_HULL_MIN, VEC_HULL_MAX, PlayerSolidMask(), player,
                           COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

            if (trace.startsolid || trace.fraction == 1.0f)
                return; // Can't even stand up, or there's no ground underneath us

            m_pPlayer->m_duckUntilOnGround = false;
            if (CanUnduck())
            {
                FinishUnDuck();
            }
            return;
        }
    }

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

            // time = max( 0.0, ( 1.0 - (float)player->m_Local.m_flDucktime / 1000.0 ) );

            if (player->m_Local.m_bDucking)
            {
                // Finish ducking immediately if duck time is over or not on ground
                if ((duckseconds > TIME_TO_DUCK) ||
                    !(m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE) && player->GetGroundEntity() == nullptr ||
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
            if (player->m_Local.m_bAllowAutoMovement ||
                !(m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE) && player->GetGroundEntity() == nullptr)
            {
                if ((buttonsReleased & IN_DUCK) && (player->GetFlags() & FL_DUCKING))
                {
                    // Use 1 second so super long jump will work
                    player->m_Local.m_flDucktime = 1000;
                    player->m_Local.m_bDucking = true; // or unducking
                }

                float duckmilliseconds = max(0.0f, 1000.0f - (float)player->m_Local.m_flDucktime);
                float duckseconds = duckmilliseconds / 1000.0f;

                if (CanUnduck())
                {
                    if (player->m_Local.m_bDucking || player->m_Local.m_bDucked) // or unducking
                    {
                        // Finish ducking immediately if duck time is over or not on ground
                        if ((duckseconds > TIME_TO_UNDUCK) ||
                            !(m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE) && player->GetGroundEntity() == nullptr)
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

//-----------------------------------------------------------------------------
// Purpose: Stop ducking
//-----------------------------------------------------------------------------
void CMomentumGameMovement::FinishUnDuck(void)
{
    trace_t trace;
    Vector newOrigin;

    VectorCopy(mv->GetAbsOrigin(), newOrigin);

    if (player->GetGroundEntity() != nullptr || (m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE))
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
    player->m_Local.m_flDucktime = 0.f;

    mv->SetAbsOrigin(newOrigin);

    // Recategorize position since ducking can change origin
    CategorizePosition();
}

//-----------------------------------------------------------------------------
// Purpose: Finish ducking
//-----------------------------------------------------------------------------
void CMomentumGameMovement::FinishDuck(void)
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

        if (player->GetGroundEntity() != nullptr)
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

void CMomentumGameMovement::PlayerMove()
{
    BaseClass::PlayerMove();

    if (player->IsAlive())
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

        Vector fudge(1, 1, 0.f);
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

bool CMomentumGameMovement::CheckJumpButton()
{
    // Avoid nullptr access, return false if somehow we don't have a player
    if (!player)
        return false;

    if (player->pl.deadflag)
    {
        mv->m_nOldButtons |= IN_JUMP; // don't jump again until released
        return false;
    }

    // See if we are waterjumping.  If so, decrement count and return.
    if (player->m_flWaterJumpTime)
    {
        player->m_flWaterJumpTime -= gpGlobals->frametime;
        if (player->m_flWaterJumpTime < 0.0f)
            player->m_flWaterJumpTime = 0.0f;

        return false;
    }

    // If we are in the water most of the way...
    if (player->GetWaterLevel() >= 2)
    {
        // swimming, not jumping
        SetGroundEntity(nullptr);

        if (player->GetWaterType() == CONTENTS_WATER) // We move up a certain amount
            mv->m_vecVelocity[2] = 100;
        else if (player->GetWaterType() == CONTENTS_SLIME)
            mv->m_vecVelocity[2] = 80;

        // play swiming sound
        if (player->m_flSwimSoundTime <= 0.0f)
        {
            // Don't play sound again for 1 second
            player->m_flSwimSoundTime = 1000;
            PlaySwimSound();
        }

        return false;
    }

    // No more effect
    if (player->GetGroundEntity() == nullptr)
    {
        mv->m_nOldButtons |= IN_JUMP;
        return false; // in air, so no effect
    }

    // AUTOBHOP---
    // only run this code if autobhop is disabled
    if (!m_pPlayer->HasAutoBhop())
    {
        if (mv->m_nOldButtons & IN_JUMP)
            return false; // don't pogo stick
    }

    // In the air now.
    SetGroundEntity(nullptr);

    // Set the last jump time
    m_pPlayer->m_SrvData.m_RunData.m_flLastJumpTime = gpGlobals->curtime;

    player->PlayStepSound(const_cast<Vector &>(mv->GetAbsOrigin()), player->m_pSurfaceData, 1.0, true);

    // MoveHelper()->PlayerSetAnimation( PLAYER_JUMP );
    // player->DoAnimationEvent(PLAYERANIMEVENT_JUMP);

    float flGroundFactor = 1.0f;
    if (player->m_pSurfaceData)
    {
        flGroundFactor = player->m_pSurfaceData->game.jumpFactor;
    }

    // if we weren't ducking, bots and hostages do a crouchjump programatically
    if (player->IsBot() && !(mv->m_nButtons & IN_DUCK))
    {
        m_pPlayer->m_duckUntilOnGround = true;
        FinishDuck();
    }

    // Acclerate upward
    // If we are ducking...
    float startz = mv->m_vecVelocity[2];
    if ((player->m_Local.m_bDucking) || (player->GetFlags() & FL_DUCKING))
    {
        mv->m_vecVelocity[2] = g_bMovementOptimizations
                                   ? flGroundFactor * GROUND_FACTOR_MULTIPLIER
                                   : flGroundFactor * sqrt(2.f * 800.f * 57.0f); // 2 * gravity * height
    }
    else
    {
        mv->m_vecVelocity[2] += g_bMovementOptimizations
                                    ? flGroundFactor * GROUND_FACTOR_MULTIPLIER
                                    : flGroundFactor * sqrt(2.f * 800.f * 57.0f); // 2 * gravity * height
    }

    // stamina stuff (scroll/kz gamemode only)
    ConVarRef gm("mom_gamemode");
    if (gm.GetInt() == MOMGM_SCROLL)
    {
        if (m_pPlayer->m_flStamina > 0)
        {
            float flRatio;

            flRatio = (STAMINA_MAX - ((m_pPlayer->m_flStamina / 1000.0) * STAMINA_RECOVER_RATE)) / STAMINA_MAX;
            mv->m_vecVelocity[2] *= flRatio;
        }

        m_pPlayer->m_flStamina = (STAMINA_COST_JUMP / STAMINA_RECOVER_RATE) * 1000.0;
    }

    FinishGravity();

    mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
    mv->m_outStepHeight += 0.1f;

    // Flag that we jumped.
    mv->m_nOldButtons |= IN_JUMP; // don't jump again until released
    return true;
}

void CMomentumGameMovement::CategorizePosition()
{
    Vector point;
    trace_t pm;

    // Reset this each time we-recategorize, otherwise we have bogus friction when we jump into water and plunge
    // downward really quickly
    player->m_surfaceFriction = 1.0f;

    // if the player hull point one unit down is solid, the player
    // is on ground

    // see if standing on something solid

    // Doing this before we move may introduce a potential latency in water detection, but
    // doing it after can get us stuck on the bottom in water if the amount we move up
    // is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
    // this several times per frame, so we really need to avoid sticking to the bottom of
    // water on each call, and the converse case will correct itself if called twice.
    CheckWater();

    // observers don't have a ground entity
    if (player->IsObserver())
        return;

    float flOffset = 2.0f;

    point[0] = mv->GetAbsOrigin()[0];
    point[1] = mv->GetAbsOrigin()[1];
    point[2] = mv->GetAbsOrigin()[2] - flOffset;

    Vector bumpOrigin;
    bumpOrigin = mv->GetAbsOrigin();

// Shooting up really fast.  Definitely not on ground.
// On ladder moving up, so not on ground either
// NOTE: 145 is a jump.
#define NON_JUMP_VELOCITY 140.0f

    float zvel = mv->m_vecVelocity[2];
    bool bMovingUp = zvel > 0.0f;
    bool bMovingUpRapidly = zvel > NON_JUMP_VELOCITY;
    float flGroundEntityVelZ = 0.0f;
    if (bMovingUpRapidly)
    {
        // Tracker 73219, 75878:  ywb 8/2/07
        // After save/restore (and maybe at other times), we can get a case where we were saved on a lift and
        //  after restore we'll have a high local velocity due to the lift making our abs velocity appear high.
        // We need to account for standing on a moving ground object in that case in order to determine if we really
        //  are moving away from the object we are standing on at too rapid a speed.  Note that CheckJump already sets
        //  ground entity to NULL, so this wouldn't have any effect unless we are moving up rapidly not from the jump
        //  button.
        CBaseEntity *ground = player->GetGroundEntity();
        if (ground)
        {
            flGroundEntityVelZ = ground->GetAbsVelocity().z;
            bMovingUpRapidly = (zvel - flGroundEntityVelZ) > NON_JUMP_VELOCITY;
        }
    }

    // Was on ground, but now suddenly am not
    if (bMovingUpRapidly || (bMovingUp && player->GetMoveType() == MOVETYPE_LADDER))
    {
        SetGroundEntity(nullptr);
    }
    else
    {
        // Try and move down.
        TryTouchGround(bumpOrigin, point, GetPlayerMins(), GetPlayerMaxs(), MASK_PLAYERSOLID,
                       COLLISION_GROUP_PLAYER_MOVEMENT, pm);

        // Was on ground, but now suddenly am not.  If we hit a steep plane, we are not on ground
        if (!pm.m_pEnt || pm.plane.normal[2] < 0.7f)
        {
            // Test four sub-boxes, to see if any of them would have found shallower slope we could actually stand on
            TryTouchGroundInQuadrants(bumpOrigin, point, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm);

            if (!pm.m_pEnt || pm.plane.normal[2] < 0.7f)
            {
                SetGroundEntity(nullptr);
                // probably want to add a check for a +z velocity too!
                if ((mv->m_vecVelocity.z > 0.0f) && (player->GetMoveType() != MOVETYPE_NOCLIP))
                {
                    player->m_surfaceFriction = 0.25f;
                }
            }
            else
            {
                if (m_flReflectNormal == NO_REFL_NORMAL_CHANGE)
                {
                    DoLateReflect();
                    CategorizePosition();

                    return;
                }

                SetGroundEntity(&pm);
            }
        }
        else
        {
            if (m_flReflectNormal == NO_REFL_NORMAL_CHANGE)
            {
                DoLateReflect();
                CategorizePosition();

                return;
            }

            SetGroundEntity(&pm); // Otherwise, point to index of ent under us.
        }

#ifndef CLIENT_DLL

        // If our gamematerial has changed, tell any player surface triggers that are watching
        IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
        surfacedata_t *pSurfaceProp = physprops->GetSurfaceData(pm.surface.surfaceProps);
        char cCurrGameMaterial = pSurfaceProp->game.material;
        if (!player->GetGroundEntity())
        {
            cCurrGameMaterial = 0;
        }

        // Changed?
        if (player->m_chPreviousTextureType != cCurrGameMaterial)
        {
            CEnvPlayerSurfaceTrigger::SetPlayerSurface(player, cCurrGameMaterial);
        }

        player->m_chPreviousTextureType = cCurrGameMaterial;
#endif
    }
}

void CMomentumGameMovement::FinishGravity(void)
{
    if (player->m_flWaterJumpTime)
        return;

    // Get the correct velocity for the end of the dt
    mv->m_vecVelocity[2] -= (player->GetGravity() * GetCurrentGravity() * 0.5 * gpGlobals->frametime);

    CheckVelocity();
}

void CMomentumGameMovement::StartGravity(void)
{
    // Add gravity so they'll be in the correct position during movement
    // yes, this 0.5 looks wrong, but it's not.
    mv->m_vecVelocity[2] -= (player->GetGravity() * GetCurrentGravity() * 0.5 * gpGlobals->frametime);
    mv->m_vecVelocity[2] += player->GetBaseVelocity()[2] * gpGlobals->frametime;

    Vector temp = player->GetBaseVelocity();
    temp[2] = 0;
    player->SetBaseVelocity(temp);

    CheckVelocity();
}

void CMomentumGameMovement::FullWalkMove()
{
    if (!(CheckWater() && !(m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE)))
    {
        StartGravity();
    }

    // If we are leaping out of the water, just update the counters.
    if (player->m_flWaterJumpTime)
    {
        WaterJump();
        TryPlayerMove();
        // See if we are still in water?
        CheckWater();
        return;
    }

    // If we are swimming in the water, see if we are nudging against a place we can jump up out
    //  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
    // If sliding is set we prefer to simulate sliding than being in water.. Could be fun for some mappers
    // that want sliding/iceskating into water. Who knows.
    if ((player->GetWaterLevel() >= WL_Waist) && !(m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE))
    {
        if (player->GetWaterLevel() == WL_Waist)
        {
            CheckWaterJump();
        }

        // If we are falling again, then we must not trying to jump out of water any more.
        if (mv->m_vecVelocity[2] < 0 && player->m_flWaterJumpTime)
        {
            player->m_flWaterJumpTime = 0;
        }

        // Was jump button pressed?
        if (mv->m_nButtons & IN_JUMP)
        {
            CheckJumpButton();
        }
        else
        {
            mv->m_nOldButtons &= ~IN_JUMP;
        }

        // Perform regular water movement
        WaterMove();

        // Redetermine position vars
        CategorizePosition();

        // If we are on ground, no downward velocity.
        if (player->GetGroundEntity() != nullptr)
        {
            mv->m_vecVelocity[2] = 0.f;
        }
    }
    else
    // Not fully underwater
    {
        // Was jump button pressed?
        if (mv->m_nButtons & IN_JUMP)
        {
            CheckJumpButton();
        }
        else
        {
            mv->m_nOldButtons &= ~IN_JUMP;
        }

        // Fricion is handled before we add in any base velocity. That way, if we are on a conveyor,
        //  we don't slow when standing still, relative to the conveyor.
        if (player->GetGroundEntity() != nullptr)
        {
            mv->m_vecVelocity[2] = 0.0f;
            Friction();
        }

        // Make sure velocity is valid.
        CheckVelocity();

        // By default assume we did the reflect for WalkMove()
        m_flReflectNormal = 1.0f;

        if (player->GetGroundEntity() != nullptr)
        {
            WalkMove();
        }
        else
        {
            AirMove(); // Take into account movement when in air.
        }

        // Set final flags.
        CategorizePosition();

        // Make sure velocity is valid.
        CheckVelocity();

        // Add any remaining gravitational component.
        if (!(CheckWater() && !(m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE)))
        {
            FinishGravity();
        }

        // If we are on ground, no downward velocity.
        if (player->GetGroundEntity() != nullptr)
        {
            mv->m_vecVelocity[2] = 0.f;
        }

        CheckFalling();

        // Stuck the player to ground, if flag on sliding is set so.
        if ((m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE_STUCKONGROUND) && (m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE))
            StuckGround();
    }

    if ((m_nOldWaterLevel == WL_NotInWater && player->GetWaterLevel() != WL_NotInWater) ||
        (m_nOldWaterLevel != WL_NotInWater && player->GetWaterLevel() == WL_NotInWater))
    {
        PlaySwimSound();
#if !defined(CLIENT_DLL)
        player->Splash();
#endif
    }
}

void CMomentumGameMovement::StuckGround(void)
{
    trace_t tr;
    Ray_t ray;

    Vector vAbsOrigin = mv->GetAbsOrigin(), vEnd = vAbsOrigin;
    vEnd[2] -= 8192.0f; // 8192 should be enough

    ray.Init(vAbsOrigin, vEnd, GetPlayerMins(), GetPlayerMaxs());

    CTraceFilterSimple tracefilter(player, COLLISION_GROUP_NONE);
    enginetrace->TraceRay(ray, MASK_PLAYERSOLID, &tracefilter, &tr);

    float fAdjust = ((vEnd[2] - vAbsOrigin[2]) * -tr.fraction) - 2.0f;

    if (abs(fAdjust) < 4096.0f) // Check if it's reasonable. If yes then apply our adjustement + our offset
        vAbsOrigin.z -= fAdjust;

    mv->SetAbsOrigin(vAbsOrigin);
}

void CMomentumGameMovement::AirMove(void)
{
    int i;
    Vector wishvel;
    float fmove, smove;
    Vector wishdir;
    float wishspeed;
    Vector forward, right, up;

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;

    // Zero out z components of movement vectors
    forward[2] = 0;
    right[2] = 0;
    VectorNormalize(forward); // Normalize remainder of vectors
    VectorNormalize(right);   //

    for (i = 0; i < 2; i++) // Determine x and y parts of velocity
        wishvel[i] = forward[i] * fmove + right[i] * smove;
    wishvel[2] = 0; // Zero out z part of velocity

    VectorCopy(wishvel, wishdir); // Determine maginitude of speed of move
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    if (wishspeed != 0 && (wishspeed > mv->m_flMaxSpeed))
    {
        VectorScale(wishvel, mv->m_flMaxSpeed / wishspeed, wishvel);
        wishspeed = mv->m_flMaxSpeed;
    }

    AirAccelerate(wishdir, wishspeed, sv_airaccelerate.GetFloat());

    // Add in any base velocity to the current velocity.
    VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

    m_flReflectNormal = NO_REFL_NORMAL_CHANGE;

    TryPlayerMove();

    // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or
    // maybe another monster?)
    VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

    CheckForLadders(false);
    // return bDidReflect;
}

void CMomentumGameMovement::DoLateReflect(void)
{
    // Don't attempt to reflect after this.
    // Return below was causing recursion.
    m_flReflectNormal = 1.0f;

    if (mv->m_vecVelocity.Length() == 0.0f || player->GetGroundEntity() != nullptr)
        return;

    Vector prevpos = mv->m_vecAbsOrigin;
    Vector prevvel = mv->m_vecVelocity;

    VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

    // Since we're doing two moves in one frame, only apply changes if we did the reflect and we gained speed.
    TryPlayerMove();
    if (m_flReflectNormal == 1.0f || prevvel.Length2DSqr() > mv->m_vecVelocity.Length2DSqr())
    {
        VectorCopy(prevpos, mv->m_vecAbsOrigin);
        VectorCopy(prevvel, mv->m_vecVelocity);
    }
    else
    {
        VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

        DevMsg("Successful late reflect! Normal: %.2f\n", m_flReflectNormal);
    }
}

int CMomentumGameMovement::TryPlayerMove(Vector *pFirstDest, trace_t *pFirstTrace)
{
    int bumpcount, numbumps;
    Vector dir;
    float d;
    int numplanes;
    Vector planes[MAX_CLIP_PLANES];
    Vector primal_velocity, original_velocity;
    Vector new_velocity;
    int i, j;
    trace_t pm;
    Vector end;
    float time_left, allFraction;
    int blocked;

    numbumps = 4; // Bump up to four times

    blocked = 0;   // Assume not blocked
    numplanes = 0; //  and not sliding along any planes

    VectorCopy(mv->m_vecVelocity, original_velocity); // Store original velocity
    VectorCopy(mv->m_vecVelocity, primal_velocity);

    allFraction = 0;
    time_left = gpGlobals->frametime; // Total time for this movement operation.

    new_velocity.Init();

    for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
    {
        if (mv->m_vecVelocity.Length() == 0.0)
            break;

        // Assume we can move all the way from the current origin to the
        //  end point.
        VectorMA(mv->GetAbsOrigin(), time_left, mv->m_vecVelocity, end);

        // See if we can make it from origin to end point.
        if (g_bMovementOptimizations)
        {
            // If their velocity Z is 0, then we can avoid an extra trace here during WalkMove.
            if (pFirstDest && end == *pFirstDest)
                pm = *pFirstTrace;
            else
            {
#if defined(PLAYER_GETTING_STUCK_TESTING)
                trace_t foo;
                TracePlayerBBox(mv->GetAbsOrigin(), mv->GetAbsOrigin(), PlayerSolidMask(),
                                COLLISION_GROUP_PLAYER_MOVEMENT, foo);
                if (foo.startsolid || foo.fraction != 1.0f)
                {
                    Msg("bah\n");
                }
#endif
                TracePlayerBBox(mv->GetAbsOrigin(), end, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);
            }
        }
        else
        {
            TracePlayerBBox(mv->GetAbsOrigin(), end, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);
        }

        allFraction += pm.fraction;

        // If we started in a solid object, or we were in solid space
        //  the whole way, zero out our velocity and return that we
        //  are blocked by floor and wall.
        if (pm.allsolid)
        {
            // entity is trapped in another solid
            VectorCopy(vec3_origin, mv->m_vecVelocity);
            return 4;
        }

        // This part can stuck the player on some surf maps, like surf_ski_2_nova
        // So I've added the rampfix convar here.
        // If we moved some portion of the total distance, then
        //  copy the end position into the pmove.origin and
        //  zero the plane counter.
        if (pm.fraction > 0)
        {
            if ((numbumps > 0 && pm.fraction == 1) && !sv_ramp_fix.GetBool())
            {
                // There's a precision issue with terrain tracing that can cause a swept box to successfully trace
                // when the end position is stuck in the triangle.  Re-run the test with an uswept box to catch that
                // case until the bug is fixed.
                // If we detect getting stuck, don't allow the movement
                trace_t stuck;
                TracePlayerBBox(pm.endpos, pm.endpos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, stuck);
                if (stuck.startsolid || stuck.fraction != 1.0f)
                {
                    Msg("Player will become stuck!!!\n");
                    VectorCopy(vec3_origin, mv->m_vecVelocity);
                    break;
                }
            }

#if defined(PLAYER_GETTING_STUCK_TESTING)
            trace_t foo;
            TracePlayerBBox(pm.endpos, pm.endpos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, foo);
            if (foo.startsolid || foo.fraction != 1.0f)
            {
                Msg("Player will become stuck!!!\n");
            }
#endif
            // actually covered some distance
            mv->SetAbsOrigin(pm.endpos);
            VectorCopy(mv->m_vecVelocity, original_velocity);
            numplanes = 0;
        }

        // If we covered the entire distance, we are done
        //  and can return.
        if (pm.fraction == 1)
        {
            break; // moved the entire distance
        }

        // Save entity that blocked us (since fraction was < 1.0)
        //  for contact
        // Add it if it's not already in the list!!!
        MoveHelper()->AddToTouched(pm, mv->m_vecVelocity);

        // If the plane we hit has a high z component in the normal, then
        //  it's probably a floor
        if (pm.plane.normal[2] > 0.7)
        {
            blocked |= 1; // floor
        }
        // If the plane has a zero z component in the normal, then it's a
        //  step or wall
        if (!pm.plane.normal[2])
        {
            blocked |= 2; // step / wall
        }

        // Reduce amount of m_flFrameTime left by total time left * fraction
        //  that we covered.
        time_left -= time_left * pm.fraction;

        // Did we run out of planes to clip against?
        if (numplanes >= MAX_CLIP_PLANES)
        {
            // this shouldn't really happen
            //  Stop our movement if so.
            VectorCopy(vec3_origin, mv->m_vecVelocity);
            // Con_DPrintf("Too many planes 4\n");

            break;
        }

        // Set up next clipping plane
        VectorCopy(pm.plane.normal, planes[numplanes]);
        numplanes++;

        // modify original_velocity so it parallels all of the clip planes
        //

        // reflect player velocity
        // Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping
        // in place
        //  and pressing forward and nobody was really using this bounce/reflection feature anyway...
        if (numplanes == 1 && player->GetMoveType() == MOVETYPE_WALK && player->GetGroundEntity() == nullptr)
        {
            // Is this a floor/slope that the player can walk on?
            if (planes[0][2] > 0.7)
            {
                // We only reflect if our velocity isn't going into the slope we're jumping on
                if (planes[0][2] < 1.0) // and if it's not the horizontal floor
                {
                    Vector planeScaled;
                    VectorMultiply(mv->m_vecVelocity, planes[0], planeScaled); // First get our plane normal up to scale with our velocity
                    if (DotProduct(mv->m_vecVelocity, planeScaled) > 0.0f) // If our velocity is NOT going into the slope
                    {
                        m_flReflectNormal = planes[0][2]; // Determines if we should late reflect in CategorizePosition
                        // (we only boost the player if the game doesn't give them one)
                    }
                }

                ClipVelocity(original_velocity, planes[0], new_velocity, 1);
                VectorCopy(new_velocity, original_velocity);
            }
            else // either the player is surfing or slammed into a wall
            {
                ClipVelocity(original_velocity, planes[0], new_velocity,
                             1.0 + sv_bounce.GetFloat() * (1 - player->m_surfaceFriction));
            }

            VectorCopy(new_velocity, mv->m_vecVelocity);
            VectorCopy(new_velocity, original_velocity);
        }
        else
        {
            for (i = 0; i < numplanes; i++)
            {
                ClipVelocity(original_velocity, planes[i], mv->m_vecVelocity, 1);
                for (j = 0; j < numplanes; j++)
                    if (j != i)
                    {
                        // Are we now moving against this plane?
                        if (mv->m_vecVelocity.Dot(planes[j]) < 0)
                            break; // not ok
                    }
                if (j == numplanes) // Didn't have to clip, so we're ok
                    break;
            }

            // Did we go all the way through plane set
            if (i != numplanes)
            { // go along this plane
              // pmove.velocity is set in clipping call, no need to set again.
                ;
            }
            else
            { // go along the crease
                if (numplanes != 2)
                {
                    VectorCopy(vec3_origin, mv->m_vecVelocity);
                    break;
                }
                CrossProduct(planes[0], planes[1], dir);
                dir.NormalizeInPlace();
                d = dir.Dot(mv->m_vecVelocity);
                VectorScale(dir, d, mv->m_vecVelocity);
            }

            //
            // if original velocity is against the original velocity, stop dead
            // to avoid tiny occilations in sloping corners
            //
            d = mv->m_vecVelocity.Dot(primal_velocity);
            if (d <= 0)
            {
                // Con_DPrintf("Back\n");
                if (!sv_ramp_fix.GetBool())
                    VectorCopy(vec3_origin, mv->m_vecVelocity); // RAMPBUG FIX #2
                break;
            }
        }
    }

    if (allFraction == 0.0f)
    {
        if (!sv_ramp_fix.GetBool())
            VectorCopy(vec3_origin, mv->m_vecVelocity); // RAMPBUG FIX #1
    }

    // Check if they slammed into a wall
    float fSlamVol = 0.0f;

    float fLateralStoppingAmount = primal_velocity.Length2D() - mv->m_vecVelocity.Length2D();
    if (fLateralStoppingAmount > PLAYER_MAX_SAFE_FALL_SPEED * 2.0f)
    {
        fSlamVol = 1.0f;
    }
    else if (fLateralStoppingAmount > PLAYER_MAX_SAFE_FALL_SPEED)
    {
        fSlamVol = 0.85f;
    }

    PlayerRoughLandingEffects(fSlamVol);

    return blocked;
}

// This was the virtual void, overriding it for snow friction
void CMomentumGameMovement::SetGroundEntity(trace_t *pm)
{
    // We check jump button because the player might want jumping while sliding
    // And it's more fun like this
    if ((m_pPlayer->m_SrvData.m_fSliding & FL_SLIDE) && !(mv->m_nButtons & IN_JUMP))
    {
        pm = nullptr;
    }

    // CMomentumPlayer *player = GetMomentumPlayer();

    CBaseEntity *newGround = pm ? pm->m_pEnt : nullptr;

    CBaseEntity *oldGround = player->GetGroundEntity();
    Vector vecBaseVelocity = player->GetBaseVelocity();

    if (!oldGround && newGround)
    {
        // Subtract ground velocity at instant we hit ground jumping
        vecBaseVelocity -= newGround->GetAbsVelocity();
        vecBaseVelocity.z = newGround->GetAbsVelocity().z;
    }
    else if (oldGround && !newGround)
    {
        // Add in ground velocity at instant we started jumping
        vecBaseVelocity += oldGround->GetAbsVelocity();
        vecBaseVelocity.z = oldGround->GetAbsVelocity().z;
    }

    player->SetBaseVelocity(vecBaseVelocity);
    player->SetGroundEntity(newGround);

    // If we are on something...

    if (newGround)
    {
        CategorizeGroundSurface(*pm); // Snow friction override

        // Then we are not in water jump sequence
        player->m_flWaterJumpTime = 0.0f;

        // Standing on an entity other than the world, so signal that we are touching something.
        if (!pm->DidHitWorld())
        {
            MoveHelper()->AddToTouched(*pm, mv->m_vecVelocity);
        }

        mv->m_vecVelocity.z = 0.0f;
    }
}

void CMomentumGameMovement::CategorizeGroundSurface(trace_t &pm)
{
    IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();

    player->m_surfaceProps = pm.surface.surfaceProps;
    player->m_pSurfaceData = physprops->GetSurfaceData(player->m_surfaceProps);
    physprops->GetPhysicsProperties(player->m_surfaceProps, nullptr, nullptr, &player->m_surfaceFriction, nullptr);

    // HACKHACK: Scale this to fudge the relationship between vphysics friction values and player friction values.
    // A value of 0.8f feels pretty normal for vphysics, whereas 1.0f is normal for players.
    // This scaling trivially makes them equivalent.  REVISIT if this affects low friction surfaces too much.
    player->m_surfaceFriction *= 1.25f;
    if (player->m_surfaceFriction > 1.0f ||
        (player->m_pSurfaceData->game.material == 'D' && player->m_pSurfaceData->physics.friction == 0.35f))
        player->m_surfaceFriction = 1.0f; // fix for snow friction

    player->m_chTextureType = player->m_pSurfaceData->game.material;
}

void CMomentumGameMovement::CheckParameters(void)
{
    QAngle v_angle;

    // shift-walking useful for some maps with tight jumps
    if (mv->m_nButtons & IN_WALK)
    {
        mv->m_flClientMaxSpeed = CS_WALK_SPEED;
    }

    BaseClass::CheckParameters();
}

void CMomentumGameMovement::ReduceTimers(void)
{
    float frame_msec = 1000.0f * gpGlobals->frametime;

    if (m_pPlayer->m_flStamina > 0)
    {
        m_pPlayer->m_flStamina -= frame_msec;

        if (m_pPlayer->m_flStamina < 0)
        {
            m_pPlayer->m_flStamina = 0;
        }
    }

    BaseClass::ReduceTimers();
}

// We're overriding this here so the game doesn't play any fall damage noises
void CMomentumGameMovement::CheckFalling(void)
{
    // this function really deals with landing, not falling, so early out otherwise
    CBaseEntity *pGroundEntity = player->GetGroundEntity();
    if (!pGroundEntity || player->m_Local.m_flFallVelocity <= 0.0f)
        return;

    if (!IsDead() && player->m_Local.m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHOLD)
    {
        bool bAlive = true;
        float fvol = 0.5f;

        if (player->GetWaterLevel() <= 0.0f)
        {
            // Scale it down if we landed on something that's floating...
            if (pGroundEntity->IsFloating())
            {
                player->m_Local.m_flFallVelocity -= PLAYER_LAND_ON_FLOATING_OBJECT;
            }

            //
            // They hit the ground.
            //
            if (pGroundEntity->GetAbsVelocity().z < 0.0f)
            {
                // Player landed on a descending object. Subtract the velocity of the ground entity.
                player->m_Local.m_flFallVelocity += pGroundEntity->GetAbsVelocity().z;
                player->m_Local.m_flFallVelocity = MAX(0.1f, player->m_Local.m_flFallVelocity);
            }

            if (player->m_Local.m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
            {
                //
                // If they hit the ground going this fast they may take damage (and die).
                //
                // NOTE: We override this here since this way we can play the noise without having to go to the
                // MoveHelper
                // MOM_TODO: Revisit if we want custom fall noises.
                bAlive = true; // MoveHelper()->PlayerFallingDamage();
                fvol = 1.0f;
            }
            else if (player->m_Local.m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2)
            {
                fvol = 0.85f;
            }
            else if (player->m_Local.m_flFallVelocity < PLAYER_MIN_BOUNCE_SPEED)
            {
                fvol = 0.0f;
            }
        }

        // MOM_TODO: This plays a step sound, revisit if we want to override
        PlayerRoughLandingEffects(fvol);

        if (bAlive)
        {
            MoveHelper()->PlayerSetAnimation(PLAYER_WALK);
        }
    }

    // let any subclasses know that the player has landed and how hard
    OnLand(player->m_Local.m_flFallVelocity);

    //
    // Clear the fall velocity so the impact doesn't happen again.
    //
    player->m_Local.m_flFallVelocity = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : in - 
//			normal - 
//			out - 
//			overbounce - 
// Output : int
//-----------------------------------------------------------------------------
int CMomentumGameMovement::ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce)
{
    float backoff;
    float change;
    float angle;
    int i, blocked;

    angle = normal[2];

    blocked = 0x00;      // Assume unblocked.
    if (angle > 0)       // If the plane that is blocking us has a positive z component, then assume it's a floor.
        blocked |= 0x01; //
    if (!angle)          // If the plane has no Z, it is vertical (wall/step)
        blocked |= 0x02; //

    // Determine how far along plane to slide based on incoming direction.
    backoff = DotProduct(in, normal) * overbounce;

    float velocity = 0.0f;
    for (i = 0; i < 3; i++)
    {
        change = normal[i] * backoff;
        velocity = in[i] - change;
        out[i] = velocity;
    }

    // iterate once to make sure we aren't still moving through the plane
    float adjust = DotProduct(out, normal);
    if (adjust < 0.0f)
    {
        out -= (normal * adjust);
        // Msg( "Adjustment = %lf\n", adjust );
    }

    // Check if we loose speed while going on a slope in front of us.
    Vector dif = mv->m_vecVelocity - out;
    if (dif.Length2D() > 0.0f && (angle > 0.7f) && (velocity > 0.0f))
    {
        out.x = mv->m_vecVelocity.x;
        out.y = mv->m_vecVelocity.y;
        // Avoid being stuck into the slope.. Or velocity reset incoming!
        // (Could be better by being more close to the slope, but for player it seems to be close enough)
        // @Gocnak: Technically the "adjust" code above does this, but to each axis, with a much higher value.
        // Tickrate will work, but keep in mind tickrates can get pretty big, though realistically this will be 0.015 or 0.01
        mv->m_vecAbsOrigin.z += abs(dif.z) * gpGlobals->interval_per_tick;
        DevMsg(2, "ClipVelocity: Fixed speed.\n");
    }

    // Return blocking flags.
    return blocked;
}

// Expose our interface.
static CMomentumGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = static_cast<IGameMovement *>(&g_GameMovement);

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CMomentumGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT, g_GameMovement);
