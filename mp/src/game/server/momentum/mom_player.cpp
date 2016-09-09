#include "cbase.h"

#include "Timer.h"
#include "in_buttons.h"
#include "mom_player.h"
#include "mom_replay_entity.h"
#include "predicted_viewmodel.h"
#include "mom_system_checkpoint.h"
#include "mom_triggers.h"
#include "momentum/weapon/weapon_csbasegun.h"

#include "tier0/memdbgon.h"

#define AVERAGE_STATS_INTERVAL 0.1

IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropExclude("DT_BaseAnimating", "m_nMuzzleFlashParity"),
SendPropInt(SENDINFO(m_iShotsFired)),
SendPropInt(SENDINFO(m_iDirection)),
SendPropBool(SENDINFO(m_bResumeZoom)),
SendPropInt(SENDINFO(m_iLastZoom)),
SendPropBool(SENDINFO(m_bDidPlayerBhop)),
SendPropInt(SENDINFO(m_iSuccessiveBhops)),
SendPropBool(SENDINFO(m_bHasPracticeMode)),
SendPropBool(SENDINFO(m_bUsingCPMenu)),
SendPropInt(SENDINFO(m_iCurrentStepCP)),
SendPropInt(SENDINFO(m_iCheckpointCount)),
SendPropDataTable(SENDINFO_DT(m_RunData), &REFERENCE_SEND_TABLE(DT_MOM_RunEntData)),
SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MOM_RunStats)),
END_SEND_TABLE();


BEGIN_DATADESC(CMomentumPlayer)
DEFINE_THINKFUNC(CheckForBhop)
, DEFINE_THINKFUNC(UpdateRunStats), DEFINE_THINKFUNC(CalculateAverageStats), DEFINE_THINKFUNC(LimitSpeedInStartZone),
    END_DATADESC();

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);

CMomentumPlayer::CMomentumPlayer()
    : m_duckUntilOnGround(false), m_flStamina(0.0f), m_flTicksOnGround(0.0f), m_flLastVelocity(0.0f), m_flLastSyncVelocity(0),
      m_nPerfectSyncTicks(0), m_nStrafeTicks(0), m_nAccelTicks(0), m_bPrevTimerRunning(false), m_nPrevButtons(0),
      m_nTicksInAir(0), m_flTweenVelValue(1.0f)
{
    m_flPunishTime = -1;
    m_iLastBlock = -1;
    m_RunData.m_iRunFlags = 0;
    m_iShotsFired = 0;
    m_iDirection = 0;
    m_bResumeZoom = false;
    m_iLastZoom = 0;
    m_bDidPlayerBhop = false;
    m_iSuccessiveBhops = 0;
    m_bHasPracticeMode = false;

    m_iCheckpointCount = 0;
    m_bUsingCPMenu = false;
    m_iCurrentStepCP = -1;

    ListenForGameEvent("mapfinished_panel_closed");
}

CMomentumPlayer::~CMomentumPlayer() {}

void CMomentumPlayer::Precache()
{
    PrecacheModel(ENTITY_MODEL);

    PrecacheScriptSound(SND_FLASHLIGHT_ON);
    PrecacheScriptSound(SND_FLASHLIGHT_OFF);

    BaseClass::Precache();
}

//Used for making the view model like CS's
void CMomentumPlayer::CreateViewModel(int index)
{
    Assert(index >= 0 && index < MAX_VIEWMODELS);

    if (GetViewModel(index))
        return;

    CPredictedViewModel *vm = dynamic_cast<CPredictedViewModel *>(CreateEntityByName("predicted_viewmodel"));
    if (vm)
    {
        vm->SetAbsOrigin(GetAbsOrigin());
        vm->SetOwner(this);
        vm->SetIndex(index);
        DispatchSpawn(vm);
        vm->FollowEntity(this, false);
        m_hViewModel.Set(index, vm);
    }
}

void CMomentumPlayer::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        // Hide the mapfinished panel and reset our speed to normal
        m_RunData.m_bMapFinished = false;
        SetLaggedMovementValue(1.0f);

        // Fix for the replay system not being able to listen to events
        if (g_ReplaySystem->GetReplayManager()->GetPlaybackReplay())
        {
            g_ReplaySystem->GetReplayManager()->UnloadPlayback();
        }
    }
}

