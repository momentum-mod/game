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
#define SAVELOC_KV_KEY_SAVELOCS "cps"
#define SAVELOC_KV_KEY_CURRENTINDEX "cur"
#define SAVELOC_KV_KEY_STARTMARKS "startmarks"

MAKE_TOGGLE_CONVAR(mom_saveloc_save_between_sessions, "1", FCVAR_ARCHIVE, "Defines if savelocs should be saved between sessions of the same map.\n");

SavedLocation_t::SavedLocation_t() : m_bCrouched(false), m_vecPos(vec3_origin), m_vecVel(vec3_origin), m_qaAng(vec3_angle),
                                     m_fGravityScale(1.0f), m_fMovementLagScale(1.0f), m_iDisabledButtons(0), m_savedComponents(SAVELOC_NONE),
                                     m_iZone(-1), m_iTrack(-1), m_iToggledButtons(0), m_iTimerTickOffset(-1)
{
    m_szTargetName[0] = '\0';
    m_szTargetClassName[0] = '\0';
}

SavedLocation_t::SavedLocation_t(CMomentumPlayer* pPlayer, int components /*= SAVELOC_ALL*/) : SavedLocation_t()
{
    m_savedComponents = components;

    if ( components & SAVELOC_TARGETNAME )
        Q_strncpy(m_szTargetName, pPlayer->GetEntityName().ToCStr(), sizeof(m_szTargetName));

    if ( components & SAVELOC_CLASSNAME )
        Q_strncpy(m_szTargetClassName, pPlayer->GetClassname(), sizeof(m_szTargetClassName));

    if ( components & SAVELOC_VEL )
        m_vecVel = pPlayer->GetAbsVelocity();

    if ( components & SAVELOC_POS )
        m_vecPos = pPlayer->GetAbsOrigin();

    if ( components & SAVELOC_ANG )
        m_qaAng = pPlayer->GetAbsAngles();

    if ( components & SAVELOC_DUCKED )
        m_bCrouched = pPlayer->m_Local.m_bDucked || pPlayer->m_Local.m_bDucking;

    if ( components & SAVELOC_GRAVITY )
        m_fGravityScale = pPlayer->GetGravity();

    if ( components & SAVELOC_MOVEMENTLAG )
        m_fMovementLagScale = pPlayer->GetLaggedMovementValue();

    if ( components & SAVELOC_DISABLED_BTNS )
        m_iDisabledButtons = pPlayer->m_afButtonDisabled.Get();

    if ( components & SAVELOC_TRACK )
        m_iTrack = pPlayer->m_Data.m_iCurrentTrack;

    if ( components & SAVELOC_ZONE )
        m_iZone = pPlayer->m_Data.m_iCurrentZone;

    if ( components & SAVELOC_TOGGLED_BTNS )
        m_iToggledButtons = pPlayer->m_nButtonsToggled;

    if ( components & SAVELOC_EVENT_QUEUE )
        g_EventQueue.SaveForTarget(pPlayer, entEventsState);

    if ( components & SAVELOC_TIME )
    {
        if (g_pMomentumTimer->IsRunning())
        {
            m_iTimerTickOffset = g_pMomentumTimer->GetCurrentTime();
        }
        else if (pPlayer->m_Data.m_iTimerState == TIMER_STATE_PRACTICE)
        {
            m_iTimerTickOffset = gpGlobals->tickcount - pPlayer->m_Data.m_iStartTick;
        }
        else
        {
            m_iTimerTickOffset = -1;
        }
    }
}

