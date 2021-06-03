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
    int jumpStyle = sv_jumpstyle.GetInt();

    // Avoid nullptr access, return false if somehow we don't have a player
    if (!player)
        return false;

    if (player->pl.deadflag)
    {
        mv->m_nOldButtons |= IN_JUMP;
        return false;
    }

    if (mv->m_nButtons & IN_DUCK && !sv_crouchjump.GetBool())
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
    if (!mv->m_bJumpReleased && !sv_autojump.GetBool())
    {
        return false;
    }

    if (mv->m_bRampSliding && sv_rampslide.GetInt() == 2 && sv_rampslide_jumps.GetInt() == 1 &&
        !(mv->m_nButtons & IN_DUCK))
    {
        mv->m_vecVelocity.z = 0;
        DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);
        mv->m_vecPreviousVelocity.z = mv->m_vecVelocity.z;
    }

    // In the air now.
    DFSetGroundEntity(nullptr);

    if (gpGlobals->curtime - m_pPlayer->m_Data.m_flLastJumpTime >= 0.075)
    {
        player->PlayStepSound(mv->GetAbsOrigin(), player->m_pSurfaceData, 1.0, true);
#ifdef GAME_DLL
        if (sv_jumpsound.GetBool())
        {
            engine->ServerCommand("play player/jump.wav\n");
        }
#endif
    }

    if (mv->m_nButtons & IN_DUCK && sv_crouchjump.GetBool())
    {
        add = 270;

        mv->m_vecVelocity.z += add;
        
        if (mv->m_vecVelocity.z > add)
        {
            mv->m_vecVelocity.z = add;
        }
    }
    else if (jumpStyle > 0 && (sv_autojump_boost.GetBool() || mv->m_bJumpReleased))
    {
        add = 270;

        if (gpGlobals->curtime - m_pPlayer->m_Data.m_flLastJumpTime < 0.4 &&
            mv->m_bCanCPMDoubleJump && jumpStyle == 2)
        {
            add = 370;
            mv->m_bCanCPMDoubleJump = false;
        }
        else
        {
            mv->m_bCanCPMDoubleJump = true;
        }

        // QW jumping
        if (jumpStyle == 1)
        {
            mv->m_vecVelocity.z += add;
        }
        // CPM jumping
        else if (jumpStyle == 2)
        {
            mv->m_vecVelocity.z += add;
            if (mv->m_vecVelocity.z < 270)
            {
                mv->m_vecVelocity.z = 270;
            }
        }
        // Q2 jumping
        else
        {
            if (mv->m_vecVelocity.z > mv->m_vecPreviousVelocity.z)
            {
                mv->m_vecVelocity.z += add;
            }
            else
            {
                mv->m_vecVelocity.z = mv->m_vecPreviousVelocity.z + add;
            }
            if (mv->m_vecVelocity.z < 270)
            {
                mv->m_vecVelocity.z = 270;
            }
        }
    }
    else
    {
        mv->m_vecVelocity.z = 270;
    }

    mv->m_bJumpReleased = false;
    m_pPlayer->m_Data.m_flLastJumpTime = gpGlobals->curtime;

#ifndef CLIENT_DLL
    m_pPlayer->SetIsInAirDueToJump(true);
    // Fire that we jumped
    m_pPlayer->OnJump();
    IGameEvent *pEvent = gameeventmanager->CreateEvent("player_jumped");
    if (pEvent)
        gameeventmanager->FireEvent(pEvent);
#endif

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

    float oldSpeed;
    Vector oldVel;

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    if (player->GetWaterLevel() > 2 && DotProduct(forward, mv->m_vecGroundNormal) > 0)
    {
        DFWaterMove();
        return;
    }

    if (DFCheckJumpButton())
    {
        if (player->GetWaterLevel() > 1)
        {
            DFWaterMove();
        }
        else
        {
            DFAirMove();
        }
        return;
     }

    // if we have no slide trigger and no knockback, do friction
    if (m_pPlayer->m_CurrentSlideTrigger == nullptr &&
        m_pPlayer->m_flKnockbackTime < gpGlobals->curtime)
    {
         DFFriction();
    }
    // otherwise do airmovement if our slickstyle is 0
    else if (sv_slickstyle.GetInt() == 0)
    {
        DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);
        DFAirMove();
        return;
    }
    // if slickstyle isn't zero, do ground movement without friction

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
    DFAccelerate(wishdir, wishspeed, sv_accelerate.GetFloat());

    // this is the part that causes overbounces   
    spd = mv->m_vecVelocity.Length();

    DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);

    VectorNormalize(mv->m_vecVelocity);
    VectorScale(mv->m_vecVelocity, spd, mv->m_vecVelocity);

    oldSpeed = mv->m_vecVelocity.Length2D();
    VectorCopy(mv->m_vecVelocity, oldVel);

    VectorCopy(mv->m_vecVelocity, mv->m_vecPreviousVelocity);
    DFStepSlideMove(false);
}