void CMomentumPlayer::Spawn()
{
    SetModel(ENTITY_MODEL);
    SetBodygroup(1, 11); // BODY_PROLATE_ELLIPSE
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
    {
        int startnum = 0;
        int endnum = 0;

        CBaseEntity *pEnt;

        pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_start");
        while (pEnt)
        {
            startnum++;
            pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_start");
        }

        pEnt = gEntList.FindEntityByClassname(nullptr, "trigger_momentum_timer_stop");
        while (pEnt)
        {
            endnum++;
            pEnt = gEntList.FindEntityByClassname(pEnt, "trigger_momentum_timer_stop");
        }

        if (startnum == 0 || endnum == 0)
        {
            CSingleUserRecipientFilter filter(this);
            filter.MakeReliable();
            UserMessageBegin(filter, "MB_NoStartOrEnd");
            MessageEnd();
        }
    }
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

    //Load the player's checkpoints
    g_MOMCheckpointSystem->LoadMapCheckpoints(this);
}

// Obtains the player's previous origin using their current origin as a base.
Vector CMomentumPlayer::GetPrevOrigin(void) { return GetPrevOrigin(GetLocalOrigin()); }

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

bool CMomentumPlayer::ClientCommand(const CCommand &args)
{
    auto cmd = args[0];

    // We're overriding this to prevent the spec_mode to change to ROAMING,
    // remove this if we want to allow the player to fly around their ghost as it goes
    // (and change the ghost entity code to match as well)
    if (FStrEq(cmd, "spec_mode")) // new observer mode
    {
        int mode;

        if (GetObserverMode() == OBS_MODE_FREEZECAM)
        {
            AttemptToExitFreezeCam();
            return true;
        }

        // check for parameters.
        if (args.ArgC() >= 2)
        {
            mode = atoi(args[1]);

            if (mode < OBS_MODE_IN_EYE || mode > OBS_MODE_CHASE)
                mode = OBS_MODE_IN_EYE;
        }
        else
        {
            // switch to next spec mode if no parameter given
            mode = GetObserverMode() + 1;

            if (mode > OBS_MODE_CHASE)
            {
                mode = OBS_MODE_IN_EYE;
            }
            else if (mode < OBS_MODE_IN_EYE)
            {
                mode = OBS_MODE_CHASE;
            }
        }

        // don't allow input while player or death cam animation
        if (GetObserverMode() > OBS_MODE_DEATHCAM)
        {
            // set new spectator mode, don't allow OBS_MODE_NONE
            if (!SetObserverMode(mode))
                ClientPrint(this, HUD_PRINTCONSOLE, "#Spectator_Mode_Unkown");
            else
                engine->ClientCommand(edict(), "cl_spec_mode %d", mode);
        }
        else
        {
            // remember spectator mode for later use
            m_iObserverLastMode = mode;
            engine->ClientCommand(edict(), "cl_spec_mode %d", mode);
        }

        return true;
    }
    if (FStrEq(cmd, "drop"))
    {
        CWeaponCSBase *pWeapon = dynamic_cast< CWeaponCSBase* >(GetActiveWeapon());

        if (pWeapon)
        {
            CSWeaponType type = pWeapon->GetCSWpnData().m_WeaponType;

            if (type != WEAPONTYPE_KNIFE && type != WEAPONTYPE_GRENADE)
            {
                MomentumWeaponDrop(pWeapon);
            }
        }

        return true;
    }

    return BaseClass::ClientCommand(args);
}

void CMomentumPlayer::MomentumWeaponDrop(CBaseCombatWeapon *pWeapon)
{
    Weapon_Drop(pWeapon, nullptr, nullptr);
    pWeapon->StopFollowingEntity();
    UTIL_Remove(pWeapon);
}

