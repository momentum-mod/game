#include "cbase.h"
#include "Timer.h"
#include "in_buttons.h"
#include "mom_player.h"
#include "mom_triggers.h"

#include "tier0/memdbgon.h"

#define AVERAGE_STATS_INTERVAL 0.1

IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropInt(SENDINFO(m_iShotsFired)),
SendPropInt(SENDINFO(m_iDirection)),
SendPropBool(SENDINFO(m_bResumeZoom)),
SendPropInt(SENDINFO(m_iLastZoom)),
SendPropBool(SENDINFO(m_bDidPlayerBhop)),
SendPropInt(SENDINFO(m_iSuccessiveBhops)),
SendPropBool(SENDINFO(m_bHasPracticeMode)),
SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumPlayer)
DEFINE_THINKFUNC(CheckForBhop),
DEFINE_THINKFUNC(UpdateRunStats),
DEFINE_THINKFUNC(CalculateAverageStats),
DEFINE_THINKFUNC(LimitSpeedInStartZone),
END_DATADESC();

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);

CMomentumPlayer::CMomentumPlayer()
{
    m_flPunishTime = -1;
    m_iLastBlock = -1;
    m_RunData.m_iRunFlags = 0;
}

CMomentumPlayer::~CMomentumPlayer() {}

void CMomentumPlayer::Precache()
{
// Name of our entity's model
    //MOM_TODO: Replace this with the custom player model
#define ENTITY_MODEL "models/gibs/airboat_broken_engine.mdl"
    PrecacheModel(ENTITY_MODEL);

    BaseClass::Precache();
}

void CMomentumPlayer::Spawn()
{
    SetModel(ENTITY_MODEL);
    // BASECLASS SPAWN MUST BE AFTER SETTING THE MODEL, OTHERWISE A NULL HAPPENS!
    BaseClass::Spawn();
    AddFlag(FL_GODMODE);
    RemoveSolidFlags(FSOLID_NOT_SOLID); // this removes the flag that was added while switching to spectator mode which
                                        // prevented the player from activating triggers
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
    m_RunData.m_bIsInZone = false;
    m_RunData.m_bMapFinished = false;
    m_RunData.m_iCurrentZone = 0;
    m_bHasPracticeMode = false;
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
        timerStartEvent->SetInt("ent", entindex());
        timerStartEvent->SetBool("is_running", false);
        gameeventmanager->FireEvent(timerStartEvent);
    }
    // Linear/etc map
    g_Timer->DispatchMapInfo();

    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("CURTIME");
    RegisterThinkContext("THINK_AVERAGE_STATS");
    RegisterThinkContext("CURTIME_FOR_START");
    RegisterThinkContext("TWEEN");
    SetContextThink(&CMomentumPlayer::UpdateRunStats, gpGlobals->curtime + gpGlobals->interval_per_tick,
                    "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CheckForBhop, gpGlobals->curtime, "CURTIME");
    SetContextThink(&CMomentumPlayer::CalculateAverageStats, gpGlobals->curtime + AVERAGE_STATS_INTERVAL,
                    "THINK_AVERAGE_STATS");
    SetContextThink(&CMomentumPlayer::LimitSpeedInStartZone, gpGlobals->curtime, "CURTIME_FOR_START");
    SetContextThink(&CMomentumPlayer::TweenSlowdownPlayer, gpGlobals->curtime, "TWEEN");

    SetNextThink(gpGlobals->curtime);
    DevLog("Finished spawn!\n");
}

// Obtains the player's previous origin using their current origin as a base.
Vector CMomentumPlayer::GetPrevOrigin(void)
{
    return GetPrevOrigin(GetLocalOrigin());
}

// Obtains the player's previous origin using a vector as the base, subtracting one tick's worth of velocity.
Vector CMomentumPlayer::GetPrevOrigin(const Vector &base)
{
    Vector velocity = GetLocalVelocity();
    Vector prevOrigin(base.x - (velocity.x * gpGlobals->interval_per_tick),
        base.y - (velocity.y * gpGlobals->interval_per_tick),
        base.z - (velocity.z * gpGlobals->interval_per_tick));
    return prevOrigin;
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
    CBaseEntity *pStart = nullptr;
    const char *spawns[] = {"info_player_counterterrorist", "info_player_terrorist", "info_player_start"};
    for (int i = 0; i < 3; i++)
    {
        if (SelectSpawnSpot(spawns[i], pStart))
            return pStart;
    }

    DevMsg("No valid spawn point found.\n");
    return Instance(INDEXENT(0));
}

