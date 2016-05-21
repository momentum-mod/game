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
SendPropBool(SENDINFO(m_bDidPlayerBhop)),
SendPropInt(SENDINFO(m_iSuccessiveBhops)),
SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)),
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
    m_RunData.m_iRunFlags = 0;
}

CMomentumPlayer::~CMomentumPlayer() 
{

}

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
    //BASECLASS SPAWN MUST BE AFTER SETTING THE MODEL, OTHERWISE A NULL HAPPENS!
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
    IGameEvent *runSaveEvent = gameeventmanager->CreateEvent("run_save");
    IGameEvent *runUploadEvent = gameeventmanager->CreateEvent("run_upload");
    IGameEvent *timerStartEvent = gameeventmanager->CreateEvent("timer_state");
    IGameEvent *practiceModeEvent = gameeventmanager->CreateEvent("practice_mode");
    m_RunData.m_bIsInZone = false;
    m_RunData.m_bMapFinished = false;
    m_RunData.m_iCurrentZone = 0;
    ResetRunStats();
    if (runSaveEvent)
    {
        runSaveEvent->SetBool("run_saved", false);
        gameeventmanager->FireEvent(runSaveEvent);
    }
    if (runUploadEvent)
    {
        runUploadEvent->SetBool("run_posted", false);
        runUploadEvent->SetString("web_msg", "");
        gameeventmanager->FireEvent(runUploadEvent);
    }
    if (timerStartEvent)
    {
        timerStartEvent->SetBool("is_running", false);
        gameeventmanager->FireEvent(timerStartEvent);
    }
    if (practiceModeEvent)
    {
        practiceModeEvent->SetBool("has_practicemode", false);
        gameeventmanager->FireEvent(practiceModeEvent);
    }
    //Linear/etc map
    g_Timer->DispatchMapInfo();

    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("CURTIME");
    RegisterThinkContext("THINK_AVERAGE_STATS");
    RegisterThinkContext("CURTIME_FOR_START");
    SetContextThink(&CMomentumPlayer::UpdateRunStats, gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CheckForBhop, gpGlobals->curtime, "CURTIME");
    SetContextThink(&CMomentumPlayer::CalculateAverageStats, gpGlobals->curtime + AVERAGE_STATS_INTERVAL, "THINK_AVERAGE_STATS");
    SetContextThink(&CMomentumPlayer::LimitSpeedInStartZone, gpGlobals->curtime, "CURTIME_FOR_START");
    SetNextThink(gpGlobals->curtime);
    DevLog("Finished spawn!\n");
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

void CMomentumPlayer::InitHUD()
{
    //g_Timer->DispatchStageCountMessage(); this was moved to spawn, under DispatchMapInfo
}