void CMomentumPlayer::CreateCheckpoint()
{
    Checkpoint c;
    c.ang = GetAbsAngles();
    c.pos = GetAbsOrigin();
    c.vel = GetAbsVelocity();
    Q_strncpy(c.targetName, GetEntityName().ToCStr(), sizeof(c.targetName));
    Q_strncpy(c.targetClassName, GetClassname(), sizeof(c.targetClassName));
    m_rcCheckpoints.AddToTail(c);
    if (m_iCurrentStepCP == m_iCheckpointCount - 1)
        ++m_iCurrentStepCP;
    else
        m_iCurrentStepCP = m_iCheckpointCount;//Set it to the new checkpoint's index
    ++m_iCheckpointCount;
}

void CMomentumPlayer::RemoveLastCheckpoint()
{
    if (m_rcCheckpoints.IsEmpty()) return;
    m_rcCheckpoints.Remove(m_iCurrentStepCP);
    //If there's one element left, we still need to decrease currentStep to -1
    if (m_iCurrentStepCP == m_iCheckpointCount - 1)
        --m_iCurrentStepCP;
    //else we want it to shift forward one until it catches back up to the last checkpoint
    --m_iCheckpointCount;
}

void CMomentumPlayer::RemoveAllCheckpoints()
{
    m_rcCheckpoints.RemoveAll();
    m_iCurrentStepCP = -1;
    m_iCheckpointCount = 0;
}

void CMomentumPlayer::TeleportToCP(int newCheckpoint)
{
    if (newCheckpoint > m_rcCheckpoints.Count() || newCheckpoint < 0) return;
    Checkpoint c = m_rcCheckpoints[newCheckpoint];
    SetName(MAKE_STRING(c.targetName));
    SetClassname(c.targetClassName);
    Teleport(&c.pos, &c.ang, &c.vel);
}

void CMomentumPlayer::SaveCPsToFile(KeyValues* kvInto)
{
    FOR_EACH_VEC(m_rcCheckpoints, i)
    {
        Checkpoint c = m_rcCheckpoints[i];
        char szCheckpointNum[6];//9 million checkpoints is pretty generous
        Q_snprintf(szCheckpointNum, 6, "%i", i);
        KeyValues *kvCP = new KeyValues(szCheckpointNum);
        kvCP->SetString("targetName", c.targetName);
        kvCP->SetString("targetClassName", c.targetClassName);
        mom_UTIL->KVSaveVector(kvCP, "vel", c.vel);
        mom_UTIL->KVSaveVector(kvCP, "pos", c.pos);
        mom_UTIL->KVSaveQAngles(kvCP, "ang", c.ang);
        kvInto->AddSubKey(kvCP);
    }
}

void CMomentumPlayer::LoadCPsFromFile(KeyValues* kvFrom)
{
    if (!kvFrom) return;
    FOR_EACH_SUBKEY(kvFrom, kvCheckpoint)
    {
        Checkpoint c;
        Q_strncpy(c.targetName, kvCheckpoint->GetString("targetName"), sizeof(c.targetName));
        Q_strncpy(c.targetClassName, kvCheckpoint->GetString("targetClassName"), sizeof(c.targetClassName));
        mom_UTIL->KVLoadVector(kvCheckpoint, "pos", c.pos);
        mom_UTIL->KVLoadVector(kvCheckpoint, "vel", c.vel);
        mom_UTIL->KVLoadQAngles(kvCheckpoint, "ang", c.ang);
        m_rcCheckpoints.AddToTail(c);
    }

    m_iCheckpointCount = m_rcCheckpoints.Count();
    m_iCurrentStepCP = m_iCheckpointCount - 1;
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
                int currentZone = m_RunData.m_iCurrentZone;
                m_RunStats.SetZoneJumps(0, m_RunStats.GetZoneJumps(0) + 1);
                m_RunStats.SetZoneJumps(currentZone, m_RunStats.GetZoneJumps(currentZone) + 1);
            }
        }
    }
    else
        m_flTicksOnGround = 0;

    SetNextThink(gpGlobals->curtime, "CURTIME");
}

