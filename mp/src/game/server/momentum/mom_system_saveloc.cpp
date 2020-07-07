#include "cbase.h"
#include "filesystem.h"
#include "mom_system_saveloc.h"
#include "mom_timer.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "ghost_client.h"
#include "mom_modulecomms.h"
#include "mom_ghostdefs.h"
#include "mom_player_shared.h"
#include "fmtstr.h"
#include "run/mom_run_safeguards.h"

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
    g_EventQueue.SaveForTarget(pPlayer, entEventsState);
}

void SavedLocation_t::Save(KeyValues* kvCP) const
{
    kvCP->SetString("targetName", targetName);
    kvCP->SetString("targetClassName", targetClassName);
    MomUtil::KVSaveVector(kvCP, "vel", vel);
    MomUtil::KVSaveVector(kvCP, "pos", pos);
    MomUtil::KVSaveQAngles(kvCP, "ang", ang);
    kvCP->SetBool("crouched", crouched);
    kvCP->SetFloat("gravityScale", gravityScale);
    kvCP->SetFloat("movementLagScale", movementLagScale);
    kvCP->SetInt("disabledButtons", disabledButtons);
    entEventsState.SaveToKeyValues(kvCP);
}

void SavedLocation_t::Load(KeyValues* pKv)
{
    Q_strncpy(targetName, pKv->GetString("targetName"), sizeof(targetName));
    Q_strncpy(targetClassName, pKv->GetString("targetClassName"), sizeof(targetClassName));
    MomUtil::KVLoadVector(pKv, "pos", pos);
    MomUtil::KVLoadVector(pKv, "vel", vel);
    MomUtil::KVLoadQAngles(pKv, "ang", ang);
    crouched = pKv->GetBool("crouched");
    gravityScale = pKv->GetFloat("gravityScale", 1.0f);
    movementLagScale = pKv->GetFloat("movementLagScale", 1.0f);
    disabledButtons = pKv->GetInt("disabledButtons");
    entEventsState.LoadFromKeyValues(pKv);
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

    // Restore entity event queue state
    g_EventQueue.RestoreForTarget(pPlayer, entEventsState);
}

bool SavedLocation_t::Read(CUtlBuffer &mem)
{
    KeyValuesAD read("From Someone");
    if (read->ReadAsBinary(mem))
    {
        Load(read);
        return true;
    }

    return false;
}

bool SavedLocation_t::Write(CUtlBuffer &mem)
{
    KeyValuesAD write("To Someone");
    Save(write);
    return write->WriteAsBinary(mem);
}

CMOMSaveLocSystem::CMOMSaveLocSystem(const char* pName): CAutoGameSystem(pName)
{
    m_pSavedLocsKV = new KeyValues(pName);
    m_iRequesting = 0;
    m_iCurrentSavelocIndx = -1;
    m_bUsingSavelocMenu = false;
}

CMOMSaveLocSystem::~CMOMSaveLocSystem()
{
    if (m_pSavedLocsKV)
        m_pSavedLocsKV->deleteThis();
    m_pSavedLocsKV = nullptr;
}

void CMOMSaveLocSystem::PostInit()
{
    g_pModuleComms->ListenForEvent("req_savelocs", UtlMakeDelegate(this, &CMOMSaveLocSystem::OnSavelocRequestEvent));
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

            if (!m_pSavedLocsKV->IsEmpty())
            {
                KeyValues *kvMapSavelocs = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
                if (kvMapSavelocs && !kvMapSavelocs->IsEmpty())
                {
                    m_iCurrentSavelocIndx = kvMapSavelocs->GetInt("cur");

                    KeyValues *kvCPs = kvMapSavelocs->FindKey("cps");
                    if (!kvCPs) return;
                    FOR_EACH_SUBKEY(kvCPs, kvCheckpoint)
                    {
                        SavedLocation_t *c = new SavedLocation_t();
                        c->Load(kvCheckpoint);
                        m_rcSavelocs.AddToTail(c);
                    }

                    // Fire the initial event
                    FireUpdateEvent();
                }
            }
        }
    }
}

