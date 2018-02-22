#include "cbase.h"

#include "in_buttons.h"
#include "info_camera_link.h"
#include "mom_player.h"
#include "mom_replay_entity.h"
#include "mom_system_checkpoint.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "weapon/weapon_csbasegun.h"
#include "player_command.h"
#include "predicted_viewmodel.h"
#include "ghost_client.h"
#include "mom_online_ghost.h"
#include "mom_blockfix.h"
#include "run/run_checkpoint.h"
#include "mom_gamemovement.h"

#include "tier0/memdbgon.h"

#define AVERAGE_STATS_INTERVAL 0.1

CON_COMMAND(mom_strafesync_reset, "Reset the strafe sync. (works only when timer is disabled)\n")
{
    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(UTIL_GetLocalPlayer());

    if (pPlayer && !g_pMomentumTimer->IsRunning())
    {
        pPlayer->GetStrafeTicks() = pPlayer->GetPerfectSyncTicks() = pPlayer->GetAccelTicks() = 0;
        pPlayer->m_SrvData.m_RunData.m_flStrafeSync = pPlayer->m_SrvData.m_RunData.m_flStrafeSync2 = 0.0f;
    }
}

IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropExclude("DT_BaseAnimating", "m_nMuzzleFlashParity"),
    SendPropInt(SENDINFO(m_afButtonDisabled)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumPlayer)
DEFINE_THINKFUNC(UpdateRunStats), 
DEFINE_THINKFUNC(CalculateAverageStats), 
DEFINE_THINKFUNC(LimitSpeedInStartZone),
END_DATADESC();

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);
void AppearanceCallback(IConVar *var, const char *pOldValue, float flOldValue);

// Ghost Apperence Convars
static ConVar mom_ghost_bodygroup("mom_ghost_bodygroup", "11",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Ghost's body group (model)", true, 0, true, 14,
    AppearanceCallback);

static ConVar mom_ghost_color("mom_ghost_color", "FF00FFFF",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Set the ghost's color. Accepts HEX color value in format RRGGBBAA. if RRGGBB is supplied, Alpha is set to 0x4B",
    AppearanceCallback);

static ConVar mom_trail_color("mom_trail_color", "FF00FFFF",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Set the player's trail color. Accepts HEX color value in format RRGGBBAA",
    AppearanceCallback);

static ConVar mom_trail_length("mom_trail_length", "4",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Length of the player's trail (in seconds).", true, 1, false, 10, AppearanceCallback);

static ConVar mom_trail_enable("mom_trail_enable", "0",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Paint a faint beam trail on the player. 0 = OFF, 1 = ON\n", true, 0, true, 1, AppearanceCallback);

// Handles ALL appearance changes by setting the proper appearance value in m_playerAppearanceProps, 
// as well as changing the appearance locally.
void AppearanceCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetLocalPlayer());

    ConVarRef cVar(var);

    if (pPlayer)
    {
        const char *pName = cVar.GetName();

        if (FStrEq(pName, mom_trail_color.GetName()) ||// the trail color changed
            FStrEq(pName, mom_trail_length.GetName()) || // the trail length changed
            FStrEq(pName, mom_trail_enable.GetName())) // the trail enable bool changed
        {
            uint32 newHexColor = g_pMomentumUtil->GetHexFromColor(mom_trail_color.GetString());
            pPlayer->m_playerAppearanceProps.GhostTrailRGBAColorAsHex = newHexColor;
            pPlayer->m_playerAppearanceProps.GhostTrailLength = mom_trail_length.GetInt();
            pPlayer->m_playerAppearanceProps.GhostTrailEnable = mom_trail_enable.GetBool();
            pPlayer->CreateTrail(); // Refresh the trail
        }
        else if (FStrEq(pName, mom_ghost_color.GetName())) // the ghost body color changed
        {
            uint32 newHexColor = g_pMomentumUtil->GetHexFromColor(mom_ghost_color.GetString());
            pPlayer->m_playerAppearanceProps.GhostModelRGBAColorAsHex = newHexColor;
            Color newColor;
            if (g_pMomentumUtil->GetColorFromHex(newHexColor, newColor))
                pPlayer->SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());
        }
        else if (FStrEq(pName, mom_ghost_bodygroup.GetName())) // the ghost bodygroup changed
        {
            int bGroup = mom_ghost_bodygroup.GetInt();
            pPlayer->m_playerAppearanceProps.GhostModelBodygroup = bGroup;
            pPlayer->SetBodygroup(1, bGroup);
        }

        pPlayer->SendAppearance();
    }
}

