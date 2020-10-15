#include "cbase.h"

#include "mom_system_tricks.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "util/mom_util.h"

#include "filesystem.h"
#include "fmtstr.h"

#ifdef CLIENT_DLL
#include "mom_map_cache.h"
#include "mom_api_requests.h"
#else
#include "momentum/mom_triggers.h"
#include "momentum/mapzones_build.h"
#include "momentum/mom_system_saveloc.h"
#endif

#include "tier0/memdbgon.h"

#ifdef GAME_DLL
CON_COMMAND_F(mom_tricks_record, "Start recording zones to make a trick.\n", FCVAR_MAPPING)
{
    g_pTrickSystem->StartRecording();
}

CON_COMMAND_F(mom_tricks_record_stop, "Stop recording zones and make a trick. Takes trick name as first param.\n", FCVAR_MAPPING)
{
    g_pTrickSystem->StopRecording(args.Arg(1));
}

CON_COMMAND_F(mom_tricks_map_tele, "Teleports to a specific map teleport.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    if (args.ArgC() <= 1)
    {
        Warning("Usage: \"mom_tricks_map_tele <num>\".\n");
        return;
    }

    g_pTrickSystem->GoToMapTeleport(Q_atoi(args.Arg(1)));
}

CON_COMMAND_F(mom_tricks_map_tele_create, "Creates a map teleport, takes a name as a parameter.\n", FCVAR_MAPPING | FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    if (args.ArgC() <= 1)
    {
        Warning("Usage: \"mom_tricks_map_tele_create \"<name>\".\n");
        return;
    }

    g_pTrickSystem->CreateMapTeleport(args.Arg(1));
}

CON_COMMAND_F(mom_tricks_track_trick, "Tracks a trick to complete, takes the trick ID as the parameter.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    if (args.ArgC() <= 1)
    {
        Warning("Usage: \"mom_tricks_track_trick <ID>\" .\n");
        return;
    }

    g_pTrickSystem->SetTrackedTrick(Q_atoi(args.Arg(1)));
}

CON_COMMAND_F(mom_tricks_tele_to_trick, "Teleports to a given trick start zone. Takes the trick ID as the parameter.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    if (args.ArgC() <= 1)
    {
        Warning("Usage: \"mom_tricks_tele_to_trick <ID>\" .\n");
        return;
    }

    g_pTrickSystem->TeleportToTrick(Q_atoi(args.Arg(1)));
}
#endif

TrickStepConstraint_MaxSpeed::TrickStepConstraint_MaxSpeed()
{
    m_flMaxSpeed = 410.0f;
}

bool TrickStepConstraint_MaxSpeed::PlayerPassesConstraint(CMomentumPlayer* pPlayer)
{
    return pPlayer->GetAbsVelocity().Length() <= m_flMaxSpeed;
}

void TrickStepConstraint_MaxSpeed::LoadFromKeyValues(KeyValues* pKvIn)
{
    m_flMaxSpeed = pKvIn->GetFloat("speed", 410.0f);
}

void TrickStepConstraint_MaxSpeed::SaveToKeyValues(KeyValues* pKvOut)
{
    pKvOut->SetFloat("speed", m_flMaxSpeed);
}

CTrickStep::CTrickStep()
{
    m_bOptional = false;
    m_iTrickZoneID = -1;
}

CTrickStep::~CTrickStep()
{
    m_vecConstraints.PurgeAndDeleteElements();
}

bool CTrickStep::PlayerPassesConstraints(CMomentumPlayer* pPlayer)
{
    FOR_EACH_VEC(m_vecConstraints, i)
    {
        if (!m_vecConstraints[i]->PlayerPassesConstraint(pPlayer))
            return false;
    }

    return true;
}

CTriggerTrickZone *CTrickStep::GetTrigger()
{
    return g_pTrickSystem->GetTrickZone(m_iTrickZoneID);
}

