#include "cbase.h"

#include "mom_timer.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

void CMomentumTimer::Start(int start)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (!pPlayer)
        return;
    // MOM_TODO: Allow it based on gametype
    if (pPlayer->m_bUsingCPMenu)
        return;
    if (ConVarRef("mom_zone_edit").GetBool())
        return;
    m_iStartTick = start;
    m_iEndTick = 0;
    m_iLastRunDate = 0;
    SetRunning(true);

    // Dispatch a start timer message for the local player
    DispatchTimerStateMessage(pPlayer, m_bIsRunning);

    IGameEvent *timeStartEvent = gameeventmanager->CreateEvent("timer_state");

    if (timeStartEvent)
    {
        timeStartEvent->SetInt("ent", pPlayer->entindex());
        timeStartEvent->SetBool("is_running", true);
        gameeventmanager->FireEvent(timeStartEvent);
    }
}

void CMomentumTimer::PostTime()
{
    if (steamapicontext->SteamHTTP() && steamapicontext->SteamUser() && !m_bWereCheatsActivated)
    {
        // MOM_TODO include the extra security measures for beta+
        uint64 steamID = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
        const char *map = gpGlobals->mapname.ToCStr();
        int ticks = gpGlobals->tickcount - m_iStartTick;

        TickSet::Tickrate tickRate = TickSet::GetCurrentTickrate();

        // Build URL
        char webURL[512];
        Q_snprintf(webURL, 512, "%s/postscore/%llu/%s/%i/%s", MOM_APIDOMAIN, steamID, map, ticks, tickRate.sType);

        DevLog("Ticks sent to server: %i\n", ticks);
        // Build request
        // mom_UTIL->PostTime("run.momrec");
    }
    else
    {
        Warning("Failed to post scores online: Cannot access STEAM HTTP or Steam User!\n");
    }
}

////MOM_TODO: REMOVEME
// CON_COMMAND(mom_test_hash, "Tests SHA1 Hashing\n")
//{
//    char pathToZone[MAX_PATH];
//    char mapName[MAX_PATH];
//    V_ComposeFileName("maps", gpGlobals->mapname.ToCStr(), mapName, MAX_PATH);
//    Q_strncat(mapName, ".zon", MAX_PATH);
//    filesystem->RelativePathToFullPath(mapName, "MOD", pathToZone, MAX_PATH);
//    Log("File path is: %s\n", pathToZone);
//
//    CSHA1 sha1;
//    sha1.HashFile(pathToZone);
//    sha1.Final();
//    unsigned char hash[20];
//    sha1.GetHash(hash);
//    Log("The hash for %s is: ", mapName);
//    for (int i = 0; i < 20; i++)
//    {
//        Log("%02x", hash[i]);
//    }
//    Log("\n");
//}

void CMomentumTimer::ConvertKVToTime(KeyValues *kvRun, Time &into) const
{
    const char *kvName = kvRun->GetName();
    into.time_sec = Q_atof(kvName);
    into.tickrate = kvRun->GetFloat("rate");
    into.date = static_cast<time_t>(kvRun->GetInt("date"));
    into.flags = kvRun->GetInt("flags");
    into.RunStats = CMomRunStats(GetZoneCount());

    for (KeyValues *subKv = kvRun->GetFirstSubKey(); subKv; subKv = subKv->GetNextKey())
    {
        if (!Q_strnicmp(subKv->GetName(), "zone", Q_strlen("zone")))
        {
            int i = Q_atoi(subKv->GetName() + 5); // atoi will need to ignore "zone " and only return the stage number

            into.RunStats.SetZoneJumps(i, subKv->GetInt("jumps"));
            into.RunStats.SetZoneStrafes(i, subKv->GetInt("strafes"));

            into.RunStats.SetZoneTime(i, subKv->GetFloat("strafes"));
            into.RunStats.SetZoneEnterTime(i, subKv->GetFloat("strafes"));

            into.RunStats.SetZoneStrafeSyncAvg(i, subKv->GetFloat("time"));
            into.RunStats.SetZoneStrafeSync2Avg(i, subKv->GetFloat("enter_time"));

            into.RunStats.SetZoneEnterSpeed(i, subKv->GetFloat("start_vel"), subKv->GetFloat("start_vel_2D"));
            into.RunStats.SetZoneExitSpeed(i, subKv->GetFloat("end_vel"), subKv->GetFloat("end_vel_2D"));
            into.RunStats.SetZoneVelocityAvg(i, subKv->GetFloat("avg_vel"), subKv->GetFloat("avg_vel_2D"));
            into.RunStats.SetZoneVelocityMax(i, subKv->GetFloat("max_vel"), subKv->GetFloat("max_vel_2D"));
        }
        else if (!Q_strncmp(subKv->GetName(), "total", Q_strlen("total")))
        {
            into.RunStats.SetZoneJumps(0, subKv->GetInt("jumps"));
            into.RunStats.SetZoneStrafes(0, subKv->GetInt("strafes"));

            into.RunStats.SetZoneStrafeSyncAvg(0, subKv->GetFloat("avgsync"));
            into.RunStats.SetZoneStrafeSync2Avg(0, subKv->GetFloat("avgsync2"));

            into.RunStats.SetZoneEnterSpeed(0, subKv->GetFloat("start_vel"), subKv->GetFloat("start_vel_2D"));
            into.RunStats.SetZoneExitSpeed(0, subKv->GetFloat("end_vel"), subKv->GetFloat("end_vel_2D"));
            into.RunStats.SetZoneVelocityAvg(0, subKv->GetFloat("avg_vel"), subKv->GetFloat("avg_vel_2D"));
            into.RunStats.SetZoneVelocityMax(0, subKv->GetFloat("max_vel"), subKv->GetFloat("max_vel_2D"));
        }
    }
}

