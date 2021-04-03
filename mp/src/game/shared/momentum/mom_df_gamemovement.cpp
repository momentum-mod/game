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

    mv->m_vecVelocity[0] *= newspeed;
    mv->m_vecVelocity[1] *= newspeed;
    mv->m_vecVelocity[2] *= newspeed;
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
    trace_t pm;
    Vector forward, right, up;
    const bool bIsSliding = m_pPlayer->m_CurrentSlideTrigger != nullptr;

    if (mv->m_nButtons & IN_JUMP)
    {
        CheckJumpButton();
    }
    else
    {
        //mv->m_bJumpReleased = true;
        //mv->m_flJumpTime = -1;
        mv->m_nOldButtons &= ~IN_JUMP;
    }

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    CHandle<CBaseEntity> oldground;
    oldground = player->GetGroundEntity();

    DFFriction();

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;

    forward[2] = 0;
    right[2] = 0;

    Vector vecStart = mv->m_vecAbsOrigin;
    Vector vecStop = vecStart - Vector(0, 0, 60.0f);

    TracePlayerBBox(vecStart, vecStop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm);

    ClipVelocity(forward, pm.plane.normal, forward, 1.001f);
    ClipVelocity(right, pm.plane.normal, right, 1.001f);
    VectorNormalize(forward);
    VectorNormalize(right);

    for (i = 0; i < 3; i++) // Determine x and y parts of velocity
        wishvel[i] = forward[i] * fmove + right[i] * smove;

    VectorCopy(wishvel, wishdir); // Determine maginitude of speed of move
    wishspeed = VectorNormalize(wishdir);

    // Set pmove velocity
    Accelerate(wishdir, wishspeed, sv_accelerate.GetFloat());

    spd = mv->m_vecVelocity.Length2D();
    ClipVelocity(mv->m_vecVelocity, pm.plane.normal, mv->m_vecVelocity, 1.0f);

    VectorNormalize(mv->m_vecVelocity);
    VectorScale(mv->m_vecVelocity, spd, mv->m_vecVelocity);

    // first try just moving to the destination
    dest[0] = mv->GetAbsOrigin()[0] + mv->m_vecVelocity[0] * gpGlobals->frametime;
    dest[1] = mv->GetAbsOrigin()[1] + mv->m_vecVelocity[1] * gpGlobals->frametime;

    // The original code was "+ mv->m_vecVelocity[1]" which was obviously incorrect and should be [2] but after changing
    // it to [2] the sliding on sloped grounds started happening, so now I think this is be the solution
    dest[2] = mv->GetAbsOrigin()[2];

    // first try moving directly to the next spot
    TracePlayerBBox(mv->GetAbsOrigin(), dest, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

    StepMove(dest, pm);
}

void CMomentumGameMovement::DFFullWalkMove()
{
    DFWalkMove();
}

void CMomentumGameMovement::DFPlayerMove()
{
	CheckParameters();

    // clear output applied velocity
    mv->m_outWishVel.Init();
    mv->m_outJumpVel.Init();

    MoveHelper()->ResetTouchList();

    ReduceTimers();

    AngleVectors(mv->m_vecViewAngles, &m_vecForward, &m_vecRight, &m_vecUp); // Determine movement angles

    DFDuck();

    SetGroundEntity(NULL);

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