void CTrickStep::SaveToKV(KeyValues* pKvOut)
{
    pKvOut->SetBool("optional", m_bOptional);
    pKvOut->SetInt("zone", m_iTrickZoneID);

    if (!m_vecConstraints.IsEmpty())
    {
        const auto pConstraintsKv = new KeyValues("constraints");

        FOR_EACH_VEC(m_vecConstraints, i)
        {
            const auto pConstraintKV = pConstraintsKv->CreateNewKey();
            pConstraintKV->SetInt("type", m_vecConstraints[i]->GetType());

            const auto pConstraintDataKV = new KeyValues("data");
            m_vecConstraints[i]->SaveToKeyValues(pConstraintDataKV);
            pConstraintKV->AddSubKey(pConstraintDataKV);
        }

        pKvOut->AddSubKey(pConstraintsKv);
    }
}

void CTrickStep::LoadFromKV(KeyValues* pKvIn)
{
    m_bOptional = pKvIn->GetBool("optional");

    m_iTrickZoneID = pKvIn->GetInt("zone", -1);

    if (m_iTrickZoneID < 0)
    {
        Warning("Failed to load required trick zone from ID %i!\n", m_iTrickZoneID);
        Assert(false);
        return;
    }

    const auto pConstraintsKV = pKvIn->FindKey("constraints");
    if (pConstraintsKV && !pConstraintsKV->IsEmpty())
    {
        FOR_EACH_SUBKEY(pConstraintsKV, pConstraintKV)
        {
            ITrickStepConstraint *pConstraint = nullptr;
            const auto iType = pConstraintKV->GetInt("type", CONSTRAINT_INVALID);
            switch (iType)
            {
            case CONSTRAINT_SPEED_MAX:
                pConstraint = new TrickStepConstraint_MaxSpeed;
            default:
                break;
            }

            if (!pConstraint)
            {
                Warning("!!! Unknown trick constraint type %i !!!\n", iType);
                continue;
            }

            const auto pDataKV = pConstraintKV->FindKey("data");
            if (!pDataKV || pDataKV->IsEmpty())
            {
                Warning("!!! Invalid trick constraint data for type %i !!!\n", iType);
                delete pConstraint;
                continue;
            }

            pConstraint->LoadFromKeyValues(pDataKV); // MOM_TODO check for failure
            m_vecConstraints.AddToTail(pConstraint);
        }
    }
}

CTrickInfo::CTrickInfo()
{
    m_szCreationDate[0] = '\0';
    m_szName[0] = '\0';
    m_szCreatorName[0] = '\0';
    m_iDifficulty = -1;
}

CTrickInfo::~CTrickInfo()
{
    m_vecTags.PurgeAndDeleteElements();
}

void CTrickInfo::SaveToKV(KeyValues* pKvOut)
{
    const auto pInfoKV = new KeyValues("info");

    pInfoKV->SetInt("tier", m_iDifficulty);
    pInfoKV->SetString("name", m_szName);
    pInfoKV->SetString("creator", m_szCreatorName);
    pInfoKV->SetString("creation_date", m_szCreationDate);

    if (!m_vecTags.IsEmpty())
    {
        const auto pTagsKv = new KeyValues("tags");
        FOR_EACH_VEC(m_vecTags, i)
        {
            const auto pTag = m_vecTags[i];
            pTagsKv->SetString(CFmtStr("%i", pTag->m_iID), pTag->m_szTagName);
        }
        pInfoKV->AddSubKey(pTagsKv);
    }

    pKvOut->AddSubKey(pInfoKV);
}

void CTrickInfo::LoadFromKV(KeyValues* pKvIn)
{
    m_iDifficulty = pKvIn->GetInt("tier", -1);
    Q_strncpy(m_szName, pKvIn->GetString("name"), sizeof(m_szName));
    Q_strncpy(m_szCreatorName, pKvIn->GetString("creator"), sizeof(m_szCreatorName));
    Q_strncpy(m_szCreationDate, pKvIn->GetString("creation_date"), sizeof(m_szCreationDate));

    const auto pTagsKV = pKvIn->FindKey("tags");
    if (pTagsKV && !pTagsKV->IsEmpty())
    {
        FOR_EACH_VALUE(pTagsKV, pKvTag)
        {
            CTrickTag *pTag = new CTrickTag;
            Q_strncpy(pTag->m_szTagName, pKvTag->GetString(), sizeof(pTag->m_szTagName));
            pTag->m_iID = Q_atoi(pKvTag->GetName());

            m_vecTags.AddToTail(pTag);
        }
    }
}