void CMomentumTimer::ConvertTimeToKV(KeyValues *kvInto, Time *t) const
{
    if (!t || !kvInto)
        return;

    // Handle "header"
    char timeName[512];
    Q_snprintf(timeName, 512, "%.8f", t->time_sec);
    kvInto->SetName(timeName);
    kvInto->SetFloat("rate", t->tickrate);
    kvInto->SetInt("date", t->date);
    kvInto->SetInt("flags", t->flags);

    // Handle "total" stats
    KeyValues *pOverallKey = new KeyValues("total");
    pOverallKey->SetInt("jumps", t->RunStats.GetZoneJumps(0));
    pOverallKey->SetInt("strafes", t->RunStats.GetZoneStrafes(0));
    pOverallKey->SetFloat("avgsync", t->RunStats.GetZoneStrafeSyncAvg(0));
    pOverallKey->SetFloat("avgsync2", t->RunStats.GetZoneStrafeSync2Avg(0));

    pOverallKey->SetFloat("start_vel", t->RunStats.GetZoneEnterSpeed(1, false));
    pOverallKey->SetFloat("end_vel", t->RunStats.GetZoneExitSpeed(0, false));
    pOverallKey->SetFloat("avg_vel", t->RunStats.GetZoneVelocityAvg(0, false));
    pOverallKey->SetFloat("max_vel", t->RunStats.GetZoneVelocityMax(0, false));

    pOverallKey->SetFloat("start_vel_2D", t->RunStats.GetZoneEnterSpeed(1, true));
    pOverallKey->SetFloat("end_vel_2D", t->RunStats.GetZoneExitSpeed(0, true));
    pOverallKey->SetFloat("avg_vel_2D", t->RunStats.GetZoneVelocityAvg(0, true));
    pOverallKey->SetFloat("max_vel_2D", t->RunStats.GetZoneVelocityMax(0, true));

    kvInto->AddSubKey(pOverallKey);

    // Handle zone stats
    char stageName[9]; // "stage 64\0"
    if (GetZoneCount() > 1)
    {
        for (int i2 = 1; i2 <= GetZoneCount(); i2++)
        {
            Q_snprintf(stageName, sizeof(stageName), "zone %d", i2);

            KeyValues *pStageKey = new KeyValues(stageName);
            pStageKey->SetFloat("time", t->RunStats.GetZoneTime(i2));
            pStageKey->SetFloat("enter_time", t->RunStats.GetZoneEnterTime(i2));
            pStageKey->SetInt("num_jumps", t->RunStats.GetZoneJumps(i2));
            pStageKey->SetInt("num_strafes", t->RunStats.GetZoneStrafes(i2));
            pStageKey->SetFloat("avg_sync", t->RunStats.GetZoneStrafeSyncAvg(i2));
            pStageKey->SetFloat("avg_sync2", t->RunStats.GetZoneStrafeSync2Avg(i2));

            pStageKey->SetFloat("avg_vel", t->RunStats.GetZoneVelocityAvg(i2, false));
            pStageKey->SetFloat("max_vel", t->RunStats.GetZoneVelocityMax(i2, false));
            pStageKey->SetFloat("enter_vel", t->RunStats.GetZoneEnterSpeed(i2, false));
            pStageKey->SetFloat("exit_vel", t->RunStats.GetZoneExitSpeed(i2, false));

            pStageKey->SetFloat("avg_vel_2D", t->RunStats.GetZoneVelocityAvg(i2, true));
            pStageKey->SetFloat("max_vel_2D", t->RunStats.GetZoneVelocityMax(i2, true));
            pStageKey->SetFloat("enter_vel_2D", t->RunStats.GetZoneEnterSpeed(i2, true));
            pStageKey->SetFloat("exit_vel_2D", t->RunStats.GetZoneExitSpeed(i2, true));

            kvInto->AddSubKey(pStageKey);
        }
    }
}