CMomentumPlayer::CMomentumPlayer()
    : m_duckUntilOnGround(false), m_flStamina(0.0f), m_RunStats(&m_SrvData.m_RunStatsData, g_pMomentumTimer->GetZoneCount()), m_pCurrentCheckpoint(nullptr),
    m_flLastVelocity(0.0f), m_nPerfectSyncTicks(0),
    m_nStrafeTicks(0), m_nAccelTicks(0), m_bPrevTimerRunning(false), m_nPrevButtons(0),
    m_nTicksInAir(0), m_flTweenVelValue(1.0f), m_bInAirDueToJump(false)
{
    m_flPunishTime = -1;
    m_iLastBlock = -1;

    m_SrvData.m_RunData.m_iRunFlags = 0;
    m_SrvData.m_iShotsFired = 0;
    m_SrvData.m_iDirection = 0;
    m_SrvData.m_bResumeZoom = false;
    m_SrvData.m_iLastZoom = 0;
    m_SrvData.m_bDidPlayerBhop = false;
    m_SrvData.m_iSuccessiveBhops = 0;
    m_SrvData.m_bHasPracticeMode = false;
    m_SrvData.m_bPreventPlayerBhop = false;
    m_SrvData.m_iLandTick = 0;

    m_SrvData.m_iCheckpointCount = 0;
    m_SrvData.m_bUsingCPMenu = false;
    m_SrvData.m_iCurrentStepCP = -1;
    
    g_ReplaySystem.m_player = this;

    Q_strncpy(m_pszDefaultEntName, GetEntityName().ToCStr(), sizeof m_pszDefaultEntName);

    ListenForGameEvent("mapfinished_panel_closed");

    // Listen for when this player jumps and lands
    g_pMomentumGameMovement->AddMovementListener(this);
}

CMomentumPlayer::~CMomentumPlayer()
{
    RemoveTrail();
    RemoveAllCheckpoints();
    RemoveAllOnehops();

    // Clear our spectating status just in case we leave the map while spectating
    g_pMomentumGhostClient->SetSpectatorTarget(k_steamIDNil, false);

    // Remove us from the gamemovement listener list
    g_pMomentumGameMovement->RemoveMovementListener(this);
}

void CMomentumPlayer::Precache()
{
    PrecacheModel(ENTITY_MODEL);

    PrecacheScriptSound(SND_FLASHLIGHT_ON);
    PrecacheScriptSound(SND_FLASHLIGHT_OFF);

    BaseClass::Precache();
}

// Used for making the view model like CS's
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

// Overridden so the player isn't frozen on a new map change
void CMomentumPlayer::PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper)
{
    m_touchedPhysObject = false;

    if (pl.fixangle == FIXANGLE_NONE)
    {
        VectorCopy(ucmd->viewangles, pl.v_angle.GetForModify());
    }

    // Handle FL_FROZEN.
    if (GetFlags() & FL_FROZEN)
    {
        ucmd->forwardmove = 0;
        ucmd->sidemove = 0;
        ucmd->upmove = 0;
        ucmd->buttons = 0;
        ucmd->impulse = 0;
        VectorCopy(pl.v_angle.Get(), ucmd->viewangles);
    }
    else if (GetToggledDuckState()) // Force a duck if we're toggled
    {
        ConVarRef xc_crouch_debounce("xc_crouch_debounce");
        // If this is set, we've altered our menu options and need to debounce the duck
        if (xc_crouch_debounce.GetBool())
        {
            ToggleDuck();

            // Mark it as handled
            xc_crouch_debounce.SetValue(0);
        }
        else
        {
            ucmd->buttons |= IN_DUCK;
        }
    }

    PlayerMove()->RunCommand(this, ucmd, moveHelper);
}

void CMomentumPlayer::SetupVisibility(CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize)
{
    BaseClass::SetupVisibility(pViewEntity, pvs, pvssize);

    int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
    PointCameraSetupVisibility(this, area, pvs, pvssize);
}