void SavedLocation_t::Save(KeyValues* kvCP) const
{
    kvCP->SetInt( "components", m_savedComponents );

    if ( m_savedComponents & SAVELOC_TARGETNAME )
        kvCP->SetString("targetName", m_szTargetName);

    if ( m_savedComponents & SAVELOC_CLASSNAME )
        kvCP->SetString("targetClassName", m_szTargetClassName);

    if ( m_savedComponents & SAVELOC_VEL )
        MomUtil::Save3DToKeyValues( kvCP, "vel", m_vecVel );

    if ( m_savedComponents & SAVELOC_POS )
        MomUtil::Save3DToKeyValues( kvCP, "pos", m_vecPos );

    if ( m_savedComponents & SAVELOC_ANG )
        MomUtil::Save3DToKeyValues( kvCP, "ang", m_qaAng );

    if ( m_savedComponents & SAVELOC_DUCKED )
        kvCP->SetBool("crouched", m_bCrouched);

    if ( m_savedComponents & SAVELOC_GRAVITY )
        kvCP->SetFloat("gravityScale", m_fGravityScale);

    if ( m_savedComponents & SAVELOC_MOVEMENTLAG )
        kvCP->SetFloat("movementLagScale", m_fMovementLagScale);

    if ( m_savedComponents & SAVELOC_DISABLED_BTNS )
        kvCP->SetInt("disabledButtons", m_iDisabledButtons);

    if ( m_savedComponents & SAVELOC_TRACK )
        kvCP->SetInt( "track", m_iTrack );

    if ( m_savedComponents & SAVELOC_ZONE )
        kvCP->SetInt( "zone", m_iZone );

    if ( m_savedComponents & SAVELOC_TOGGLED_BTNS )
        kvCP->SetInt("toggledButtons", m_iToggledButtons);

    if ( m_savedComponents & SAVELOC_EVENT_QUEUE )
        entEventsState.SaveToKeyValues(kvCP);

    if ( m_savedComponents & SAVELOC_TIME )
        kvCP->SetInt("time", m_iTimerTickOffset);
}

void SavedLocation_t::Load(KeyValues* pKv)
{
    m_savedComponents = pKv->GetInt( "components", SAVELOC_ALL );
    Q_strncpy(m_szTargetName, pKv->GetString("targetName"), sizeof(m_szTargetName));
    Q_strncpy(m_szTargetClassName, pKv->GetString("targetClassName"), sizeof(m_szTargetClassName));
    MomUtil::Load3DFromKeyValues( pKv, "pos", m_vecPos );
    MomUtil::Load3DFromKeyValues( pKv, "vel", m_vecVel );
    MomUtil::Load3DFromKeyValues( pKv, "ang", m_qaAng );
    m_bCrouched = pKv->GetBool("crouched");
    m_fGravityScale = pKv->GetFloat("gravityScale", 1.0f);
    m_fMovementLagScale = pKv->GetFloat("movementLagScale", 1.0f);
    m_iDisabledButtons = pKv->GetInt("disabledButtons");
    m_iTrack = pKv->GetInt( "track", -1 );
    m_iZone = pKv->GetInt( "zone", -1 );
    m_iToggledButtons = pKv->GetInt("toggledButtons");
    entEventsState.LoadFromKeyValues(pKv);
    m_iTimerTickOffset = pKv->GetInt("time", -1);
}