// Called upon map load, loads any and all times stored in the <mapname>.tim file
void CMomentumTimer::LoadLocalTimes(const char *szMapname)
{
    // Build the file to load from
    char timesFilePath[MAX_PATH];
    V_ComposeFileName(MAP_FOLDER, UTIL_VarArgs("%s%s", szMapname, EXT_TIME_FILE), timesFilePath, MAX_PATH);

    // Unload (if necessary), then load the new times
    UnloadLoadedLocalTimes();
    m_pLocalTimes = new KeyValues(szMapname);

    if (m_pLocalTimes->LoadFromFile(filesystem, timesFilePath, "MOD"))
    {
        DevLog("Successfully loaded times for map %s\n", szMapname);
    }
    else
    {
        DevLog("Failed to load local times; no local file was able to be loaded!\n");
    }
}

void CMomentumTimer::AddNewTime(Time *t) const
{
    if (!t || !m_pLocalTimes)
        return;
    // Don't worry, this KeyValues name gets overridden
    KeyValues *pNewTime = new KeyValues("New Time!");
    ConvertTimeToKV(pNewTime, t);
    m_pLocalTimes->AddSubKey(pNewTime);
}

// Called every time a new time is achieved
void CMomentumTimer::SaveTimeToFile() const
{
    const char *szMapName = gpGlobals->mapname.ToCStr();
    IGameEvent *runSaveEvent = gameeventmanager->CreateEvent("run_save");

    char file[MAX_PATH];
    V_ComposeFileName(MAP_FOLDER, UTIL_VarArgs("%s%s", szMapName, EXT_TIME_FILE), file, MAX_PATH);

    bool saved = false;
    if (m_pLocalTimes->SaveToFile(filesystem, file, "MOD", true))
    {
        saved = true;
        Log("Successfully saved new time!\n");
    }
    if (runSaveEvent)
    {
        runSaveEvent->SetBool("run_saved", saved);
        gameeventmanager->FireEvent(runSaveEvent);
    }
}

void CMomentumTimer::Stop(bool endTrigger /* = false */)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    IGameEvent *runSaveEvent = gameeventmanager->CreateEvent("run_save");
    IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");

    if (endTrigger && !m_bWereCheatsActivated && pPlayer)
    {
        m_iEndTick = gpGlobals->tickcount;

        // Save times locally too, regardless of SteamAPI condition
        Time t = Time();
        t.time_sec = GetLastRunTime();

        t.tickrate = gpGlobals->interval_per_tick;                   // Set the tickrate
        t.flags = pPlayer->m_RunData.m_iRunFlags;                    // Set the run flags of this run
        time(&t.date);                                               // Set the date of this run
        m_iLastRunDate = t.date;                                     // Use this date for the replay file
        t.RunStats = static_cast<CMomRunStats>(pPlayer->m_RunStats); // copy all the run stats

        AddNewTime(&t);

        SaveTimeToFile();
        // Post time to leaderboards if they're online
        // and if cheats haven't been turned on this session
        // MOM_TODO: Post the time when ready
        //if (SteamAPI_IsSteamRunning())
        //    PostTime();
    }
    else if (runSaveEvent) // reset run saved status to false if we cant or didn't save
    {
        runSaveEvent->SetBool("run_saved", false);
        gameeventmanager->FireEvent(runSaveEvent);
    }
    if (timerStateEvent && pPlayer)
    {
        timerStateEvent->SetInt("ent", pPlayer->entindex());
        timerStateEvent->SetBool("is_running", false);
        gameeventmanager->FireEvent(timerStateEvent);
    }

    // stop replay recording
    if (g_ReplaySystem->GetReplayManager()->Recording())
        g_ReplaySystem->StopRecording(!endTrigger, endTrigger);

    SetRunning(false);
    DispatchTimerStateMessage(UTIL_GetLocalPlayer(), m_bIsRunning);
}
void CMomentumTimer::OnMapEnd(const char *pMapName)
{
    if (IsRunning())
        Stop(false);
    m_bWereCheatsActivated = false;
    SetCurrentCheckpointTrigger(nullptr);
    SetStartTrigger(nullptr);
    SetCurrentZone(nullptr);
    UnloadLoadedLocalTimes();
    ClearStartMark();
    // MOM_TODO: UnloadLoadedOnlineTimes();
}

