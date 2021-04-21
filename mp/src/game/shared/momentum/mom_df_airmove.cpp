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

    float realAcceleration;
    float realMaxSpeed;

    float oldSpeed;
    Vector oldVel;

    AngleVectors(mv->m_vecViewAngles, &forward, &right, &up); // Determine movement angles

    // Copy movement amounts
    fmove = clamp(mv->m_flForwardMove, -127, 127);
    smove = clamp(mv->m_flSideMove, -127, 127);

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

    if (sv_differentialstrafing.GetBool() && wishdir.Length() > 0.1)
    {
        double angle = acos(DotProduct(wishdir, mv->m_vecVelocity) / (wishdir.Length() * mv->m_vecVelocity.Length2D()));
        angle *= (180 / 3.14159265);
        double minQWAngle = acos((wishspeed * DFScale(sv_maxairstrafespeed.GetFloat())) / mv->m_vecVelocity.Length2D());
        minQWAngle *= (180 / 3.14159265);

        if (angle <= minQWAngle)
        {
            realAcceleration = sv_airaccelerate.GetFloat();
            realMaxSpeed = sv_maxairspeed.GetFloat();
        }
        else
        {
            realAcceleration = sv_airstrafeaccelerate.GetFloat();
            realMaxSpeed = sv_maxairstrafespeed.GetFloat();
        }
    }
    else if (sv_cpm_physics.GetBool())
    {
        if ((smove > 0.1 || smove < -0.1) && !(fmove > 0.1 || fmove < -0.1))
        {
            realAcceleration = sv_airstrafeaccelerate.GetFloat();
            realMaxSpeed = sv_maxairstrafespeed.GetFloat();
        }
        else
        {
            realAcceleration = sv_airaccelerate.GetFloat();
            realMaxSpeed = sv_maxairspeed.GetFloat();
        }
    }
    else
    {
        realAcceleration = sv_airaccelerate.GetFloat();
        realMaxSpeed = sv_maxairspeed.GetFloat();
    }

    // less accel if holding jump for too long
    if (mv->m_flJumpHoldTime > 0 && gpGlobals->curtime - mv->m_flJumpHoldTime > sv_airdeceleratetime.GetFloat())
    {
        realAcceleration *= (1 - sv_airdecelerate.GetFloat());
    }

    wishspeed *= DFScale(realMaxSpeed);

    DFAirAccelerate(wishdir, wishspeed, realAcceleration, realMaxSpeed);

    if (sv_cpm_physics.GetBool() && !sv_differentialstrafing.GetBool())
    {
        if (!(smove > 0.1 || smove < -0.1) && (fmove > 0.1 || fmove < -0.1))
        {
            DFAirControl(wishdir, wishspeed);
        }
    }

    oldSpeed = mv->m_vecVelocity.Length2D();
    VectorCopy(mv->m_vecVelocity, oldVel);

    DFStepSlideMove(true);

    if (gpGlobals->curtime - mv->m_flWallClipTime < sv_wallcliptime.GetFloat() &&
        oldSpeed > mv->m_vecVelocity.Length2D())
    {
        VectorCopy(oldVel, mv->m_vecVelocity);
    }
}