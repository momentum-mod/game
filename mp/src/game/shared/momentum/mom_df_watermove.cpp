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


bool CMomentumGameMovement::DFCheckWaterJump()
{
    Vector spot;
    int cont;
    Vector flatForward;
    Vector forward, right, up;

    if (player->m_flWaterJumpTime > 0 && gpGlobals->curtime - player->m_flWaterJumpTime < 2)
    {
        return false;
    }

    if (player->GetWaterLevel() != 2)
    {
        return false;
    }

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    flatForward[0] = forward[0];
    flatForward[1] = forward[1];
    flatForward[2] = 0;
    VectorNormalize(flatForward);

    VectorMA(mv->GetAbsOrigin(), 30, flatForward, spot);
    spot[2] += 4;
    cont = GetPointContentsCached(spot, 0);
    if (!(cont & CONTENTS_SOLID))
    {
        return false;
    }

    spot[2] += 16;
    cont = GetPointContentsCached(spot, 1);
    // no idea why we have to mask out CONTENTS_TESTFOGVOLUME here but it works
    if (cont & (~CONTENTS_TESTFOGVOLUME))
    {
        return false;
    }

    VectorScale(forward, 50, mv->m_vecVelocity);
    mv->m_vecVelocity[2] = 350;

    player->m_flWaterJumpTime = gpGlobals->curtime;

	return true;
}

void CMomentumGameMovement::DFWaterJumpMove()
{
    Vector oldVel;
    VectorCopy(mv->m_vecVelocity, oldVel);

    DFStepSlideMove(true);

    if (oldVel.Length2D() > mv->m_vecVelocity.Length2D())
    {
        mv->m_vecVelocity[0] = oldVel[0];
        mv->m_vecVelocity[1] = oldVel[1];
    }

    mv->m_vecVelocity[2] -= sv_gravity.GetFloat() * gpGlobals->frametime;
    if (mv->m_vecVelocity[2] < 0)
    {
        player->m_flWaterJumpTime = 0;
    }
}

void CMomentumGameMovement::DFWaterMove()
{
    int i;

    Vector wishvel;
    float spd;
    float fmove, smove, umove;
    Vector wishdir;
    float wishspeed;
    Vector dest;
    Vector forward, right, up;

    if (DFCheckWaterJump())
    {
        DFWaterJumpMove();
        return;
    }

    DFFriction();

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;
    umove = mv->m_flUpMove;

    if (mv->m_nButtons & IN_JUMP)
    {
        umove = sv_maxspeed.GetFloat();
    }

    if (!fmove && !smove && !umove)
    {
        wishvel[0] = 0;
        wishvel[1] = 1;
        wishvel[2] = -60;
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            wishvel[i] = forward[i] * fmove + right[i] * smove;
        }

        wishvel[2] += umove;
    }

    VectorCopy(wishvel, wishdir); // Determine maginitude of speed of move
    wishspeed = VectorNormalize(wishdir);

    if (wishspeed > sv_maxspeed.GetFloat() * sv_swimscale.GetFloat())
    {
        wishspeed = sv_maxspeed.GetFloat() * sv_swimscale.GetFloat();
    }

    // Set pmove velocity
    Accelerate(wishdir, wishspeed, sv_wateraccelerate.GetFloat());

    if (player->GetGroundEntity() != nullptr && DotProduct(mv->m_vecVelocity, mv->m_vecGroundNormal) < 0)
    {
        spd = mv->m_vecVelocity.Length();
        DFClipVelocity(mv->m_vecVelocity, mv->m_vecGroundNormal, mv->m_vecVelocity, 1.001f);

        VectorNormalize(mv->m_vecVelocity);
        VectorScale(mv->m_vecVelocity, spd, mv->m_vecVelocity);
    }

    DFStepSlideMove(false);
}