void SavedLocation_t::Teleport(CMomentumPlayer* pPlayer, bool bStopTimer /*= true*/)
{
    if ( m_savedComponents & SAVELOC_TARGETNAME )
        pPlayer->SetName(MAKE_STRING(m_szTargetName));

    if ( m_savedComponents & SAVELOC_CLASSNAME )
        pPlayer->SetClassname(m_szTargetClassName);

    if ( m_savedComponents & SAVELOC_DUCKED )
    {
        if ( m_bCrouched && !pPlayer->m_Local.m_bDucked )
        {
            pPlayer->ToggleDuckThisFrame( true );
        }
        else if ( !m_bCrouched && pPlayer->m_Local.m_bDucked )
        {
            pPlayer->ToggleDuckThisFrame( false );
        }
    }

    pPlayer->ManualTeleport(&m_vecPos, m_savedComponents & SAVELOC_ANG ? &m_qaAng : nullptr,
                            m_savedComponents & SAVELOC_VEL ? &m_vecVel : &vec3_origin, bStopTimer);

    if ( m_savedComponents & SAVELOC_GRAVITY )
        pPlayer->SetGravity(m_fGravityScale);

    if ( m_savedComponents & SAVELOC_DISABLED_BTNS )
        pPlayer->m_afButtonDisabled = m_iDisabledButtons;

    if ( m_savedComponents & SAVELOC_MOVEMENTLAG )
        pPlayer->SetLaggedMovementValue(m_fMovementLagScale);

    if ( m_savedComponents & SAVELOC_TRACK && m_iTrack > -1 )
        pPlayer->m_Data.m_iCurrentTrack = m_iTrack;

    if ( m_savedComponents & SAVELOC_ZONE && m_iZone > -1 )
        pPlayer->m_Data.m_iCurrentZone = m_iZone;

    if ( m_savedComponents & SAVELOC_TOGGLED_BTNS )
        pPlayer->m_nButtonsToggled = m_iToggledButtons;

    if ( m_savedComponents & SAVELOC_EVENT_QUEUE )
        g_EventQueue.RestoreForTarget(pPlayer, entEventsState);

    if ( m_savedComponents & SAVELOC_TIME )
    {
        if (m_iTimerTickOffset != -1)
        {
            pPlayer->m_Data.m_iTimerState = TIMER_STATE_PRACTICE;
            pPlayer->m_Data.m_iStartTick = gpGlobals->tickcount - m_iTimerTickOffset;
        }
        else
        {
            pPlayer->m_Data.m_iTimerState = TIMER_STATE_NOT_RUNNING;
            pPlayer->m_Data.m_iStartTick = 0;
        }
    }
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

CSaveLocSystem::CSaveLocSystem(const char* pName): CAutoGameSystem(pName)
{
    m_pSavedLocsKV = new KeyValues(pName);
    m_iRequesting = 0;
    m_iCurrentSavelocIndx = -1;
    m_bUsingSavelocMenu = false;
    m_bHintedStartMarkForLevel = false;

    m_vecStartMarks.EnsureCount(MAX_TRACKS);
    for (int i = 0; i < MAX_TRACKS; i++)
        m_vecStartMarks[i] = nullptr;

    // all stages within a track; 0 and 1 are ignored
    // track doesn't matter as it wipes every run
    m_vecStageMarks.EnsureCount(MAX_ZONES);
    for (int i = 0; i < MAX_ZONES; i++)
        m_vecStageMarks[i] = nullptr;
}

CSaveLocSystem::~CSaveLocSystem()
{
    m_pSavedLocsKV->deleteThis();
    m_pSavedLocsKV = nullptr;
}

void CSaveLocSystem::PostInit()
{
    g_pModuleComms->ListenForEvent("req_savelocs", UtlMakeDelegate(this, &CSaveLocSystem::OnSavelocRequestEvent));
}

void CSaveLocSystem::LevelInitPreEntity()
{
    m_bHintedStartMarkForLevel = false;
    ClearAllStartMarks(START_MARK);

    // Note: We are not only loading in PostInit because if players edit their savelocs file (add
    // savelocs from a friend or something), then we want to reload on map load again,
    // and not force the player to restart the mod every time.
    if (!LoadSavelocsIntoKV())
        return;

    KeyValues *kvMapSavelocs = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
    if (!kvMapSavelocs || kvMapSavelocs->IsEmpty())
        return;

    m_iCurrentSavelocIndx = kvMapSavelocs->GetInt(SAVELOC_KV_KEY_CURRENTINDEX);
    AddSavelocsFromKV(kvMapSavelocs->FindKey(SAVELOC_KV_KEY_SAVELOCS));

    if (LoadStartMarks())
        DevLog("Loaded startmarks from the savedlocs KV!\n");
    else
        DevWarning("Failed loading startmarks from the savedlocs file.\n");
}

bool CSaveLocSystem::LoadStartMarks()
{
    if (!m_pSavedLocsKV || m_pSavedLocsKV->IsEmpty())
        return false;

    KeyValues *kvMapSavelocs = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
    if (!kvMapSavelocs || kvMapSavelocs->IsEmpty())
        return false;

    KeyValues *kvStartMarks = kvMapSavelocs->FindKey(SAVELOC_KV_KEY_STARTMARKS);
    if (!kvStartMarks)
        return false;

    FOR_EACH_SUBKEY(kvStartMarks, kvStartMark)
    {
        int track = Q_atoi(kvStartMark->GetName());

        const auto pStartmark = new SavedLocation_t;
        pStartmark->Load(kvStartMark);

        if (!pStartmark || track < 0 || track >= MAX_TRACKS || pStartmark->m_savedComponents != (SAVELOC_POS | SAVELOC_ANG))
        {
            delete pStartmark;
            return false;
        }

        m_vecStartMarks[track] = pStartmark;
    }

    return true;
}

void CSaveLocSystem::LevelShutdownPreEntity()
{
    if (mom_saveloc_save_between_sessions.GetBool())
    {
        SaveSavelocsToKV();
    }

    // Remove all requesters if we had any
    m_vecRequesters.RemoveAll();
    m_bUsingSavelocMenu = false;
    RemoveAllSavelocs();
}

void CSaveLocSystem::OnSavelocRequestEvent(KeyValues* pKv)
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

bool CSaveLocSystem::AddSavelocRequester(const uint64& newReq)
{
    if (m_vecRequesters.HasElement(newReq))
        return false;

    m_vecRequesters.AddToTail(newReq);
    return true;
}

void CSaveLocSystem::RequesterLeft(const uint64& requester)
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

void CSaveLocSystem::SetRequestingSavelocsFrom(const uint64& from)
{
    if (m_iRequesting && from != 0)
    {
        DevWarning("We're already requesting savelocs from %llu!!!\n", m_iRequesting);
        return;
    }

    m_iRequesting = from;
}

bool CSaveLocSystem::WriteRequestedSavelocs(SavelocReqPacket *input, SavelocReqPacket *output, const uint64 &requester)
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
    // (eg. when we delete some savelocs while the packet to request the original amount/indices is still live)
    output->saveloc_count = count;

    return true;
}

