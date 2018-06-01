#include "cbase.h"

#include "in_buttons.h"
#include "mom_timer.h"
#include "movevars_shared.h"
#include "mom_system_saveloc.h"
#include "mom_triggers.h"
#include "mom_player_shared.h"
#include "mom_replay_system.h"
#include <ctime>

#include "tier0/memdbgon.h"

void CMomentumTimer::Start(int start)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());
    if (!pPlayer)
        return;
    // MOM_TODO: Allow it based on gametype
    if (g_pMOMSavelocSystem->IsUsingSaveLocMenu())
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

void CMomentumTimer::Stop(bool endTrigger /* = false */, bool stopRecording /* = true*/)
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
    if (g_ReplaySystem.m_bRecording && stopRecording)
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

int CMomentumTimer::GetCurrentZoneNumber() const
{
    return m_pCurrentZone && m_pCurrentZone->GetStageNumber();
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
    Vector start, end, offset;
	
    // Since EndTouch is called after PostThink (which is where previous origins are stored) we need to go 1 more tick in the previous data
    // to get the real previous origin.
    if (zoneType == ZONETYPE_START) // EndTouch
    {
        start = pPlayer->GetLocalOrigin();
        end = pPlayer->GetPreviousOrigin(1);
    }
    else // StartTouch
    {
        start = pPlayer->GetPreviousOrigin();
        end = pPlayer->GetLocalOrigin();
    }

    ray.Init(start, end, pPlayer->CollisionProp()->OBBMins(), pPlayer->CollisionProp()->OBBMaxs());
    CTimeTriggerTraceEnum endTriggerTraceEnum(&ray, pPlayer->GetAbsVelocity());
    enginetrace->EnumerateEntities(ray, true, &endTriggerTraceEnum);

    DevLog("Time offset was %f seconds (%s)\n", endTriggerTraceEnum.GetOffset(), zoneType == ZONETYPE_START ? "EndTouch" : "StartTouch");
    SetIntervalOffset(GetCurrentZoneNumber(), endTriggerTraceEnum.GetOffset());
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
        return false;

    // In this case, we want to continue in case we hit another type of trigger.

    enginetrace->ClipRayToEntity(*m_pRay, MASK_ALL, pHandleEntity, &tr);
    if (tr.fraction > 0.0f)
    {
        m_flOffset = tr.startpos.DistTo(tr.endpos) / m_vecVelocity.Length();

        // Account for slowmotion/timescale
        m_flOffset /= gpGlobals->interval_per_tick / gpGlobals->frametime;
        return true; // We hit our target
    }

    return false;
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

    if (m_pStartTrigger && m_pStartTrigger->IsTouching(pPlayer))
    {
        // Rid the previous one
        ClearStartMark();

        m_pStartZoneMark = g_pMOMSavelocSystem->CreateSaveloc();
        if (m_pStartZoneMark)
        {
            m_pStartZoneMark->vel = vec3_origin; // Rid the velocity
            DevLog("Successfully created a starting mark!\n");
        }
        else
        {
            Warning("Could not create the start mark for some reason!\n");
        }
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
            SavedLocation_t *pStartMark = g_pMomentumTimer->GetStartMark();
            if (pStartMark)
            {
                pStartMark->Teleport(pPlayer);
            }
            else
            {
                // Don't set angles if still in start zone.
                QAngle ang = start->GetLookAngles();
                pPlayer->Teleport(&start->WorldSpaceCenter(), (start->HasLookAngles() ? &ang : nullptr), &vec3_origin);
            }
            pPlayer->ResetRunStats();
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
        if (!pPlayer || !pPlayer->m_bAllowUserTeleports || pPlayer->IsSpectatingGhost())
            return;

        if (!pPlayer->m_SrvData.m_bHasPracticeMode)
        {
            if (g_pMomentumTimer->IsRunning() && mom_practice_safeguard.GetBool())
            {
                bool safeGuard = (pPlayer->m_nButtons & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK | IN_JUMP | IN_DUCK | IN_WALK)) != 0;
                if (safeGuard)
                {
                    Warning("You cannot enable practice mode while moving, while the timer is running! Toggle this with \"mom_practice_safeguard\"!\n");
                    return;
                }
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
                SavedLocation_t *startMark = g_pMomentumTimer->GetStartMark();
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