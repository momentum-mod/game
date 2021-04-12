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

bool CMomentumGameMovement::DFCanUnDuck()
{
    trace_t trace;
    Vector point;

    VectorCopy(mv->m_vecAbsOrigin, point);
    point[2] += 0.01;

    TracePlayerBBox(mv->m_vecAbsOrigin, point, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

    return !(trace.fraction < 1.0);
}

void CMomentumGameMovement::DFSetWaterLevel()
{
    Vector point;
    int contents = CONTENTS_EMPTY;
    int sample1, sample2;
    int waterlevel = WL_NotInWater;
    int watertype;
    
    VectorCopy(mv->GetAbsOrigin(), point);
    point[2] = mv->GetAbsOrigin()[2] + VEC_HULL_MIN[2] + 1;

    contents = GetPointContentsCached(point, 0);

    if (contents & MASK_WATER)
    {
        watertype = contents;

        sample2 = player->GetViewOffset()[2] - VEC_HULL_MIN[2];
        sample1 = sample2 / 2;
        
        waterlevel = WL_Feet;
        point[2] = mv->GetAbsOrigin()[2] + VEC_HULL_MIN[2] + sample1;
        contents = GetPointContentsCached(point, 1);
        if (contents & MASK_WATER)
        {
            waterlevel = WL_Waist;
            point[2] = mv->GetAbsOrigin()[2] + VEC_HULL_MIN[2] + sample2;
            contents = GetPointContentsCached(point, 2);
            if (contents & MASK_WATER)
            {
                waterlevel = WL_Eyes;
            }
        }
    }

    player->SetWaterLevel(waterlevel);
    player->SetWaterType(contents);
}

void CMomentumGameMovement::DFDuck()
{
    if (mv->m_nButtons & IN_DUCK)
    {
        // we want to stand while ducking, but we don't want to actually "unduck", since
        // we can't jump while doing this
        if (mv->m_nButtons & IN_JUMP && DFCanUnDuck())
        {
            player->RemoveFlag(FL_DUCKING);
            player->m_Local.m_bDucked = false;

            player->SetViewOffset(GetPlayerViewOffset(false));
        }
        else
        {
            player->AddFlag(FL_DUCKING);
            player->m_Local.m_bDucked = true;

            player->SetViewOffset(GetPlayerViewOffset(true));
        }
    }
    else
    {
        // set this to false so tracebox uses standing bbox
        player->m_Local.m_bDucked = false;

        if (!DFCanUnDuck())
        {
            player->AddFlag(FL_DUCKING);
            player->m_Local.m_bDucked = true;

            player->SetViewOffset(GetPlayerViewOffset(true));
        }
        else
        {
            player->RemoveFlag(FL_DUCKING);
            player->m_Local.m_bDucked = false;

            player->SetViewOffset(GetPlayerViewOffset(false));
        }
    }
    return;
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

void CMomentumGameMovement::DFFullWalkMove()
{
    if (!(mv->m_nButtons & IN_JUMP))
    {
        mv->m_flJumpHoldTime = -1;
        mv->m_nOldButtons &= ~IN_JUMP;
        mv->m_bJumpReleased = true;
    }
    else if (mv->m_flJumpHoldTime < 0)
    {
        mv->m_flJumpHoldTime = gpGlobals->curtime;
    }

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


void CMomentumGameMovement::DFSetGroundEntity(const trace_t *pm)
{
    CBaseEntity *newGround = pm ? pm->m_pEnt : nullptr;
    CBaseEntity *oldGround = player->GetGroundEntity();
    Vector vecBaseVelocity = player->GetBaseVelocity();

    if (!oldGround && newGround)
    {
        mv->m_flWallClipTime = gpGlobals->curtime;

        // Subtract ground velocity at instant we hit ground jumping
        vecBaseVelocity -= newGround->GetAbsVelocity();
        vecBaseVelocity.z = newGround->GetAbsVelocity().z;
        
        // if this is on, then trimping isn't possible in CPM
        // however, turning it off means there is a weird visual glitch where
        // your vertical velocity will oscillate quickly until you move.
        // the glitch only happens when landing on the ground with very little
        // horizontal velocity. you would hardly trimp anyway with that much speed so
        // its alright to set it to zero.
        if (!sv_cpm_physics.GetBool() || mv->m_vecVelocity.Length2D() < 2)
        {
            mv->m_vecVelocity.z = 0;
        }
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

void CMomentumGameMovement::DFGroundTrace()
{
    Vector point;
    trace_t trace;

    VectorCopy(mv->m_vecAbsOrigin, point);
    point[2] -= 0.1f;

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

    DFSetWaterLevel();

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