bool CSaveLocSystem::ReadReceivedSavelocs(SavelocReqPacket *input, const uint64 &sender)
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

SavedLocation_t* CSaveLocSystem::CreateSaveloc(int components /*= SAVELOC_ALL*/)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    return pPlayer ? new SavedLocation_t(pPlayer, components) : nullptr;
}

void CSaveLocSystem::CreateAndSaveLocation()
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    const auto bCreatedStartMark = CreateStartMark();
    if (bCreatedStartMark && !m_bHintedStartMarkForLevel)
    {
        UTIL_HudHintText(pPlayer, "#MOM_Hint_CreateStartMark");
        m_bHintedStartMarkForLevel = true;
    }

    AddSaveloc(CreateSaveloc(bCreatedStartMark ? (SAVELOC_POS | SAVELOC_ANG) : SAVELOC_ALL));

    if (!bCreatedStartMark)
        ClientPrint(pPlayer, HUD_PRINTTALK, CFmtStr("Saveloc #%i created!", m_rcSavelocs.Count()));

    UpdateRequesters();
}

void CSaveLocSystem::AddSaveloc(SavedLocation_t* saveloc)
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

void CSaveLocSystem::RemoveCurrentSaveloc()
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

void CSaveLocSystem::RemoveAllSavelocs()
{
    m_rcSavelocs.PurgeAndDeleteElements();
    m_iCurrentSavelocIndx = -1;

    FireUpdateEvent();
    UpdateRequesters();
}

void CSaveLocSystem::GotoNextSaveloc()
{
    auto count = GetSavelocCount();
    if (count > 0)
    {
        SetCurrentSavelocMenuIndex((m_iCurrentSavelocIndx + 1) % count);
        TeleportToCurrentSaveloc();
    }
}

void CSaveLocSystem::GotoFirstSaveloc()
{
    if (!m_rcSavelocs.IsEmpty())
    {
        SetCurrentSavelocMenuIndex(0);
        TeleportToCurrentSaveloc();
    }
}

void CSaveLocSystem::GotoLastSaveloc()
{
    auto count = GetSavelocCount();
    if (count > 0)
    {
        SetCurrentSavelocMenuIndex(count - 1);
        TeleportToCurrentSaveloc();
    }
}

void CSaveLocSystem::GotoPrevSaveloc()
{
    auto count = GetSavelocCount();
    if (count > 0)
    {
        SetCurrentSavelocMenuIndex(m_iCurrentSavelocIndx == 0 ? count - 1 : m_iCurrentSavelocIndx - 1);
        TeleportToCurrentSaveloc();
    }
}

void CSaveLocSystem::TeleportToSavelocIndex(int indx)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (indx < 0 || indx > m_rcSavelocs.Count() || !pPlayer || !pPlayer->AllowUserTeleports())
        return;
    // Check if the timer is running and if we should stop it
    CheckTimer();
    m_rcSavelocs[indx]->Teleport(pPlayer);
}

void CSaveLocSystem::TeleportToCurrentSaveloc()
{
    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_SAVELOC_TELE))
        return; 

    TeleportToSavelocIndex(m_iCurrentSavelocIndx);
    FireUpdateEvent();
}

void CSaveLocSystem::SetUsingSavelocMenu(bool bIsUsingSLMenu)
{
    m_bUsingSavelocMenu = bIsUsingSLMenu;
    FireUpdateEvent();
}