bool CMomentumPlayer::SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pStart)
{
#define SF_PLAYER_START_MASTER 1
    pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    if (pStart == nullptr) // skip over the null point
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    CBaseEntity *pLast = nullptr;
    while (pStart != nullptr)
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
    if (GetGroundEntity() != nullptr)
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
                int currentZone = m_RunData.m_iCurrentZone;//g_Timer->GetCurrentZoneNumber();
                m_PlayerRunStats.m_iZoneJumps[0]++;
                m_PlayerRunStats.m_iZoneJumps[currentZone]++;
            }
        }
    }
    else
        m_flTicksOnGround = 0;

    SetNextThink(gpGlobals->curtime, "CURTIME");
}

void CMomentumPlayer::UpdateRunStats()
{
    IGameEvent *playerMoveEvent = gameeventmanager->CreateEvent("keypress");
    float velocity = GetLocalVelocity().Length();
    float velocity2D = GetLocalVelocity().Length2D();

    if (g_Timer->IsRunning())
    {
        int currentZone = m_RunData.m_iCurrentZone;//g_Timer->GetCurrentZoneNumber();
        if (!m_bPrevTimerRunning) // timer started on this tick
        {
            // Compare against successive bhops to avoid incrimenting when the player was in the air without jumping
            // (for surf)
            if (GetGroundEntity() == nullptr && m_iSuccessiveBhops)
            {
                m_PlayerRunStats.m_iZoneJumps[0]++;
                m_PlayerRunStats.m_iZoneJumps[currentZone]++;
            }
            if (m_nButtons & IN_MOVERIGHT || m_nButtons & IN_MOVELEFT)
            {
                m_PlayerRunStats.m_iZoneStrafes[0]++;
                m_PlayerRunStats.m_iZoneStrafes[currentZone]++;
            }
        }
        if (m_nButtons & IN_MOVELEFT && !(m_nPrevButtons & IN_MOVELEFT))
        {
            m_PlayerRunStats.m_iZoneStrafes[0]++;
            m_PlayerRunStats.m_iZoneStrafes[currentZone]++;
        }
        else if (m_nButtons & IN_MOVERIGHT && !(m_nPrevButtons & IN_MOVERIGHT))
        {
            m_PlayerRunStats.m_iZoneStrafes[0]++;
            m_PlayerRunStats.m_iZoneStrafes[currentZone]++;
        }
        //  ---- MAX VELOCITY ----
        if (velocity > m_PlayerRunStats.m_flZoneVelocityMax[0][0])
            m_PlayerRunStats.m_flZoneVelocityMax[0][0] = velocity;
        if (velocity2D > m_PlayerRunStats.m_flZoneVelocityMax[0][1])
            m_PlayerRunStats.m_flZoneVelocityMax[0][1] = velocity;
        // also do max velocity per stage
        if (velocity > m_PlayerRunStats.m_flZoneVelocityMax[currentZone][0])
            m_PlayerRunStats.m_flZoneVelocityMax[currentZone][0] = velocity;
        if (velocity2D > m_PlayerRunStats.m_flZoneVelocityMax[currentZone][1])
            m_PlayerRunStats.m_flZoneVelocityMax[currentZone][1] = velocity2D;
        // ----------

        //  ---- STRAFE SYNC -----
        float SyncVelocity = GetLocalVelocity().Length2DSqr(); // we always want HVEL for checking velocity sync
        if (!(GetFlags() & (FL_ONGROUND | FL_INWATER)) && GetMoveType() != MOVETYPE_LADDER)
        {
            if (EyeAngles().y > m_qangLastAngle.y) // player turned left
            {
                m_nStrafeTicks++;
                if ((m_nButtons & IN_MOVELEFT) && !(m_nButtons & IN_MOVERIGHT))
                    m_nPerfectSyncTicks++;
                if (SyncVelocity > m_flLastSyncVelocity)
                    m_nAccelTicks++;
            }
            else if (EyeAngles().y < m_qangLastAngle.y) // player turned right
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
            m_RunData.m_flStrafeSync = (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) *
                                       100.0f; // ticks strafing perfectly / ticks strafing
            m_RunData.m_flStrafeSync2 =
                (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f; // ticks gaining speed / ticks strafing
        }
        // ----------

        m_qangLastAngle = EyeAngles();
        m_flLastSyncVelocity = SyncVelocity;
        // this might be used in a later update
        // m_flLastVelocity = velocity;

        m_bPrevTimerRunning = g_Timer->IsRunning();
        m_nPrevButtons = m_nButtons;
    }

    if (playerMoveEvent)
    {
        playerMoveEvent->SetInt("ent", entindex());
        playerMoveEvent->SetInt("num_strafes", m_PlayerRunStats.m_iZoneStrafes[0]);
        playerMoveEvent->SetInt("num_jumps", m_PlayerRunStats.m_iZoneJumps[0]);
        bool onGround = GetFlags() & FL_ONGROUND;
        if ((m_nButtons & IN_JUMP) && onGround || m_nButtons & (IN_MOVELEFT | IN_MOVERIGHT))
            gameeventmanager->FireEvent(playerMoveEvent);
    }

    // think once per tick
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
}
void CMomentumPlayer::ResetRunStats()
{
    m_nPerfectSyncTicks = 0;
    m_nStrafeTicks = 0;
    m_nAccelTicks = 0;
    m_RunData.m_flStrafeSync = 0;
    m_RunData.m_flStrafeSync2 = 0;
    m_PlayerRunStats = RunStats_t(g_Timer->GetZoneCount());
}
void CMomentumPlayer::CalculateAverageStats()
{

    if (g_Timer->IsRunning())
    {
        int currentZone = m_RunData.m_iCurrentZone;//g_Timer->GetCurrentZoneNumber();

        m_flZoneTotalSync[currentZone] += m_RunData.m_flStrafeSync;
        m_flZoneTotalSync2[currentZone] += m_RunData.m_flStrafeSync2;
        m_flZoneTotalVelocity[currentZone][0] += GetLocalVelocity().Length();
        m_flZoneTotalVelocity[currentZone][1] += GetLocalVelocity().Length2D();

        m_nZoneAvgCount[currentZone]++;

        m_PlayerRunStats.m_flZoneStrafeSyncAvg[currentZone] =
            m_flZoneTotalSync[currentZone] / float(m_nZoneAvgCount[currentZone]);
        m_PlayerRunStats.m_flZoneStrafeSync2Avg[currentZone] =
            m_flZoneTotalSync2[currentZone] / float(m_nZoneAvgCount[currentZone]);
        m_PlayerRunStats.m_flZoneVelocityAvg[currentZone][0] =
            m_flZoneTotalVelocity[currentZone][0] / float(m_nZoneAvgCount[currentZone]);
        m_PlayerRunStats.m_flZoneVelocityAvg[currentZone][1] =
            m_flZoneTotalVelocity[currentZone][1] / float(m_nZoneAvgCount[currentZone]);

        // stage 0 is "overall" - also update these as well, no matter which stage we are on
        m_flZoneTotalSync[0] += m_RunData.m_flStrafeSync;
        m_flZoneTotalSync2[0] += m_RunData.m_flStrafeSync2;
        m_flZoneTotalVelocity[0][0] += GetLocalVelocity().Length();
        m_flZoneTotalVelocity[0][1] += GetLocalVelocity().Length2D();
        m_nZoneAvgCount[0]++;

        m_PlayerRunStats.m_flZoneStrafeSyncAvg[0] =
            m_flZoneTotalSync[currentZone] / float(m_nZoneAvgCount[currentZone]);
        m_PlayerRunStats.m_flZoneStrafeSync2Avg[0] =
            m_flZoneTotalSync2[currentZone] / float(m_nZoneAvgCount[currentZone]);
        m_PlayerRunStats.m_flZoneVelocityAvg[0][0] =
            m_flZoneTotalVelocity[currentZone][0] / float(m_nZoneAvgCount[currentZone]);
        m_PlayerRunStats.m_flZoneVelocityAvg[0][1] =
            m_flZoneTotalVelocity[currentZone][1] / float(m_nZoneAvgCount[currentZone]);
    }

    // think once per 0.1 second interval so we avoid making the totals extremely large
    SetNextThink(gpGlobals->curtime + AVERAGE_STATS_INTERVAL, "THINK_AVERAGE_STATS");
}
// This limits the player's speed in the start zone, depending on which gamemode the player is currently playing.
// On surf/other, it only limits practice mode speed. On bhop/scroll, it limits the movement speed above a certain
// threshhold, and clamps the player's velocity if they go above it. 
// This is to prevent prespeeding and is different per gamemode due to the different respective playstyles of surf and bhop.
// MOM_TODO: Update this to extend to start zones of stages (if doing ILs)
void CMomentumPlayer::LimitSpeedInStartZone()
{
    ConVarRef gm("mom_gamemode");
    CTriggerTimerStart *startTrigger = g_Timer->GetStartTrigger();
    bool bhopGameMode = (gm.GetInt() == MOMGM_BHOP || gm.GetInt() == MOMGM_SCROLL);
    if (m_RunData.m_bIsInZone && m_RunData.m_iCurrentZone == 1)
    {
        if (GetGroundEntity() == nullptr &&
            !m_bHasPracticeMode) // don't count ticks in air if we're in practice mode
            m_nTicksInAir++;
        else
            m_nTicksInAir = 0;

        // set bhop flag to true so we can't prespeed with practice mode
        if (m_bHasPracticeMode)
            m_bDidPlayerBhop = true;

        // depending on gamemode, limit speed outright when player exceeds punish vel
        if (bhopGameMode && ((!g_Timer->IsRunning() && m_nTicksInAir > MAX_AIRTIME_TICKS)))
        {
            Vector velocity = GetLocalVelocity();
            float PunishVelSquared = startTrigger->GetPunishSpeed() * startTrigger->GetPunishSpeed();
            if (velocity.Length2DSqr() > PunishVelSquared) // more efficent to check agaisnt the square of velocity
            {
                velocity = (velocity / velocity.Length()) * startTrigger->GetPunishSpeed();
                SetAbsVelocity(Vector(velocity.x, velocity.y, velocity.z));
            }
        }
    }
    SetNextThink(gpGlobals->curtime, "CURTIME_FOR_START");
}
// override of CBasePlayer::IsValidObserverTarget that allows us to spectate replay ghosts
bool CMomentumPlayer::IsValidObserverTarget(CBaseEntity *target)
{
    if (target == nullptr)
        return false;

    if (!target->IsPlayer())
    {
        if (!Q_strcmp(target->GetClassname(), "mom_replay_ghost")) // target is a replay ghost
        {
            return true;
        }
        return false;
    }

    return BaseClass::IsValidObserverTarget(target);
}

// Override of CBasePlayer::SetObserverTarget that lets us add/remove ourselves as spectors to the ghost
bool CMomentumPlayer::SetObserverTarget(CBaseEntity* target)
{
    CMomentumReplayGhostEntity *pGhostToSpectate = dynamic_cast<CMomentumReplayGhostEntity*>(target),
        *pCurrentGhost = GetReplayEnt();

    if (pCurrentGhost)
    {
        pCurrentGhost->RemoveSpectator(this);
    }

    bool base = BaseClass::SetObserverTarget(target);

    if (pGhostToSpectate && base)
    {
        pGhostToSpectate->AddSpectator(this);
    }

    return base;
}
void CMomentumPlayer::TweenSlowdownPlayer()
{
    if (m_RunData.m_bMapFinished) //slowdown when map is finished
    {
        //decrease our lagged movement value by 10% every tick
        m_flTweenVelValue += (0.01f - m_flTweenVelValue) * 0.1f; 
    }
    else
    {
        m_flTweenVelValue = 1.0f;
    }
    SetLaggedMovementValue(m_flTweenVelValue);

    SetNextThink(gpGlobals->curtime, "TWEEN");
}
