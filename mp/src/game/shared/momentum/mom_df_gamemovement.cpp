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
    SetGroundEntity(nullptr);

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

    mv->m_vecVelocity[0] *= newspeed;
    mv->m_vecVelocity[1] *= newspeed;
    mv->m_vecVelocity[2] *= newspeed;
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

    TryPlayerMove();
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

    StayOnGround();
}

void CMomentumGameMovement::DFFullWalkMove()
{
    StartGravity();
    CheckVelocity();

    if (player->GetGroundEntity() != nullptr)
    {
        mv->m_vecVelocity[2] = 0.0f;
        DFWalkMove();
    }
    else
    {
        DFAirMove();
    }

    CheckFalling();
    StuckGround();
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

    CategorizePosition();

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