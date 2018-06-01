#include "cbase.h"

#include <rumble_shared.h>
#include <stdarg.h>
#include "IMovementListener.h"
#include "in_buttons.h"
#include "mom_gamemovement.h"
#include "movevars_shared.h"

#include "tier0/memdbgon.h"

// remove this eventually
ConVar sv_slope_fix("sv_slope_fix", "1");
ConVar sv_ramp_fix("sv_ramp_fix", "1");
ConVar sv_ramp_bumpcount("sv_ramp_bumpcount", "8", 0, "Helps with fixing surf/ramp bugs", true, 4, true, 16);
ConVar sv_ramp_retrace_length("sv_ramp_retrace_length", "0.1", 0, "Amount of units used in offset for retraces", true, 0.0f, true, 5.f);
ConVar sv_jump_z_offset("sv_jump_z_offset", "1.5", 0, "Amount of units in axis z to offset every time a player jumps", true, 0.0f, true, 5.f);
ConVar sv_considered_on_ground("sv_considered_on_ground", "2.0", 0, "Amount of units you have to be above the ground to be considered on ground", true, 0.0f, true, 5.f);

ConVar sv_ladder_dampen("sv_ladder_dampen", "0.2", FCVAR_REPLICATED, "Amount to dampen perpendicular movement on a ladder", true, 0.0f, true, 1.0f);
ConVar sv_ladder_angle("sv_ladder_angle", "-0.707", FCVAR_REPLICATED, "Cos of angle of incidence to ladder perpendicular for applying ladder_dampen", true, -1.0f, true, 1.0f);

#ifndef CLIENT_DLL
#include "env_player_surface_trigger.h"
static ConVar dispcoll_drawplane("dispcoll_drawplane", "0");
static MAKE_TOGGLE_CONVAR(mom_punchangle_enable, "0", FCVAR_ARCHIVE | FCVAR_REPLICATED,
                          "Toggle landing punchangle. 0 = OFF, 1 = ON\n");
#endif

CMomentumGameMovement::CMomentumGameMovement()
    : m_pPlayer(nullptr), mom_gamemode("mom_gamemode")
{
}

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
            player->m_Local.m_vecPunchAngle.Set(ROLL, player->m_Local.m_flFallVelocity * 0.013 *
                                                          mom_punchangle_enable.GetInt());

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

bool CMomentumGameMovement::IsValidMovementTrace(trace_t &tr)
{
    trace_t stuck;

    // Apparently we can be stuck with pm.allsolid without having valid plane info ok..
    if (tr.allsolid || tr.startsolid)
        return false;

    // We didnt move anywhere, bad trace..
    if (CloseEnough(tr.fraction, 0.0f, FLT_EPSILON))
        return false;

    // Maybe we dont need this one
    if (CloseEnough(tr.fraction, 0.0f, FLT_EPSILON) && CloseEnough(tr.plane.normal, Vector(0.0f, 0.0f, 0.0f), FLT_EPSILON))
        return false;

    // Is the plane deformed or some stupid shit?
    if (fabs(tr.plane.normal.x) > 1.0f || fabs(tr.plane.normal.y) > 1.0f || fabs(tr.plane.normal.z) > 1.0f)
        return false;

    TracePlayerBBox(tr.endpos, tr.endpos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, stuck);
    if (stuck.startsolid || !CloseEnough(stuck.fraction, 1.0f, FLT_EPSILON))
        return false;

    return true;
}

float CMomentumGameMovement::ClimbSpeed(void) const
{
    return (mv->m_nButtons & IN_DUCK ? BaseClass::ClimbSpeed() * DUCK_SPEED_MULTIPLIER : BaseClass::ClimbSpeed());
}