void CMomentumPlayer::UpdateRunStats()
{
    float velocity = GetLocalVelocity().Length();
    float velocity2D = GetLocalVelocity().Length2D();

    if (g_Timer->IsRunning())
    {
        int currentZone = m_RunData.m_iCurrentZone; // g_Timer->GetCurrentZoneNumber();
        if (!m_bPrevTimerRunning)                   // timer started on this tick
        {
            // Compare against successive bhops to avoid incrimenting when the player was in the air without jumping
            // (for surf)
            if (GetGroundEntity() == nullptr && m_iSuccessiveBhops)
            {
                m_RunStats.SetZoneJumps(0, m_RunStats.GetZoneJumps(0) + 1);
                m_RunStats.SetZoneJumps(currentZone, m_RunStats.GetZoneJumps(currentZone) + 1);
            }
            if (m_nButtons & IN_MOVERIGHT || m_nButtons & IN_MOVELEFT)
            {
                m_RunStats.SetZoneStrafes(0, m_RunStats.GetZoneStrafes(0) + 1);
                m_RunStats.SetZoneStrafes(currentZone, m_RunStats.GetZoneStrafes(currentZone) + 1);
            }
        }
        if (m_nButtons & IN_MOVELEFT && !(m_nPrevButtons & IN_MOVELEFT))
        {
            m_RunStats.SetZoneStrafes(0, m_RunStats.GetZoneStrafes(0) + 1);
            m_RunStats.SetZoneStrafes(currentZone, m_RunStats.GetZoneStrafes(currentZone) + 1);
        }
        else if (m_nButtons & IN_MOVERIGHT && !(m_nPrevButtons & IN_MOVERIGHT))
        {
            m_RunStats.SetZoneStrafes(0, m_RunStats.GetZoneStrafes(0) + 1);
            m_RunStats.SetZoneStrafes(currentZone, m_RunStats.GetZoneStrafes(currentZone) + 1);
        }
        //  ---- MAX VELOCITY ----
        float maxOverallVel = velocity;
        float maxOverallVel2D = velocity2D;

        float maxCurrentVel = velocity;
        float maxCurrentVel2D = velocity2D;

        if (maxOverallVel <= m_RunStats.GetZoneVelocityMax(0, false))
            maxOverallVel = m_RunStats.GetZoneVelocityMax(0, false);

        if (maxOverallVel2D <= m_RunStats.GetZoneVelocityMax(0, true))
            maxOverallVel2D = m_RunStats.GetZoneVelocityMax(0, true);

        if (maxCurrentVel <= m_RunStats.GetZoneVelocityMax(currentZone, false))
            maxCurrentVel = m_RunStats.GetZoneVelocityMax(currentZone, false);

        if (maxCurrentVel2D <= m_RunStats.GetZoneVelocityMax(currentZone, true))
            maxCurrentVel2D = m_RunStats.GetZoneVelocityMax(currentZone, true);

        m_RunStats.SetZoneVelocityMax(0, maxOverallVel, maxOverallVel2D);
        m_RunStats.SetZoneVelocityMax(currentZone, maxCurrentVel, maxCurrentVel2D);
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
    m_RunStats.Init(g_Timer->GetZoneCount());
}
void CMomentumPlayer::CalculateAverageStats()
{
    if (g_Timer->IsRunning())
    {
        int currentZone = m_RunData.m_iCurrentZone; // g_Timer->GetCurrentZoneNumber();

        m_flZoneTotalSync[currentZone] += m_RunData.m_flStrafeSync;
        m_flZoneTotalSync2[currentZone] += m_RunData.m_flStrafeSync2;
        m_flZoneTotalVelocity[currentZone][0] += GetLocalVelocity().Length();
        m_flZoneTotalVelocity[currentZone][1] += GetLocalVelocity().Length2D();

        m_nZoneAvgCount[currentZone]++;

        m_RunStats.SetZoneStrafeSyncAvg(currentZone,
                                        m_flZoneTotalSync[currentZone] / float(m_nZoneAvgCount[currentZone]));
        m_RunStats.SetZoneStrafeSync2Avg(currentZone,
                                         m_flZoneTotalSync2[currentZone] / float(m_nZoneAvgCount[currentZone]));
        m_RunStats.SetZoneVelocityAvg(currentZone,
                                      m_flZoneTotalVelocity[currentZone][0] / float(m_nZoneAvgCount[currentZone]),
                                      m_flZoneTotalVelocity[currentZone][1] / float(m_nZoneAvgCount[currentZone]));

        // stage 0 is "overall" - also update these as well, no matter which stage we are on
        m_flZoneTotalSync[0] += m_RunData.m_flStrafeSync;
        m_flZoneTotalSync2[0] += m_RunData.m_flStrafeSync2;
        m_flZoneTotalVelocity[0][0] += GetLocalVelocity().Length();
        m_flZoneTotalVelocity[0][1] += GetLocalVelocity().Length2D();
        m_nZoneAvgCount[0]++;

        m_RunStats.SetZoneStrafeSyncAvg(0, m_flZoneTotalSync[currentZone] / float(m_nZoneAvgCount[currentZone]));
        m_RunStats.SetZoneStrafeSync2Avg(0, m_flZoneTotalSync2[currentZone] / float(m_nZoneAvgCount[currentZone]));
        m_RunStats.SetZoneVelocityAvg(0, m_flZoneTotalVelocity[currentZone][0] / float(m_nZoneAvgCount[currentZone]),
                                      m_flZoneTotalVelocity[currentZone][1] / float(m_nZoneAvgCount[currentZone]));
    }

    // think once per 0.1 second interval so we avoid making the totals extremely large
    SetNextThink(gpGlobals->curtime + AVERAGE_STATS_INTERVAL, "THINK_AVERAGE_STATS");
}
// This limits the player's speed in the start zone, depending on which gamemode the player is currently playing.
// On surf/other, it only limits practice mode speed. On bhop/scroll, it limits the movement speed above a certain
// threshhold, and clamps the player's velocity if they go above it.
// This is to prevent prespeeding and is different per gamemode due to the different respective playstyles of surf and
// bhop.
// MOM_TODO: Update this to extend to start zones of stages (if doing ILs)
void CMomentumPlayer::LimitSpeedInStartZone()
{
    ConVarRef gm("mom_gamemode");
    CTriggerTimerStart *startTrigger = g_Timer->GetStartTrigger();
    bool bhopGameMode = (gm.GetInt() == MOMGM_BHOP || gm.GetInt() == MOMGM_SCROLL);
    if (m_RunData.m_bIsInZone && m_RunData.m_iCurrentZone == 1)
    {
        if (GetGroundEntity() == nullptr && !m_bHasPracticeMode) // don't count ticks in air if we're in practice mode
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
bool CMomentumPlayer::SetObserverTarget(CBaseEntity *target)
{
    CMomentumReplayGhostEntity *pGhostToSpectate = dynamic_cast<CMomentumReplayGhostEntity *>(target),
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

CBaseEntity *CMomentumPlayer::FindNextObserverTarget(bool bReverse)
{
    int startIndex = GetNextObserverSearchStartPoint(bReverse);

    int currentIndex = startIndex;
    int iDir = bReverse ? -1 : 1;

    do
    {
        CBaseEntity *nextTarget = UTIL_EntityByIndex(currentIndex);

        if (IsValidObserverTarget(nextTarget))
        {
            return nextTarget; // found next valid player
        }

        currentIndex += iDir;

        // Loop through the entities
        if (currentIndex > gEntList.NumberOfEntities())
            currentIndex = 1;
        else if (currentIndex < 1)
            currentIndex = gEntList.NumberOfEntities();

    } while (currentIndex != startIndex);

    return nullptr;
}

void CMomentumPlayer::TweenSlowdownPlayer()
{
    // slowdown when map is finished
    if (m_RunData.m_bMapFinished)
        // decrease our lagged movement value by 10% every tick
        m_flTweenVelValue *= 0.9f;
    else
        m_flTweenVelValue = 1.0f; // Reset the tweened value back to normal

    SetLaggedMovementValue(m_flTweenVelValue);

    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "TWEEN");
}

CMomentumReplayGhostEntity *CMomentumPlayer::GetReplayEnt() const
{
    return dynamic_cast<CMomentumReplayGhostEntity *>(m_hObserverTarget.Get());
}

void CMomentumPlayer::StopSpectating()
{
    CMomentumReplayGhostEntity *pGhost = GetReplayEnt();
    if (pGhost)
        pGhost->RemoveSpectator(this);

    StopObserverMode();
    m_hObserverTarget.Set(nullptr);
    ForceRespawn();
    SetMoveType(MOVETYPE_WALK);
}