void CMOMSaveLocSystem::LevelShutdownPreEntity()
{
    if (CMomentumPlayer::GetLocalPlayer() && m_pSavedLocsKV && mom_saveloc_save_between_sessions.GetBool())
    {
        DevLog("Saving map %s savelocs to %s ...\n", gpGlobals->mapname.ToCStr(), SAVELOC_FILE_NAME);
        // Make the KV to save into and save into it
        KeyValues *pKvMapSavelocs = new KeyValues(gpGlobals->mapname.ToCStr());
        // Set the current index
        pKvMapSavelocs->SetInt("cur", m_iCurrentSavelocIndx);

        // Add all your savelocs
        KeyValues *kvCPs = new KeyValues("cps");
        FOR_EACH_VEC(m_rcSavelocs, i)
        {
            char szCheckpointNum[10]; // 999 million savelocs is pretty generous
            Q_snprintf(szCheckpointNum, sizeof(szCheckpointNum), "%09i", i); // %09 because '\0' is the last (10)
            KeyValues *kvCP = new KeyValues(szCheckpointNum);
            m_rcSavelocs[i]->Save(kvCP);
            kvCPs->AddSubKey(kvCP);
        }

        // Save them into the keyvalues
        pKvMapSavelocs->AddSubKey(kvCPs);
       
        // Remove the map if it already exists in there
        KeyValues *pExisting = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
        if (pExisting)
        {
            m_pSavedLocsKV->RemoveSubKey(pExisting);
            pExisting->deleteThis();
        }

        // Add the new one
        m_pSavedLocsKV->AddSubKey(pKvMapSavelocs);

        // Save everything to file
        if (m_pSavedLocsKV->SaveToFile(filesystem, SAVELOC_FILE_NAME, "MOD", true))
            DevLog("Saved map %s savelocs to %s!\n", gpGlobals->mapname.ToCStr(), SAVELOC_FILE_NAME);
    }

    // Remove all requesters if we had any
    m_vecRequesters.RemoveAll();
    m_bUsingSavelocMenu = false;
    RemoveAllSavelocs();
}

void CMOMSaveLocSystem::OnSavelocRequestEvent(KeyValues* pKv)
{
    int stage = pKv->GetInt("stage", SAVELOC_REQ_STAGE_INVALID);
    CSteamID target(pKv->GetUint64("target"));
    if (stage == SAVELOC_REQ_STAGE_COUNT_REQ)
    {
        // They clicked "request savelocs" from a player, UI just opened, get the count to send back
        SavelocReqPacket packet;
        packet.stage = SAVELOC_REQ_STAGE_COUNT_REQ;
        if (g_pMomentumGhostClient->SendSavelocReqPacket(target, &packet))
        {
            // Also keep track of this in the saveloc system
            SetRequestingSavelocsFrom(target.ConvertToUint64());
        }

    }
    else if (stage == SAVELOC_REQ_STAGE_SAVELOC_REQ)
    {
        SavelocReqPacket packet;
        packet.stage = SAVELOC_REQ_STAGE_SAVELOC_REQ;
        packet.saveloc_count = pKv->GetInt("count");
        packet.dataBuf.CopyBuffer(pKv->GetPtr("nums"), sizeof(int) * packet.saveloc_count);
        g_pMomentumGhostClient->SendSavelocReqPacket(target, &packet);
    }
    else if (stage == SAVELOC_REQ_STAGE_CLICKED_CANCEL)
    {
        SetRequestingSavelocsFrom(0);

        // Let our requestee know
        SavelocReqPacket packet;
        packet.stage = SAVELOC_REQ_STAGE_DONE;
        g_pMomentumGhostClient->SendSavelocReqPacket(target, &packet);
    }
}

bool CMOMSaveLocSystem::AddSavelocRequester(const uint64& newReq)
{
    if (m_vecRequesters.HasElement(newReq))
        return false;

    m_vecRequesters.AddToTail(newReq);
    return true;
}