void CMomentumPlayer::EnableAutoBhop()
{
    m_RunData.m_bAutoBhop = true;
    DevLog("Enabled autobhop\n");
}
void CMomentumPlayer::DisableAutoBhop()
{
    m_RunData.m_bAutoBhop = false;
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
            m_RunData.m_flLastJumpVel = GetLocalVelocity().Length2D();
            m_iSuccessiveBhops++;
            if (g_Timer->IsRunning())
            {
                int currentStage = g_Timer->GetCurrentStageNumber();
                m_PlayerRunStats.m_iStageJumps[0]++;
                m_PlayerRunStats.m_iStageJumps[currentStage]++;
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
    IGameEvent *playerMoveEvent = gameeventmanager->CreateEvent("keypress");
    float velocity =  GetLocalVelocity().Length();
    float velocity2D = GetLocalVelocity().Length2D();

    if (g_Timer->IsRunning())
    {
        int currentStage = g_Timer->GetCurrentStageNumber();
        if (!m_bPrevTimerRunning) //timer started on this tick
        {
            //Reset old run stats -- moved to on start's touch
            m_PlayerRunStats.m_flStageEnterSpeed[0][0] = velocity;
            m_PlayerRunStats.m_flStageEnterSpeed[0][1] = velocity2D;
            //Compare against successive bhops to avoid incrimenting when the player was in the air without jumping (for surf)
            if (GetGroundEntity() == NULL && m_iSuccessiveBhops)
            {
                m_PlayerRunStats.m_iStageJumps[0]++;
                m_PlayerRunStats.m_iStageJumps[currentStage]++;
            }
            if (m_nButtons & IN_MOVERIGHT || m_nButtons & IN_MOVELEFT)
            {
                m_PlayerRunStats.m_iStageStrafes[0]++;
                m_PlayerRunStats.m_iStageStrafes[currentStage]++;
            }
        }
        if (m_nButtons & IN_MOVELEFT && !(m_nPrevButtons & IN_MOVELEFT))
        {
            m_PlayerRunStats.m_iStageStrafes[0]++;
            m_PlayerRunStats.m_iStageStrafes[currentStage]++;
        }
        else if (m_nButtons & IN_MOVERIGHT && !(m_nPrevButtons & IN_MOVERIGHT))
        {
            m_PlayerRunStats.m_iStageStrafes[0]++;
            m_PlayerRunStats.m_iStageStrafes[currentStage]++;
        }
        //  ---- MAX VELOCITY ----
        if (velocity > m_PlayerRunStats.m_flStageVelocityMax[0][0])
            m_PlayerRunStats.m_flStageVelocityMax[0][0] = velocity;
        if (velocity2D > m_PlayerRunStats.m_flStageVelocityMax[0][1])
            m_PlayerRunStats.m_flStageVelocityMax[0][1] = velocity;
        //also do max velocity per stage
        if (velocity >m_PlayerRunStats.m_flStageVelocityMax[currentStage][0])
            m_PlayerRunStats.m_flStageVelocityMax[currentStage][0] = velocity;
        if (velocity2D > m_PlayerRunStats.m_flStageVelocityMax[currentStage][1])
            m_PlayerRunStats.m_flStageVelocityMax[currentStage][1] = velocity2D;
        // ----------

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
            m_RunData.m_flStrafeSync = (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks strafing perfectly / ticks strafing
            m_RunData.m_flStrafeSync2 = (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
        }
        // ----------

        m_qangLastAngle = EyeAngles();
        m_flLastSyncVelocity = SyncVelocity;
        //this might be used in a later update
        //m_flLastVelocity = velocity;

        m_bPrevTimerRunning = g_Timer->IsRunning();
        m_nPrevButtons = m_nButtons;
    }   

    if (playerMoveEvent)
    {
        playerMoveEvent->SetInt("num_strafes", m_PlayerRunStats.m_iStageStrafes[0]);
        playerMoveEvent->SetInt("num_jumps", m_PlayerRunStats.m_iStageJumps[0]);
        bool onGround = GetFlags() & FL_ONGROUND;
        if ((m_nButtons & IN_JUMP) && onGround || m_nButtons & (IN_MOVELEFT | IN_MOVERIGHT))
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
    m_RunData.m_flStrafeSync = 0;
    m_RunData.m_flStrafeSync2 = 0;

    m_PlayerRunStats = RunStats_t();
}
void CMomentumPlayer::CalculateAverageStats()
{

    if (g_Timer->IsRunning())
    {
        int currentStage = g_Timer->GetCurrentStageNumber();

        m_flStageTotalSync[currentStage] += m_RunData.m_flStrafeSync;
        m_flStageTotalSync2[currentStage] += m_RunData.m_flStrafeSync2;
        m_flStageTotalVelocity[currentStage][0] += GetLocalVelocity().Length();
        m_flStageTotalVelocity[currentStage][1] += GetLocalVelocity().Length2D();

        m_nStageAvgCount[currentStage]++;

        m_PlayerRunStats.m_flStageStrafeSyncAvg[currentStage] = m_flStageTotalSync[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_PlayerRunStats.m_flStageStrafeSync2Avg[currentStage] = m_flStageTotalSync2[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_PlayerRunStats.m_flStageVelocityAvg[currentStage][0] = m_flStageTotalVelocity[currentStage][0] / float(m_nStageAvgCount[currentStage]);
        m_PlayerRunStats.m_flStageVelocityAvg[currentStage][1] = m_flStageTotalVelocity[currentStage][1] / float(m_nStageAvgCount[currentStage]);

        //stage 0 is "overall" - also update these as well, no matter which stage we are on
        m_flStageTotalSync[0] += m_RunData.m_flStrafeSync;
        m_flStageTotalSync2[0] += m_RunData.m_flStrafeSync2;
        m_flStageTotalVelocity[0][0] += GetLocalVelocity().Length();
        m_flStageTotalVelocity[0][1] += GetLocalVelocity().Length2D();
        m_nStageAvgCount[0]++;

        m_PlayerRunStats.m_flStageStrafeSyncAvg[0] = m_flStageTotalSync[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_PlayerRunStats.m_flStageStrafeSync2Avg[0] = m_flStageTotalSync2[currentStage] / float(m_nStageAvgCount[currentStage]);
        m_PlayerRunStats.m_flStageVelocityAvg[0][0] = m_flStageTotalVelocity[currentStage][0] / float(m_nStageAvgCount[currentStage]);
        m_PlayerRunStats.m_flStageVelocityAvg[0][1] = m_flStageTotalVelocity[currentStage][1] / float(m_nStageAvgCount[currentStage]);
    }

    // think once per 0.1 second interval so we avoid making the totals extremely large
    SetNextThink(gpGlobals->curtime + AVERAGE_STATS_INTERVAL, "THINK_AVERAGE_STATS");
}
//This limits the player's speed in the start zone, depending on which gamemode the player is currently playing.
//On surf/other, it only limits practice mode speed. On bhop/scroll, it limits the movement speed above a certain threshhold, and 
//clamps the player's velocity if they go above it. This is to prevent prespeeding and is different per gamemode due to the different
//respective playstyles of surf and bhop.
//MOM_TODO: Update this to extend to start zones of stages (if doing ILs)
void CMomentumPlayer::LimitSpeedInStartZone()
{
    ConVarRef gm("mom_gamemode");
    CTriggerTimerStart *startTrigger = g_Timer->GetStartTrigger();
    bool bhopGameMode = (gm.GetInt() == MOMGM_BHOP || gm.GetInt() == MOMGM_SCROLL);
    if (m_RunData.m_bIsInZone && m_RunData.m_iCurrentZone == 1)
    {
        if (GetGroundEntity() == nullptr && !g_Timer->IsPracticeMode(this)) //don't count ticks in air if we're in practice mode
            m_nTicksInAir++;
        else
            m_nTicksInAir = 0;

        //set bhop flag to true so we can't prespeed with practice mode
        if (g_Timer->IsPracticeMode(this)) m_bDidPlayerBhop = true;

        //depending on gamemode, limit speed outright when player exceeds punish vel
        if (bhopGameMode && ((!g_Timer->IsRunning() && m_nTicksInAir > MAX_AIRTIME_TICKS)))
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
//override of CBasePlayer::IsValidObserverTarget that allows us to spectate replay ghosts
bool CMomentumPlayer::IsValidObserverTarget(CBaseEntity *target)
{
    if (target == NULL)
        return false;

    if (!target->IsPlayer())
    {
        if (!Q_strcmp(target->GetClassname(), "mom_replay_ghost")) //target is a replay ghost
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    CMomentumPlayer *player = ToCMOMPlayer( target );

    /* Don't spec observers or players who haven't picked a class yet */
    if ( player->IsObserver() )
        return false;

    if( player == this )
        return false; // We can't observe ourselves.

    if ( player->IsEffectActive( EF_NODRAW ) ) // don't watch invisible players
        return false;

    if ( player->m_lifeState == LIFE_RESPAWNABLE ) // target is dead, waiting for respawn
        return false;

    if ( player->m_lifeState == LIFE_DEAD || player->m_lifeState == LIFE_DYING )
    {
        if ( (player->m_flDeathTime + DEATH_ANIMATION_TIME ) < gpGlobals->curtime )
        {
            return false;	// allow watching until 3 seconds after death to see death animation
        }
    }

    return true;	// passed all tests
}