void CMomentumTimer::DispatchMapInfo() const
{
    IGameEvent *mapInitEvent = gameeventmanager->CreateEvent("map_init", true);
    if (mapInitEvent)
    {
        // MOM_TODO: for now it's assuming stages are on staged maps, load this from
        // either the RequestStageCount() method, or something else (map info file?)
        mapInitEvent->SetBool("is_linear", m_iZoneCount == 0);
        mapInitEvent->SetInt("num_zones", m_iZoneCount);
        gameeventmanager->FireEvent(mapInitEvent);
    }
}

void CMomentumTimer::OnMapStart(const char *pMapName)
{
    SetGameModeConVars();
    m_bWereCheatsActivated = false;
    RequestZoneCount();
    LoadLocalTimes(pMapName);
    ClearStartMark();
    // MOM_TODO: LoadOnlineTimes();
}

// MOM_TODO: This needs to update to include checkpoint triggers placed in linear
// maps to allow players to compare at certain points.
void CMomentumTimer::RequestZoneCount()
{
    CTriggerStage *stage =
        static_cast<CTriggerStage *>(gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_stage"));
    int iCount =
        gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_start") ? 1 : 0; // CTriggerStart counts as one
    while (stage)
    {
        iCount++;
        stage = static_cast<CTriggerStage *>(gEntList.FindEntityByClassname(stage, "trigger_momentum_timer_stage"));
    }
    m_iZoneCount = iCount;
}
// This function is called every time CTriggerStage::StartTouch is called
float CMomentumTimer::CalculateStageTime(int stage)
{
    if (stage > m_iLastZone)
    {
        float originalTime = GetCurrentTime();
        // If the stage is a new one, we store the time we entered this stage in
        m_flZoneEnterTime[stage] = stage == 1 ? 0.0f : // Always returns 0 for first stage.
                                       originalTime + m_flTickOffsetFix[stage - 1];
        DevLog("Original Time: %f\n New Time: %f\n", originalTime, m_flZoneEnterTime[stage]);
    }
    m_iLastZone = stage;
    return m_flZoneEnterTime[stage];
}
void CMomentumTimer::DispatchResetMessage()
{
    CSingleUserRecipientFilter user(UTIL_GetLocalPlayer());
    user.MakeReliable();
    UserMessageBegin(user, "Timer_Reset");
    MessageEnd();
}

void CMomentumTimer::DispatchTimerStateMessage(CBasePlayer *pPlayer, bool isRunning) const
{
    if (pPlayer)
    {
        CSingleUserRecipientFilter user(pPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "Timer_State");
        WRITE_BOOL(isRunning);
        MessageEnd();
    }
}

void CMomentumTimer::SetRunning(bool isRunning)
{
    m_bIsRunning = isRunning;
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (pPlayer)
    {
        pPlayer->m_RunData.m_bTimerRunning = isRunning;
    }
}
void CMomentumTimer::CalculateTickIntervalOffset(CMomentumPlayer *pPlayer, const int zoneType)
{
    if (!pPlayer)
        return;
    Ray_t ray;
    Vector rewoundTracePoint, tracePoint, velocity = pPlayer->GetLocalVelocity();
    // Because trigger touch is calculated using collision hull rather than the player's origin (which is based on their
    // world space center in XY and their feet in Z),
    // the trace point is actually the player's local origin offset by their collision hull. We trace a ray from all 8
    // corners of their collision hull and pick the trace that
    // is the shortest distance, since the trace with the shortest distance originated from that point that was last
    // touching the trigger volume.
    for (int i = 0; i < 8; i++)
    {
        switch (i) // depending on which corner number we've iterated to so far, the origin is one of the eight corners
                   // of the bbox.
        {
        case 0:
            tracePoint = pPlayer->GetLocalOrigin() + pPlayer->CollisionProp()->OBBMins();
            break;
        case 1:
            tracePoint = pPlayer->GetLocalOrigin() + Vector(pPlayer->CollisionProp()->OBBMins().x,
                                                            pPlayer->CollisionProp()->OBBMaxs().y,
                                                            pPlayer->CollisionProp()->OBBMins().z);
            break;
        case 2:
            tracePoint = pPlayer->GetLocalOrigin() + Vector(pPlayer->CollisionProp()->OBBMins().x,
                                                            pPlayer->CollisionProp()->OBBMins().y,
                                                            pPlayer->CollisionProp()->OBBMaxs().z);
            break;
        case 3:
            tracePoint = pPlayer->GetLocalOrigin() + Vector(pPlayer->CollisionProp()->OBBMins().x,
                                                            pPlayer->CollisionProp()->OBBMaxs().y,
                                                            pPlayer->CollisionProp()->OBBMaxs().z);
            break;
        case 4:
            tracePoint = pPlayer->GetLocalOrigin() + Vector(pPlayer->CollisionProp()->OBBMaxs().x,
                                                            pPlayer->CollisionProp()->OBBMins().y,
                                                            pPlayer->CollisionProp()->OBBMaxs().z);
            break;
        case 5:
            tracePoint = pPlayer->GetLocalOrigin() + Vector(pPlayer->CollisionProp()->OBBMaxs().x,
                                                            pPlayer->CollisionProp()->OBBMins().y,
                                                            pPlayer->CollisionProp()->OBBMins().z);
            break;
        case 6:
            tracePoint = pPlayer->GetLocalOrigin() + Vector(pPlayer->CollisionProp()->OBBMaxs().x,
                                                            pPlayer->CollisionProp()->OBBMaxs().y,
                                                            pPlayer->CollisionProp()->OBBMins().z);
            break;
        case 7:
            tracePoint = pPlayer->GetLocalOrigin() + pPlayer->CollisionProp()->OBBMaxs();
            break;
        }
        // The previous trace point is the trace point "rewound" in time a single tick, scaled by player's current
        // velocity
        rewoundTracePoint = pPlayer->GetPrevOrigin(tracePoint);

        if (zoneType == ZONETYPE_START)
            ray.Init(tracePoint, rewoundTracePoint);
        else
        {
            // ending zones have to have the ray start _before_ we entered the zone bbox, hence why we start with
            // rewoundTracePoint
            // and trace "forwards" to the tracing point, hitting the trigger on the way.
            ray.Init(rewoundTracePoint, tracePoint);
        }

        CTimeTriggerTraceEnum endTriggerTraceEnum(&ray, pPlayer->GetAbsVelocity(), zoneType, i);
        enginetrace->EnumerateEntities(ray, true, &endTriggerTraceEnum);
    }
    // we calculate the smallest trace distance...
    float smallestDist = FLT_MAX;
    int smallestCornerNum = -1;
    for (int i = 0; i < 8; i++)
    {
        if (m_flDistFixTraceCorners[i] < smallestDist && !mom_UTIL->FloatEquals(m_flDistFixTraceCorners[i], 0.0f))
        {
            smallestDist = m_flDistFixTraceCorners[i];
            smallestCornerNum = i;
        }
    }

    if (smallestCornerNum > -1)
    {
        // velocity = dist / time, so it follows that time = distance / velocity.
        float offset = smallestDist /pPlayer->GetLocalVelocity().Length(); 
        DevLog("Smallest time offset was %f seconds, traced from bbox corner %i (trace distance: %f units)\n", offset,
               smallestCornerNum, smallestDist);
        // ...and set the interval offset as this smallest time
        SetIntervalOffset(GetCurrentZoneNumber(), offset);
    }

    // ..then reset the flCorners array
    for (int i = 0; i < 8; i++)
    {
        m_flDistFixTraceCorners[i] = 0.0f;
    }
}

// override of IEntityEnumerator's EnumEntity() in order for our trace to hit zone triggers
bool CTimeTriggerTraceEnum::EnumEntity(IHandleEntity *pHandleEntity)
{
    trace_t tr;
    // store entity that we found on the trace
    CBaseEntity *pEnt = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());

    // Stop the trace if this entity is solid.
    if (pEnt->IsSolid())
        return false;

    // if we aren't hitting a momentum trigger
    // the return type of EnumEntity tells the engine whether to continue enumerating future entities
    // or not.
    if (Q_strnicmp(pEnt->GetClassname(), "trigger_momentum_", Q_strlen("trigger_momentum_")) == 1)           
        return true; 
                     
    // In this case, we want to continue in case we hit another type of trigger.

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);

    if (tr.fraction < 1.0f) // tr.fraction = 1.0 means the trace completed
    {
        float dist = tr.startpos.DistTo(tr.endpos);

        if (!mom_UTIL->FloatEquals(dist, 0.0f))
        {
            g_pMomentumTimer->m_flDistFixTraceCorners[m_iCornerNumber] = dist;
        }

        return false; // Stop the enumeration, we hit our target
    }
    // Continue until tr.fraction == 1.0f
    return true;
}

