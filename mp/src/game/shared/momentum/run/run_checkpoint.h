#pragma once

#include "cbase.h"
#include "util/mom_util.h"

// Checkpoints used in the "Checkpoint menu"
struct Checkpoint_t
{
    bool crouched;
    Vector pos;
    Vector vel;
    QAngle ang;
    char targetName[512];
    char targetClassName[512];

    Checkpoint_t() : crouched(false), pos(vec3_origin), vel(vec3_origin), ang(vec3_angle)
    {
        targetName[0] = '\0';
        targetClassName[0] = '\0';
    }

    Checkpoint_t(KeyValues *pKv)
    {
        Q_strncpy(targetName, pKv->GetString("targetName"), sizeof(targetName));
        Q_strncpy(targetClassName, pKv->GetString("targetClassName"), sizeof(targetClassName));
        g_pMomentumUtil->KVLoadVector(pKv, "pos", pos);
        g_pMomentumUtil->KVLoadVector(pKv, "vel", vel);
        g_pMomentumUtil->KVLoadQAngles(pKv, "ang", ang);
        crouched = pKv->GetBool("crouched");
    }
};