CTrick::CTrick()
{
    m_iID = -1;
}

void CTrick::SetName(const char* pName)
{
    Q_strncpy(m_Info.m_szName, pName, sizeof(m_Info.m_szName));
}

CTrickStep* CTrick::Step(int iStepIndx)
{
    if (iStepIndx < 0 || iStepIndx >= m_vecSteps.Count())
        return nullptr;

    return m_vecSteps[iStepIndx];
}

void CTrick::SaveToKV(KeyValues* pKvOut)
{
    pKvOut->SetName(CFmtStr("%i", m_iID));

    m_Info.SaveToKV(pKvOut);

    KeyValues *pKvSteps = new KeyValues("steps");
    FOR_EACH_VEC(m_vecSteps, i)
    {
        const auto pKvStep = pKvSteps->CreateNewKey();
        m_vecSteps[i]->SaveToKV(pKvStep);
    }
    pKvOut->AddSubKey(pKvSteps);
}

bool CTrick::LoadFromKV(KeyValues* pKvIn)
{
    m_iID = Q_atoi(pKvIn->GetName());

    const auto pInfoKV = pKvIn->FindKey("info");
    if (!pInfoKV || pInfoKV->IsEmpty())
        return false;

    m_Info.LoadFromKV(pInfoKV);

    const auto pStepsKV = pKvIn->FindKey("steps");
    if (!pStepsKV || pStepsKV->IsEmpty())
        return false;

    FOR_EACH_SUBKEY(pStepsKV, pStepKV)
    {
        const auto pStep = new CTrickStep;
        pStep->LoadFromKV(pStepKV);
        m_vecSteps.AddToTail(pStep);
    }

    return true;
}

CTrickAttempt::CTrickAttempt(CTrick *pTrick) : m_pTrick(pTrick)
{
    m_iStartTick = gpGlobals->tickcount;
    m_iCurrentStep = 0;
}

#ifdef GAME_DLL
bool CTrickAttempt::ShouldContinue(CTriggerTrickZone *pZone, CMomentumPlayer *pPlayer)
{
    int iTotalSteps = m_pTrick->StepCount();
    int iNextStep = m_iCurrentStep + 1;

    // Go through the trick stepping over optional zones to find our next step
    for (auto pNextStep = m_pTrick->Step(iNextStep);
         iNextStep < iTotalSteps && pNextStep;
         pNextStep = m_pTrick->Step(++iNextStep))
    {
        // Early out if this trick's sequence is properly broken
        if (pNextStep->GetTrigger() != pZone && !pNextStep->IsOptional())
            return false;

        // Here: the trigger is our zone, AND/OR we're optional

        if (pNextStep->GetTrigger() == pZone)
        {
            if (pNextStep->IsOptional())
            {
                // Increase our step and continue
                m_iCurrentStep = iNextStep;
                return true;
            }

            // Otherwise we're necessary

            // Do we pass the constraints?
            if (!pNextStep->PlayerPassesConstraints(pPlayer))
                return false;

            // Trigger was our guy, wasn't optional, and we passed the constraints. Get outta here.
            break;
        }

        // Otherwise the trigger isn't our guy, but we're still optional, so we continue the loop.
        // We either find the next optional zone in the sequence, the next required zone,
        // or you hit a no-no zone and your trick will crash and burn
    }

    // Is the trick done?
    if (iNextStep == iTotalSteps - 1)
    {
        Complete(pPlayer);
        g_pTrickSystem->CompleteTrick(this);
        return false; // This removes the attempt automatically
    }

    if (iNextStep >= iTotalSteps)
    {
        return false;
    }

    m_iCurrentStep = iNextStep;
    return true;
}

void CTrickAttempt::Complete(CMomentumPlayer* pPlayer)
{
    ClientPrint(pPlayer, HUD_PRINTTALK, CFmtStr("Trick \"%s\" completed in %.2fs!", m_pTrick->GetName(), GetElapsed()));
}
#endif

CMapTeleport::CMapTeleport()
{
#ifdef GAME_DLL
    m_pLoc = nullptr;
#endif
    m_szName[0] = '\0';
}

