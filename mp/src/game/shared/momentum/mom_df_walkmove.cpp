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
    float add;

    // Avoid nullptr access, return false if somehow we don't have a player
    if (!player)
        return false;

    if (player->pl.deadflag)
    {
        mv->m_nOldButtons |= IN_JUMP;
        return false;
    }

    if (mv->m_nButtons & IN_DUCK)
    {
        return false;
    }
    
    if (!(mv->m_nButtons & IN_JUMP))
    {
        mv->m_nOldButtons &= ~IN_JUMP;
        mv->m_bJumpReleased = true;
        return false;
    }

    // don't jump again until released
    if (!mv->m_bJumpReleased)
    {
        return false;
    }

    // In the air now.
    DFSetGroundEntity(nullptr);
    mv->m_bJumpReleased = false;

    player->PlayStepSound(mv->GetAbsOrigin(), player->m_pSurfaceData, 1.0, true);

    if (gpGlobals->curtime - m_pPlayer->m_Data.m_flLastJumpTime < 0.4 &&
        mv->m_bCanCPMDoubleJump && sv_cpm_physics.GetBool())
    {
        add = 370;
        mv->m_bCanCPMDoubleJump = false;
    }
    else
    {
        add = 270;
        mv->m_bCanCPMDoubleJump = true;
    }

    if (sv_cpm_physics.GetBool())
    {
        mv->m_vecVelocity[2] += add;
    }
    else
    {
        mv->m_vecVelocity[2] = add;
    }

    if (mv->m_vecVelocity[2] < 270)
    {
        mv->m_vecVelocity[2] = 270;
    }

    m_pPlayer->m_Data.m_flLastJumpTime = gpGlobals->curtime;

    return true;
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

    float oldSpeed;
    Vector oldVel;

    if (DFCheckJumpButton())
    {
        DFAirMove();
        return;
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

    // clamp the speed lower if ducking
    if (player->GetFlags() & FL_DUCKING)
    {
        if (wishspeed > sv_maxspeed.GetFloat() * sv_duckscale.GetFloat())
        {
            wishspeed = sv_maxspeed.GetFloat() * sv_duckscale.GetFloat();
        }
    }

    // Set pmove velocity
    Accelerate(wishdir, wishspeed, sv_accelerate.GetFloat());

    spd = mv->m_vecVelocity.Length();
    DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);

    VectorNormalize(mv->m_vecVelocity);
    VectorScale(mv->m_vecVelocity, spd, mv->m_vecVelocity);

    oldSpeed = mv->m_vecVelocity.Length2D();
    VectorCopy(mv->m_vecVelocity, oldVel);

    DFStepSlideMove(false);

    if (gpGlobals->curtime - mv->m_flWallClipTime < sv_wallcliptime.GetFloat() &&
        oldSpeed > mv->m_vecVelocity.Length2D())
    {
        VectorCopy(oldVel, mv->m_vecVelocity);
    }
}