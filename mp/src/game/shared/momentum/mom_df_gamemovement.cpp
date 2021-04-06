#include "cbase.h"

#include "in_buttons.h"
#include "mom_gamemovement.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "movevars_shared.h"
#include "coordsize.h"
#include "mom_system_gamemode.h"

#ifdef CLIENT_DLL
#include "c_mom_triggers.h"
#else
#include "env_player_surface_trigger.h"
#include "momentum/mom_triggers.h"
#include "momentum/mom_system_saveloc.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

bool CMomentumGameMovement::DFCheckJumpButton()
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

    // In the air now.
    DFSetGroundEntity(nullptr);

    mv->m_vecVelocity[2] = 270;

    return true;
}

void CMomentumGameMovement::DFDuck()
{
    Duck();
}

void CMomentumGameMovement::DFFriction()
{
    Vector vel;
    float speed, newspeed, control;
    float drop;

    VectorCopy(mv->m_vecVelocity, vel);

    vel[2] = 0;

    speed = VectorLength(vel);

    if (speed < 1)
    {
        mv->m_vecVelocity[0] = 0;
        mv->m_vecVelocity[1] = 0;
        return;
    }

    drop = 0;

    control = speed < sv_stopspeed.GetFloat() ? sv_stopspeed.GetFloat() : speed;
    drop += control * sv_friction.GetFloat() * gpGlobals->frametime;

    newspeed = speed - drop;
    if (newspeed < 0)
    {
        newspeed = 0;
    }

    newspeed /= speed;

    mv->m_vecVelocity[0] = mv->m_vecVelocity[0] * newspeed;
    mv->m_vecVelocity[1] = mv->m_vecVelocity[1] * newspeed;
    mv->m_vecVelocity[2] = mv->m_vecVelocity[2] * newspeed;
}

void CMomentumGameMovement::DFAirAccelerate(Vector wishdir, float wishspeed, float accel, float maxspeed)
{
    int i;
    float addspeed, accelspeed, currentspeed;
    float wishspd;

    wishspd = wishspeed;

    if (player->pl.deadflag)
        return;

    if (player->m_flWaterJumpTime)
        return;

    // Cap speed
    if (wishspd > maxspeed)
        wishspd = maxspeed;

    // Determine veer amount
    currentspeed = mv->m_vecVelocity.Dot(wishdir);

    // See how much to add
    addspeed = wishspd - currentspeed;

    // If not adding any, done.
    if (addspeed <= 0)
        return;

    // Determine acceleration speed after acceleration
    accelspeed = accel * wishspeed * gpGlobals->frametime * player->m_surfaceFriction;

    // Cap it
    if (accelspeed > addspeed)
        accelspeed = addspeed;

    // Adjust pmove vel.
    for (i = 0; i < 3; i++)
    {
        mv->m_vecVelocity[i] += accelspeed * wishdir[i];
        mv->m_outWishVel[i] += accelspeed * wishdir[i];
    }
}

void CMomentumGameMovement::DFAirMove()
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

    VectorCopy(wishvel, wishdir); // Determine magnitude of speed of move
    wishspeed = VectorNormalize(wishdir);

    //
    // clamp to server defined max speed
    //
    if (wishspeed != 0 && (wishspeed > mv->m_flMaxSpeed))
    {
        VectorScale(wishvel, mv->m_flMaxSpeed / wishspeed, wishvel);
        wishspeed = mv->m_flMaxSpeed;
    }

    DFAirAccelerate(wishdir, wishspeed, sv_airaccelerate.GetFloat(), 30);

    DFStepSlideMove(true);
}