void CMapTeleport::SaveToKV(KeyValues* pKvOut)
{
    pKvOut->SetString("name", m_szName);
#ifdef GAME_DLL
    KeyValues *pLocKV = new KeyValues("loc");
    m_pLoc->Save(pLocKV);
    pKvOut->AddSubKey(pLocKV);
#endif
}

void CMapTeleport::LoadFromKV(KeyValues* pKvIn)
{
    Q_strncpy(m_szName, pKvIn->GetString("name", "UNNAMED MAP TELEPORT!!!"), sizeof(m_szName));

#ifdef GAME_DLL
    const auto pLocKV = pKvIn->FindKey("loc");
    if (pLocKV && !pLocKV->IsEmpty())
    {
        m_pLoc = new SavedLocation_t;
        m_pLoc->Load(pKvIn->FindKey("loc"));
    }
#endif
}

CTrickSystem::CTrickSystem() : CAutoGameSystem("CTrickSystem"
#ifdef CLIENT_DLL
"_CLIENT"
#endif
)
{
#ifdef GAME_DLL
    m_iTrackedTrick = -1;
    m_bRecording = false;
#endif
}

#define GetTricksFileName() CFmtStr("%s/%s.tricks", ZONE_FOLDER, MapName()).Get()

#ifdef CLIENT_DLL
void CTrickSystem::LevelInitPreEntity()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF))
        return;

    const auto pMapData = g_pMapCache->GetCurrentMapData();
    if (pMapData && pMapData->m_bInLibrary)
    {
        g_pAPIRequests->GetMapTrickData(pMapData->m_uID, UtlMakeDelegate(this, &CTrickSystem::OnTrickDataReceived));
    }
    else
    {
        const auto trickData = new KeyValues(TRICK_DATA_KEY);
        if (!trickData->LoadFromFile(g_pFullFileSystem, GetTricksFileName(), "MOD"))
        {
            Warning("No trick data file found for the map %s !\n", MapName());
            return;
        }

        InitializeTrickData(trickData);
    }
}

void CTrickSystem::OnTrickDataReceived(KeyValues *pFromSite)
{
    const auto pData = pFromSite->FindKey("data");
    const auto pErr = pFromSite->FindKey("error");
    if (pData)
    {
        const auto pTrickData = pData->FindKey(TRICK_DATA_KEY);

        InitializeTrickData(pTrickData);
    }
    else if (pErr)
    {
        // MOM_TODO error handle
    }
}

void CTrickSystem::InitializeTrickData(KeyValues *pTrickData)
{
    LoadTrickDataFromFile(pTrickData);

    engine->ServerCmdKeyValues(pTrickData);

    const auto pEvent = gameeventmanager->CreateEvent("trick_data_loaded");
    if (pEvent)
    {
        gameeventmanager->FireEventClientSide(pEvent);
    }
}
#endif

void CTrickSystem::LevelShutdownPreEntity()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF) || !MapName())
        return;

#ifdef GAME_DLL
    SaveTrickDataToFile();
    ClearTrickAttempts();
    SetTrackedTrick(-1);
#endif

    m_vecRecordedZones.RemoveAll();
    m_llTrickList.PurgeAndDeleteElements();
    m_vecTrickZones.RemoveAll();
    m_vecMapTeleports.PurgeAndDeleteElements();
}

#ifdef GAME_DLL
// If on zone enter the zone is either one of the optionals until a required, or is the required,
// the current trick stays in and the iterator increases. If neither of those, the trick is removed from m_llCurrentTricks.
void CTrickSystem::OnTrickZoneEnter(CTriggerTrickZone *pZone, CMomentumPlayer *pPlayer)
{
    if (m_bRecording)
    {
        m_vecRecordedZones.AddToTail(pZone);
    }
    else
    {
        FOR_EACH_VEC_BACK(m_vecCurrentTrickAttempts, i)
        {
            const auto pTrickAttempt = m_vecCurrentTrickAttempts[i];

            if (pTrickAttempt->ShouldContinue(pZone, pPlayer))
            {
                DevMsg("Continuing trick %s ...\n", pTrickAttempt->GetTrick()->GetName());

                if (m_iTrackedTrick == pTrickAttempt->GetTrick()->GetID())
                {
                    UpdateTrackedTrickStep(pTrickAttempt, pPlayer);
                }
            }
            else
            {
                OnTrickFailed(pTrickAttempt);

                m_vecCurrentTrickAttempts.Remove(i);
                delete pTrickAttempt;
            }
        }

        if (m_iTrackedTrick != -1)
        {
            UpdateTrackedTrickTriggers();
        }
    }
}

