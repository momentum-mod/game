#include "cbase.h"
#include "mom_system_checkpoint.h"
#include "Timer.h"

#include "tier0/memdbgon.h"



void CMOMCheckpointSystem::LevelInitPostEntity()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        //MOM_TODO: Load the checkpoints from file, only pass the given map's checkpoints to player
        DevLog("MOM_TODO: Loading checkpoints from %s.dat ...\n", gpGlobals->mapname.ToCStr());
    }
}

void CMOMCheckpointSystem::LevelShutdownPreEntity()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        //MOM_TODO: Save the checkpoints to file under the given map name


    }
}

inline void CheckTimer()
{
    if (g_Timer->IsRunning())
    {
        // MOM_TODO: consider
        // 1. having a local timer running, as people may want to time their routes they're using CP menu for
        // 2. gamemodes (KZ) where this is allowed

        ConVarRef gm("mom_gamemode");
        switch (gm.GetInt())
        {
        case MOMGM_SURF:
        case MOMGM_BHOP:
        case MOMGM_SCROLL:
            g_Timer->Stop(false);

            //case MOMGM_KZ:
        default:
            break;
        }
    }

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->SetUsingCPMenu(true);
    }
}

CON_COMMAND_F(mom_checkpoint_create, "Creates a checkpoint that saves a player's state.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->CreateCheckpoint();
    }
}

CON_COMMAND_F(mom_checkpoint_prev, "Teleports the player to their most recent checkpoint.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->TeleportToCurrentCP();
    }
}
CON_COMMAND_F(mom_checkpoint_nav_next, "Goes forwards through the checkpoint list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->SetCurrentCPMenuStep((pPlayer->GetCurrentCPMenuStep() + 1) % pPlayer->GetCPCount());
        pPlayer->TeleportToCurrentCP();
    }
}
CON_COMMAND_F(mom_checkpoint_nav_prev, "Goes backwards through the checkpoint list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->SetCurrentCPMenuStep(pPlayer->GetCurrentCPMenuStep() == 0 ? pPlayer->GetCPCount() - 1 : pPlayer->GetCurrentCPMenuStep() - 1);
        pPlayer->TeleportToCurrentCP();
    }
}
CON_COMMAND_F(mom_checkpoint_remove_prev, "Removes the previously created checkpoint.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->RemoveLastCheckpoint();
    }
}
CON_COMMAND_F(mom_checkpoint_remove_all, "Removes all of the created checkpoints for this map.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->RemoveAllCheckpoints();
    }
}
CON_COMMAND_F(mom_checkpoint_close, "Closes the checkpoint menu.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->SetUsingCPMenu(false);
    }
}

//Expose this to the DLL
static CMOMCheckpointSystem s_MOMCheckpointSystem("MOMCheckpointSystem");
CMOMCheckpointSystem *g_MOMCheckpointSystem = &s_MOMCheckpointSystem;