void CMomentumGameMovement::DFWalkMove()
{
    int i;

    Vector wishvel;
    float spd;
    float fmove, smove;
    Vector wishdir;
    float wishspeed;

    Vector dest;
    Vector forward, right, up;
    const bool bIsSliding = m_pPlayer->m_CurrentSlideTrigger != nullptr;

    if (mv->m_nButtons & IN_JUMP)
    {
        if (DFCheckJumpButton())
        {
            DFAirMove();
            return;
        }
    }
    else
    {
        //mv->m_bJumpReleased = true;
        //mv->m_flJumpTime = -1;
        mv->m_nOldButtons &= ~IN_JUMP;
    }

    DFFriction();

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;

    forward[2] = 0;
    right[2] = 0;

    DFClipVelocity(forward, mv->m_vecGroundNormal, forward, 1.001f);
    DFClipVelocity(right, mv->m_vecGroundNormal, right, 1.001f);
    VectorNormalize(forward);
    VectorNormalize(right);

    for (i = 0; i < 3; i++) // Determine x and y parts of velocity
    {
        wishvel[i] = forward[i] * fmove + right[i] * smove;
    }

    VectorCopy(wishvel, wishdir); // Determine maginitude of speed of move
    wishspeed = VectorNormalize(wishdir);

    // Set pmove velocity
    Accelerate(wishdir, wishspeed, sv_accelerate.GetFloat());

    spd = mv->m_vecVelocity.Length();
    DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);

    VectorNormalize(mv->m_vecVelocity);
    VectorScale(mv->m_vecVelocity, spd, mv->m_vecVelocity);

    DFStepSlideMove(false);
}

void CMomentumGameMovement::DFFullWalkMove()
{

    if (player->GetGroundEntity() != nullptr)
    {
        DFWalkMove();
    }
    else
    {
        DFAirMove();

        StartGravity();
        FinishGravity();
    }
}

void CMomentumGameMovement::DFClipVelocity(Vector in, Vector &normal, Vector &out, float overbounce)
{
    float backoff;
    float change;
    int i;

    backoff = DotProduct(in, normal);

    if (backoff < 0)
    {
        backoff *= overbounce;
    }
    else
    {
        backoff /= overbounce;
    }

    for (i = 0; i < 3; i++)
    {
        change = normal[i] * backoff;
        out[i] = in[i] - change;
    }
}