bool CSaveLocSystem::CreateStartMark()
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return false;

    if (!pPlayer->GetGroundEntity())
    {
        Warning("Must be on ground to create a start mark!\n");
        return false;
    }

    StartMarkType_t type;
    int iMarkIndex;
    SavedLocation_t **pSaveLocAtIndex;
    if (pPlayer->IsInZone(ZONE_TYPE_START))
    {
        int iCurrentTrack = pPlayer->m_Data.m_iCurrentTrack.Get();
        if (iCurrentTrack < 0 || iCurrentTrack >= MAX_TRACKS)
        {
            Warning("Could not create start mark; invalid track (%i)!\n", iCurrentTrack);
            return false;
        }

        type = START_MARK;
        iMarkIndex = iCurrentTrack;
        pSaveLocAtIndex = &m_vecStartMarks[iCurrentTrack];
    }
    else if (pPlayer->IsInZone(ZONE_TYPE_STAGE))
    {
        int iCurrentStage = pPlayer->m_Data.m_iCurrentZone.Get();
        if (iCurrentStage < 2 || iCurrentStage >= MAX_ZONES)
        {
            Warning("Could not create stage mark; invalid stage (%i)!\n", iCurrentStage);
            return false;
        }

        type = START_MARK_STAGE;
        iMarkIndex = iCurrentStage;
        pSaveLocAtIndex = &m_vecStageMarks[iCurrentStage];
    }
    else
    {
        Warning("Could not create start mark; you are not in a valid start or stage zone!\n");
        return false;
    }

    const auto pSaveloc = CreateSaveloc(SAVELOC_POS | SAVELOC_ANG);
    if (!pSaveloc)
        return false;

    ClearStartMark(type, iMarkIndex, false);
    *pSaveLocAtIndex = pSaveloc;
    ClientPrint(pPlayer, HUD_PRINTTALK, type == START_MARK ? "Start Mark Created!" : "Stage Start Mark Created!");

    return true;
}

void CSaveLocSystem::ClearStartMark(StartMarkType_t eMarktype, int iMarkIndex, bool bPrintMsg /*= true*/)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    if (eMarktype == START_MARK)
    {
        if (iMarkIndex < 0 || iMarkIndex >= MAX_TRACKS)
            return;

        delete m_vecStartMarks[iMarkIndex];
        m_vecStartMarks[iMarkIndex] = nullptr;
    }
    else
    {
        if (iMarkIndex < 2 || iMarkIndex >= MAX_ZONES)
            return;

        delete m_vecStageMarks[iMarkIndex];
        m_vecStageMarks[iMarkIndex] = nullptr;
    }

    if (!bPrintMsg)
        return;

    ClientPrint(pPlayer, HUD_PRINTTALK, eMarktype == START_MARK ? "Start Mark Cleared!" : "Stage Start Mark Cleared!");
}

void CSaveLocSystem::ClearCurrentStartMark(StartMarkType_t eMarktype, bool bPrintMsg /*= true*/)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    ClearStartMark(eMarktype, eMarktype == START_MARK ? pPlayer->m_Data.m_iCurrentTrack.Get() : pPlayer->m_Data.m_iCurrentZone.Get(), bPrintMsg);
}

void CSaveLocSystem::ClearAllStartMarks(StartMarkType_t eMarktype)
{
    if (eMarktype == START_MARK)
    {
        for (int i = 0; i < MAX_TRACKS; i++)
        {
            delete m_vecStartMarks[i];
            m_vecStartMarks[i] = nullptr;
        }
    }
    else
    {
        for (int i = 0; i < MAX_ZONES; i++)
        {
            delete m_vecStageMarks[i];
            m_vecStageMarks[i] = nullptr;
        }
    }
}

bool CSaveLocSystem::TeleportToStartMark(StartMarkType_t eMarktype, int iMarkIndex)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return false;

    if (eMarktype == START_MARK)
    {
        if (iMarkIndex < 0 || iMarkIndex >= MAX_TRACKS)
            return false;

        if (!m_vecStartMarks[iMarkIndex])
            return false;

        m_vecStartMarks[iMarkIndex]->Teleport(pPlayer);
    }
    else
    {
        if (iMarkIndex < 2 || iMarkIndex >= MAX_ZONES)
            return false;

        if (!m_vecStageMarks[iMarkIndex])
            return false;

        m_vecStageMarks[iMarkIndex]->Teleport(pPlayer, false);
    }

    return true;
}

void CSaveLocSystem::CheckTimer()
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

void CSaveLocSystem::FireUpdateEvent() const
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

