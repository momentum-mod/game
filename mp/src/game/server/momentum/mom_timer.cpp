#include "cbase.h"

#include "in_buttons.h"
#include "mom_timer.h"

#include "tier0/memdbgon.h"

void CMomentumTimer::Start(int start)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (!pPlayer)
        return;
    // MOM_TODO: Allow it based on gametype
    if (pPlayer->m_SrvData.m_bUsingCPMenu)
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

void CMomentumTimer::Stop(bool endTrigger /* = false */)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    IGameEvent *timerStateEvent = gameeventmanager->CreateEvent("timer_state");

    if (pPlayer)
    {
        // Set our end time and date
        if (endTrigger && !m_bWereCheatsActivated)
        {
            m_iEndTick = gpGlobals->tickcount;
            time(&m_iLastRunDate); // Set the last run date for the replay
        }

        // Fire off the timer_state event
        if (timerStateEvent)
        {
            timerStateEvent->SetInt("ent", pPlayer->entindex());
            timerStateEvent->SetBool("is_running", false);
            gameeventmanager->FireEvent(timerStateEvent);
        }
    }

    // Stop replay recording, if there was any
    if (g_ReplaySystem.m_bRecording)
        g_ReplaySystem.StopRecording(!endTrigger || m_bWereCheatsActivated, endTrigger);

    SetRunning(false);
    DispatchTimerStateMessage(pPlayer, m_bIsRunning);
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

void CMomentumTimer::LevelInitPostEntity()
{
    SetGameModeConVars();
    m_bWereCheatsActivated = false;
    RequestZoneCount();
    ClearStartMark();
}

void CMomentumTimer::LevelShutdownPreEntity()
{
    if (IsRunning())
        Stop(false);
    m_bWereCheatsActivated = false;
    SetStartTrigger(nullptr);
    SetCurrentZone(nullptr);
    ClearStartMark();
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
        pPlayer->m_SrvData.m_RunData.m_bTimerRunning = isRunning;
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
        default:
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
        if (m_flDistFixTraceCorners[i] < smallestDist && !g_pMomentumUtil->FloatEquals(m_flDistFixTraceCorners[i], 0.0f))
        {
            smallestDist = m_flDistFixTraceCorners[i];
            smallestCornerNum = i;
        }
    }

    if (smallestCornerNum > -1)
    {
        // velocity = dist / time, so it follows that time = distance / velocity.
        float offset = smallestDist / pPlayer->GetLocalVelocity().Length();
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

        if (!g_pMomentumUtil->FloatEquals(dist, 0.0f))
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
    pPlayer->m_SrvData.m_bHasPracticeMode = true;
    Stop(false);
}
void CMomentumTimer::DisablePractice(CMomentumPlayer *pPlayer)
{
    pPlayer->RemoveEFlags(EFL_NOCLIP_ACTIVE);
    ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Practice mode OFF!\n");
    pPlayer->SetMoveType(MOVETYPE_WALK);
    pPlayer->m_SrvData.m_bHasPracticeMode = false;
}


//--------- Commands --------------------------------
static MAKE_TOGGLE_CONVAR(
    mom_practice_safeguard, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Toggles the safeguard for enabling practice mode (not pressing any movement keys to enable). 0 = OFF, 1 = ON.\n");

class CTimerCommands
{
  public:
    static void ResetToStart()
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetCommandClient());
        if (!pPlayer || !pPlayer->m_bAllowUserTeleports)
            return;
        CTriggerTimerStart *start = g_pMomentumTimer->GetStartTrigger();
        if (start)
        {
            Checkpoint *pStartMark = g_pMomentumTimer->GetStartMark();
            if (pStartMark)
            {
                pPlayer->TeleportToCheckpoint(pStartMark);
            }
            else
            {
                // Don't set angles if still in start zone.
                QAngle ang = start->GetLookAngles();
                pPlayer->Teleport(&start->WorldSpaceCenter(), (start->HasLookAngles() ? &ang : nullptr), &vec3_origin);
            }
        }
        else
        {
            CBaseEntity *startPoint = pPlayer->EntSelectSpawnPoint();
            if (startPoint)
            {
                pPlayer->Teleport(&startPoint->GetAbsOrigin(), &startPoint->GetAbsAngles(), &vec3_origin);
                pPlayer->ResetRunStats();
            }
        }
    }

    static void ResetToCheckpoint()
    {
        CTriggerStage *stage = g_pMomentumTimer->GetCurrentStage();
        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetCommandClient());
        if (stage && pPlayer && pPlayer->m_bAllowUserTeleports)
        {
            pPlayer->Teleport(&stage->WorldSpaceCenter(), nullptr, &vec3_origin);
        }
    }

    static void PracticeMove()
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
        if (!pPlayer || !pPlayer->m_bAllowUserTeleports)
            return;

        if (!pPlayer->m_SrvData.m_bHasPracticeMode)
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

    static void TeleToStage(const CCommand &args)
    {
        CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
        const Vector *pVec = nullptr;
        const QAngle *pAng = nullptr;
        if (pPlayer && args.ArgC() >= 2)
        {
            if (!pPlayer->m_bAllowUserTeleports)
                return;

            // We get the desried index from the command (Remember that for us, args are 1 indexed)
            int desiredIndex = Q_atoi(args[1]);
            if (desiredIndex == 1)
            {
                // Index 1 is the start. If the timer has a mark, we use it
                Checkpoint *startMark = g_pMomentumTimer->GetStartMark();
                if (startMark)
                {
                    pVec = &startMark->pos;
                    pAng = &startMark->ang;
                }
                else
                {
                    // If no mark was found, we teleport to the center of the first trigger_momentum_timer_start we find
                    CBaseEntity *pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_start");
                    if (pEnt)
                    {
                        pVec = &pEnt->GetAbsOrigin();
                    }
                }
            }
            else
            {
                // Every other index is probably a stage (What about < 1 indexes? Mappers are weird and do "weirder"
                // stuff so...)
                CTriggerStage *pStage = nullptr;

                while ((pStage = static_cast<CTriggerStage*>(gEntList.FindEntityByClassname(pStage, "trigger_momentum_timer_stage"))) != nullptr)
                {
                    if (pStage && pStage->GetStageNumber() == desiredIndex)
                    {
                        pVec = &pStage->GetAbsOrigin();
                        pAng = &pStage->GetAbsAngles();
                        break;
                    }
                }
            }

            // Teleport if we have a destination
            if (pVec)
            {
                // pAng can be null here, it's okay
                pPlayer->Teleport(pVec, pAng, &vec3_origin);
                // Untouch our triggers
                pPlayer->PhysicsCheckForEntityUntouch();
                // Stop *after* the teleport
                g_pMomentumTimer->Stop();
            } 
            else
            {
                Warning("Could not teleport to stage %i! Perhaps it doesn't exist?\n", desiredIndex);
            }
        }
    }
};

static ConCommand mom_practice("mom_practice", CTimerCommands::PracticeMove,
                               "Toggle. Stops timer and allows player to fly around in noclip.\n"
                               "Only activates when player is not pressing any movement inputs.\n",
                               FCVAR_CLIENTCMD_CAN_EXECUTE);
static ConCommand
    mom_mark_start("mom_mark_start", CTimerCommands::MarkStart,
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

static ConCommand mom_stage_tele("mom_stage_tele", CTimerCommands::TeleToStage,
                                 "Teleports the player to the desired stage. Stops the timer (Useful for mappers)\n",
                                 FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_SERVER_CAN_EXECUTE);

static CMomentumTimer s_Timer("CMomentumTimer");
CMomentumTimer *g_pMomentumTimer = &s_Timer;