void CMomentumGameMovement::DFGroundTrace()
{
    Vector point;
    trace_t trace;
    float initVel;

    initVel = mv->m_vecVelocity.Length();

    VectorCopy(mv->m_vecAbsOrigin, point);
    point[2] -= 0.25f;

    TracePlayerBBox(mv->m_vecAbsOrigin, point, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

    if (trace.fraction == 1)
    {
        DFSetGroundEntity(nullptr);
        return;
    }

    if (mv->m_vecVelocity[2] > 0 && DotProduct(mv->m_vecVelocity, trace.plane.normal) > 10)
    {
        DFSetGroundEntity(nullptr);
        return;
    }

    if (trace.plane.normal[2] < 0.7f)
    {
        DFSetGroundEntity(nullptr);
        return;
    }

    DFSetGroundEntity(&trace);
    Msg("slowdown: %f - %f =  %f\n", initVel, mv->m_vecVelocity.Length(), initVel - mv->m_vecVelocity.Length());
    VectorCopy(trace.plane.normal, mv->m_vecGroundNormal);
}

void CMomentumGameMovement::DFPlayerMove()
{

	CheckParameters();

    // clear output applied velocity
    mv->m_outWishVel.Init();
    mv->m_outJumpVel.Init();

    MoveHelper()->ResetTouchList();

    ReduceTimers();

    DFDuck();

    DFGroundTrace();

    switch (player->GetMoveType())
    {
        case MOVETYPE_NONE:
            break;

        case MOVETYPE_NOCLIP:
            FullNoClipMove(sv_noclipspeed.GetFloat(), sv_noclipaccelerate.GetFloat());
            break;

        case MOVETYPE_FLY:
        case MOVETYPE_FLYGRAVITY:
            FullTossMove();
            break;

        case MOVETYPE_LADDER:
            FullLadderMove();
            break;

        case MOVETYPE_WALK:
            DFFullWalkMove();
            break;

        case MOVETYPE_ISOMETRIC:
            // IsometricMove();
            // Could also try:  FullTossMove();
            DFFullWalkMove();
            break;

        case MOVETYPE_OBSERVER:
            FullObserverMove(); // clips against world&players
            break;

        default:
            DevMsg(1, "Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", player->GetMoveType(), player->IsServer());
            break;
    }
}

#define MAX_CLIP_PLANES 5
bool CMomentumGameMovement::DFSlideMove(bool inAir)
{
    int bumpCount, numBumps;
    Vector direction;
    float dot;
    int numPlanes;
    Vector planes[MAX_CLIP_PLANES];
    Vector primalVelocity;
    Vector clipVelocity;
    int i, j, k;
    trace_t trace;
    Vector end;
    float timeLeft;
    float into;
    Vector endVelocity;
    Vector endClipVelocity;

    numBumps = 4;

    VectorCopy(mv->m_vecVelocity, primalVelocity);

    if (inAir)
    {
        VectorCopy(mv->m_vecVelocity, endVelocity);
        //endVelocity[2] -= sv_gravity.GetFloat() * gpGlobals->frametime;

        mv->m_vecVelocity[2] = (mv->m_vecVelocity[2] + endVelocity[2]) * 0.5;

        primalVelocity[2] = endVelocity[2];

        if (player->GetGroundEntity() != nullptr)
        {
            // slide along the ground plane
            DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.00f);
        }
    }

    timeLeft = gpGlobals->frametime;

    // never turn against the ground plane
    if (player->GetGroundEntity() != nullptr)
    {
        numPlanes = 1;
        VectorCopy(mv->m_vecGroundNormal, planes[0]);
    }
    else
    {
        numPlanes = 0;
    }

    // never turn against original velocity
    VectorCopy(mv->m_vecVelocity, planes[numPlanes]);
    VectorNormalize(planes[numPlanes]);
    numPlanes++;

    for (bumpCount = 0; bumpCount < numBumps; bumpCount++)
    {
        // calculate where we are trying to move
        VectorMA(mv->m_vecAbsOrigin, timeLeft, mv->m_vecVelocity, end);

        // see if we can move there
        TracePlayerBBox(mv->m_vecAbsOrigin, end, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

        if (trace.allsolid)
        {
            // entity is completely trapped in a solid
            // so allow horizontal velocity but no vertical
            mv->m_vecVelocity[2] = 0;
            return true;
        }

        if (trace.fraction > 0)
        {
            VectorCopy(trace.endpos, mv->m_vecAbsOrigin);
        }

        if (trace.fraction == 1)
        {
            break;
        }

        timeLeft -= timeLeft * trace.fraction;

        if (numPlanes >= MAX_CLIP_PLANES)
        {
            // according to the q3 developers, this shouldn't happen :)
            VectorClear(mv->m_vecVelocity);
            return true;
        }

        // if this is the same plane we hit before, nudge velocity
        // out along it, which fixes some epsilon issues with
        // non-axial planes

        for (i = 0; i < numPlanes; i++)
        {
            if (DotProduct(trace.plane.normal, planes[i]) > 0.99)
            {
                VectorAdd(trace.plane.normal, mv->m_vecVelocity, mv->m_vecVelocity);
                break;
            }
        }

        if (i < numPlanes)
        {
            continue;
        }

        VectorCopy(trace.plane.normal, planes[numPlanes]);
        numPlanes++;

        // modify velocity so it parallels all of the clip planes
        for (i = 0; i < numPlanes; i++)
        {
            into = DotProduct(mv->m_vecVelocity, planes[i]);
            if (into >= 0.1)
            {
                continue; // move doesn't interact with the plane
            }

            // slide along the plane
            DFClipVelocity(mv->m_vecVelocity, planes[i], clipVelocity, 1.001f);
            DFClipVelocity(endVelocity, planes[i], endClipVelocity, 1.001f);

            // see if there is a second plane that the new move enters

            for (j = 0; j < numPlanes; j++)
            {
                if (j == 1)
                {
                    continue;
                }

                if (DotProduct(clipVelocity, planes[j]) >= 0.1)
                {
                    continue;
                }

                // try clipping move to the new plane
                DFClipVelocity(clipVelocity, planes[j], clipVelocity, 1.001f);
                DFClipVelocity(endClipVelocity, planes[j], endClipVelocity, 1.001f);

                // see if it goes back into the first clip plane
                if (DotProduct(clipVelocity, planes[i]) >= 0)
                {
                    continue;
                }

                // slide the original velocity along the crease
                CrossProduct(planes[i], planes[j], direction);
                VectorNormalize(direction);
                dot = DotProduct(direction, mv->m_vecVelocity);
                VectorScale(direction, dot, clipVelocity);

                CrossProduct(planes[i], planes[j], direction);
                VectorNormalize(direction);
                dot = DotProduct(direction, endVelocity);
                VectorScale(direction, dot, endClipVelocity);

                // see if there is a third plane the new move enters
                for (k = 0; k < numPlanes; k++)
                {
                    if (k == i || k == j)
                    {
                        continue;
                    }

                    if (DotProduct(clipVelocity, planes[k]) >= 0.1)
                    {
                        continue;
                    }

                    // stop dead at a triple interaction
                    VectorClear(mv->m_vecVelocity);
                    return true;
                }
            }

            // if we have failed all interactions, try another move
            VectorCopy(clipVelocity, mv->m_vecVelocity);
            VectorCopy(endClipVelocity, endVelocity);
        }
    }

    if (inAir)
    {
        VectorCopy(endVelocity, mv->m_vecVelocity);
    }

    return (bumpCount != 0);
}

void CMomentumGameMovement::DFStepSlideMove(bool inAir)
{
    Vector startOrigin, startVel;
    Vector downOrigin, downVel;
    trace_t trace;
    Vector up, down;
    float stepSize;

    // can we move without having to step?
    if (!DFSlideMove(inAir))
    {
        return;
    }

    VectorCopy(mv->m_vecAbsOrigin, startOrigin);
    VectorCopy(mv->m_vecVelocity, startVel);

    VectorCopy(startOrigin, down);
    down[2] -= sv_stepsize.GetFloat();

    TracePlayerBBox(startOrigin, down, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

    // never step up when you still have velocity
    if (mv->m_vecVelocity[2] > 0 && (trace.fraction == 1.0 ||
        DotProduct(trace.plane.normal, up) < 0.7))
    {
        return;        
    }

    VectorCopy(mv->m_vecAbsOrigin, downOrigin);
    VectorCopy(mv->m_vecVelocity, downVel);

    VectorCopy(startOrigin, up);
    up[2] += sv_stepsize.GetFloat();

    // test the player position if they were a stepheight higher
    TracePlayerBBox(startOrigin, up, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
    
    if (trace.allsolid)
    {
        return;
    }

    stepSize = trace.endpos[2] - startOrigin[2];
    VectorCopy(trace.endpos, mv->m_vecAbsOrigin);
    VectorCopy(startVel, mv->m_vecVelocity);

    DFSlideMove(inAir);

    // push down the final amount
    VectorCopy(mv->m_vecAbsOrigin, down);
    down[2] -= stepSize;
    TracePlayerBBox(mv->m_vecAbsOrigin, down, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
    if (!trace.allsolid)
    {
        VectorCopy(trace.endpos, mv->m_vecAbsOrigin);
    }
    if (trace.fraction < 1.0)
    {
        DFClipVelocity(mv->m_vecVelocity, trace.plane.normal, mv->m_vecVelocity, 1.001f);
    }
}

void CMomentumGameMovement::DFSetGroundEntity(const trace_t *pm)
{
    CBaseEntity *newGround = pm ? pm->m_pEnt : nullptr;
    CBaseEntity *oldGround = player->GetGroundEntity();
    Vector vecBaseVelocity = player->GetBaseVelocity();

    if (!oldGround && newGround)
    {
        // Subtract ground velocity at instant we hit ground jumping
        vecBaseVelocity -= newGround->GetAbsVelocity();
        vecBaseVelocity.z = newGround->GetAbsVelocity().z;
        mv->m_vecVelocity.z = 0;
    }
    else if (oldGround && !newGround)
    {
        // Add in ground velocity at instant we started jumping
        vecBaseVelocity += oldGround->GetAbsVelocity();
        vecBaseVelocity.z = oldGround->GetAbsVelocity().z;
    }

    player->SetBaseVelocity(vecBaseVelocity);
    player->SetGroundEntity(newGround);

    if (newGround)
    {
        CategorizeGroundSurface(*pm);

        // Then we are not in water jump sequence
        player->m_flWaterJumpTime = 0;

        // Standing on an entity other than the world, so signal that we are touching something.
        if (!pm->DidHitWorld())
        {
            MoveHelper()->AddToTouched(*pm, mv->m_vecVelocity);
        }
    }
}