// set ConVars according to Gamemode. Tickrate is by in tickset.h
void CMomentumTimer::SetGameModeConVars()
{
    ConVarRef gm("mom_gamemode");
    switch (gm.GetInt())
    {
    case MOMGM_SURF:
        sv_maxvelocity.SetValue(3500);
        sv_airaccelerate.SetValue(150);
        sv_maxspeed.SetValue(260);
        break;
    case MOMGM_BHOP:
        sv_maxvelocity.SetValue(100000);
        sv_airaccelerate.SetValue(1000);
        sv_maxspeed.SetValue(260);
        break;
    case MOMGM_SCROLL:
        sv_maxvelocity.SetValue(3500);
        sv_airaccelerate.SetValue(100);
        sv_maxspeed.SetValue(250);
        break;
    case MOMGM_UNKNOWN:
    case MOMGM_ALLOWED:
        sv_maxvelocity.SetValue(3500);
        sv_airaccelerate.SetValue(150);
        sv_maxspeed.SetValue(260);
        break;
    default:
        DevWarning("[%i] GameMode not defined.\n", gm.GetInt());
        break;
    }
    DevMsg("CTimer set values:\nsv_maxvelocity: %i\nsv_airaccelerate: %i \nsv_maxspeed: %i\n", sv_maxvelocity.GetInt(),
           sv_airaccelerate.GetInt(), sv_maxspeed.GetInt());
}

