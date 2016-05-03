#include "cbase.h"
#include "mom_player.h"
#include "mom_triggers.h"
#include "in_buttons.h"
#include "Timer.h"

#include "tier0/memdbgon.h"

#define AVERAGE_STATS_INTERVAL 0.1

IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropInt(SENDINFO(m_iShotsFired)),
SendPropInt(SENDINFO(m_iDirection)),
SendPropBool(SENDINFO(m_bResumeZoom)),
SendPropInt(SENDINFO(m_iLastZoom)),
SendPropBool(SENDINFO(m_bAutoBhop)),
SendPropBool(SENDINFO(m_bDidPlayerBhop)),
SendPropInt(SENDINFO(m_iSuccessiveBhops)),
SendPropFloat(SENDINFO(m_flStrafeSync)),
SendPropFloat(SENDINFO(m_flStrafeSync2)),
SendPropFloat(SENDINFO(m_flLastJumpVel)),
END_SEND_TABLE()

BEGIN_DATADESC(CMomentumPlayer)
DEFINE_THINKFUNC(CheckForBhop),
DEFINE_THINKFUNC(UpdateRunStats),
DEFINE_THINKFUNC(CalculateAverageStats),
DEFINE_THINKFUNC(LimitSpeedInStartZone),
END_DATADESC()

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);


CMomentumPlayer::CMomentumPlayer()
{
    m_flPunishTime = -1;
    m_iLastBlock = -1;
}

CMomentumPlayer::~CMomentumPlayer() {}

void CMomentumPlayer::Precache()
{
    // Name of our entity's model
#define ENTITY_MODEL "models/gibs/airboat_broken_engine.mdl"
    PrecacheModel(ENTITY_MODEL);

    BaseClass::Precache();
}

void CMomentumPlayer::Spawn()
{
    SetModel(ENTITY_MODEL);
    BaseClass::Spawn();
    AddFlag(FL_GODMODE);
    RemoveSolidFlags(FSOLID_NOT_SOLID); //this removes the flag that was added while switching to spectator mode which prevented the player from activating triggers
    // do this here because we can't get a local player in the timer class
    ConVarRef gm("mom_gamemode");
    switch (gm.GetInt())
    {
    case MOMGM_BHOP:
    case MOMGM_SURF:
    case MOMGM_UNKNOWN:
    default:
        EnableAutoBhop();
        break;
    case MOMGM_SCROLL:
        DisableAutoBhop();
        break;
    }
    // Reset all bool gameevents 
    IGameEvent *mapZoneEvent = gameeventmanager->CreateEvent("player_inside_mapzone");
    IGameEvent *runSaveEvent = gameeventmanager->CreateEvent("run_save");
    IGameEvent *timerStartEvent = gameeventmanager->CreateEvent("timer_started");
    IGameEvent *practiceModeEvent = gameeventmanager->CreateEvent("practice_mode");
    if (mapZoneEvent)
    {
        mapZoneEvent->SetBool("inside_startzone", false);
        mapZoneEvent->SetBool("inside_endzone", false);
        mapZoneEvent->SetBool("map_finished", false);
        mapZoneEvent->SetInt("current_stage", 0);
        mapZoneEvent->SetInt("stage_ticks", 0);
        gameeventmanager->FireEvent(mapZoneEvent);
    }
    if (runSaveEvent)
    {
        runSaveEvent->SetBool("run_saved", false);
        runSaveEvent->SetBool("run_posted", false);
        gameeventmanager->FireEvent(runSaveEvent);
    }
    if (timerStartEvent)
    {
        timerStartEvent->SetBool("timer_isrunning", false);
        gameeventmanager->FireEvent(timerStartEvent);
    }
    if (practiceModeEvent)
    {
        practiceModeEvent->SetBool("has_practicemode", false);
        gameeventmanager->FireEvent(practiceModeEvent);
    }
    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("CURTIME");
    RegisterThinkContext("THINK_AVERAGE_STATS");
    RegisterThinkContext("CURTIME_FOR_START");
    SetContextThink(&CMomentumPlayer::UpdateRunStats, gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CheckForBhop, gpGlobals->curtime, "CURTIME");
    SetContextThink(&CMomentumPlayer::CalculateAverageStats, gpGlobals->curtime + AVERAGE_STATS_INTERVAL, "THINK_AVERAGE_STATS");
    SetContextThink(&CMomentumPlayer::LimitSpeedInStartZone, gpGlobals->curtime, "CURTIME_FOR_START");
    SetNextThink(gpGlobals->curtime);
}

