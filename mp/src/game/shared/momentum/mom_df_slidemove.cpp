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

#define SLIDEMOVE_PLANEINTERACT_EPSILON 0.05
#define SLIDEMOVEFLAG_PLANE_TOUCHED 16
#define SLIDEMOVEFLAG_WALL_BLOCKED 8
#define SLIDEMOVEFLAG_TRAPPED 4
#define SLIDEMOVEFLAG_BLOCKED 2 // it was blocked at some point, doesn't mean it didn't slide along the blocking object
#define SLIDEMOVEFLAG_MOVED 1

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
        VectorCopy(mv->m_vecGroundNormal, planes[0]);
        numPlanes = 1;
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
                if (j == i)
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
            break;
        }
    }

    if (inAir)
    {
        VectorCopy(endVelocity, mv->m_vecVelocity);
    }

    return (bumpCount != 0);
}

int CMomentumGameMovement::DFCPMSlideMove()
{
    Vector end, dir;
    Vector oldVel, lastValidOrigin;
    float value;
    Vector planes[MAX_CLIP_PLANES];
    int numplanes = 0;
    trace_t trace;
    int moves, i, j, k;
    int maxmoves = 4;
    float remainingTime = gpGlobals->frametime;
    int blockedMask = 0;

    VectorCopy(mv->m_vecVelocity, oldVel);
    VectorCopy(mv->GetAbsOrigin(), lastValidOrigin);

    if (player->GetGroundEntity() != nullptr)
    {
        if (mv->m_vecGroundNormal[2] == 1.0f && mv->m_vecVelocity[2] < 0.0f)
        {
            mv->m_vecVelocity[2] = 0.0f;
        }
    }
    
    for (moves = 0; moves < maxmoves; moves++)
    {
        VectorMA(mv->GetAbsOrigin(), remainingTime, mv->m_vecVelocity, end);
        TracePlayerBBox(mv->m_vecAbsOrigin, end, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
        if (trace.allsolid)
        {
            VectorCopy(lastValidOrigin, mv->m_vecAbsOrigin);
            return SLIDEMOVEFLAG_TRAPPED;
        }

        if (trace.fraction > 0)
        {
            VectorCopy(trace.endpos, mv->m_vecAbsOrigin);
            VectorCopy(trace.endpos, lastValidOrigin);
        }

        if (trace.fraction == 1)
        {
            break;
        }

        blockedMask |= SLIDEMOVEFLAG_WALL_BLOCKED;
        if (trace.plane.normal[2] < SLIDEMOVE_PLANEINTERACT_EPSILON)
        {
            blockedMask |= SLIDEMOVEFLAG_WALL_BLOCKED;
        }

        remainingTime -= (trace.fraction * remainingTime);

        for (i = 0; i < numplanes; i++)
        {
            if (DotProduct(trace.plane.normal, planes[i]) > (1.0f - SLIDEMOVE_PLANEINTERACT_EPSILON))
            {
                VectorAdd(trace.plane.normal, mv->m_vecVelocity, mv->m_vecVelocity);
                break;
            }
        }

        if (i < numplanes)
        {
            continue;
        }

        if (numplanes > MAX_CLIP_PLANES)
        {
            VectorClear(mv->m_vecVelocity);
            return SLIDEMOVEFLAG_TRAPPED;
        }

        VectorCopy(trace.plane.normal, planes[numplanes]);
        numplanes++;

        for (i = 0; i < numplanes; i++)
        {
            if (DotProduct(mv->m_vecVelocity, planes[i]) >= SLIDEMOVE_PLANEINTERACT_EPSILON)
            {
                continue;
            }

            DFClipVelocity(mv->m_vecVelocity, planes[i], mv->m_vecVelocity, 1.001f);

            for (j = 0; j < numplanes; j++)
            {
                if (j == i)
                {
                    continue;
                }
                if (DotProduct(mv->m_vecVelocity, planes[i]) >= SLIDEMOVE_PLANEINTERACT_EPSILON)
                {
                    continue;
                }

                DFClipVelocity(mv->m_vecVelocity, planes[i], mv->m_vecVelocity, 1.001f);
                if (DotProduct(mv->m_vecVelocity, planes[i]) >= SLIDEMOVE_PLANEINTERACT_EPSILON)
                {
                    continue;
                }

                CrossProduct(planes[i], planes[j], dir);
                VectorNormalize(dir);
                value = DotProduct(dir, mv->m_vecVelocity);
                VectorScale(dir, value, mv->m_vecVelocity);

                for (k = 0; k < numplanes; k++)
                {
                    if (j == k || i == k)
                    {
                        continue;
                    }
                    if (DotProduct(mv->m_vecVelocity, planes[i]) >= SLIDEMOVE_PLANEINTERACT_EPSILON)
                    {
                        continue;
                    }

                    VectorClear(mv->m_vecVelocity);
                    break;
                }
            }
        }
    }

    return blockedMask;
}

void CMomentumGameMovement::DFStepSlideMove(bool inAir)
{
    Vector startOrigin, startVel;
    Vector downOrigin, downVel;
    trace_t trace;
    Vector up, down;
    float stepSize;
    int blocked;

    if (sv_cpm_physics.GetBool())
    {
        VectorCopy(mv->m_vecAbsOrigin, startOrigin);
        VectorCopy(mv->m_vecVelocity, startVel);

        blocked = DFCPMSlideMove();
        if (!blocked)
        {
            return;
        }

        VectorCopy(mv->m_vecAbsOrigin, downOrigin);
        VectorCopy(mv->m_vecVelocity, downVel);

        VectorCopy(startOrigin, up);
        up[2] += sv_stepsize.GetFloat();

        TracePlayerBBox(up, up, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
        if (trace.allsolid)
        {
            return;
        }

        // try sliding above
        VectorCopy(up, mv->m_vecAbsOrigin);
        VectorCopy(startVel, mv->m_vecVelocity);

        DFCPMSlideMove();

        // push down the final amount
        VectorCopy(mv->m_vecAbsOrigin, down);
        down[2] -= sv_stepsize.GetFloat();
        TracePlayerBBox(mv->m_vecAbsOrigin, down, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
        if (!trace.allsolid)
        {
            VectorCopy(trace.endpos, mv->m_vecAbsOrigin);
        }

        VectorCopy(mv->m_vecAbsOrigin, up);

        if (trace.fraction < 1.0)
        {
            DFClipVelocity(mv->m_vecVelocity, trace.plane.normal, mv->m_vecVelocity, 1.001f);
        }
        
    }
    else
    {
        VectorCopy(mv->m_vecAbsOrigin, startOrigin);
        VectorCopy(mv->m_vecVelocity, startVel);

        // can we move without having to step?
        if (!DFSlideMove(inAir))
        {
            return;
        }

        VectorCopy(startOrigin, down);
        down[2] -= sv_stepsize.GetFloat();

        TracePlayerBBox(startOrigin, down, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

        // never step up when you still have velocity
        if (mv->m_vecVelocity[2] > 0 && (trace.fraction == 1.0 || DotProduct(trace.plane.normal, up) < 0.7))
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
}