// If the zone is the start of a trick and we pass the start's constraints, add it to the list of current tricks (if not already there)
void CTrickSystem::OnTrickZoneExit(CTriggerTrickZone *pZone, CMomentumPlayer *pPlayer)
{
    if (m_bRecording)
    {
        if (m_vecRecordedZones.IsEmpty())
            m_vecRecordedZones.AddToTail(pZone);
    }
    else
    {
        FOR_EACH_LL(m_llTrickList, i)
        {
            const auto pTrick = m_llTrickList[i];
            const auto pTrickFirstStep = pTrick->Step(0);
            if (pTrickFirstStep->GetTrigger() == pZone && pTrickFirstStep->PlayerPassesConstraints(pPlayer))
            {
                bool bFound = false;
                FOR_EACH_VEC(m_vecCurrentTrickAttempts, trickItr)
                {
                    const auto pTrickAttempt = m_vecCurrentTrickAttempts[trickItr];
                    if (pTrickAttempt->GetTrick() == pTrick)
                    {
                        bFound = true;
                        break;
                    }
                }

                if (!bFound)
                {
                    const auto pTrickAttempt = new CTrickAttempt(pTrick);
                    m_vecCurrentTrickAttempts.AddToTail(pTrickAttempt);
                    DevMsg("Added trick attempt for trick %s !\n", pTrick->GetName());

                    if (m_iTrackedTrick == pTrick->GetID())
                    {
                        pPlayer->m_Data.m_iCurrentZone = 0;

                        SendTrickTrackEvent(TRICK_TRACK_UPDATE_STEP, 0);
                    }
                }
            }
        }

        if (m_iTrackedTrick != -1)
        {
            UpdateTrackedTrickTriggers();
        }
    }
}

void CTrickSystem::CompleteTrick(CTrickAttempt* pAttempt)
{
    // Woohoo!

    if (pAttempt->GetTrick()->GetID() == m_iTrackedTrick)
    {
        OnTrickTrackTerminate();
    }
}

void CTrickSystem::OnTrickFailed(CTrickAttempt *pAttempt)
{
    DevMsg("Not continuing trick %s !\n", pAttempt->GetTrick()->GetName());

    if (pAttempt->GetTrick()->GetID() == m_iTrackedTrick)
    {
        if (pAttempt->GetCurrentStep() == 0)
        {
            // Refresh our tracking so that accidentally entering/exiting the start doesn't cancel tracking altogether
            SendTrickTrackEvent(TRICK_TRACK_UPDATE_TRICK, m_iTrackedTrick);
        }
        else
        {
            OnTrickTrackTerminate();
        }
    }
}

void CTrickSystem::ClearTrickAttempts()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF))
        return;

    m_vecCurrentTrickAttempts.PurgeAndDeleteElements();
}

void CTrickSystem::PostPlayerManualTeleport(CMomentumPlayer *pPlayer)
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF))
        return;

    ClearTrickAttempts();

    const auto iTracked = m_iTrackedTrick;

    OnTrickTrackTerminate();

    if (iTracked != -1)
    {
        const auto pTrick = GetTrickByID(iTracked);
        if (!pTrick)
            return;

        if (pTrick->Step(0)->GetTrigger()->ContainsPosition(pPlayer->GetAbsOrigin()))
        {
            SetTrackedTrick(iTracked);
        }
    }
}