void CMomentumPlayer::SurpressLadderChecks(const Vector &pos, const Vector &normal)
{
    m_ladderSurpressionTimer.Start(1.0f);
    m_lastLadderPos = pos;
    m_lastLadderNormal = normal;
}

bool CMomentumPlayer::CanGrabLadder(const Vector &pos, const Vector &normal)
{
    if (m_ladderSurpressionTimer.GetRemainingTime() <= 0.0f)
    {
        return true;
    }

    const float MaxDist = 64.0f;
    if (pos.AsVector2D().DistToSqr(m_lastLadderPos.AsVector2D()) < MaxDist * MaxDist)
    {
        return false;
    }

    if (normal != m_lastLadderNormal)
    {
        return true;
    }

    return false;
}

CBaseEntity *CMomentumPlayer::EntSelectSpawnPoint()
{
    CBaseEntity *pStart;
    pStart = NULL;
    if (SelectSpawnSpot("info_player_counterterrorist", pStart))
    {
        return pStart;
    }
    else if (SelectSpawnSpot("info_player_terrorist", pStart))
    {
        return pStart;
    }
    else if (SelectSpawnSpot("info_player_start", pStart))
    {
        return pStart;
    }
    else
    {
        DevMsg("No valid spawn point found.\n");
        return BaseClass::Instance(INDEXENT(0));
    }
}

bool CMomentumPlayer::SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pStart)
{
#define SF_PLAYER_START_MASTER 1
    pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    if (pStart == NULL) // skip over the null point
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    CBaseEntity *pLast;
    pLast = NULL;
    while (pStart != NULL)
    {
        if (g_pGameRules->IsSpawnPointValid(pStart, this))
        {
            if (pStart->HasSpawnFlags(SF_PLAYER_START_MASTER))
            {
                g_pLastSpawn = pStart;
                return true;
            }
        }
        pLast = pStart;
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    }
    if (pLast)
    {
        g_pLastSpawn = pLast;
        pStart = pLast;
        return true;
    }

    return false;
}

void CMomentumPlayer::Touch(CBaseEntity *pOther)
{
    BaseClass::Touch(pOther);

    if (g_MOMBlockFixer->IsBhopBlock(pOther->entindex()))
        g_MOMBlockFixer->PlayerTouch(this, pOther);
}