void CMomentumTimer::CreateStartMark()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (!pPlayer)
        return;

    CTriggerTimerStart *start = GetStartTrigger();
    if (start && start->IsTouching(pPlayer))
    {
        // Rid the previous one
        ClearStartMark();

        m_pStartZoneMark = pPlayer->CreateCheckpoint();
        m_pStartZoneMark->vel = vec3_origin; // Rid the velocity
        DevLog("Successfully created a starting mark!\n");
    }
}

void CMomentumTimer::ClearStartMark()
{
    if (m_pStartZoneMark)
        delete m_pStartZoneMark;
    m_pStartZoneMark = nullptr;
}

// Practice mode that stops the timer and allows the player to noclip.
void CMomentumTimer::EnablePractice(CMomentumPlayer *pPlayer)
{
    pPlayer->SetParent(nullptr);
    pPlayer->SetMoveType(MOVETYPE_NOCLIP);
    ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Practice mode ON!\n");
    pPlayer->AddEFlags(EFL_NOCLIP_ACTIVE);
    pPlayer->m_bHasPracticeMode = true;
    pPlayer->m_RunData.m_iCurrentZone = 0;
    Stop(false);
}
void CMomentumTimer::DisablePractice(CMomentumPlayer *pPlayer)
{
    pPlayer->RemoveEFlags(EFL_NOCLIP_ACTIVE);
    ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Practice mode OFF!\n");
    pPlayer->SetMoveType(MOVETYPE_WALK);
    pPlayer->m_bHasPracticeMode = false;
}

//--------- CTriggerOnehop stuff --------------------------------

int CMomentumTimer::AddOnehopToListTail(CTriggerOnehop *pTrigger) { return onehops.AddToTail(pTrigger); }