void CSaveLocSystem::UpdateRequesters()
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

bool CSaveLocSystem::LoadSavelocsIntoKV()
{
    // We don't check mom_savelocs_save_between_sessions because we want to be able to load savelocs from friends
    DevLog("Loading savelocs from %s ...\n", SAVELOC_FILE_NAME);

    // Remove the past loaded stuff
    m_pSavedLocsKV->Clear();

    // Note: This loading is going to contain all of the other maps' savelocs as well!
    if (!m_pSavedLocsKV->LoadFromFile(filesystem, SAVELOC_FILE_NAME, "MOD"))
    {
        DevWarning("Failed to savelocs file! It probably doesn't exist yet...\n");
        return false;
    }

    if (m_pSavedLocsKV->IsEmpty())
        return false;

    DevLog("Loaded savelocs from %s!\n", SAVELOC_FILE_NAME);

    return true;
}

bool CSaveLocSystem::SaveSavelocsToKV()
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return false;

    const auto pKvMapSavelocs = new KeyValues(gpGlobals->mapname.ToCStr());

    // Set the current index
    pKvMapSavelocs->SetInt(SAVELOC_KV_KEY_CURRENTINDEX, m_iCurrentSavelocIndx);

    // Add all your savelocs
    const auto kvCPs = new KeyValues(SAVELOC_KV_KEY_SAVELOCS);
    FOR_EACH_VEC(m_rcSavelocs, i)
    {
        char szCheckpointNum[10]; // 999 million savelocs is pretty generous
        Q_snprintf(szCheckpointNum, sizeof(szCheckpointNum), "%09i", i); // %09 because '\0' is the last (10)
        KeyValues *kvCP = new KeyValues(szCheckpointNum);
        m_rcSavelocs[i]->Save(kvCP);
        kvCPs->AddSubKey(kvCP);
    }

    // Add startmarks
    const auto kvStartMarks = new KeyValues(SAVELOC_KV_KEY_STARTMARKS);
    for (int track = 0; track < MAX_TRACKS; track++)
    {
        SavedLocation_t *startMark = m_vecStartMarks[track];
        if (!startMark) // Save location of valid startmarks only
            continue;

        char szTrackNum[4]; // 999 tracks should be enough
        Q_snprintf(szTrackNum, sizeof(szTrackNum), "%03i", track);
        const auto kvStartMark = new KeyValues(szTrackNum);

        startMark->Save(kvStartMark);
        kvStartMarks->AddSubKey(kvStartMark);
    }

    pKvMapSavelocs->AddSubKey(kvCPs);
    pKvMapSavelocs->AddSubKey(kvStartMarks);

    // Remove the map if it already exists in there
    KeyValues *pExisting = m_pSavedLocsKV->FindKey(gpGlobals->mapname.ToCStr());
    if (pExisting)
    {
        m_pSavedLocsKV->RemoveSubKey(pExisting);
        pExisting->deleteThis();
    }

    m_pSavedLocsKV->AddSubKey(pKvMapSavelocs);

    return m_pSavedLocsKV->SaveToFile(filesystem, SAVELOC_FILE_NAME, "MOD", true);
}

void CSaveLocSystem::AddSavelocsFromKV(KeyValues *pSavelocData)
{
    if (!pSavelocData || pSavelocData->IsEmpty())
        return;

    FOR_EACH_SUBKEY(pSavelocData, kvSaveloc)
    {
        auto pSaveloc = new SavedLocation_t;
        pSaveloc->Load(kvSaveloc);
        m_rcSavelocs.AddToTail(pSaveloc);
    }

    UpdateRequesters();
    FireUpdateEvent();
}

void CSaveLocSystem::ImportMapSavelocs(const char *pImportMapName)
{
    if (!CMomentumPlayer::GetLocalPlayer())
    {
        Warning("This command can only be used while in-game!\n");
        return;
    }

    if (!pImportMapName || !pImportMapName[0])
        return;

    const auto pImportKV = m_pSavedLocsKV->FindKey(pImportMapName);
    if (!pImportKV)
        return;

    const auto pImportSavelocsKV = pImportKV->FindKey(SAVELOC_KV_KEY_SAVELOCS);
    if (!pImportSavelocsKV)
    {
        Warning("Failed to import savelocs; map to import has no savelocs!\n");
        return;
    }

    AddSavelocsFromKV(pImportSavelocsKV);
    SaveSavelocsToKV();
}

