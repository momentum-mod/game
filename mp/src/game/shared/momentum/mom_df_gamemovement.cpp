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

void CMomentumGameMovement::DFDuck()
{
    BaseClass::Duck();
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
        mv->m_nOldButtons &= ~IN_JUMP;
        mv->m_bJumpReleased = true;
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