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
	return false;
}
void CMomentumGameMovement::DFWaterJumpMove() {}
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
    const bool bIsSliding = m_pPlayer->m_CurrentSlideTrigger != nullptr;

    if (DFCheckWaterJump())
    {
        DFWaterMove();
        return;
    }

    DFFriction();

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;
    umove = mv->m_flUpMove;

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