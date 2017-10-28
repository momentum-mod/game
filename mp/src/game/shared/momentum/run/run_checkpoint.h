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
    float gravityScale;
    float movementLagScale;
    int disabledButtons;

    Checkpoint_t() : crouched(false), pos(vec3_origin), vel(vec3_origin), ang(vec3_angle), gravityScale(1.0f), movementLagScale(1.0f), disabledButtons(0)
    {
        targetName[0] = '\0';
        targetClassName[0] = '\0';
    }

    // Called when loading this checkpoint from file
    Checkpoint_t(KeyValues *pKv)
    {
        Q_strncpy(targetName, pKv->GetString("targetName"), sizeof(targetName));
        Q_strncpy(targetClassName, pKv->GetString("targetClassName"), sizeof(targetClassName));
        g_pMomentumUtil->KVLoadVector(pKv, "pos", pos);
        g_pMomentumUtil->KVLoadVector(pKv, "vel", vel);
        g_pMomentumUtil->KVLoadQAngles(pKv, "ang", ang);
        crouched = pKv->GetBool("crouched");
        gravityScale = pKv->GetFloat("gravityScale", 1.0f);
        movementLagScale = pKv->GetFloat("movementLagScale", 1.0f);
        disabledButtons = pKv->GetInt("disabledButtons");
    }

    // Called when the player creates a checkpoint
    Checkpoint_t(CMomentumPlayer *pPlayer)
    {
        Q_strncpy(targetName, pPlayer->GetEntityName().ToCStr(), sizeof(targetName));
        Q_strncpy(targetClassName, pPlayer->GetClassname(), sizeof(targetClassName));
        vel = pPlayer->GetAbsVelocity(); 
        pos = pPlayer->GetAbsOrigin();
        ang = pPlayer->GetAbsAngles();
        crouched = pPlayer->m_Local.m_bDucked || pPlayer->m_Local.m_bDucking;
        gravityScale = pPlayer->GetGravity();
        movementLagScale = pPlayer->GetLaggedMovementValue();
        disabledButtons = pPlayer->m_afButtonDisabled.Get();
    }

    // Called when saving the checkpoint to file
    void Save(KeyValues *kvCP) const
    {
        kvCP->SetString("targetName", targetName);
        kvCP->SetString("targetClassName", targetClassName);
        g_pMomentumUtil->KVSaveVector(kvCP, "vel", vel);
        g_pMomentumUtil->KVSaveVector(kvCP, "pos", pos);
        g_pMomentumUtil->KVSaveQAngles(kvCP, "ang", ang);
        kvCP->SetBool("crouched", crouched);
        kvCP->SetFloat("gravityScale", gravityScale);
        kvCP->SetFloat("movementLagScale", movementLagScale);
        kvCP->SetInt("disabledButtons", disabledButtons);
    }

    // Called when the player wants to teleport to this checkpoint 
    void Teleport(CMomentumPlayer *pPlayer)
    {
        // Handle custom ent flags that old maps do
        pPlayer->SetName(MAKE_STRING(targetName));
        pPlayer->SetClassname(targetClassName);

        // Handle the crouched state
        if (crouched && !pPlayer->m_Local.m_bDucked)
            pPlayer->ToggleDuckThisFrame(true);
        else if (!crouched && pPlayer->m_Local.m_bDucked)
            pPlayer->ToggleDuckThisFrame(false);

        // Teleport the player
        pPlayer->Teleport(&pos, &ang, &vel);

        // Handle miscellaneous states like gravity and speed
        pPlayer->SetGravity(gravityScale);
        pPlayer->DisableButtons(disabledButtons);
        pPlayer->SetLaggedMovementValue(movementLagScale);
    }
};