void CTrickSystem::SaveTrickDataToFile()
{
    const auto pMapName = MapName();
    if (!pMapName || !pMapName[0])
        return;

    KeyValuesAD trickDataKV(TRICK_DATA_KEY);

    if (!m_vecTrickZones.IsEmpty())
    {
        KeyValues *pZonesKV = new KeyValues("zones");

        FOR_EACH_VEC(m_vecTrickZones, i)
        {
            const auto pZoneKV = pZonesKV->CreateNewKey();

            const auto pZoneTrigger = m_vecTrickZones[i];

            bool bSuccess = false;
            if (pZoneTrigger->ToKeyValues(pZoneKV))
            {
                auto pBuilder = CreateZoneBuilderFromExisting(pZoneTrigger);

                bSuccess = pBuilder->Save(pZoneKV);

                delete pBuilder;
            }

            if (!bSuccess)
            {
                Warning("Failed to save zone to file!\n");
                pZonesKV->RemoveSubKey(pZoneKV);
                pZoneKV->deleteThis();
            }
        }

        trickDataKV->AddSubKey(pZonesKV);
    }

    if (m_llTrickList.Count())
    {
        KeyValues *pTricksKV = new KeyValues("tricks");

        FOR_EACH_LL(m_llTrickList, i)
        {
            const auto pTrickKV = pTricksKV->CreateNewKey();

            m_llTrickList[i]->SaveToKV(pTrickKV);
        }

        trickDataKV->AddSubKey(pTricksKV);
    }

    if (m_vecMapTeleports.Count())
    {
        KeyValues *pMapTeleKV = new KeyValues("map_teles");

        FOR_EACH_VEC(m_vecMapTeleports, i)
        {
            const auto pTeleKV = pMapTeleKV->CreateNewKey();

            m_vecMapTeleports[i]->SaveToKV(pTeleKV);
        }

        trickDataKV->AddSubKey(pMapTeleKV);
    }

    trickDataKV->SaveToFile(g_pFullFileSystem, GetTricksFileName(), "MOD");
}

void CTrickSystem::StartRecording()
{
    if (!g_pGameModeSystem->GameModeIs(GAMEMODE_TRICKSURF))
    {
        Warning("Cannot record tricks when the gamemode is not tricksurf! Make sure the map starts with \"%s\"!\n", g_pGameModeSystem->GetGameMode(GAMEMODE_TRICKSURF)->GetMapPrefix());
        return;
    }

    if (m_bRecording)
    {
        Warning("Already recording tricks!\n");
        return;
    }

    m_vecRecordedZones.RemoveAll();
    m_bRecording = true;

    Warning("================= MOM_TODO: Needs to network to the client!\n");
}

void CTrickSystem::StopRecording(const char *pTrickName)
{
    if (!m_bRecording)
    {
        Warning("Not recording tricks!\n");
        return;
    }

    if (!pTrickName || !pTrickName[0])
    {
        Warning("Invalid trick name!\n");
        return;
    }

    CTrick *pTrick = new CTrick;
    pTrick->SetID(m_llTrickList.AddToTail(pTrick));
    pTrick->SetName(pTrickName);

    FOR_EACH_VEC(m_vecRecordedZones, i)
    {
        const auto pZone = m_vecRecordedZones[i];

        auto pTrickStep = new CTrickStep;
        pTrickStep->SetTriggerID(pZone->m_iID);

        if (i == 0)
        {
            pTrickStep->AddConstraint(new TrickStepConstraint_MaxSpeed);
        }

        pTrick->AddStep(pTrickStep);
    }

    m_bRecording = false;

    Warning("================= MOM_TODO: Needs to network to the client!\n");

    SaveTrickDataToFile();
}
#endif

CTrick* CTrickSystem::GetTrickByID(int iID)
{
    if (iID < 0)
        return nullptr;

    FOR_EACH_LL(m_llTrickList, i)
    {
        const auto pTrick = m_llTrickList[i];
        if (pTrick->GetID() == iID)
            return pTrick;
    }

    return nullptr;
}

CTriggerTrickZone* CTrickSystem::GetTrickZone(int id)
{
    if (id < 0 || id >= m_vecTrickZones.Count())
        return nullptr;

    return m_vecTrickZones[id];
}

void CTrickSystem::AddZone(CTriggerTrickZone* pZone)
{
    if (pZone->m_iID == -1) // Created by zone creator, assign an ID
    {
        pZone->m_iID = m_vecTrickZones.AddToTail();
    }
    else
    {
        m_vecTrickZones.EnsureCount(pZone->m_iID + 1); // ensure we can support this new ID
    }

    m_vecTrickZones[pZone->m_iID] = pZone;
}