CON_COMMAND_F(mom_saveloc_create, "Creates a saveloc that saves a player's state.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->CreateAndSaveLocation();
}
CON_COMMAND_F(mom_saveloc_current, "Teleports the player to their current saved location.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->TeleportToCurrentSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_next, "Goes forwards through the saveloc list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->GotoNextSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_prev, "Goes backwards through the saveloc list, while teleporting the player to each.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->GotoPrevSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_first, "Goes to the first saveloc in the list, teleporting the player to it.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->GotoFirstSaveloc();
}
CON_COMMAND_F(mom_saveloc_nav_last, "Goes to the last saveloc in the list, teleporting the player to it.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->GotoLastSaveloc();
}
CON_COMMAND_F(mom_saveloc_remove_current, "Removes the current saveloc.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->RemoveCurrentSaveloc();
}
CON_COMMAND_F(mom_saveloc_remove_all, "Removes all of the created savelocs for this map.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->RemoveAllSavelocs();
}
CON_COMMAND_F(mom_saveloc_close, "Closes the saveloc menu.\n", FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    g_pSavelocSystem->SetUsingSavelocMenu(false);
}

static int SavelocImportCompletion(const char *pPartial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    if (!CMomentumPlayer::GetLocalPlayer())
        return 0;

    const auto pCmdName = "mom_saveloc_import";
    char *pSubstring = nullptr;
    if (Q_strstr(pPartial, pCmdName) && strlen(pPartial) > strlen(pCmdName) + 1)
    {
        pSubstring = (char *)pPartial + strlen(pCmdName) + 1;
    }

    const bool bSubstringNullOrEmpty = !pSubstring || !pSubstring[0];

    int current = 0;

    const auto pSavelocsKV = g_pSavelocSystem->GetSavelocKV();
    AssertMsg(pSavelocsKV, "Failed to complete saveloc autocomplete due to null KV!");
    FOR_EACH_SUBKEY(pSavelocsKV, pSavelocMapKV)
    {
        const auto pMapName = pSavelocMapKV->GetName();
        const auto bMatchedName = bSubstringNullOrEmpty || Q_stristr(pMapName, pSubstring);
        if (bMatchedName && !pSavelocMapKV->IsEmpty(SAVELOC_KV_KEY_SAVELOCS))
        {
            char command[COMMAND_COMPLETION_ITEM_LENGTH];
            Q_snprintf(command, COMMAND_COMPLETION_ITEM_LENGTH, "%s %s", pCmdName, pMapName);
            Q_strncpy(commands[current], command, COMMAND_COMPLETION_ITEM_LENGTH);
            current++;
        }
    }

    return current;
}

CON_COMMAND_F_COMPLETION(mom_saveloc_import, "Imports savelocs from a previous map's block.\nUsage:\n"
    "\"mom_saveloc_import map_1\" - imports map_1's savelocs into the current map's block.\n"
    "This command can only be used when in a map!\n",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE, SavelocImportCompletion)
{
    g_pSavelocSystem->ImportMapSavelocs(args.Arg(1));
}

CON_COMMAND(mom_start_mark_create, "Marks a starting point inside the start or stage trigger for a more customized starting location.\n"
                                   "Stage start marks are wiped every run, while start marks are saved.\n")
{
    g_pSavelocSystem->CreateStartMark();
}

CON_COMMAND(mom_start_mark_clear, "Clears the saved start location for your current track, if there is one.\n"
                                  "You may also specify the track number to clear as the parameter; "
                                  "\"mom_start_mark_clear 2\" clears track 2's start mark.")
{
    if (args.ArgC() > 1)
        g_pSavelocSystem->ClearStartMark(START_MARK, Q_atoi(args[1]));
    else
        g_pSavelocSystem->ClearCurrentStartMark(START_MARK);
}

CON_COMMAND(mom_stage_mark_clear, "Clears the current stage start location. Also accepts a stage number as parameter.\n")
{
    if (args.ArgC() > 1)
        g_pSavelocSystem->ClearStartMark(START_MARK_STAGE, Q_atoi(args[1]));
    else
        g_pSavelocSystem->ClearCurrentStartMark(START_MARK_STAGE);
}

//Expose this to the DLL
static CSaveLocSystem s_MOMSavelocSystem("MOMSavelocSystem");
CSaveLocSystem *g_pSavelocSystem = &s_MOMSavelocSystem;