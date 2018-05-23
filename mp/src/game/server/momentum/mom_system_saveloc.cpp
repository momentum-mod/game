#include "cbase.h"
#include "mom_system_saveloc.h"
#include "mom_timer.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "ghost_client.h"

#include "tier0/memdbgon.h"

#define SAVELOC_FILE_NAME "savedlocs.txt"

MAKE_TOGGLE_CONVAR(mom_saveloc_save_between_sessions, "1", FCVAR_ARCHIVE, "Defines if savelocs should be saved between sessions of the same map.\n");

SavedLocation_t::SavedLocation_t(): crouched(false), pos(vec3_origin), vel(vec3_origin), ang(vec3_angle),
                                    gravityScale(1.0f), movementLagScale(1.0f), disabledButtons(0)
{
    targetName[0] = '\0';
    targetClassName[0] = '\0';
}

SavedLocation_t::SavedLocation_t(CMomentumPlayer* pPlayer)
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

void SavedLocation_t::Save(KeyValues* kvCP) const
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

void SavedLocation_t::Load(KeyValues* pKv)
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

void SavedLocation_t::Teleport(CMomentumPlayer* pPlayer)
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

void SavedLocation_t::Read(CUtlBuffer& mem)
{
    KeyValues *read = new KeyValues("From Someone");
    read->ReadAsBinary(mem);
    Load(read);
    read->deleteThis();
}

void SavedLocation_t::Write(CUtlBuffer& mem)
{
    KeyValues *test = new KeyValues("To Someone");
    Save(test);
    test->WriteAsBinary(mem);
    test->deleteThis();
}

CMOMSaveLocSystem::CMOMSaveLocSystem(const char* pName): CAutoGameSystem(pName)
{
    m_pSavedLocsKV = new KeyValues(pName);
    m_iRequesting = 0;

    g_pModuleComms->ListenForEvent("req_savelocs", this);
}

CMOMSaveLocSystem::~CMOMSaveLocSystem()
{
    if (m_pSavedLocsKV)
        m_pSavedLocsKV->deleteThis();
    m_pSavedLocsKV = nullptr;
}

void CMOMSaveLocSystem::LevelInitPreEntity()
{
    // We don't check mom_savelocs_save_between_sessions because we want to be able to load savelocs from friends
    DevLog("Loading savelocs from %s ...\n", SAVELOC_FILE_NAME);

    if (m_pSavedLocsKV)
    {
        //Remove the past loaded stuff
        m_pSavedLocsKV->Clear();

        // Note: This loading is going to contain all of the other maps' savelocs as well!
        // Note: We are not in PostInit because if players edit their savelocs file (add
        // savelocs from a friend or something), then we want to reload on map load again,
        // and not force the player to restart the mod every time.
        if (m_pSavedLocsKV->LoadFromFile(filesystem, SAVELOC_FILE_NAME, "MOD"))
        {
            DevLog("Loaded savelocs from %s!\n", SAVELOC_FILE_NAME);
        }
    }
}

void CMOMSaveLocSystem::LevelShutdownPreEntity()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && m_pSavedLocsKV && mom_saveloc_save_between_sessions.GetBool())
    {
        DevLog("Saving map %s savelocs to %s ...\n", gpGlobals->mapname.ToCStr(), SAVELOC_FILE_NAME);
        // Make the KV to save into and save into it
        KeyValues *pKvMapSavelocs = new KeyValues(gpGlobals->mapname.ToCStr());
        pPlayer->SaveSavelocsToFile(pKvMapSavelocs);

        // Remove the map if it already exists in there
        KeyValues *pExisting = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
        if (pExisting)
            m_pSavedLocsKV->RemoveSubKey(pExisting);

        // Add the new one
        m_pSavedLocsKV->AddSubKey(pKvMapSavelocs);

        // Save everything to file
        if (m_pSavedLocsKV->SaveToFile(filesystem, SAVELOC_FILE_NAME, "MOD", true))
            DevLog("Saved map %s savelocs to %s!\n", gpGlobals->mapname.ToCStr(), SAVELOC_FILE_NAME);
    }

    // Remove all requesters if we had any
    m_vecRequesters.RemoveAll();
}

void CMOMSaveLocSystem::FireEvent(KeyValues* pKv)
{
    if (FStrEq(pKv->GetName(), "req_savelocs"))
    {
        int stage = pKv->GetInt("stage");
        CSteamID target(pKv->GetUint64("target"));
        if (stage == 1)
        {
            // They clicked "request savelocs" from a player, UI just opened, get the count to send back
            SavelocReqPacket_t packet;
            packet.stage = 1;
            g_pMomentumGhostClient->SendSavelocReqPacket(target, &packet);

            // Also keep track of this in the saveloc system
            SetRequestingSavelocsFrom(target.ConvertToUint64());
        }
        else if (stage == 3)
        {
            // They clicked on the # of savelocs to request, build a request packet and get these bad boys
            SavelocReqPacket_t packet;
            packet.stage = 3;
            packet.saveloc_count = pKv->GetInt("count");
            packet.dataBuf.CopyBuffer(pKv->GetPtr("nums"), sizeof(int) * packet.saveloc_count);
            g_pMomentumGhostClient->SendSavelocReqPacket(target, &packet);
        }
        else if (stage == -3)
        {
            // The player clicked cancel
            SetRequestingSavelocsFrom(0);

            // Let our requestee know
            SavelocReqPacket_t packet;
            packet.stage = -1;
            g_pMomentumGhostClient->SendSavelocReqPacket(target, &packet);
        }
    }
}