#ifdef GAME_DLL
CTrickAttempt* CTrickSystem::GetTrickAttemptForTrick(int iTrickID)
{
    FOR_EACH_VEC(m_vecCurrentTrickAttempts, i)
    {
        const auto pAttempt = m_vecCurrentTrickAttempts[i];

        if (pAttempt->GetTrick()->GetID() == iTrickID)
            return pAttempt;
    }

    return nullptr;
}

void CTrickSystem::SetTrackedTrick(int iTrickID)
{
    if (iTrickID < 0 || m_iTrackedTrick > -1)
    {
        ClearTrackedTrickTriggers();
    }

    m_iTrackedTrick = iTrickID;

    if (m_iTrackedTrick > -1)
    {
        OnTrickTrackStart();
    }

    SendTrickTrackEvent(TRICK_TRACK_UPDATE_TRICK, m_iTrackedTrick);

    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        pPlayer->m_Data.m_iCurrentTrack = m_iTrackedTrick;

        if (m_iTrackedTrick == -1)
        {
            pPlayer->m_Data.m_iCurrentZone = 0;
        }
        else
        {
            UpdateTrackedTrickStep(GetTrickAttemptForTrick(iTrickID), pPlayer);
        }
    }
}

void CTrickSystem::SendTrickTrackEvent(TrickTrackingUpdateType_t type, int numeric)
{
    const auto pEvent = gameeventmanager->CreateEvent("tricks_tracking");
    if (pEvent)
    {
        pEvent->SetInt("type", type);
        pEvent->SetInt("num", numeric);

        gameeventmanager->FireEvent(pEvent);
    }
}

void CTrickSystem::OnTrickTrackStart()
{
    UpdateTrackedTrickTriggers();
}

void CTrickSystem::UpdateTrackedTrickStep(CTrickAttempt *pAttempt, CMomentumPlayer *pPlayer)
{
    if (!pAttempt)
        return;

    const auto iCurrentStep = pAttempt->GetCurrentStep();

    pPlayer->m_Data.m_iCurrentZone = iCurrentStep;

    SendTrickTrackEvent(TRICK_TRACK_UPDATE_STEP, iCurrentStep);
}

void CTrickSystem::UpdateTrackedTrickTriggers()
{
    if (m_iTrackedTrick == -1)
        return;

    const auto pTrick = GetTrickByID(m_iTrackedTrick);
    if (!pTrick)
    {
        Warning("Failed to update tracking for trick with ID %i\n", m_iTrackedTrick);
        return;
    }

    const auto pAttempt = GetTrickAttemptForTrick(m_iTrackedTrick);
    const auto iCurrentStep = pAttempt ? pAttempt->GetCurrentStep() : 0;

    // All of the steps before your current step should be hidden
    // Current step (going towards) should be rendered:
    //  - White if optional, continue loop
    //  - Orange if required, stop loop
    //  - Red if end zone, stop loop (duh)

    const auto iTrickStepCount = pTrick->StepCount();
    for (int i = 0; i < iTrickStepCount; i++)
    {
        const auto pStep = pTrick->Step(i);
        const auto pStepTrigger = pStep->GetTrigger();

        if (i < iCurrentStep)
        {
            pStepTrigger->m_iDrawState = TRICK_DRAW_NONE;
        }
        else if (i == 0)
        {
            pStepTrigger->m_iDrawState = TRICK_DRAW_START;
        }
        else if (pStep->IsOptional())
        {
            pStepTrigger->m_iDrawState = TRICK_DRAW_OPTIONAL;
        }
        else if (i == iTrickStepCount - 1)
        {
            pStepTrigger->m_iDrawState = TRICK_DRAW_END;
        }
        else
        {
            pStepTrigger->m_iDrawState = TRICK_DRAW_REQUIRED;

            if (i >= iCurrentStep + 2)
                break;
        }
    }
}

void CTrickSystem::ClearTrackedTrickTriggers()
{
    const auto pTrackedTrick = GetTrickByID(m_iTrackedTrick);
    if (!pTrackedTrick)
        return;

    const auto iTrackedTrickSteps = pTrackedTrick->StepCount();

    for (int i = 0; i < iTrackedTrickSteps; i++)
    {
        pTrackedTrick->Step(i)->GetTrigger()->m_iDrawState = 0;
    }
}