void CMomentumPlayer::EnableAutoBhop()
{
    m_bAutoBhop = true;
    DevLog("Enabled autobhop\n");
}
void CMomentumPlayer::DisableAutoBhop()
{
    m_bAutoBhop = false;
    DevLog("Disabled autobhop\n");
}
void CMomentumPlayer::CheckForBhop()
{
    if (GetGroundEntity() != NULL)
    {
        m_flTicksOnGround += gpGlobals->interval_per_tick;
        // true is player is on ground for less than 10 ticks, false if they are on ground for more s
        m_bDidPlayerBhop = (m_flTicksOnGround < NUM_TICKS_TO_BHOP * gpGlobals->interval_per_tick) != 0;
        if (!m_bDidPlayerBhop)
            m_iSuccessiveBhops = 0;
        if (m_nButtons & IN_JUMP)
        {
            m_flLastJumpVel = GetLocalVelocity().Length2D();
            m_iSuccessiveBhops++;
            if (g_Timer.IsRunning())
            {
                int currentStage = g_Timer.GetCurrentStageNumber();
                m_nStageJumps[0]++;
                m_nStageJumps[currentStage]++;
            }
        }
    }
    else
        m_flTicksOnGround = 0;

    SetNextThink(gpGlobals->curtime, "CURTIME");
}
void CMomentumPlayer::UpdateRunStats()
{
    //should velocity be XY or XYZ?
    ConVarRef hvel("mom_speedometer_hvel");
    IGameEvent *playerMoveEvent = gameeventmanager->CreateEvent("keypress");
    float velocity = hvel.GetBool() ? GetLocalVelocity().Length2D() : GetLocalVelocity().Length();

    if (g_Timer.IsRunning())
    {
        int currentStage = g_Timer.GetCurrentStageNumber();
        if (!m_bPrevTimerRunning) //timer started on this tick
        {    
            //Reset old run stats
            ResetRunStats();
            m_flStartSpeed = GetLocalVelocity().Length2D(); //prestrafe should always be XY only
            //Comapre against successive bhops to avoid incrimenting when the player was in the air without jumping (for surf)
            if (GetGroundEntity() == NULL && m_iSuccessiveBhops)
            {
                m_nStageJumps[0]++;
            }
        }
        if (m_nButtons & IN_MOVELEFT && !(m_nPrevButtons & IN_MOVELEFT))
        {
            m_nStageStrafes[0]++;
            m_nStageStrafes[currentStage]++;
        }
        else if (m_nButtons & IN_MOVERIGHT && !(m_nPrevButtons & IN_MOVERIGHT))
        {
            m_nStageStrafes[0]++;
            m_nStageStrafes[currentStage]++;
        }
        //  ---- MAX VELOCITY ----
        if (velocity > m_flStageVelocityMax[0])
            m_flStageVelocityMax[0] = velocity;
        //also do max velocity per stage
        if (velocity > m_flStageVelocityMax[currentStage])
            m_flStageVelocityMax[currentStage] = velocity;
        // ----------

        // --- STAGE ENTER VELOCITY ---
    }   
    //  ---- STRAFE SYNC -----
    float SyncVelocity = GetLocalVelocity().Length2DSqr(); //we always want HVEL for checking velocity sync
    if (!(GetFlags() & (FL_ONGROUND | FL_INWATER)) && GetMoveType() != MOVETYPE_LADDER)
    {
        if (EyeAngles().y > m_qangLastAngle.y) //player turned left 
        {
            m_nStrafeTicks++;
            if ((m_nButtons & IN_MOVELEFT) && !(m_nButtons & IN_MOVERIGHT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
        else if (EyeAngles().y < m_qangLastAngle.y) //player turned right 
        {
            m_nStrafeTicks++;
            if ((m_nButtons & IN_MOVERIGHT) && !(m_nButtons & IN_MOVELEFT))
                m_nPerfectSyncTicks++;
            if (SyncVelocity > m_flLastSyncVelocity)
                m_nAccelTicks++;
        }
    }
    if (m_nStrafeTicks && m_nAccelTicks && m_nPerfectSyncTicks)
    {
        m_flStrafeSync = ((float)m_nPerfectSyncTicks / (float)m_nStrafeTicks) * 100; // ticks strafing perfectly / ticks strafing
        m_flStrafeSync2 = ((float)m_nAccelTicks / (float)m_nStrafeTicks) * 100; // ticks gaining speed / ticks strafing
    }
    // ----------


    m_qangLastAngle = EyeAngles();
    m_flLastSyncVelocity = SyncVelocity;
    //this might be used in a later update
    //m_flLastVelocity = velocity;

    m_bPrevTimerRunning = g_Timer.IsRunning();
    m_nPrevButtons = m_nButtons;

    if (playerMoveEvent)
    {
        playerMoveEvent->SetInt("num_strafes", m_nStageStrafes[0]);
        playerMoveEvent->SetInt("num_jumps", m_nStageJumps[0]);
        if ((m_nButtons & IN_JUMP && GetGroundEntity() != NULL) || m_nButtons & IN_MOVELEFT | IN_MOVERIGHT)
            gameeventmanager->FireEvent(playerMoveEvent);
    }

    //think once per tick   
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
}
void CMomentumPlayer::ResetRunStats()
{
    m_nPerfectSyncTicks = 0;
    m_nStrafeTicks = 0;
    m_nAccelTicks = 0;
    m_flStrafeSync = 0;
    m_flStrafeSync2 = 0;

    for (int i = 0; i < MAX_STAGES; i++)
    {
        m_nStageAvgCount[i] = 0;
        m_nStageJumps[i] = 0;
        m_nStageStrafes[i] = 0;
        m_flStageTotalSync[i] = 0; 
        m_flStageTotalSync2[i] = 0;
        m_flStageTotalVelocity[i] = 0;
        m_flStageVelocityMax[i] = 0; 
        m_flStageVelocityAvg[i] = 0;
        m_flStageStrafeSyncAvg[i] = 0;
        m_flStageStrafeSync2Avg[i] = 0;
    }
}
void CMomentumPlayer::CalculateAverageStats()
{
    ConVarRef hvel("mom_speedometer_hvel");

    if (g_Timer.IsRunning())
    {
        int currentStage = g_Timer.GetCurrentStageNumber();

        m_flStageTotalSync[currentStage] += m_flStrafeSync;
        m_flStageTotalSync2[currentStage] += m_flStrafeSync2;
        m_flStageTotalVelocity[currentStage] += hvel.GetBool() ? GetLocalVelocity().Length2D() : GetLocalVelocity().Length();

        m_nStageAvgCount[currentStage]++;

        m_flStageStrafeSyncAvg[currentStage] = m_flStageTotalSync[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_flStageStrafeSync2Avg[currentStage] = m_flStageTotalSync2[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_flStageVelocityAvg[currentStage] = m_flStageTotalVelocity[currentStage] / float(m_nStageAvgCount[currentStage]);

        //stage 0 is "overall" - also update these as well, no matter which stage we are on
        m_flStageTotalSync[0] += m_flStrafeSync;
        m_flStageTotalSync2[0] += m_flStrafeSync2;
        m_flStageTotalVelocity[0] += hvel.GetBool() ? GetLocalVelocity().Length2D() : GetLocalVelocity().Length();
        m_nStageAvgCount[0]++;

        m_flStageStrafeSyncAvg[0] = m_flStageTotalSync[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_flStageStrafeSync2Avg[0] = m_flStageTotalSync2[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_flStageVelocityAvg[0] = m_flStageTotalVelocity[currentStage] / float(m_nStageAvgCount[currentStage]);
    }

    // think once per 0.1 second interval so we avoid making the totals extremely large
    SetNextThink(gpGlobals->curtime + AVERAGE_STATS_INTERVAL, "THINK_AVERAGE_STATS");
}
//This limits the player's speed in the start zone, depending on which gamemode the player is currently playing.
//On surf/other, it only limits practice mode speed. On bhop/scroll, it limits the movement speed above a certain threshhold, and 
//clamps the player's velocity if they go above it. This is to prevent prespeeding and is different per gamemode due to the different
//respective playstyles of surf and bhop.
void CMomentumPlayer::LimitSpeedInStartZone()
{
    ConVarRef gm("mom_gamemode");
    CTriggerTimerStart *startTrigger = g_Timer.GetStartTrigger();
    bool bhopGameMode = (gm.GetInt() == MOMGM_BHOP || gm.GetInt() == MOMGM_SCROLL);
    if (m_bInsideStartZone)
    {
        if (GetGroundEntity() == NULL && !g_Timer.IsPracticeMode(this)) //don't count ticks in air if we're in practice mode
            m_nTicksInAir++;
        else
            m_nTicksInAir = 0;

        //set bhop flag to true so we can't prespeed with practice mode
        if (g_Timer.IsPracticeMode(this)) m_bDidPlayerBhop = true; 

        //depending on gamemode, limit speed outright when player exceeds punish vel
        if (bhopGameMode  && ((!g_Timer.IsRunning() && m_nTicksInAir > MAX_AIRTIME_TICKS)))
        {
            Vector velocity = GetLocalVelocity();
            float PunishVelSquared = startTrigger->GetPunishSpeed()*startTrigger->GetPunishSpeed();
            if (velocity.Length2DSqr() > PunishVelSquared) //more efficent to check agaisnt the square of velocity
            {
                velocity = (velocity / velocity.Length()) * startTrigger->GetPunishSpeed();
                SetAbsVelocity(Vector(velocity.x, velocity.y, velocity.z));
            }
        }
    }
    SetNextThink(gpGlobals->curtime, "CURTIME_FOR_START");
}