// Called from player spawn because LevelInitPostEntity is annoying and doesn't allow
// to cast to a CMomentumPlayer. 
void CMOMSaveLocSystem::LoadMapSaveLocs(CMomentumPlayer* pPlayer) const
{
    if (pPlayer && m_pSavedLocsKV && !m_pSavedLocsKV->IsEmpty())
    {
        KeyValues *kvMapSavelocs = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
        if (kvMapSavelocs)
        {
            pPlayer->LoadSavelocsFromFile(kvMapSavelocs);
        }
    }
}

void CMOMSaveLocSystem::AddSavelocRequester(const uint64& newReq)
{
    if (m_vecRequesters.Find(newReq) != m_vecRequesters.InvalidIndex())
        m_vecRequesters.AddToTail(newReq);
}

int CMOMSaveLocSystem::GetSavelocCount()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
        return pPlayer->GetSavelocCount();

    return 0;
}

void CMOMSaveLocSystem::RequesterLeft(const uint64& requester)
{
    // The person we are requesting savelocs from just left
    if (requester == m_iRequesting)
    {
        // Fire event for client, 
        KeyValues *pKv = new KeyValues("req_savelocs");
        pKv->SetInt("stage", -2);
        g_pModuleComms->FireEvent(pKv);

        m_iRequesting = 0;
    }

    // If they were a requester to us (as well), remove them
    m_vecRequesters.FindAndFastRemove(requester);
}

void CMOMSaveLocSystem::SetRequestingSavelocsFrom(const uint64& from)
{
    if (m_iRequesting && from != 0)
    {
        DevWarning("We're already requesting savelocs from %llu!!!\n", m_iRequesting);
        return;
    }

    m_iRequesting = from;
}

bool CMOMSaveLocSystem::FillSavelocReq(const bool sending, SavelocReqPacket_t *input, SavelocReqPacket_t *outputBuf)
{
    // Get the local player, cancel if we can't get at him
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (!pPlayer)
        return false;

    if (sending)
    {
        // Go through each requested saveloc and copy the binary into outputBuf
        for (int i = 0; i < input->saveloc_count; i++)
        {
            int requested = input->dataBuf.GetInt();

            SavedLocation_t *savedLoc = pPlayer->GetSaveloc(requested);
            if (savedLoc)
            {
                savedLoc->Write(outputBuf->dataBuf);
            }
        }
    }
    else // receiving savelocs
    {
        // They gave us their savelocs, read them and add them to the player
        for (int i = 0; i < input->saveloc_count; i++)
        {
            // Memory managed by the player, when they exit the level
            SavedLocation_t *newSavedLoc = new SavedLocation_t;
            newSavedLoc->Read(input->dataBuf);
            pPlayer->AddSaveloc(newSavedLoc);
        }
    }

    return true;
}

inline void CheckTimer(CMomentumPlayer *pPlayer)
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

    pPlayer->SetUsingSavelocMenu(true);
}

CON_COMMAND_F(mom_saveloc_create, "Creates a saveloc that saves a player's state.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->CreateAndSaveLocation();
    }
}

CON_COMMAND_F(mom_saveloc_current, "Teleports the player to their current saved location.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        CheckTimer(pPlayer);
        pPlayer->TeleportToCurrentSaveloc();
    }
}
CON_COMMAND_F(mom_saveloc_nav_next, "Goes forwards through the saveloc list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && pPlayer->GetSavelocCount() > 0)
    {
        CheckTimer(pPlayer);
        pPlayer->SetCurrentSavelocMenuIndex((pPlayer->GetCurrentSavelocMenuIndex() + 1) % pPlayer->GetSavelocCount());
        pPlayer->TeleportToCurrentSaveloc();
    }
}
CON_COMMAND_F(mom_saveloc_nav_prev, "Goes backwards through the saveloc list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer && pPlayer->GetSavelocCount() > 0)
    {
        CheckTimer(pPlayer);
        pPlayer->SetCurrentSavelocMenuIndex(pPlayer->GetCurrentSavelocMenuIndex() == 0 ? pPlayer->GetSavelocCount() - 1 : pPlayer->GetCurrentSavelocMenuIndex() - 1);
        pPlayer->TeleportToCurrentSaveloc();
    }
}
CON_COMMAND_F(mom_saveloc_remove_current, "Removes the previously created saveloc.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        CheckTimer(pPlayer);
        pPlayer->RemoveLastSaveloc();
    }
}
CON_COMMAND_F(mom_saveloc_remove_all, "Removes all of the created savelocs for this map.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        CheckTimer(pPlayer);
        pPlayer->RemoveAllSavelocs();
    }
}
CON_COMMAND_F(mom_saveloc_close, "Closes the saveloc menu.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->SetUsingSavelocMenu(false);
    }
}

//Expose this to the DLL
static CMOMSaveLocSystem s_MOMSavelocSystem("MOMSavelocSystem");
CMOMSaveLocSystem *g_pMOMSavelocSystem = &s_MOMSavelocSystem;