void CTrickSystem::OnTrickTrackTerminate()
{
    SetTrackedTrick(-1);
}

void CTrickSystem::TeleportToTrick(int iTrickID)
{
    const auto pTrick = GetTrickByID(iTrickID);
    if (!pTrick)
    {
        Warning("Cannot teleport to invalid trick ID %i!\n", iTrickID);
        return;
    }

    const auto pTrigger = pTrick->Step(0)->GetTrigger();

    CMomentumPlayer::GetLocalPlayer()->ManualTeleport(&pTrigger->GetRestartPosition(), nullptr, &vec3_origin);
}

void CTrickSystem::CreateMapTeleport(const char* pName)
{
    const auto pMapTeleport = new CMapTeleport;
    Q_strncpy(pMapTeleport->m_szName, pName, sizeof(pMapTeleport->m_szName));
    pMapTeleport->m_pLoc = g_pSavelocSystem->CreateSaveloc(SAVELOC_POS | SAVELOC_ANG | SAVELOC_VEL);

    m_vecMapTeleports.AddToTail(pMapTeleport);

    SaveTrickDataToFile();
}

void CTrickSystem::GoToMapTeleport(int iTeleportNum)
{
    if (iTeleportNum <= 0 || iTeleportNum > m_vecMapTeleports.Count())
    {
        Warning("Invalid map teleport number %i!\n", iTeleportNum);
        return;
    }

    ClearTrickAttempts();

    m_vecMapTeleports[iTeleportNum - 1]->m_pLoc->Teleport(CMomentumPlayer::GetLocalPlayer());
}
#endif

void CTrickSystem::LoadTrickDataFromFile(KeyValues *pKvTrickData)
{
    const auto pMapName = MapName();
    Assert(pMapName && *pMapName);

    const auto pMapTelesKV = pKvTrickData->FindKey("map_teles");
    if (pMapTelesKV && !pMapTelesKV->IsEmpty())
    {
        FOR_EACH_SUBKEY(pMapTelesKV, pMapTeleKV)
        {
            const auto pMapTele = new CMapTeleport;
            pMapTele->LoadFromKV(pMapTeleKV);

            m_vecMapTeleports.AddToTail(pMapTele);
        }
    }

#ifdef GAME_DLL
    const auto pZones = pKvTrickData->FindKey("zones");
    if (!pZones || pZones->IsEmpty())
    {
        Warning("No zones found in the trick data for map %s!\n", pMapName);
        return;
    }

    FOR_EACH_SUBKEY(pZones, pZoneKV)
    {
        const auto pEntity = dynamic_cast<CTriggerTrickZone *>(CreateEntityByName("trigger_momentum_trick"));

        AssertMsg(pEntity, "Trick zone entity failed to create!!");

        if (pEntity)
        {
            if (!pEntity->LoadFromKeyValues(pZoneKV))
            {
                Warning("Failed to load trick zone!\n");
                Assert(false);
                return;
            }

            CMomBaseZoneBuilder *pBaseBuilder = CreateZoneBuilderFromKeyValues(pZoneKV);

            pBaseBuilder->BuildZone();
            pEntity->Spawn();
            pBaseBuilder->FinishZone(pEntity);

            pEntity->Activate();

            delete pBaseBuilder;
        }
    }
#endif

    // Load tricks for map
    const auto pTricks = pKvTrickData->FindKey("tricks");
    if (!pTricks || pTricks->IsEmpty())
    {
        Warning("No tricks found in the trick data for map %s!\n", pMapName);
        return;
    }

    FOR_EACH_SUBKEY(pTricks, pTrickKV)
    {
        CTrick *pNewTrick = new CTrick;

        if (pNewTrick->LoadFromKV(pTrickKV))
        {
            m_llTrickList.AddToTail(pNewTrick);
        }
    }
}

void CTrickSystem::LoadTrickDataFromSite(KeyValues* pKvTrickData)
{
    AssertMsg(false, "Implement me!!");
}

static CTrickSystem s_TricksurfSystem;
CTrickSystem *g_pTrickSystem = &s_TricksurfSystem;