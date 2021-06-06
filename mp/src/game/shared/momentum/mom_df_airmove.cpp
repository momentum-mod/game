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

#define VectorMAM(scale1, b1, scale2, b2, c)                                                                           \
    (c.x = scale1 * b1.x + scale2 * b2.x, c.y = scale1 * b1.y + scale2 * b2.y, c.z = scale1 * b1.z + scale2 * b2.z)
void CMomentumGameMovement::DFAirControl(Vector &wishdir, float wishspeed)
{
    float k = 32;
    float kMult = wishspeed / sv_maxairspeed.GetFloat();
    float speed;
    float dot;
    float zVel = mv->m_vecVelocity.z;

    mv->m_vecVelocity.z = 0;

    kMult = clamp(kMult, 0, 1);
    k *= kMult;

    speed = mv->m_vecVelocity.Length();
    mv->m_vecVelocity = mv->m_vecVelocity.Normalized();
    dot = mv->m_vecVelocity.Dot(wishdir);

    if (dot > 0)
    {
        k *= powf(dot, sv_aircontrolpower.GetFloat()) * gpGlobals->frametime;
        speed = max(0, speed);
        k *= sv_aircontrol.GetFloat();
        VectorMAM(speed, mv->m_vecVelocity, k, wishdir, mv->m_vecVelocity);
        mv->m_vecVelocity = mv->m_vecVelocity.Normalized();
    }

    mv->m_vecVelocity.x *= speed;
    mv->m_vecVelocity.y *= speed;
    mv->m_vecVelocity.z = zVel;
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

    if (m_pPlayer->m_flRemainingHaste < 0 || m_pPlayer->m_flRemainingHaste > gpGlobals->curtime)
    {
        wishspd *= 1.3;
    }

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
    Vector wishvel, clampedWishvel;
    float fmove, smove;
    float clampedFmove, clampedSmove;
    Vector wishdir;
    float wishspeed, clampedWishspeed;
    Vector forward, right, up;

    float realAcceleration;
    float realMaxSpeed;
    float realWishspeed;
    bool doAircontrol = false;

    float oldSpeed;
    Vector oldVel;

    if (mv->m_bRampSliding && sv_rampslide_jumps.GetBool())
    {
        DFCheckJumpButton();
    }

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    // Copy movement amounts
    fmove = mv->m_flForwardMove;
    smove = mv->m_flSideMove;
    // clamped to 127 for vq3
    clampedFmove = clamp(fmove, -127, 127);
    clampedSmove = clamp(smove, -127, 127);

    // Zero out z components of movement vectors
    forward[2] = 0;
    right[2] = 0;
    VectorNormalize(forward); // Normalize remainder of vectors
    VectorNormalize(right);

    for (i = 0; i < 2; i++) // Determine x and y parts of velocity
    {
        wishvel[i] = forward[i] * fmove + right[i] * smove;
        clampedWishvel[i] = forward[i] * clampedFmove + right[i] * clampedSmove;
    }
    wishvel[2] = 0; // Zero out z part of velocity
    clampedWishvel[2] = 0;

    VectorCopy(wishvel, wishdir); // Determine magnitude of speed of move
    wishspeed = VectorNormalize(wishdir);
    clampedWishspeed = clampedWishvel.Length2D();

    if (sv_aircontrol_enable.GetBool() && !(smove > 0.1 || smove < -0.1) && (fmove > 0.1 || fmove < -0.1))
    {
        doAircontrol = true;
    }

    if (sv_strafestyle.GetInt() == 3 && wishdir.Length() > 0.1)
    {
        Vector vel2D;
        VectorCopy(mv->m_vecVelocity, vel2D);
        vel2D[2] = 0;
        double angle = acos(DotProduct(wishdir, vel2D) / (wishdir.Length() * vel2D.Length2D()));
        angle *= (180 / M_PI);
        double minQWAngle = acos(sv_maxairstrafespeed.GetFloat() / mv->m_vecVelocity.Length2D());
        double minQ3Angle = acos(sv_maxairspeed.GetFloat() / mv->m_vecVelocity.Length2D());
        minQWAngle *= (180 / M_PI);
        minQ3Angle *= (180 / M_PI);
        
        if (angle < minQWAngle)
        {
            realAcceleration = sv_airaccelerate.GetFloat();
            realMaxSpeed = sv_maxairspeed.GetFloat();
            realWishspeed = clampedWishspeed * DFScale(realMaxSpeed);
        }
        else
        {
            realAcceleration = sv_airstrafeaccelerate.GetFloat();
            realMaxSpeed = sv_maxairstrafespeed.GetFloat();
            realWishspeed = wishspeed;
        }
    }
    else if (sv_strafestyle.GetInt() == 2)
    {
        if ((smove > 0.1 || smove < -0.1) && !(fmove > 0.1 || fmove < -0.1))
        {
            realAcceleration = sv_airstrafeaccelerate.GetFloat();
            realMaxSpeed = sv_maxairstrafespeed.GetFloat();
            realWishspeed = wishspeed;
        }
        else
        {
            realAcceleration = sv_airaccelerate.GetFloat();
            realMaxSpeed = sv_maxairspeed.GetFloat();
            realWishspeed = clampedWishspeed * DFScale(realMaxSpeed);
        }
    }
    else if (sv_strafestyle.GetInt() == 1)
    {
        realAcceleration = sv_airstrafeaccelerate.GetFloat();
        realMaxSpeed = sv_maxairstrafespeed.GetFloat();
        realWishspeed = wishspeed;
    }
    else
    {
        realAcceleration = sv_airaccelerate.GetFloat();
        realMaxSpeed = sv_maxairspeed.GetFloat();
        realWishspeed = clampedWishspeed * DFScale(realMaxSpeed);
    }

    DFAirAccelerate(wishdir, realWishspeed, realAcceleration, realMaxSpeed);

    if (doAircontrol)
    {
        DFAirControl(wishdir, realWishspeed);
    }

    // figure in gravity but only if not rampsliding w/o gravity, and not slick
    if (!(mv->m_bRampSliding && !sv_rampslide_gravity.GetBool()) && m_pPlayer->GetGroundEntity() == nullptr)
    {
        mv->m_vecVelocity[2] -= sv_gravity.GetFloat() * gpGlobals->frametime;
    }

    oldSpeed = mv->m_vecVelocity.Length2D();
    VectorCopy(mv->m_vecVelocity, oldVel);

    VectorCopy(mv->m_vecVelocity, mv->m_vecPreviousVelocity);
    DFStepSlideMove(true);
}