void CMOMSaveLocSystem::RequesterLeft(const uint64& requester)
{
    // The person we are requesting savelocs from just left
    if (requester == m_iRequesting)
    {
        // Fire event for client
        KeyValues *pKv = new KeyValues("req_savelocs");
        pKv->SetInt("stage", SAVELOC_REQ_STAGE_REQUESTER_LEFT);
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

bool CMOMSaveLocSystem::WriteRequestedSavelocs(SavelocReqPacket *input, SavelocReqPacket *output, const uint64 &requester)
{
    if (!m_vecRequesters.HasElement(requester))
        return false;

    int count = 0;
    for (int i = 0; i < input->saveloc_count && input->dataBuf.IsValid(); i++)
    {
        const auto requestedIndex = input->dataBuf.GetInt();

        const auto savedLoc = GetSaveloc(requestedIndex);
        if (savedLoc)
        {
            if (!savedLoc->Write(output->dataBuf))
                return false;

            count++;
        }
    }

    if (count == 0)
        return false;

    // We set the count here because we may have a different number of savelocs than requested
    // (eg. when we delete some savelocs while the packet to request the original amount/indicies is still live)
    output->saveloc_count = count;

    return true;
}

bool CMOMSaveLocSystem::ReadReceivedSavelocs(SavelocReqPacket *input, const uint64 &sender)
{
    if (sender != m_iRequesting)
        return false;

    if (input->saveloc_count > 0)
    {
        for (int i = 0; i < input->saveloc_count && input->dataBuf.IsValid(); i++)
        {
            auto newSavedLoc = new SavedLocation_t;
            if (newSavedLoc->Read(input->dataBuf))
            {
                m_rcSavelocs.AddToTail(newSavedLoc);
            }
            else
            {
                delete newSavedLoc;
                return false;
            }
        }

        FireUpdateEvent();
        UpdateRequesters();

        return true;
    }

    return false;
}

SavedLocation_t* CMOMSaveLocSystem::CreateSaveloc()
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    return pPlayer ? new SavedLocation_t(pPlayer) : nullptr;
}

void CMOMSaveLocSystem::CreateAndSaveLocation()
{
    SavedLocation_t *created = CreateSaveloc();
    if (created)
    {
        AddSaveloc(created);
        ClientPrint(CMomentumPlayer::GetLocalPlayer(), HUD_PRINTTALK, CFmtStr("Saveloc #%i created!", m_rcSavelocs.Count()));
        UpdateRequesters();
    }
}

void CMOMSaveLocSystem::AddSaveloc(SavedLocation_t* saveloc)
{
    if (!saveloc)
        return;

    auto priorCount = m_rcSavelocs.Count();
    m_rcSavelocs.AddToTail(saveloc);
    if (m_iCurrentSavelocIndx == priorCount - 1)
        ++m_iCurrentSavelocIndx;
    else
        m_iCurrentSavelocIndx = priorCount; // Set it to the new checkpoint's index

    FireUpdateEvent();
}

void CMOMSaveLocSystem::RemoveCurrentSaveloc()
{
    if (m_rcSavelocs.IsEmpty())
        return;

    auto prevCount = m_rcSavelocs.Count();
    m_rcSavelocs.PurgeAndDeleteElement(m_iCurrentSavelocIndx);
    // If there's one element left, we still need to decrease currentStep to -1
    if (m_iCurrentSavelocIndx == prevCount - 1)
        --m_iCurrentSavelocIndx;
    // else we want it to shift forward one until it catches back up to the last checkpoint

    FireUpdateEvent();
    UpdateRequesters();
}

void CMOMSaveLocSystem::RemoveAllSavelocs()
{
    m_rcSavelocs.PurgeAndDeleteElements();
    m_iCurrentSavelocIndx = -1;

    FireUpdateEvent();
    UpdateRequesters();
}

void CMOMSaveLocSystem::GotoNextSaveloc()
{
    auto count = GetSavelocCount();
    if (count > 0)
    {
        SetCurrentSavelocMenuIndex((m_iCurrentSavelocIndx + 1) % count);
        TeleportToCurrentSaveloc();
    }
}

void CMOMSaveLocSystem::GotoFirstSaveloc()
{
    if (!m_rcSavelocs.IsEmpty())
    {
        SetCurrentSavelocMenuIndex(0);
        TeleportToCurrentSaveloc();
    }
}

void CMOMSaveLocSystem::GotoLastSaveloc()
{
    auto count = GetSavelocCount();
    if (count > 0)
    {
        SetCurrentSavelocMenuIndex(count - 1);
        TeleportToCurrentSaveloc();
    }
}

void CMOMSaveLocSystem::GotoPrevSaveloc()
{
    auto count = GetSavelocCount();
    if (count > 0)
    {
        SetCurrentSavelocMenuIndex(m_iCurrentSavelocIndx == 0 ? count - 1 : m_iCurrentSavelocIndx - 1);
        TeleportToCurrentSaveloc();
    }
}

void CMOMSaveLocSystem::TeleportToSavelocIndex(int indx)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (indx < 0 || indx > m_rcSavelocs.Count() || !pPlayer || !pPlayer->AllowUserTeleports())
        return;
    // Check if the timer is running and if we should stop it
    CheckTimer();
    m_rcSavelocs[indx]->Teleport(pPlayer);
}