bool CMomentumTimer::RemoveOnehopFromList(CTriggerOnehop *pTrigger) { return onehops.FindAndRemove(pTrigger); }

int CMomentumTimer::FindOnehopOnList(CTriggerOnehop *pTrigger) { return onehops.Find(pTrigger); }

CTriggerOnehop *CMomentumTimer::FindOnehopOnList(int pIndexOnList) { return onehops.Element(pIndexOnList); }

//--------- Commands --------------------------------
static MAKE_TOGGLE_CONVAR(
    mom_practice_safeguard, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Toggles the safeguard for enabling practice mode (not pressing any movement keys to enable). 0 = OFF, 1 = ON.\n");

class CTimerCommands
{
  public:
    static void ResetToStart()
    {
        CMomentumPlayer *cPlayer = ToCMOMPlayer(UTIL_GetCommandClient());
        if (!cPlayer)
            return;
        CTriggerTimerStart *start = g_pMomentumTimer->GetStartTrigger();
        if (start)
        {
            Checkpoint *pStartMark = g_pMomentumTimer->GetStartMark();
            if (pStartMark)
            {
                cPlayer->TeleportToCheckpoint(pStartMark);
            }
            else
            {
                // Don't set angles if still in start zone.
                if (start->GetHasLookAngles())
                {
                    QAngle ang = start->GetLookAngles();
                    cPlayer->Teleport(&start->WorldSpaceCenter(), &ang, &vec3_origin);
                }
                else
                {
                    cPlayer->Teleport(&start->WorldSpaceCenter(), nullptr, &vec3_origin);
                }
            }
        }
        else
        {
            CBaseEntity *startPoint = cPlayer->EntSelectSpawnPoint();
            if (startPoint)
            {
                cPlayer->Teleport(&startPoint->GetAbsOrigin(), &startPoint->GetAbsAngles(), &vec3_origin);
                cPlayer->ResetRunStats();
            }
        }
    }

    static void ResetToCheckpoint()
    {
        CTriggerStage *stage;
        CBaseEntity *pPlayer = UTIL_GetCommandClient();
        if ((stage = g_pMomentumTimer->GetCurrentStage()) != nullptr && pPlayer)
        {
            pPlayer->Teleport(&stage->WorldSpaceCenter(), nullptr, &vec3_origin);
        }
    }

    static void PracticeMove()
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
        if (!pPlayer)
            return;

        if (!pPlayer->m_bHasPracticeMode)
        {
            int b = pPlayer->m_nButtons;
            bool safeGuard = b & IN_FORWARD || b & IN_LEFT || b & IN_RIGHT || b & IN_BACK || b & IN_JUMP ||
                             b & IN_DUCK || b & IN_WALK;
            if (mom_practice_safeguard.GetBool() && safeGuard)
            {
                Warning("You cannot enable practice mode while moving!\n");
                return;
            }

            g_pMomentumTimer->EnablePractice(pPlayer);
        }
        else
            g_pMomentumTimer->DisablePractice(pPlayer);
    }

    static void MarkStart() { g_pMomentumTimer->CreateStartMark(); }

    static void ClearStart() { g_pMomentumTimer->ClearStartMark(); }
};

static ConCommand mom_practice("mom_practice", CTimerCommands::PracticeMove,
                               "Toggle. Stops timer and allows player to fly around in noclip.\n"
                               "Only activates when player is not pressing any movement inputs.\n",
                               FCVAR_CLIENTCMD_CAN_EXECUTE);
static ConCommand mom_mark_start("mom_mark_start", CTimerCommands::MarkStart,
                   "Marks a starting point inside the start trigger for a more customized starting location.\n",
                   FCVAR_NONE);
static ConCommand mom_mark_start_clear("mom_mark_start_clear", CTimerCommands::ClearStart,
                                       "Clears the saved start location, if there is one.\n", FCVAR_NONE);
static ConCommand mom_reset_to_start("mom_restart", CTimerCommands::ResetToStart,
                                     "Restarts the player to the start trigger.\n",
                                     FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE);
static ConCommand mom_reset_to_checkpoint("mom_reset", CTimerCommands::ResetToCheckpoint,
                                          "Teleports the player back to the start of the current stage.\n",
                                          FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE);

static CMomentumTimer s_Timer;
CMomentumTimer *g_pMomentumTimer = &s_Timer;