void CMomentumPlayer::FireGameEvent(IGameEvent *pEvent)
{
    if (!Q_strcmp(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        // Hide the mapfinished panel and reset our speed to normal
        m_SrvData.m_RunData.m_bMapFinished = false;
        SetLaggedMovementValue(1.0f);

        // Fix for the replay system not being able to listen to events
        if (g_ReplaySystem.m_pPlaybackReplay && !pEvent->GetBool("restart"))
        {
            g_ReplaySystem.UnloadPlayback();
        }
    }
}

void CMomentumPlayer::SendAppearance()
{
    g_pMomentumGhostClient->SendAppearanceData(m_playerAppearanceProps);
}

void CMomentumPlayer::Spawn()
{
    SetName(MAKE_STRING(m_pszDefaultEntName));
    SetModel(ENTITY_MODEL);
    SetBodygroup(1, 11); // BODY_PROLATE_ELLIPSE
    // BASECLASS SPAWN MUST BE AFTER SETTING THE MODEL, OTHERWISE A NULL HAPPENS!
    BaseClass::Spawn();
    AddFlag(FL_GODMODE);
    // this removes the flag that was added while switching to spectator mode which prevented the player from activating triggers
    RemoveSolidFlags(FSOLID_NOT_SOLID); 
    // do this here because we can't get a local player in the timer class
    ConVarRef gm("mom_gamemode");
    switch (gm.GetInt())
    {
    case MOMGM_SCROLL:
        DisableAutoBhop();
        break;
    case MOMGM_BHOP:
    case MOMGM_SURF:
    {
        if (!g_pMomentumTimer->GetZoneCount())
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
    }
    // Reset all bool gameevents
    IGameEvent *runSaveEvent = gameeventmanager->CreateEvent("replay_save");
    IGameEvent *runUploadEvent = gameeventmanager->CreateEvent("run_upload");
    IGameEvent *timerStartEvent = gameeventmanager->CreateEvent("timer_state");
    m_bAllowUserTeleports = true;
    m_SrvData.m_RunData.m_bIsInZone = false;
    m_SrvData.m_RunData.m_bMapFinished = false;
    m_SrvData.m_RunData.m_iCurrentZone = 0;
    m_SrvData.m_bHasPracticeMode = false;
    m_SrvData.m_bPreventPlayerBhop = false;
    m_SrvData.m_iLandTick = 0;
    ResetRunStats();
    if (runSaveEvent)
    {
        runSaveEvent->SetBool("save", false);
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
    g_pMomentumTimer->DispatchMapInfo();

    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("THINK_AVERAGE_STATS");
    RegisterThinkContext("CURTIME_FOR_START");
    RegisterThinkContext("TWEEN");
    SetContextThink(&CMomentumPlayer::UpdateRunStats, gpGlobals->curtime + gpGlobals->interval_per_tick,
                    "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CalculateAverageStats, gpGlobals->curtime + AVERAGE_STATS_INTERVAL,
                    "THINK_AVERAGE_STATS");
    SetContextThink(&CMomentumPlayer::LimitSpeedInStartZone, gpGlobals->curtime, "CURTIME_FOR_START");
    SetContextThink(&CMomentumPlayer::TweenSlowdownPlayer, gpGlobals->curtime, "TWEEN");

    
    // initilize appearance properties based on Convars
    if (g_pMomentumUtil)
    {
        uint32 newHexColor = g_pMomentumUtil->GetHexFromColor(mom_trail_color.GetString());
        m_playerAppearanceProps.GhostTrailRGBAColorAsHex = newHexColor;
        m_playerAppearanceProps.GhostTrailLength = mom_trail_length.GetInt();
        m_playerAppearanceProps.GhostTrailEnable = mom_trail_enable.GetBool();

        newHexColor = g_pMomentumUtil->GetHexFromColor(mom_ghost_color.GetString());
        m_playerAppearanceProps.GhostModelRGBAColorAsHex = newHexColor;
        Color newColor;
        if (g_pMomentumUtil->GetColorFromHex(newHexColor, newColor))
            SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());

        int bGroup = mom_ghost_bodygroup.GetInt();
        m_playerAppearanceProps.GhostModelBodygroup = bGroup;
        SetBodygroup(1, bGroup);

        // Send our appearance to the server/lobby if we're in one
        SendAppearance();
    }
    else
    {
        Warning("Could not set appearance properties! g_pMomentumUtil is NULL!\n");
    }
    
    // If wanted, create trail
    if (mom_trail_enable.GetBool())
        CreateTrail();

    SetNextThink(gpGlobals->curtime);

    // Load the player's checkpoints, only if we are spawning for the first time
    if (m_rcCheckpoints.IsEmpty())
        g_MOMCheckpointSystem->LoadMapCheckpoints(this);

    // Reset current checkpoint trigger upon spawn
    m_pCurrentCheckpoint = nullptr;
}

// Obtains the player's previous origin using their current origin as a base.
Vector CMomentumPlayer::GetPrevOrigin(void) const { return GetPrevOrigin(GetLocalOrigin()); }

// Obtains the player's previous origin using a vector as the base, subtracting one tick's worth of velocity.
Vector CMomentumPlayer::GetPrevOrigin(const Vector &base) const
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
    const char *spawns[] = {"info_player_start", "info_player_counterterrorist", "info_player_terrorist"};
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
    pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    if (pStart == nullptr) // skip over the null point
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    CBaseEntity *pLast = nullptr;
    while (pStart != nullptr)
    {
        if (g_pGameRules->IsSpawnPointValid(pStart, this))
        {
            if (pStart->HasSpawnFlags(1)) // SF_PLAYER_START_MASTER
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
        CWeaponCSBase *pWeapon = dynamic_cast<CWeaponCSBase *>(GetActiveWeapon());
        
        if (pWeapon)
        {
            if (pWeapon->GetWeaponID() != WEAPON_GRENADE)
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

Checkpoint_t *CMomentumPlayer::CreateCheckpoint()
{
    Checkpoint_t *c = new Checkpoint_t(this);
    return c;
}

void CMomentumPlayer::CreateAndSaveCheckpoint()
{
    Checkpoint_t *c = CreateCheckpoint();
    m_rcCheckpoints.AddToTail(c);
    if (m_SrvData.m_iCurrentStepCP == m_SrvData.m_iCheckpointCount - 1)
        ++m_SrvData.m_iCurrentStepCP;
    else
        m_SrvData.m_iCurrentStepCP = m_SrvData.m_iCheckpointCount; // Set it to the new checkpoint's index
    ++m_SrvData.m_iCheckpointCount;
}

void CMomentumPlayer::RemoveLastCheckpoint()
{
    if (m_rcCheckpoints.IsEmpty())
        return;
    m_rcCheckpoints.Remove(m_SrvData.m_iCurrentStepCP);
    // If there's one element left, we still need to decrease currentStep to -1
    if (m_SrvData.m_iCurrentStepCP == m_SrvData.m_iCheckpointCount - 1)
        --m_SrvData.m_iCurrentStepCP;
    // else we want it to shift forward one until it catches back up to the last checkpoint
    --m_SrvData.m_iCheckpointCount;
}

void CMomentumPlayer::RemoveAllCheckpoints()
{
    m_rcCheckpoints.PurgeAndDeleteElements();
    m_SrvData.m_iCurrentStepCP = -1;
    m_SrvData.m_iCheckpointCount = 0;
}

void CMomentumPlayer::AddOnehop(CTriggerOnehop* pTrigger)
{
    if (m_vecOnehops.Count() > 0)
    {
        // Go backwards so we don't have to worry about anything
        FOR_EACH_VEC_BACK(m_vecOnehops, i)
        {
            CTriggerOnehop *pOnehop = m_vecOnehops[i];
            if (pOnehop && pOnehop->HasSpawnFlags(SF_TELEPORT_RESET_ONEHOP))
                m_vecOnehops.Remove(i);
        }
    }

    m_vecOnehops.AddToTail(pTrigger);
}

bool CMomentumPlayer::FindOnehopOnList(CTriggerOnehop* pTrigger) const
{
    return m_vecOnehops.Find(pTrigger) != m_vecOnehops.InvalidIndex();
}

void CMomentumPlayer::RemoveAllOnehops()
{
    m_vecOnehops.RemoveAll();
}

void CMomentumPlayer::DoMuzzleFlash()
{
    // Don't do the muzzle flash for the paint gun
    CWeaponCSBase *pWeapon = dynamic_cast<CWeaponCSBase *>(GetActiveWeapon());
    if (!(pWeapon && pWeapon->GetWeaponID() == WEAPON_PAINTGUN))
    {
        BaseClass::DoMuzzleFlash();
    }
}

void CMomentumPlayer::ToggleDuckThisFrame(bool bState)
{
    if (m_Local.m_bDucked != bState)
    {
        m_Local.m_bDucked = bState;
        bState ? AddFlag(FL_DUCKING) : RemoveFlag(FL_DUCKING);
        SetVCollisionState(GetAbsOrigin(), GetAbsVelocity(), bState ? VPHYS_CROUCH : VPHYS_WALK);
    }
}

void CMomentumPlayer::RemoveTrail()
{
    UTIL_RemoveImmediate(m_eTrail);
    m_eTrail = nullptr;
}

void CMomentumPlayer::CheckChatText(char* p, int bufsize)
{
    g_pMomentumGhostClient->SendChatMessage(p);
}

// Overrides Teleport() so we can take care of the trail
void CMomentumPlayer::Teleport(const Vector* newPosition, const QAngle* newAngles, const Vector* newVelocity)
{
    // No need to remove the trail here, CreateTrail() already does it for us
    BaseClass::Teleport(newPosition, newAngles, newVelocity);
    CreateTrail();
}

void CMomentumPlayer::CreateTrail()
{
    RemoveTrail();

    if (!mom_trail_enable.GetBool()) return;

    // Ty GhostingMod
    m_eTrail = CreateEntityByName("env_spritetrail");
    m_eTrail->SetAbsOrigin(GetAbsOrigin());
    m_eTrail->SetParent(this);
    m_eTrail->KeyValue("rendermode", "5");
    m_eTrail->KeyValue("spritename", "materials/sprites/laser.vmt");
    m_eTrail->KeyValue("startwidth", "9.5");
    m_eTrail->KeyValue("endwidth", "1.05");
    m_eTrail->KeyValue("lifetime", mom_trail_length.GetInt());
    Color newColor;
    if (g_pMomentumUtil->GetColorFromHex(mom_trail_color.GetString(), newColor))
    {
        m_eTrail->SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());
        m_eTrail->KeyValue("renderamt", newColor.a());
    }
    DispatchSpawn(m_eTrail);
}   

void CMomentumPlayer::TeleportToCheckpoint(int newCheckpoint)
{
    if (newCheckpoint > m_rcCheckpoints.Count() || newCheckpoint < 0 || !m_bAllowUserTeleports)
        return;
    Checkpoint_t *pCheckpoint = m_rcCheckpoints[newCheckpoint];
    if (pCheckpoint)
        pCheckpoint->Teleport(this);
}

void CMomentumPlayer::SaveCPsToFile(KeyValues *kvInto)
{
    // Set the current index
    kvInto->SetInt("cur", m_SrvData.m_iCurrentStepCP);

    // Add all your checkpoints
    KeyValues *kvCPs = new KeyValues("cps");
    FOR_EACH_VEC(m_rcCheckpoints, i)
    {
        Checkpoint_t *c = m_rcCheckpoints[i];
        char szCheckpointNum[10]; // 999 million checkpoints is pretty generous
        Q_snprintf(szCheckpointNum, sizeof(szCheckpointNum), "%09i", i); // %09 because '\0' is the last (10)
        KeyValues *kvCP = new KeyValues(szCheckpointNum);
        c->Save(kvCP);
        kvCPs->AddSubKey(kvCP);
    }

    // Save them into the keyvalues
    kvInto->AddSubKey(kvCPs);
}

void CMomentumPlayer::LoadCPsFromFile(KeyValues *kvFrom)
{
    if (!kvFrom || kvFrom->IsEmpty()) return;

    m_SrvData.m_iCurrentStepCP = kvFrom->GetInt("cur");

    KeyValues *kvCPs = kvFrom->FindKey("cps");
    if (!kvCPs) return;
    FOR_EACH_SUBKEY(kvCPs, kvCheckpoint)
    {
        Checkpoint_t *c = new Checkpoint_t(kvCheckpoint);
        m_rcCheckpoints.AddToTail(c);
    }

    m_SrvData.m_iCheckpointCount = m_rcCheckpoints.Count();
}

void CMomentumPlayer::Touch(CBaseEntity *pOther)
{
    BaseClass::Touch(pOther);

    if (g_MOMBlockFixer->IsBhopBlock(pOther->entindex()))
        g_MOMBlockFixer->PlayerTouch(this, pOther);
}

void CMomentumPlayer::EnableAutoBhop()
{
    m_SrvData.m_RunData.m_bAutoBhop = true;
    DevLog("Enabled autobhop\n");
}
void CMomentumPlayer::DisableAutoBhop()
{
    m_SrvData.m_RunData.m_bAutoBhop = false;
    DevLog("Disabled autobhop\n");
}

void CMomentumPlayer::OnPlayerJump()
{
    // OnCheckBhop code 
    m_SrvData.m_bDidPlayerBhop = gpGlobals->tickcount - m_SrvData.m_iLandTick < NUM_TICKS_TO_BHOP;
    if (!m_SrvData.m_bDidPlayerBhop)
        m_SrvData.m_iSuccessiveBhops = 0;

    m_SrvData.m_RunData.m_flLastJumpVel = GetLocalVelocity().Length2D();
    m_SrvData.m_iSuccessiveBhops++;

    m_bInAirDueToJump = true;

    // Set our runstats jump count
    if (g_pMomentumTimer->IsRunning())
    {
        int currentZone = m_SrvData.m_RunData.m_iCurrentZone;
        m_RunStats.SetZoneJumps(0, m_RunStats.GetZoneJumps(0) + 1); // Increment total jumps
        m_RunStats.SetZoneJumps(currentZone, m_RunStats.GetZoneJumps(currentZone) + 1); // Increment zone jumps
    }
}


void CMomentumPlayer::OnPlayerLand()
{
    // Set the tick that we landed on something solid (can jump off of this)
    m_SrvData.m_iLandTick = gpGlobals->tickcount;

    m_bInAirDueToJump = false;
}

void CMomentumPlayer::UpdateRunStats()
{
    // ---- Jumps and Strafes ----
    UpdateJumpStrafes();

    //  ---- MAX VELOCITY ----
    UpdateMaxVelocity();
    // ----------

    //  ---- STRAFE SYNC -----
    UpdateRunSync();
    // ----------

    // this might be used in a later update
    // m_flLastVelocity = velocity;
    
    StdDataToPlayer(&m_SrvData); 

    // think once per tick
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
}

void CMomentumPlayer::UpdateRunSync()
{
    if (g_pMomentumTimer->IsRunning() || (ConVarRef("mom_strafesync_draw").GetInt() == 2 && !m_SrvData.m_bHasPracticeMode))
    {
        if (!(GetFlags() & (FL_ONGROUND | FL_INWATER)) && GetMoveType() != MOVETYPE_LADDER)
        {
            float dtAngle = EyeAngles().y - m_qangLastAngle.y;
            if (dtAngle > 180.f)
                dtAngle -= 360.f;
            else if (dtAngle < -180.f)
                dtAngle += 360.f;

            if (dtAngle > 0) // player turned left
            {
                m_nStrafeTicks++;
                if ((m_nButtons & IN_MOVELEFT) && !(m_nButtons & IN_MOVERIGHT))
                    m_nPerfectSyncTicks++;
                if (m_flSideMove < 0)
                    m_nAccelTicks++;
            }
            else if (dtAngle < 0) // player turned right
            {
                m_nStrafeTicks++;
                if ((m_nButtons & IN_MOVERIGHT) && !(m_nButtons & IN_MOVELEFT))
                    m_nPerfectSyncTicks++;
                if (m_flSideMove > 0)
                    m_nAccelTicks++;
            }
        }
        if (m_nStrafeTicks && m_nAccelTicks && m_nPerfectSyncTicks)
        {
            // ticks strafing perfectly / ticks strafing
            m_SrvData.m_RunData.m_flStrafeSync = (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f;
            // ticks gaining speed / ticks strafing
            m_SrvData.m_RunData.m_flStrafeSync2 = (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f;
        }

        m_qangLastAngle = EyeAngles();
    }
}

void CMomentumPlayer::UpdateJumpStrafes()
{
    if (!g_pMomentumTimer->IsRunning())
        return;

    int currentZone = m_SrvData.m_RunData.m_iCurrentZone;
    if (!m_bPrevTimerRunning)                   // timer started on this tick
    {
        // Compare against successive bhops to avoid incrimenting when the player was in the air without jumping
        // (for surf)
        if (GetGroundEntity() == nullptr && m_SrvData.m_iSuccessiveBhops)
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

    m_bPrevTimerRunning = g_pMomentumTimer->IsRunning();
    m_nPrevButtons = m_nButtons;
}

void CMomentumPlayer::UpdateMaxVelocity()
{
    if (!g_pMomentumTimer->IsRunning())
        return;

    int currentZone = m_SrvData.m_RunData.m_iCurrentZone;
    float velocity = GetLocalVelocity().Length();
    float velocity2D = GetLocalVelocity().Length2D();
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
}

void CMomentumPlayer::ResetRunStats()
{
    SetName(MAKE_STRING(m_pszDefaultEntName)); // Reset name
    // MOM_TODO: Consider any other resets needed (classname, any flags, etc)

    m_nPerfectSyncTicks = 0;
    m_nStrafeTicks = 0;
    m_nAccelTicks = 0;
    m_SrvData.m_RunData.m_flStrafeSync = 0;
    m_SrvData.m_RunData.m_flStrafeSync2 = 0;
    m_RunStats.Init(g_pMomentumTimer->GetZoneCount());
}
void CMomentumPlayer::CalculateAverageStats()
{
    if (g_pMomentumTimer->IsRunning())
    {
        int currentZone = m_SrvData.m_RunData.m_iCurrentZone; // g_Timer->GetCurrentZoneNumber();

        m_flZoneTotalSync[currentZone] += m_SrvData.m_RunData.m_flStrafeSync;
        m_flZoneTotalSync2[currentZone] += m_SrvData.m_RunData.m_flStrafeSync2;
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
        m_flZoneTotalSync[0] += m_SrvData.m_RunData.m_flStrafeSync;
        m_flZoneTotalSync2[0] += m_SrvData.m_RunData.m_flStrafeSync2;
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
    if (m_SrvData.m_RunData.m_bIsInZone && m_SrvData.m_RunData.m_iCurrentZone == 1 && !m_SrvData.m_bUsingCPMenu) // MOM_TODO: && g_Timer->IsForILs()
    {
        if (GetGroundEntity() == nullptr && !m_SrvData.m_bHasPracticeMode) // don't count ticks in air if we're in practice mode
            m_nTicksInAir++;
        else
            m_nTicksInAir = 0;

        // set bhop flag to true so we can't prespeed with practice mode
        if (m_SrvData.m_bHasPracticeMode)
            m_SrvData.m_bDidPlayerBhop = true;

        // depending on gamemode, limit speed outright when player exceeds punish vel
        ConVarRef gm("mom_gamemode");
        CTriggerTimerStart *startTrigger = g_pMomentumTimer->GetStartTrigger();
        // This does not look pretty but saves us a branching. The checks are:
        // no nullptr, correct gamemode, is limiting leave speed and 
        //    enough ticks on air have passed
        if (startTrigger && (gm.GetInt() == MOMGM_BHOP || gm.GetInt() == MOMGM_SCROLL) && startTrigger->HasSpawnFlags(SF_LIMIT_LEAVE_SPEED) &&
            (!g_pMomentumTimer->IsRunning() && m_nTicksInAir > MAX_AIRTIME_TICKS))
        {
            Vector velocity = GetLocalVelocity();
            float PunishVelSquared = startTrigger->GetMaxLeaveSpeed() * startTrigger->GetMaxLeaveSpeed();
            if (velocity.Length2DSqr() > PunishVelSquared) // more efficent to check against the square of velocity
            {
                // New velocity is the unitary form of the current vel vector times the max speed amount
                SetAbsVelocity((velocity / velocity.Length()) * startTrigger->GetMaxLeaveSpeed());
            }
        }
    }
    SetNextThink(gpGlobals->curtime, "CURTIME_FOR_START");
}
// override of CBasePlayer::IsValidObserverTarget that allows us to spectate ghosts
bool CMomentumPlayer::IsValidObserverTarget(CBaseEntity *target)
{
    if (target == nullptr)
        return false;

    if (!target->IsPlayer())
    {
        if (FStrEq(target->GetClassname(), "mom_replay_ghost")) // target is a replay ghost
        {
            return true;
        }
        if (FStrEq(target->GetClassname(), "mom_online_ghost")) // target is an online ghost
        {
            CMomentumOnlineGhostEntity *pEntity = dynamic_cast<CMomentumOnlineGhostEntity*>(target);
            return pEntity && !pEntity->m_bSpectating;
        }
        return false;
    }

    return BaseClass::IsValidObserverTarget(target);
}

// Override of CBasePlayer::SetObserverTarget that lets us add/remove ourselves as spectors to the ghost
bool CMomentumPlayer::SetObserverTarget(CBaseEntity *target)
{
    CMomentumGhostBaseEntity *pGhostToSpectate = dynamic_cast<CMomentumGhostBaseEntity *>(target);
    CMomentumGhostBaseEntity *pCurrentGhost = GetGhostEnt();

    if (pCurrentGhost == pGhostToSpectate)
    {
        return false;
    }
    
    if (pCurrentGhost)
    {
        pCurrentGhost->RemoveSpectator();
    }

    bool base = BaseClass::SetObserverTarget(target);

    if (pGhostToSpectate && base)
    {
        // Don't allow user teleports when spectating. Checkpoints can be created, but the
        // teleporting logic needs to not be allowed.
        m_bAllowUserTeleports = false;

        RemoveTrail();

        pGhostToSpectate->SetSpectator(this);

        CMomentumOnlineGhostEntity *pOnlineEnt = dynamic_cast<CMomentumOnlineGhostEntity *>(target);
        if (pOnlineEnt)
        {
            m_sSpecTargetSteamID = pOnlineEnt->GetGhostSteamID();
        }
        else if (pGhostToSpectate->IsReplayGhost())
        {
            m_sSpecTargetSteamID = CSteamID(uint64(1));
        }

        g_pMomentumGhostClient->SetSpectatorTarget(m_sSpecTargetSteamID, pCurrentGhost == nullptr);

        if (pCurrentGhost == nullptr)
        {
            FIRE_GAME_WIDE_EVENT("spec_start");
        }
    }

    return base;
}
int CMomentumPlayer::GetNextObserverSearchStartPoint(bool bReverse)
{
    int iDir = bReverse ? -1 : 1;

    int startIndex;

    if (m_hObserverTarget)
    {
        // start using last followed player
        startIndex = m_hObserverTarget->entindex();
    }
    else
    {
        // start using own player index
        startIndex = this->entindex();
    }

    startIndex += iDir;
    return startIndex;
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

// Overridden for custom IN_EYE check
void CMomentumPlayer::CheckObserverSettings()
{
    // check if we are in forced mode and may go back to old mode
    if (m_bForcedObserverMode)
    {
        CBaseEntity *target = m_hObserverTarget;

        if (!IsValidObserverTarget(target))
        {
            // if old target is still invalid, try to find valid one
            target = FindNextObserverTarget(false);
        }

        if (target)
        {
            // we found a valid target
            m_bForcedObserverMode = false;        // disable force mode
            SetObserverMode(m_iObserverLastMode); // switch to last mode
            SetObserverTarget(target);            // goto target

            // TODO check for HUD icons
            return;
        }
        // else stay in forced mode, no changes
        return;
    }

    // make sure our last mode is valid
    if (m_iObserverLastMode < OBS_MODE_FIXED)
    {
        m_iObserverLastMode = OBS_MODE_ROAMING;
    }

    // check if our spectating target is still a valid one
    if (m_iObserverMode == OBS_MODE_IN_EYE || m_iObserverMode == OBS_MODE_CHASE || m_iObserverMode == OBS_MODE_FIXED ||
        m_iObserverMode == OBS_MODE_POI)
    {
        ValidateCurrentObserverTarget();

        CMomentumGhostBaseEntity *target = GetGhostEnt();
        // for ineye mode we have to copy several data to see exactly the same

        if (target && m_iObserverMode == OBS_MODE_IN_EYE)
        {
            int flagMask = FL_ONGROUND | FL_DUCKING;

            int flags = target->GetFlags() & flagMask;

            if ((GetFlags() & flagMask) != flags)
            {
                flags |= GetFlags() & (~flagMask); // keep other flags
                ClearFlags();
                AddFlag(flags);
            }

            if (target->GetViewOffset() != GetViewOffset())
            {
                SetViewOffset(target->GetViewOffset());
            }
            return;
        }
    }

    // Call base class for player check
    BaseClass::CheckObserverSettings();
}

void CMomentumPlayer::TweenSlowdownPlayer()
{
    // slowdown when map is finished
    if (m_SrvData.m_RunData.m_bMapFinished)
        // decrease our lagged movement value by 10% every tick
        m_flTweenVelValue *= 0.9f;
    else
        m_flTweenVelValue = GetLaggedMovementValue(); // Reset the tweened value back to normal

    SetLaggedMovementValue(m_flTweenVelValue);

    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "TWEEN");
}

CMomentumGhostBaseEntity *CMomentumPlayer::GetGhostEnt() const
{
    return dynamic_cast<CMomentumGhostBaseEntity*>(m_hObserverTarget.Get());
}

void CMomentumPlayer::StopSpectating()
{
    CMomentumGhostBaseEntity *pGhost = GetGhostEnt();
    if (pGhost)
        pGhost->RemoveSpectator();

    StopObserverMode();
    m_hObserverTarget.Set(nullptr);
    ForceRespawn();
    SetMoveType(MOVETYPE_WALK);

    FIRE_GAME_WIDE_EVENT("spec_stop");
   
    // Update the lobby/server if there is one
    m_sSpecTargetSteamID.Clear(); //reset steamID when we stop spectating
    g_pMomentumGhostClient->SetSpectatorTarget(m_sSpecTargetSteamID, false);
}