void CMomentumGameMovement::WalkMove()
{
    int i;

    Vector wishvel;
    float spd;
    float fmove, smove;
    Vector wishdir;
    float wishspeed;

    Vector dest;
    trace_t pm;
    Vector forward, right, up;

    if (mom_gamemode.GetInt() == MOMGM_SCROLL)
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

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles

    CHandle< CBaseEntity > oldground;
    oldground = player->GetGroundEntity();

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;

    // Zero out z components of movement vectors
    if (forward[2] != 0)
    {
        forward[2] = 0;
        VectorNormalize(forward);
    }

    if (right[2] != 0)
    {
        right[2] = 0;
        VectorNormalize(right);
    }

    for (i = 0; i<2; i++)       // Determine x and y parts of velocity
        wishvel[i] = forward[i] * fmove + right[i] * smove;

    wishvel[2] = 0.0f;             // Zero out z part of velocity

    VectorCopy(wishvel, wishdir);   // Determine maginitude of speed of move
    wishspeed = VectorNormalize(wishdir);

    //
    // Clamp to server defined max speed
    //
    if ((wishspeed != 0.0f) && (wishspeed > mv->m_flMaxSpeed))
    {
        VectorScale(wishvel, mv->m_flMaxSpeed / wishspeed, wishvel);
        wishspeed = mv->m_flMaxSpeed;
    }

    // Set pmove velocity
    // Commented these 2 out, so we wont be 2 units above the ground
    // This only happens when the player jumps straight up and lands while standing still
    //mv->m_vecVelocity[2] = 0;
    Accelerate(wishdir, wishspeed, sv_accelerate.GetFloat());
    //mv->m_vecVelocity[2] = 0;

    // Add in any base velocity to the current velocity.
    VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

    spd = VectorLength(mv->m_vecVelocity);

    if (CloseEnough(spd, 0.0f))
    {
        mv->m_vecVelocity.Init();
        // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
        VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
        return;
    }

    // first try just moving to the destination  
    dest[0] = mv->GetAbsOrigin()[0] + mv->m_vecVelocity[0] * gpGlobals->frametime;
    dest[1] = mv->GetAbsOrigin()[1] + mv->m_vecVelocity[1] * gpGlobals->frametime;
    dest[2] = mv->GetAbsOrigin()[2] + mv->m_vecVelocity[2] * gpGlobals->frametime;

    // first try moving directly to the next spot
    TracePlayerBBox(mv->GetAbsOrigin(), dest, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

    // If we made it all the way, then copy trace end as new player position.
    mv->m_outWishVel += wishdir * wishspeed;

    if (pm.fraction == 1)
    {
        mv->SetAbsOrigin(pm.endpos);
        // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
        VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

        StayOnGround();
        return;
    }

    // Don't walk up stairs if not on ground.
    if (oldground == nullptr && player->GetWaterLevel() == 0)
    {
        // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
        VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
        return;
    }

    // If we are jumping out of water, don't do anything more.
    if (player->m_flWaterJumpTime)
    {
        // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
        VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
        return;
    }

    StepMove(dest, pm);

    // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
    VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

    StayOnGround();
}

bool CMomentumGameMovement::LadderMove(void)
{
    trace_t pm;
    bool onFloor;
    Vector floor;
    Vector wishdir;
    Vector end;

    if (player->GetMoveType() == MOVETYPE_NOCLIP)
        return false;

    if (!GameHasLadders())
    {
        return false;
    }

    if (m_pPlayer->GetGrabbableLadderTime() > 0.0f)
    {
        m_pPlayer->SetGrabbableLadderTime(m_pPlayer->GetGrabbableLadderTime() - gpGlobals->frametime);
    }

    // If I'm already moving on a ladder, use the previous ladder direction
    if (player->GetMoveType() == MOVETYPE_LADDER)
    {
        wishdir = -player->m_vecLadderNormal;
    }
    else
    {
        // otherwise, use the direction player is attempting to move
        if (mv->m_flForwardMove || mv->m_flSideMove)
        {
            for (int i = 0; i<3; i++)       // Determine x and y parts of velocity
                wishdir[i] = m_vecForward[i] * mv->m_flForwardMove + m_vecRight[i] * mv->m_flSideMove;

            VectorNormalize(wishdir);
        }
        else
        {
            // Player is not attempting to move, no ladder behavior
            return false;
        }
    }

    // wishdir points toward the ladder if any exists

    if (m_pPlayer->GetGrabbableLadderTime() > 0.0f && m_bCheckForGrabbableLadder)
    {
        Vector temp = mv->m_vecVelocity * 2.0f;

        temp.z = -temp.z;

        VectorNormalize(temp);
        VectorMA(mv->GetAbsOrigin(), 10.0f, -temp, end);
    }
    else
    {
        VectorMA(mv->GetAbsOrigin(), LadderDistance(), wishdir, end);
    }

    TracePlayerBBox(mv->GetAbsOrigin(), end, LadderMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

    // no ladder in that direction, return
    if (pm.fraction == 1.0f || pm.plane.normal.z == 1.0f || !OnLadder(pm))
    {
        return false;
    }

    if (m_pPlayer->GetGrabbableLadderTime() > 0.0f && m_bCheckForGrabbableLadder)
    {
        mv->m_vecVelocity.Init();
        mv->SetAbsOrigin(pm.endpos);
        m_pPlayer->SetGrabbableLadderTime(0.1f);
    }

    player->SetMoveType(MOVETYPE_LADDER);
    player->SetMoveCollide(MOVECOLLIDE_DEFAULT);

    // On ladder, convert movement to be relative to the ladder
    player->SetLadderNormal(pm.plane.normal);
    
    VectorCopy(mv->GetAbsOrigin(), floor);
    floor[2] += GetPlayerMins()[2] - 1;

    if (enginetrace->GetPointContents(floor) == CONTENTS_SOLID || player->GetGroundEntity() != nullptr)
    {
        onFloor = true;
    }
    else
    {
        onFloor = false;
    }

    player->SetGravity(1.0f); //Should be always set on 1.0..

    float climbSpeed = ClimbSpeed();

    float forwardSpeed = 0, rightSpeed = 0;
    if (mv->m_nButtons & IN_BACK)
        forwardSpeed -= climbSpeed;

    if (mv->m_nButtons & IN_FORWARD)
        forwardSpeed += climbSpeed;

    if (mv->m_nButtons & IN_MOVELEFT)
        rightSpeed -= climbSpeed;

    if (mv->m_nButtons & IN_MOVERIGHT)
        rightSpeed += climbSpeed;

    if (mv->m_nButtons & IN_JUMP)
    {
        player->SetMoveType(MOVETYPE_WALK);
        player->SetMoveCollide(MOVECOLLIDE_DEFAULT);

        VectorScale(pm.plane.normal, 270, mv->m_vecVelocity);
    }
    else
    {
        if (forwardSpeed != 0 || rightSpeed != 0)
        {
            Vector velocity, perp, cross, lateral, tmp;

            //ALERT(at_console, "pev %.2f %.2f %.2f - ",
            //    pev->velocity.x, pev->velocity.y, pev->velocity.z);
            // Calculate player's intended velocity
            //Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
            VectorScale(m_vecForward, forwardSpeed, velocity);
            VectorMA(velocity, rightSpeed, m_vecRight, velocity);

            // Perpendicular in the ladder plane
            VectorCopy(vec3_origin, tmp);
            tmp[2] = 1;
            CrossProduct(tmp, pm.plane.normal, perp);
            VectorNormalize(perp);

            // decompose velocity into ladder plane
            float normal = DotProduct(velocity, pm.plane.normal);

            // This is the velocity into the face of the ladder
            VectorScale(pm.plane.normal, normal, cross);

            // This is the player's additional velocity
            VectorSubtract(velocity, cross, lateral);

            // This turns the velocity into the face of the ladder into velocity that
            // is roughly vertically perpendicular to the face of the ladder.
            // NOTE: It IS possible to face up and move down or face down and move up
            // because the velocity is a sum of the directional velocity and the converted
            // velocity through the face of the ladder -- by design.
            CrossProduct(pm.plane.normal, perp, tmp);

            //=============================================================================
            // HPE_BEGIN
            // [sbodenbender] make ladders easier to climb in cstrike
            //=============================================================================
            // break lateral into direction along tmp (up the ladder) and direction along perp (perpendicular to ladder)
            float tmpDist = DotProduct(tmp, lateral);
            float perpDist = DotProduct(perp, lateral);

            Vector angleVec = perp * perpDist;
            angleVec += cross;
            // angleVec is our desired movement in the ladder normal/perpendicular plane
            VectorNormalize(angleVec);
            float angleDot = DotProduct(angleVec, pm.plane.normal);
            // angleDot is our angle of incidence to the laddernormal in the ladder normal/perpendicular plane

            if (angleDot < sv_ladder_angle.GetFloat())
                lateral = (tmp * tmpDist) + (perp * sv_ladder_dampen.GetFloat() * perpDist);
            //=============================================================================
            // HPE_END
            //=============================================================================

            VectorMA(lateral, -normal, tmp, mv->m_vecVelocity);

            if (onFloor && normal > 0)    // On ground moving away from the ladder
            {
                VectorMA(mv->m_vecVelocity, MAX_CLIMB_SPEED, pm.plane.normal, mv->m_vecVelocity);
            }
            //pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
        }
        else
        {
            mv->m_vecVelocity.Init();
        }
    }

    return true;
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

    if (player->GetGroundEntity() != nullptr || m_pPlayer->m_SrvData.m_SlideData.IsEnabled())
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

void CMomentumGameMovement::Friction(void)
{
    float    speed, newspeed, control;
    float    friction;
    float    drop;

    // Friction should be affected by z velocity
    Vector    velocity = mv->m_vecVelocity;
    velocity.z = 0.0f;

    // If we are in water jump cycle, don't apply friction
    if (player->m_flWaterJumpTime)
        return;

    // Calculate speed
    speed = VectorLength(velocity);

    // If too slow, return
    if (speed < 0.1f)
    {
        return;
    }

    drop = 0;

    // apply ground friction
    if (player->GetGroundEntity() != NULL)  // On an entity that is the ground
    {
        friction = sv_friction.GetFloat() * player->m_surfaceFriction;

        // Bleed off some speed, but if we have less than the bleed
        //  threshold, bleed the threshold amount.

        if (IsX360())
        {
            if (player->m_Local.m_bDucked)
            {
                control = (speed < sv_stopspeed.GetFloat()) ? sv_stopspeed.GetFloat() : speed;
            }
            else
            {
#if defined ( TF_DLL ) || defined ( TF_CLIENT_DLL )
                control = (speed < sv_stopspeed.GetFloat()) ? sv_stopspeed.GetFloat() : speed;
#else
                control = (speed < sv_stopspeed.GetFloat()) ? (sv_stopspeed.GetFloat() * 2.0f) : speed;
#endif
            }
        }
        else
        {
            control = (speed < sv_stopspeed.GetFloat()) ? sv_stopspeed.GetFloat() : speed;
        }

        // Add the amount to the drop amount.
        drop += control*friction*gpGlobals->frametime;
    }

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0)
        newspeed = 0;

    if (newspeed != speed)
    {
        // Determine proportion of old speed we are using.
        newspeed /= speed;
        // Adjust velocity according to proportion.
        VectorScale(velocity, newspeed, velocity);
    }

    mv->m_outWishVel -= (1.f - newspeed) * velocity;

    mv->m_vecVelocity.x = velocity.x;
    mv->m_vecVelocity.y = velocity.y;
}

void CMomentumGameMovement::Duck(void)
{
    int buttonsChanged = (mv->m_nOldButtons ^ mv->m_nButtons); // These buttons have changed this frame
    int buttonsPressed = buttonsChanged & mv->m_nButtons;      // The changed ones still down are "pressed"
    int buttonsReleased =
        buttonsChanged & mv->m_nOldButtons; // The changed ones which were previously down are "released"

    // Check to see if we are in the air.
    bool bInAir = player->GetGroundEntity() == nullptr && player->GetMoveType() != MOVETYPE_LADDER;

    bool bIsSliding = m_pPlayer->m_SrvData.m_SlideData.IsEnabled();

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
                if ((duckseconds > TIME_TO_DUCK) || !bIsSliding && player->GetGroundEntity() == nullptr ||
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
            if (player->m_Local.m_bAllowAutoMovement || !bIsSliding && player->GetGroundEntity() == nullptr)
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
                        if ((duckseconds > TIME_TO_UNDUCK) || !bIsSliding && player->GetGroundEntity() == nullptr)
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

    if (player->GetGroundEntity() != nullptr || m_pPlayer->m_SrvData.m_SlideData.IsEnabled())
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

        Vector vHullMin = GetPlayerMins((player->m_Local.m_bDucked && !player->m_Local.m_bDucking));
        Vector vHullMax = GetPlayerMaxs((player->m_Local.m_bDucked && !player->m_Local.m_bDucking));
        vHullMax.z = (player->m_Local.m_bDucked && !player->m_Local.m_bDucking) ? VEC_DUCK_VIEW.z : VEC_VIEW.z;

        Vector start = mv->GetAbsOrigin();

        Vector end = start;
        end.z += eyeClearance;
        
        trace_t trace;
        Ray_t ray;
        ray.Init(start, end, vHullMin, vHullMax);
        UTIL_TraceRay(ray, PlayerSolidMask(), mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

        if (trace.fraction < 1.0f)
        {
            float est = vHullMax.z + trace.endpos.z - mv->GetAbsOrigin().z - eyeClearance;

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
    trace_t pm;

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

    // Prevent jump if needed
    const bool bPlayerBhopBlocked = m_pPlayer->m_SrvData.m_bPreventPlayerBhop &&
                                    gpGlobals->tickcount - m_pPlayer->m_SrvData.m_iLandTick < BHOP_DELAY_TIME;
    if (bPlayerBhopBlocked)
    {
        m_pPlayer->m_afButtonDisabled |= IN_BULLRUSH; // For the HUD
        return false;
    }

    if (m_pPlayer->m_afButtonDisabled & IN_BULLRUSH)
        m_pPlayer->m_afButtonDisabled &= ~IN_BULLRUSH; // For the HUD

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
    
    mv->m_vecVelocity[2] += flGroundFactor * sqrt(2.f * GetCurrentGravity() * 57.0f); // 2 * gravity * height

    // stamina stuff (scroll/kz gamemode only)
    if (mom_gamemode.GetInt() == MOMGM_SCROLL)
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

    // First do a trace all the way down to the ground
    TracePlayerBBox(mv->GetAbsOrigin(), mv->GetAbsOrigin() + Vector(0.0f, 0.0f, -(sv_considered_on_ground.GetFloat() + 0.1f)), PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

    // Did we hit ground (ground is at max 2 units away so fraction cant be 1.0f)
    if (pm.fraction != 1.0f &&
        !pm.startsolid &&
        !pm.allsolid)
    {
        // Now we find 1.5f above ground
        TracePlayerBBox(pm.endpos, pm.endpos + Vector(0.0f, 0.0f, sv_jump_z_offset.GetFloat()), PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

        if (pm.fraction == 1.0f &&
            !pm.startsolid &&
            !pm.allsolid)
        {
            // Everything is p100
            mv->SetAbsOrigin(pm.endpos);
        }
    }

    // Flag that we jumped.
    mv->m_nOldButtons |= IN_JUMP; // don't jump again until released
    // Fire that we jumped
    FIRE_GAMEMOVEMENT_EVENT(OnPlayerJump);
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

    float flOffset = sv_considered_on_ground.GetFloat();

    const Vector bumpOrigin = mv->GetAbsOrigin();

    point[0] = bumpOrigin[0];
    point[1] = bumpOrigin[1];
    point[2] = bumpOrigin[2] - flOffset;

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
        }
        else
        {
            if (sv_slope_fix.GetBool())
            {
                // Make sure we apply clip velocity on slopes/surfs before setting the ground entity and nulling out velocity.z
                if (player->GetGroundEntity() == nullptr)
                {
                    if (pm.plane.normal.z >= 0.7f && pm.plane.normal.z < 1.0f)
                    {
                        ClipVelocity(mv->m_vecVelocity, pm.plane.normal, mv->m_vecVelocity, 1.0f);
                    }
                    else if (pm.plane.normal.z < 0.7f)
                    {
                        ClipVelocity(mv->m_vecVelocity, pm.plane.normal, mv->m_vecVelocity,
                            1.0 + sv_bounce.GetFloat() * (1 - player->m_surfaceFriction));
                    }
                }
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

    if (m_pPlayer->m_SrvData.m_SlideData.IsEnabled() && !m_pPlayer->m_SrvData.m_SlideData.IsGravityEnabled())
        return;

    // Get the correct velocity for the end of the dt
    mv->m_vecVelocity[2] -= (player->GetGravity() * GetCurrentGravity() * 0.5 * gpGlobals->frametime);

    CheckVelocity();
}

void CMomentumGameMovement::StartGravity(void)
{
    if (m_pPlayer->m_SrvData.m_SlideData.IsEnabled() && !m_pPlayer->m_SrvData.m_SlideData.IsGravityEnabled())
        return;

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
    bool bIsSliding = m_pPlayer->m_SrvData.m_SlideData.IsEnabled();

    if (!CheckWater())
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
    if ((player->GetWaterLevel() >= WL_Waist) && !bIsSliding)
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
            Friction();
        }

        // Make sure velocity is valid.
        CheckVelocity();
        
        if (player->GetGroundEntity() != nullptr)
        {
            WalkMove();

            CategorizePosition();
            m_bCheckForGrabbableLadder = m_pPlayer->GetGroundEntity() == nullptr;
            if (m_bCheckForGrabbableLadder)
            {
                // Next 0.1 seconds you can grab the ladder
                m_pPlayer->SetGrabbableLadderTime(0.1f);
                LadderMove();
                m_bCheckForGrabbableLadder = false;
            }
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
        if (!CheckWater())
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
        // MOM_TODO: The player can't jump with it.
        if (bIsSliding && m_pPlayer->m_SrvData.m_SlideData.IsStuckGround())
        {
            StuckGround();
        }
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

    {
        CTraceFilterSimple tracefilter(player, COLLISION_GROUP_NONE);
        enginetrace->TraceRay(ray, MASK_PLAYERSOLID, &tracefilter, &tr);
    }

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
    
    TryPlayerMove();

    // Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or
    // maybe another monster?)
    VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

    if (m_pPlayer->GetGrabbableLadderTime() > 0.0f)
    {
        m_bCheckForGrabbableLadder = true;
        LadderMove();
        m_bCheckForGrabbableLadder = false;
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
    Vector fixed_origin;
    Vector valid_plane;
    int i, j, h, fixed_origins;
    trace_t pm;
    Vector end;
    float time_left, allFraction;
    int blocked;
    bool stuck_on_ramp;
    bool has_valid_plane;
    numbumps = sv_ramp_bumpcount.GetInt();

    blocked = 0;   // Assume not blocked
    numplanes = 0; //  and not sliding along any planes
    fixed_origins = 0; // Assume no fixed origins yet

    stuck_on_ramp = false; // lets assume client isnt stuck already
    has_valid_plane = false; // no plane info gathered yet

    VectorCopy(mv->m_vecVelocity, original_velocity); // Store original velocity
    VectorCopy(mv->m_vecVelocity, primal_velocity);
    VectorCopy(mv->GetAbsOrigin(), fixed_origin);

    allFraction = 0;
    time_left = gpGlobals->frametime; // Total time for this movement operation.

    new_velocity.Init();
    valid_plane.Init();

    for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
    {
        if (mv->m_vecVelocity.Length() == 0.0)
            break;

        if (stuck_on_ramp && sv_ramp_fix.GetBool())
        {
            if (!has_valid_plane)
            {
                if (!CloseEnough(pm.plane.normal, Vector(0.0f, 0.0f, 0.0f), FLT_EPSILON) && valid_plane != pm.plane.normal)
                {
                    valid_plane = pm.plane.normal;
                    has_valid_plane = true;
                }
                else
                {
                    for (i = numplanes; i-- > 0; )
                    {
                        if (!CloseEnough(planes[i], Vector(0.0f, 0.0f, 0.0f), FLT_EPSILON) &&
                            fabs(planes[i].x) <= 1.0f && fabs(planes[i].y) <= 1.0f && fabs(planes[i].z) <= 1.0f &&
                            valid_plane != planes[i])
                        {
                            valid_plane = planes[i];
                            has_valid_plane = true;
                            break;
                        }
                    }
                }
            }

            if (valid_plane.z >= 0.7f && valid_plane.z <= 1.0f)
            {
                ClipVelocity(mv->m_vecVelocity, valid_plane, mv->m_vecVelocity, 1);
                VectorCopy(mv->m_vecVelocity, original_velocity);
            }
            else if (has_valid_plane)
            {
                ClipVelocity(mv->m_vecVelocity, valid_plane, mv->m_vecVelocity,
                    1.0 + sv_bounce.GetFloat() * (1 - player->m_surfaceFriction));
                VectorCopy(mv->m_vecVelocity, original_velocity);
            }
            else // We were actually going to be stuck, lets try and find a valid plane..
            {
                // this way we know fixed_origin isnt going to be stuck
                float offsets[] = { -(sv_ramp_retrace_length.GetFloat() / sv_ramp_bumpcount.GetFloat()) * (float)(fixed_origins + 1), 0.0f, (sv_ramp_retrace_length.GetFloat() / sv_ramp_bumpcount.GetFloat()) * (float)(fixed_origins + 1) };
                int valid_planes = 0;

                // we have 0 plane info, so lets increase our bbox and search in all 27 directions to get a valid plane!
                for (i = 0; i < 3; i++)
                {
                    for (j = 0; j < 3; j++)
                    {
                        for (h = 0; h < 3; h++)
                        {
                            Vector offset = { offsets[i], offsets[j], offsets[h] };
                            Vector offset_mins = offset / 2.0f;
                            Vector offset_maxs = offset / 2.0f;

                            if (offset.x > 0.0f)
                                offset_mins.x /= 2.0f;
                            if (offset.y > 0.0f)
                                offset_mins.y /= 2.0f;
                            if (offset.z > 0.0f)
                                offset_mins.z /= 2.0f;

                            if (offset.x < 0.0f)
                                offset_maxs.x /= 2.0f;
                            if (offset.y < 0.0f)
                                offset_maxs.y /= 2.0f;
                            if (offset.z < 0.0f)
                                offset_maxs.z /= 2.0f;

                            Ray_t ray;
                            ray.Init(fixed_origin + offset, end - offset, GetPlayerMins() - offset_mins, GetPlayerMaxs() + offset_maxs);
                            UTIL_TraceRay(ray, PlayerSolidMask(), mv->m_nPlayerHandle.Get(), COLLISION_GROUP_PLAYER_MOVEMENT, &pm);

                            // Only use non deformed planes and planes with values where the start point is not from a solid
                            if (fabs(pm.plane.normal.x) <= 1.0f && fabs(pm.plane.normal.y) <= 1.0f && fabs(pm.plane.normal.z) <= 1.0f && !CloseEnough(pm.plane.normal, Vector(0.0f, 0.0f, 0.0f), FLT_EPSILON) && !pm.startsolid)
                            {
                                valid_planes++;
                                valid_plane += pm.plane.normal;
                            }
                        }
                    }
                }

                if (valid_planes && !CloseEnough(valid_plane, Vector(0.0f, 0.0f, 0.0f), FLT_EPSILON))
                {
                    has_valid_plane = true;
                    valid_plane /= (float)valid_planes;
                    continue;
                }
            }

            if (has_valid_plane)
            {
                VectorMA(fixed_origin, sv_ramp_retrace_length.GetFloat(), valid_plane, fixed_origin);
                fixed_origins++;
            }
            else
            {
                stuck_on_ramp = false;
                continue;
            }
        }

        // Assume we can move all the way from the current origin to the
        //  end point.

        VectorMA(fixed_origin, time_left, mv->m_vecVelocity, end);

        // See if we can make it from origin to end point.
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
            if (stuck_on_ramp && has_valid_plane && sv_ramp_fix.GetBool())
            {
                TracePlayerBBox(fixed_origin, end, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);
                pm.plane.normal = valid_plane;
            }
            else
            {
                TracePlayerBBox(mv->GetAbsOrigin(), end, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);
            }
        }

        if (bumpcount && sv_ramp_fix.GetBool() && player->GetGroundEntity() == nullptr && !IsValidMovementTrace(pm))
        {
            has_valid_plane = false;
            stuck_on_ramp = true;
            continue;
        }

        // If we started in a solid object, or we were in solid space
        //  the whole way, zero out our velocity and return that we
        //  are blocked by floor and wall.
        if (pm.allsolid && !sv_ramp_fix.GetBool())
        {
            // entity is trapped in another solid
            VectorCopy(vec3_origin, mv->m_vecVelocity);
            return 4;
        }

        // If we moved some portion of the total distance, then
        //  copy the end position into the pmove.origin and
        //  zero the plane counter.
        if (pm.fraction > 0)
        {
            if ((!bumpcount || player->GetGroundEntity() != nullptr || !sv_ramp_fix.GetBool()) && numbumps > 0 && pm.fraction == 1)
            {
                // There's a precision issue with terrain tracing that can cause a swept box to successfully trace
                // when the end position is stuck in the triangle.  Re-run the test with an uswept box to catch that
                // case until the bug is fixed.
                // If we detect getting stuck, don't allow the movement
                trace_t stuck;
                TracePlayerBBox(pm.endpos, pm.endpos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, stuck);

                if ((stuck.startsolid || stuck.fraction != 1.0f) && !bumpcount && sv_ramp_fix.GetBool())
                {
                    has_valid_plane = false;
                    stuck_on_ramp = true;
                    continue;
                }
                else if (stuck.startsolid || stuck.fraction != 1.0f)
                {
                    Msg("Player will become stuck!!! allfrac: %f pm: %i, %f, %f, %f vs stuck: %i, %f, %f\n", allFraction, pm.startsolid, pm.fraction, pm.plane.normal.z, pm.fractionleftsolid, stuck.startsolid, stuck.fraction, stuck.plane.normal.z);
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
            if (sv_ramp_fix.GetBool())
            {
                has_valid_plane = false;
                stuck_on_ramp = false;
            }

            // actually covered some distance
            VectorCopy(mv->m_vecVelocity, original_velocity);
            mv->SetAbsOrigin(pm.endpos);
            VectorCopy(mv->GetAbsOrigin(), fixed_origin);
            allFraction += pm.fraction;
            numplanes = 0;
        }

        // If we covered the entire distance, we are done
        //  and can return.
        if (CloseEnough(pm.fraction, 1.0f, FLT_EPSILON))
        {
            break; // moved the entire distance
        }

        // Save entity that blocked us (since fraction was < 1.0)
        //  for contact
        // Add it if it's not already in the list!!!
        MoveHelper()->AddToTouched(pm, mv->m_vecVelocity);

        // If the plane we hit has a high z component in the normal, then
        //  it's probably a floor
        if (pm.plane.normal[2] >= 0.7)
        {
            blocked |= 1; // floor
        }
        // If the plane has a zero z component in the normal, then it's a
        //  step or wall
        if (CloseEnough(pm.plane.normal[2], 0.0f, FLT_EPSILON))
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
            if (planes[0][2] >= 0.7)
            {
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
                ClipVelocity(original_velocity, planes[i], mv->m_vecVelocity, 1.0);
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
            {
                // go along this plane
                // pmove.velocity is set in clipping call, no need to set again.
                // DevMsg("Moving along this plane! i: %i || numplanes: %i\n", i, numplanes);
            }
            else
            { // go along the crease
                if (numplanes != 2)
                {
                    VectorCopy(vec3_origin, mv->m_vecVelocity);
                    break;
                }

                // Fun fact time: these next five lines of code fix (vertical) rampbug
                if (CloseEnough(planes[0], planes[1], FLT_EPSILON))
                {
                    // Why did the above return true? Well, when surfing, you can "clip" into the
                    // ramp, due to the ramp not pushing you away enough, and when that happens,
                    // a surfer cries. So the game thinks the surfer is clipping along two of the exact
                    // same planes. So what we do here is take the surfer's original velocity,
                    // and add the along the normal of the surf ramp they're currently riding down,
                    // essentially pushing them away from the ramp.

                    // Note: Technically the 20.0 here can be 2.0, but that causes "jitters" sometimes, so I found 20
                    // to be pretty safe and smooth. If it causes any unforeseen consequences, tweak it!
                    VectorMA(original_velocity, 20.0f, planes[0], new_velocity);
                    mv->m_vecVelocity.x = new_velocity.x;
                    mv->m_vecVelocity.y = new_velocity.y;
                    // Note: We don't want the player to gain any Z boost/reduce from this, gravity should be the
                    // only force working in the Z direction!

                    // Lastly, let's get out of here before the following lines of code make the surfer lose speed.
                    break;
                }

                // Though now it's good to note: the following code is needed for when a ramp creates a "V" shape,
                // and pinches the surfer between two planes of differing normals.
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
                VectorCopy(vec3_origin, mv->m_vecVelocity);
                break;
            }
        }
    }

    if (CloseEnough(allFraction, 0.0f, FLT_EPSILON))
    {
        // We dont want to touch this!
        // If a client is triggering this, and if they are on a surf ramp they will stand still but gain velocity that can build up for ever.
        VectorCopy(vec3_origin, mv->m_vecVelocity);
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
    if ((m_pPlayer->m_SrvData.m_SlideData.IsEnabled()) &&
        (!(mv->m_nButtons & IN_JUMP) || !m_pPlayer->m_SrvData.m_SlideData.IsAllowingJump()))
    {
        pm = nullptr;
    }

    CBaseEntity *newGround = pm ? pm->m_pEnt : nullptr;

    CBaseEntity *oldGround = player->GetGroundEntity();
    Vector vecBaseVelocity = player->GetBaseVelocity();

    bool bLanded = false;
    if (!oldGround && newGround)
    {
        // Subtract ground velocity at instant we hit ground jumping
        vecBaseVelocity -= newGround->GetAbsVelocity();
        vecBaseVelocity.z = newGround->GetAbsVelocity().z;

        // Fire that we landed on ground
        bLanded = true;
    }
    else if (oldGround && !newGround)
    {
        // Add in ground velocity at instant we started jumping
        vecBaseVelocity += oldGround->GetAbsVelocity();
        vecBaseVelocity.z = oldGround->GetAbsVelocity().z;
    }

    player->SetBaseVelocity(vecBaseVelocity);
    player->SetGroundEntity(newGround);

    if (bLanded)
        FIRE_GAMEMOVEMENT_EVENT(OnPlayerLand);

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

void CMomentumGameMovement::CheckParameters(void)
{
    // shift-walking useful for some maps with tight jumps
    if (mv->m_nButtons & IN_SPEED)
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
    if (CloseEnough(angle, 0.0f, FLT_EPSILON)) // If the plane has no Z, it is vertical (wall/step)
        blocked |= 0x02;                       //

    // Determine how far along plane to slide based on incoming direction.
    backoff = DotProduct(in, normal) * overbounce;

    for (i = 0; i < 3; i++)
    {
        change = normal[i] * backoff;
        out[i] = in[i] - change;
        // DevMsg("Backoff: %f || Change: %f || Velocity: %f\n", backoff, change, velocity);
    }

    // iterate once to make sure we aren't still moving through the plane
    float adjust = DotProduct(out, normal);
    if (adjust < 0.0f)
    {
        out -= (normal * adjust);
        // DevMsg( "Adjustment = %lf\n", adjust );
    }

    // Check if we loose speed while going on a slope in front of us.

    // MOM_TODO: Make this only bhop gametype?
    // Enable this when we konw that we are sliding.
    Vector dif = mv->m_vecVelocity - out;
    if ((dif.Length2D() > 0.0f && (angle >= 0.7f) && (out[2] > 0.0f)) &&
        (m_pPlayer->m_SrvData.m_SlideData.IsEnabled() && m_pPlayer->m_SrvData.m_SlideData.IsFixUpsideSlope()))
    {
        out.x = mv->m_vecVelocity.x;
        out.y = mv->m_vecVelocity.y;
        // Avoid being stuck into the slope.. Or velocity reset incoming!
        // (Could be better by being more close to the slope, but for player it seems to be close enough)
        // @Gocnak: Technically the "adjust" code above does this, but to each axis, with a much higher value.
        // Tickrate will work, but keep in mind tickrates can get pretty big, though realistically this will be 0.015 or
        // 0.01
        mv->m_vecAbsOrigin.z += abs(dif.z) * gpGlobals->interval_per_tick;
        DevMsg(2, "ClipVelocity: Fixed speed.\n");
    }

    // Return blocking flags.
    return blocked;
}

// Expose our interface.
static CMomentumGameMovement g_GameMovement;
CMomentumGameMovement *g_pMomentumGameMovement = &g_GameMovement;
IGameMovement *g_pGameMovement = static_cast<IGameMovement *>(&g_GameMovement);

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CMomentumGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT, g_GameMovement);
