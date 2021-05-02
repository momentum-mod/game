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

bool CMomentumGameMovement::DFCanUnDuck()
{
    trace_t trace;
    Vector point;

    VectorCopy(mv->m_vecAbsOrigin, point);
    point[2] += 0.01;

    TracePlayerBBox(mv->m_vecAbsOrigin, point, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

    return !(trace.fraction < 1.0);
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

void CMomentumGameMovement::DFAccelerate(Vector& wishdir, float wishspeed, float accel)
{
    int i;
    float addspeed, accelspeed, currentspeed;

    currentspeed = DotProduct(mv->m_vecVelocity, wishdir);
    addspeed = wishspeed - currentspeed;

    if (addspeed <= 0)
    {
        return;
    }

    accelspeed = accel * wishspeed * gpGlobals->frametime;

    if (accelspeed > addspeed)
    {
        accelspeed = addspeed;
    }

    for (i = 0; i < 3; i++)
    {
        mv->m_vecVelocity[i] += accelspeed * wishdir[i];
    }
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

    if (player->GetWaterLevel() <= 1)
    {
        if (!(m_pPlayer->m_flKnockbackTime > gpGlobals->curtime))
        {
            control = speed < sv_stopspeed.GetFloat() ? sv_stopspeed.GetFloat() : speed;
            drop += control * sv_friction.GetFloat() * gpGlobals->frametime;
        }
    }
    else
    {
        drop += speed * sv_waterfriction.GetFloat() * gpGlobals->frametime;
    }

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

    if (player->m_flWaterJumpTime > 0 && gpGlobals->curtime - player->m_flWaterJumpTime < 2)
    {
        DFWaterJumpMove();
    }
    else if (player->GetWaterLevel() > 1)
    {
        DFWaterMove();
    }
    else if (player->GetGroundEntity() != nullptr)
    {
        DFWalkMove();
    }
    else
    {
        DFAirMove();
    }
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

    DFGroundTrace();

    if (sv_snapvelocity.GetBool())
    {
        DFSnapVector(mv->m_vecVelocity);
    }
}

void CMomentumGameMovement::DFSetGroundEntity(trace_t *pm)
{
    CBaseEntity *newGround = pm ? pm->m_pEnt : nullptr;
    CBaseEntity *oldGround = player->GetGroundEntity();
    Vector vecBaseVelocity = player->GetBaseVelocity();
    const auto pOverbounceTrigger = m_pPlayer->m_CurrentOverbounceTrigger.Get();

    if (!oldGround && newGround)
    {
        mv->m_flWallClipTime = gpGlobals->curtime;

        // Subtract ground velocity at instant we hit ground jumping
        vecBaseVelocity -= newGround->GetAbsVelocity();
        vecBaseVelocity.z = newGround->GetAbsVelocity().z;

        if (pOverbounceTrigger && abs(mv->m_vecPreviousVelocity.z) >= pOverbounceTrigger->m_flMinSpeed)
        {
            VectorCopy(mv->m_vecPreviousVelocity, mv->m_vecVelocity);
        }
        else
        {
            DFClipVelocity(mv->m_vecVelocity, pm->plane.normal, mv->m_vecVelocity, 1.001f);
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

    mv->m_bRampSliding = false;
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

    VectorCopy(trace.plane.normal, mv->m_vecGroundNormal);

    // if we got this far then we would be standing on the ground
    if (sv_rampslide.GetBool() && mv->m_vecVelocity[2] >= sv_rampslide_speed.GetFloat())
    {
        mv->m_bRampSliding = true;

        float spd = mv->m_vecVelocity.Length();
        DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);

        VectorNormalize(mv->m_vecVelocity);
        VectorScale(mv->m_vecVelocity, spd, mv->m_vecVelocity);

        DFSetGroundEntity(nullptr);
        return;
    }

    DFSetGroundEntity(&trace);
}

void CMomentumGameMovement::DFSnapVector(Vector& v)
{
    v.x = roundf(v.x);
    v.y = roundf(v.y);
    v.z = roundf(v.z);
}

float CMomentumGameMovement::DFScale(float maxspeed)
{
    float fmove, smove, umove;
    float total, scale;
    int max;

    fmove = clamp(abs(mv->m_flForwardMove), 0, 127);
    smove = clamp(abs(mv->m_flSideMove), 0, 127);
    umove = clamp(abs(mv->m_flUpMove), 0, 127);

    if (mv->m_nButtons & IN_JUMP)
    {
        umove = 127;
    }

    max = fmove;
    if (smove > max)
    {
        max = smove;
    }
    if (umove > max && !sv_scalecmd_fix.GetBool())
    {
        max = umove;
    }
    if (!max)
    {
        return 0;
    }

    if (sv_scalecmd_fix.GetBool())
    {
        // this is equal to 320 * 127 / (127 * sqrt(127^2 * 2))
        //return 1.781686378;
        total = sqrt(fmove * fmove + smove * smove);
    }
    else
    {
        total = sqrt(fmove * fmove + smove * smove + umove * umove);
    }
    scale = (float)maxspeed * max / (127.0 * total);
    return scale;
}