void CMOMSaveLocSystem::TeleportToCurrentSaveloc()
{
    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_SAVELOC_TELE))
        return; 

    TeleportToSavelocIndex(m_iCurrentSavelocIndx);
    FireUpdateEvent();
}

void CMOMSaveLocSystem::SetUsingSavelocMenu(bool bIsUsingSLMenu)
{
    m_bUsingSavelocMenu = bIsUsingSLMenu;
    FireUpdateEvent();
}

void CMOMSaveLocSystem::CheckTimer()
{
    if (g_pMomentumTimer->IsRunning())
    {

        g_pMomentumTimer->Stop(CMomentumPlayer::GetLocalPlayer());

        // MOM_TODO: consider
        // 1. having a local timer running, as people may want to time their routes they're using CP menu for
        // 2. gamemodes (KZ) where this is allowed
    }

    m_bUsingSavelocMenu = true;
}

void CMOMSaveLocSystem::FireUpdateEvent() const
{
    const auto pEvent = gameeventmanager->CreateEvent("saveloc_upd8");
    if (pEvent)
    {
        pEvent->SetInt("count", m_rcSavelocs.Count());
        pEvent->SetInt("current", m_iCurrentSavelocIndx);
        pEvent->SetBool("using", m_bUsingSavelocMenu);
        gameeventmanager->FireEvent(pEvent);
    }
}

void CMOMSaveLocSystem::UpdateRequesters()
{
    if (m_vecRequesters.IsEmpty())
        return;

    // Send them our saveloc count
    SavelocReqPacket response;
    response.stage = SAVELOC_REQ_STAGE_COUNT_ACK;
    response.saveloc_count = m_rcSavelocs.Count();

    FOR_EACH_VEC(m_vecRequesters, i)
    {
        CSteamID requester(m_vecRequesters[i]);
        g_pMomentumGhostClient->SendSavelocReqPacket(requester, &response);
    }
}

CON_COMMAND_F(mom_saveloc_create, "Creates a saveloc that saves a player's state.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->CreateAndSaveLocation();
}
CON_COMMAND_F(mom_saveloc_current, "Teleports the player to their current saved location.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->TeleportToCurrentSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_next, "Goes forwards through the saveloc list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->GotoNextSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_prev, "Goes backwards through the saveloc list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->GotoPrevSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_first, "Goes to the first saveloc in the list, teleporting the player to it.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->GotoFirstSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_last, "Goes to the last saveloc in the list, teleporting the player to it.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->GotoLastSaveloc();
}
CON_COMMAND_F(mom_saveloc_remove_current, "Removes the current saveloc.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->RemoveCurrentSaveloc();
}
CON_COMMAND_F(mom_saveloc_remove_all, "Removes all of the created savelocs for this map.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->RemoveAllSavelocs();
}
CON_COMMAND_F(mom_saveloc_close, "Closes the saveloc menu.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pMOMSavelocSystem->SetUsingSavelocMenu(false);
}

//Expose this to the DLL
static CMOMSaveLocSystem s_MOMSavelocSystem("MOMSavelocSystem");
CMOMSaveLocSystem *g_pMOMSavelocSystem = &s_MOMSavelocSystem;
