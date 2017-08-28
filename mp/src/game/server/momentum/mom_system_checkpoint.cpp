#include "cbase.h"
#include "mom_system_checkpoint.h"
#include "mom_timer.h"
#include "mom_shareddefs.h"
#include "filesystem.h"

#include "tier0/memdbgon.h"

#define CHECKPOINTS_FILE_NAME "checkpoints.txt"

MAKE_TOGGLE_CONVAR(mom_checkpoint_save_between_sessions, "1", FCVAR_ARCHIVE, "Defines if checkpoints should be saved between sessions of the same map.\n");

void CMOMCheckpointSystem::LevelInitPreEntity()
{
    // We don't check mom_checkpoints_save_between_sessions because we want to be able to load checkpoints from friends
    DevLog("Loading checkpoints from %s ...\n", CHECKPOINTS_FILE_NAME);

    if (m_pCheckpointsKV)
    {
        //Remove the past loaded stuff
        m_pCheckpointsKV->Clear();

        // Note: This loading is going to contain all of the other maps' checkpoints as well!
        // Note: We are not in PostInit because if players edit their checkpoints file (add
        // checkpoints from a friend or something), then we want to reload on map load again,
        // and not force the player to restart the mod every time.
        if (m_pCheckpointsKV->LoadFromFile(filesystem, CHECKPOINTS_FILE_NAME, "MOD"))
        {
            DevLog("Loaded checkpoints from %s!\n", CHECKPOINTS_FILE_NAME);
        }
    }
}

void CMOMCheckpointSystem::LevelShutdownPreEntity()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && m_pCheckpointsKV && mom_checkpoint_save_between_sessions.GetBool())
    {
        DevLog("Saving map %s checkpoints to %s ...\n", gpGlobals->mapname.ToCStr(), CHECKPOINTS_FILE_NAME);
        // Make the KV to save into and save into it
        KeyValues *pKvMapCheckpoints = new KeyValues(gpGlobals->mapname.ToCStr());
        pPlayer->SaveCPsToFile(pKvMapCheckpoints);

        // Remove the map if it already exists in there
        KeyValues *pExisting = m_pCheckpointsKV->FindKey(gpGlobals->mapname.ToCStr());
        if (pExisting)
            m_pCheckpointsKV->RemoveSubKey(pExisting);

        // Add the new one
        m_pCheckpointsKV->AddSubKey(pKvMapCheckpoints);

        // Save everything to file
        if (m_pCheckpointsKV->SaveToFile(filesystem, CHECKPOINTS_FILE_NAME, "MOD", true))
            DevLog("Saved map %s checkpoints to %s!\n", gpGlobals->mapname.ToCStr(), CHECKPOINTS_FILE_NAME);
    }
}

// Called from player spawn because LevelInitPostEntity is annoying and doesn't allow
// to cast to a CMomentumPlayer. 
void CMOMCheckpointSystem::LoadMapCheckpoints(CMomentumPlayer* pPlayer) const
{
    if (pPlayer && m_pCheckpointsKV && !m_pCheckpointsKV->IsEmpty())
    {
        KeyValues *kvMapCheckpoints = m_pCheckpointsKV->FindKey(gpGlobals->mapname.ToCStr());
        if (kvMapCheckpoints)
        {
            pPlayer->LoadCPsFromFile(kvMapCheckpoints);
        }
    }
}

inline void CheckTimer()
{
    if (g_pMomentumTimer->IsRunning())
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
            g_pMomentumTimer->Stop(false);

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
        pPlayer->CreateAndSaveCheckpoint();
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
    if (pPlayer && pPlayer->GetCPCount() > 0)
    {
        pPlayer->SetCurrentCPMenuStep((pPlayer->GetCurrentCPMenuStep() + 1) % pPlayer->GetCPCount());
        pPlayer->TeleportToCurrentCP();
    }
}
CON_COMMAND_F(mom_checkpoint_nav_prev, "Goes backwards through the checkpoint list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CheckTimer();

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && pPlayer->GetCPCount() > 0)
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