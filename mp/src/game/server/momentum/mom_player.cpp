#include "cbase.h"

#include "mom_player.h"

#include "ghost_client.h"
#include "in_buttons.h"
#include "info_camera_link.h"
#include "mom_blockfix.h"
#include "mom_online_ghost.h"
#include "mom_replay_entity.h"
#include "mom_timer.h"
#include "mom_triggers.h"
#include "player_command.h"
#include "predicted_viewmodel.h"
#include "weapon/weapon_base_gun.h"
#include "mom_system_saveloc.h"
#include "util/mom_util.h"
#include "mom_replay_system.h"
#include "run/mom_replay_base.h"

#include "tier0/memdbgon.h"

#define AVERAGE_STATS_INTERVAL 0.1

CON_COMMAND(mom_strafesync_reset, "Reset the strafe sync. (works only when timer is disabled)\n")
{
    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer *>(UTIL_GetLocalPlayer());

    if (pPlayer && !g_pMomentumTimer->IsRunning())
    {
        pPlayer->SetStrafeTicks(0);
        pPlayer->SetPerfectSyncTicks(0);
        pPlayer->SetAccelTicks(0);
        pPlayer->m_Data.m_flStrafeSync = 0.0f; 
        pPlayer->m_Data.m_flStrafeSync2 = 0.0f;
    }
}

IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropExclude("DT_BaseAnimating", "m_nMuzzleFlashParity"), 
SendPropBool(SENDINFO(m_bHasPracticeMode)),
SendPropBool(SENDINFO(m_bPreventPlayerBhop)),
SendPropInt(SENDINFO(m_iLandTick)),
SendPropBool(SENDINFO(m_bResumeZoom)),
SendPropInt(SENDINFO(m_iShotsFired), 16, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_iDirection), 4, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_iLastZoomFOV), 8, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_afButtonDisabled)),
SendPropEHandle(SENDINFO(m_CurrentSlideTrigger)),
SendPropBool(SENDINFO(m_bAutoBhop)),
SendPropDataTable(SENDINFO_DT(m_Data), &REFERENCE_SEND_TABLE(DT_MomRunEntityData)),
SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MomRunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumPlayer)
    DEFINE_THINKFUNC(UpdateRunStats),
    DEFINE_THINKFUNC(CalculateAverageStats),
    /*DEFINE_THINKFUNC(LimitSpeedInStartZone),*/
END_DATADESC();

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);
void AppearanceCallback(IConVar *var, const char *pOldValue, float flOldValue);

// Ghost Apperence Convars
static ConVar mom_ghost_bodygroup("mom_ghost_bodygroup", "11", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                  "Ghost's body group (model)", true, 0, true, 14, AppearanceCallback);

static ConVar mom_ghost_color(
    "mom_ghost_color", "FF00FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Set the ghost's color. Accepts HEX color value in format RRGGBBAA. if RRGGBB is supplied, Alpha is set to 0x4B",
    AppearanceCallback);

static ConVar mom_trail_color("mom_trail_color", "FF00FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                              "Set the player's trail color. Accepts HEX color value in format RRGGBBAA",
                              AppearanceCallback);

static ConVar mom_trail_length("mom_trail_length", "4", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                               "Length of the player's trail (in seconds).", true, 1, false, 10, AppearanceCallback);

static ConVar mom_trail_enable("mom_trail_enable", "0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                               "Paint a faint beam trail on the player. 0 = OFF, 1 = ON\n", true, 0, true, 1,
                               AppearanceCallback);

// Handles ALL appearance changes by setting the proper appearance value in m_playerAppearanceProps,
// as well as changing the appearance locally.
void AppearanceCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    CMomentumPlayer *pPlayer = dynamic_cast<CMomentumPlayer*>(UTIL_GetLocalPlayer());

    ConVarRef cVar(var);

    if (pPlayer)
    {
        const char *pName = cVar.GetName();

        if (FStrEq(pName, mom_trail_color.GetName()) ||  // the trail color changed
            FStrEq(pName, mom_trail_length.GetName()) || // the trail length changed
            FStrEq(pName, mom_trail_enable.GetName()))   // the trail enable bool changed
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

static CMomentumPlayer *s_pPlayer = nullptr;

CMomentumPlayer::CMomentumPlayer()
    : m_duckUntilOnGround(false), m_flStamina(0.0f),
      m_flLastVelocity(0.0f), m_nPerfectSyncTicks(0), m_nStrafeTicks(0), m_nAccelTicks(0), m_bPrevTimerRunning(false),
      m_nPrevButtons(0), m_flTweenVelValue(1.0f), m_bInAirDueToJump(false), m_iProgressNumber(-1)
{
    m_flPunishTime = -1;
    m_iLastBlock = -1;

    m_bWasSpectating = false;

    m_CurrentSlideTrigger = nullptr;

    m_Data.m_iRunFlags = 0;
    m_Data.m_iRunFlags = 0;

    m_bHasPracticeMode = false;
    m_iShotsFired = 0;
    m_iDirection = 0;
    m_bResumeZoom = false;
    m_iLastZoomFOV = 0;
    m_bDidPlayerBhop = false;
    m_iSuccessiveBhops = 0;
    m_bPreventPlayerBhop = false;
    m_iLandTick = 0;

    m_RunStats.Init(g_pMomentumTimer->GetZoneCount());

    Q_strncpy(m_pszDefaultEntName, GetEntityName().ToCStr(), sizeof m_pszDefaultEntName);

    ListenForGameEvent("mapfinished_panel_closed");

    for (int i = 0; i < MAX_TRACKS; i++)
    {
        m_pStartZoneMarks[i] = nullptr;
    }
}

CMomentumPlayer::~CMomentumPlayer()
{
    if (this == s_pPlayer)
        s_pPlayer = nullptr;

    RemoveTrail();
    RemoveAllOnehops();

    // Clear our spectating status just in case we leave the map while spectating
    g_pMomentumGhostClient->SetSpectatorTarget(k_steamIDNil, false, true);
}

CMomentumPlayer* CMomentumPlayer::CreatePlayer(const char *className, edict_t *ed)
{
    s_PlayerEdict = ed;
    auto toRet = static_cast<CMomentumPlayer *>(CreateEntityByName(className));
    if (ed->m_EdictIndex == 1)
        s_pPlayer = toRet;
    return toRet;
}

CMomentumPlayer* CMomentumPlayer::GetLocalPlayer()
{
    return s_pPlayer;
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
    else if (pl.fixangle == FIXANGLE_ABSOLUTE)
    {
        VectorCopy(pl.v_angle.GetForModify(), ucmd->viewangles);
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
        m_Data.m_bMapFinished = false;
        SetLaggedMovementValue(1.0f);

        // Fix for the replay system not being able to listen to events
        if (g_ReplaySystem.GetPlaybackReplay() && !pEvent->GetBool("restart"))
        {
            g_ReplaySystem.UnloadPlayback();
        }
    }
}

void CMomentumPlayer::FlashlightTurnOn()
{
    AddEffects(EF_DIMLIGHT);
    EmitSound(SND_FLASHLIGHT_ON);
    m_playerAppearanceProps.FlashlightOn = true;
    SendAppearance();
}

void CMomentumPlayer::FlashlightTurnOff()
{
    RemoveEffects(EF_DIMLIGHT);
    EmitSound(SND_FLASHLIGHT_OFF);
    m_playerAppearanceProps.FlashlightOn = false;
    SendAppearance();
}

void CMomentumPlayer::SendAppearance() { g_pMomentumGhostClient->SendAppearanceData(m_playerAppearanceProps); }

void CMomentumPlayer::Spawn()
{
    SetName(MAKE_STRING(m_pszDefaultEntName));
    SetModel(ENTITY_MODEL);
    SetBodygroup(1, 11); // BODY_PROLATE_ELLIPSE
    // BASECLASS SPAWN MUST BE AFTER SETTING THE MODEL, OTHERWISE A NULL HAPPENS!
    BaseClass::Spawn();
    AddFlag(FL_GODMODE);

    // this removes the flag that was added while switching to spectator mode which prevented the player from activating
    // triggers
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    m_bAllowUserTeleports = true;

    // Handle resetting only if we weren't spectating
    if (m_bWasSpectating)
    {
        RestoreRunState();
        m_bWasSpectating = false;
    }
    else
    {
        m_Data.m_bIsInZone = false;
        m_Data.m_bMapFinished = false;
        m_Data.m_iCurrentZone = 0;
        m_bHasPracticeMode = false;
        m_bPreventPlayerBhop = false;
        m_iLandTick = 0;
        ResetRunStats();

        for (int i = 0; i < MAX_TRACKS; i++)
        {
            ClearStartMark(i);
        }
    }

    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("THINK_AVERAGE_STATS");
    // RegisterThinkContext("CURTIME_FOR_START");
    RegisterThinkContext("TWEEN");
    SetContextThink(&CMomentumPlayer::UpdateRunStats, gpGlobals->curtime + gpGlobals->interval_per_tick,
                    "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CalculateAverageStats, gpGlobals->curtime + AVERAGE_STATS_INTERVAL,
                    "THINK_AVERAGE_STATS");
    // SetContextThink(&CMomentumPlayer::LimitSpeedInStartZone, gpGlobals->curtime, "CURTIME_FOR_START");
    SetContextThink(&CMomentumPlayer::TweenSlowdownPlayer, gpGlobals->curtime, "TWEEN");

    // initilize appearance properties based on Convars
    uint32 newHexColor = g_pMomentumUtil->GetHexFromColor(mom_trail_color.GetString());
    m_playerAppearanceProps.GhostTrailRGBAColorAsHex = newHexColor;
    m_playerAppearanceProps.GhostTrailLength = mom_trail_length.GetInt();
    m_playerAppearanceProps.GhostTrailEnable = mom_trail_enable.GetBool();

    newHexColor = g_pMomentumUtil->GetHexFromColor(mom_ghost_color.GetString());
    m_playerAppearanceProps.GhostModelRGBAColorAsHex = newHexColor;
    Color newColor;
    if (g_pMomentumUtil->GetColorFromHex(newHexColor, newColor))
        SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());

    int bodyGroup = mom_ghost_bodygroup.GetInt();
    m_playerAppearanceProps.GhostModelBodygroup = bodyGroup;
    SetBodygroup(1, bodyGroup);

    // Send our appearance to the server/lobby if we're in one
    SendAppearance();

    // If wanted, create trail
    if (mom_trail_enable.GetBool())
        CreateTrail();

    SetNextThink(gpGlobals->curtime);

    // Reset current checkpoint trigger upon spawn
    m_CurrentProgress.Term();

    g_pMomentumTimer->OnPlayerSpawn(this);
}

// Obtains a player's previous origin X ticks backwards (0 is still previous, depends when this is called ofc!)
Vector CMomentumPlayer::GetPreviousOrigin(unsigned int previous_count) const
{
    return previous_count < MAX_PREVIOUS_ORIGINS ? m_vecPreviousOrigins[previous_count] : Vector(0.0f, 0.0f, 0.0f);
}

void CMomentumPlayer::NewPreviousOrigin(Vector origin)
{
    for (int i = MAX_PREVIOUS_ORIGINS; i-- > 1;)
    {
        m_vecPreviousOrigins[i] = m_vecPreviousOrigins[i - 1];
    }
    m_vecPreviousOrigins[0] = origin;
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

void CMomentumPlayer::OnJump()
{
    // OnCheckBhop code
    m_bDidPlayerBhop = gpGlobals->tickcount - m_iLandTick < NUM_TICKS_TO_BHOP;
    if (!m_bDidPlayerBhop)
        m_iSuccessiveBhops = 0;

    m_Data.m_flLastJumpVel = GetLocalVelocity().Length2D();
    m_iSuccessiveBhops++;

    if (m_Data.m_bIsInZone && m_Data.m_iCurrentZone == 1)
    {
        g_pMomentumTimer->TryStart(this, false);
    }

    // Set our runstats jump count
    if (g_pMomentumTimer->IsRunning())
    {
        const int currentZone = m_Data.m_iCurrentZone;
        m_RunStats.SetZoneJumps(0, m_RunStats.GetZoneJumps(0) + 1); // Increment total jumps
        m_RunStats.SetZoneJumps(currentZone,
                                         m_RunStats.GetZoneJumps(currentZone) + 1); // Increment zone jumps
    }
}

void CMomentumPlayer::OnLand()
{
    m_iLandTick = gpGlobals->tickcount;

    if (m_Data.m_bIsInZone && m_Data.m_iCurrentZone == 1)
    {
        // If we start timer on jump then we should reset on land
        g_pMomentumTimer->Reset(this);
    }
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
                return true;
            }
        }
        pLast = pStart;
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    }
    if (pLast)
    {
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
    if (FStrEq(cmd, "spec_next")) // chase next player
    {
        TravelSpectateTargets(false);
        return true;
    }
    if (FStrEq(cmd, "spec_prev")) // chase previous player
    {
        TravelSpectateTargets(true);
        return true;
    }
    if (FStrEq(cmd, "drop"))
    {
        CWeaponBase *pWeapon = dynamic_cast<CWeaponBase *>(GetActiveWeapon());

        if (pWeapon && pWeapon->GetWeaponID() != WEAPON_GRENADE)
        {
           MomentumWeaponDrop(pWeapon);
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

void CMomentumPlayer::AddOnehop(CTriggerOnehop* pTrigger)
{
    if (m_vecOnehops.Count() > 0)
    {
        // Go backwards so we don't have to worry about vector being invalidated once we remove an entry
        FOR_EACH_VEC_BACK(m_vecOnehops, i)
        {
            CTriggerOnehop *pOnehop = m_vecOnehops[i];
            if (pOnehop && pOnehop->HasSpawnFlags(SF_TELEPORT_RESET_ONEHOP))
                m_vecOnehops.Remove(i);
        }
    }

    m_vecOnehops.AddToTail(pTrigger);
}

bool CMomentumPlayer::FindOnehopOnList(CTriggerOnehop *pTrigger) const
{
    return m_vecOnehops.Find(pTrigger) != m_vecOnehops.InvalidIndex();
}

void CMomentumPlayer::RemoveAllOnehops()
{
    FOR_EACH_VEC(m_vecOnehops, i)
    {
        const auto pTrigger = m_vecOnehops[i];
        if (pTrigger)
            pTrigger->SetNoLongerJumpableFired(false);
    }

    m_vecOnehops.RemoveAll();
}

void CMomentumPlayer::SetCurrentProgressTrigger(CBaseMomentumTrigger *pTrigger)
{
    const auto pProgressCheck = dynamic_cast<CTriggerProgress*>(pTrigger);
    if (pProgressCheck)
        m_iProgressNumber = pProgressCheck->GetProgressNumber();
    else
        m_iProgressNumber = -1;

    m_CurrentProgress.Set(pTrigger);

    RemoveAllOnehops();
}

CBaseMomentumTrigger* CMomentumPlayer::GetCurrentProgressTrigger() const
{
    return m_CurrentProgress.Get();
}

void CMomentumPlayer::CreateStartMark()
{
    const auto pStartTrigger = g_pMomentumTimer->GetStartTrigger(m_Data.m_iCurrentTrack);
    if (pStartTrigger && pStartTrigger->IsTouching(this))
    {
        ClearStartMark(m_Data.m_iCurrentTrack);

        m_pStartZoneMarks[m_Data.m_iCurrentTrack] = g_pMOMSavelocSystem->CreateSaveloc();
        if (m_pStartZoneMarks[m_Data.m_iCurrentTrack])
        {
            m_pStartZoneMarks[m_Data.m_iCurrentTrack]->vel = vec3_origin; // Rid the velocity
            DevLog("Successfully created a starting mark!\n");
        }
        else
        {
            Warning("Could not create the start mark for some reason!\n");
        }
    }
}

void CMomentumPlayer::ClearStartMark(int track)
{
    if (track >= 0 && track < MAX_TRACKS)
    {
        if (m_pStartZoneMarks[track])
            delete m_pStartZoneMarks[track];
        m_pStartZoneMarks[track] = nullptr;
    }
}

void CMomentumPlayer::DoMuzzleFlash()
{
    // Don't do the muzzle flash for the paint gun
    CWeaponBase *pWeapon = dynamic_cast<CWeaponBase *>(GetActiveWeapon());
    if (!(pWeapon && pWeapon->GetWeaponID() == WEAPON_PAINTGUN))
    {
        BaseClass::DoMuzzleFlash();
    }
}

void CMomentumPlayer::PreThink()
{
    BaseClass::PreThink();

    if (m_nButtons & IN_SCORE)
        m_Local.m_iHideHUD |= HIDEHUD_LEADERBOARDS;
    else
        m_Local.m_iHideHUD &= ~HIDEHUD_LEADERBOARDS;
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

void CMomentumPlayer::CheckChatText(char *p, int bufsize) { g_pMomentumGhostClient->SendChatMessage(p); }

// Overrides Teleport() so we can take care of the trail
void CMomentumPlayer::Teleport(const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity)
{
    // No need to remove the trail here, CreateTrail() already does it for us
    BaseClass::Teleport(newPosition, newAngles, newVelocity);
    PhysicsCheckForEntityUntouch();
    CreateTrail();

    g_ReplaySystem.SetTeleportedThisFrame();
}

bool CMomentumPlayer::KeyValue(const char *szKeyName, const char *szValue)
{
    //
    // It's possible for some maps to use "AddOutput -> origin x y z
    // for teleports.
    //
    if (FStrEq(szKeyName, "origin"))
    {
        // Copy of CBaseEntity::KeyValue
        Vector vecOrigin;
        UTIL_StringToVector(vecOrigin.Base(), szValue);

        // If you're hitting this assert, it's probably because you're
        // calling SetLocalOrigin from within a KeyValues method.. use SetAbsOrigin instead!
        Assert((GetMoveParent() == NULL) && !IsEFlagSet(EFL_DIRTY_ABSTRANSFORM));
        SetAbsOrigin(vecOrigin);



        // TODO: Throw this into OnTeleport() or something?
        CreateTrail();

        g_ReplaySystem.SetTeleportedThisFrame();

        return true;
    }

    return BaseClass::KeyValue(szKeyName, szValue);
}

// These two are overridden because of a weird compiler error,
// otherwise they serve no purpose.
bool CMomentumPlayer::KeyValue(const char *szKeyName, float flValue) 
{
    return BaseClass::KeyValue(szKeyName, flValue);
}

bool CMomentumPlayer::KeyValue(const char *szKeyName, const Vector &vecValue)
{
    return BaseClass::KeyValue(szKeyName, vecValue);
}

void CMomentumPlayer::CreateTrail()
{
    RemoveTrail();

    if (!mom_trail_enable.GetBool())
        return;

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

void CMomentumPlayer::SetButtonsEnabled(int iButtonFlags, bool bEnable)
{
    if (bEnable)
        EnableButtons(iButtonFlags);
    else
        DisableButtons(iButtonFlags);
}

void CMomentumPlayer::SetBhopEnabled(bool bEnable)
{
    m_bPreventPlayerBhop = !bEnable;
}

bool CMomentumPlayer::GetBhopEnabled() const
{
    return !m_bPreventPlayerBhop;
}

void CMomentumPlayer::OnZoneEnter(CTriggerZone *pTrigger)
{
    // Zone-specific things first
    const auto iZoneType = pTrigger->GetZoneType();
    switch (iZoneType)
    {
    case ZONE_TYPE_START:
        {
            const auto pStartTrigger = static_cast<CTriggerTimerStart*>(pTrigger);

            m_Data.m_iCurrentTrack = pStartTrigger->GetTrackNumber();
            // m_bTimerStartOnJump = pStartTrigger->StartOnJump();
            m_iLimitSpeedType = pStartTrigger->GetLimitSpeedType();
            m_bShouldLimitPlayerSpeed = pStartTrigger->IsLimitingSpeed();

            // Reset timer when we enter start zone
            SetCurrentZoneTrigger(pStartTrigger);
            SetCurrentProgressTrigger(pStartTrigger);
            g_pMomentumTimer->Reset(this);
        }
        break;
    case ZONE_TYPE_STOP:
        {
            // We've reached end zone, stop here
            auto pStopTrigger = static_cast<CTriggerTimerStop *>(pTrigger);

            if (g_pMomentumTimer->IsRunning() && !IsSpectatingGhost() && !m_bHasPracticeMode)
            {
                const int zoneNum = m_Data.m_iCurrentZone;

                // This is needed so we have an ending velocity.

                const float endvel = GetLocalVelocity().Length();
                const float endvel2D = GetLocalVelocity().Length2D();

                m_RunStats.SetZoneExitSpeed(zoneNum, endvel, endvel2D);

                // Check to see if we should calculate the timer offset fix
                if (pStopTrigger->ContainsPosition(GetPreviousOrigin()))
                    DevLog("PrevOrigin inside of end trigger, not calculating offset!\n");
                else
                {
                    DevLog("Previous origin is NOT inside the trigger, calculating offset...\n");
                    g_pMomentumTimer->CalculateTickIntervalOffset(this, ZONE_TYPE_STOP, zoneNum);
                }

                // This is needed for the final stage
                m_RunStats.SetZoneTime(zoneNum, g_pMomentumTimer->GetCurrentTime() -
                                                m_RunStats.GetZoneEnterTime(zoneNum));

                // Ending velocity checks
                float finalVel = endvel;
                float finalVel2D = endvel2D;

                if (endvel <= m_RunStats.GetZoneVelocityMax(0, false))
                    finalVel = m_RunStats.GetZoneVelocityMax(0, false);

                if (endvel2D <= m_RunStats.GetZoneVelocityMax(0, true))
                    finalVel2D = m_RunStats.GetZoneVelocityMax(0, true);

                m_RunStats.SetZoneVelocityMax(0, finalVel, finalVel2D);
                m_RunStats.SetZoneExitSpeed(0, endvel, endvel2D);

                // Stop the timer
                g_pMomentumTimer->Stop(this, true);
                m_Data.m_flTickRate = gpGlobals->interval_per_tick;
                m_Data.m_iRunTimeTicks = g_pMomentumTimer->GetLastRunTimeTicks();
                // The map is now finished, show the mapfinished panel
                m_Data.m_bMapFinished = true;
                m_Data.m_bTimerRunning = false;
            }
        }
        break;
    case ZONE_TYPE_CHECKPOINT:
        break;
    case ZONE_TYPE_STAGE:
        {
            const auto pStageTrigger = static_cast<CTriggerStage *>(pTrigger);
            SetCurrentProgressTrigger(pStageTrigger);
            SetCurrentZoneTrigger(pStageTrigger);

            if (g_pMomentumTimer->IsRunning())
            {
                const int zoneNum = pTrigger->GetZoneNumber();
                m_RunStats.SetZoneExitSpeed(zoneNum - 1, GetLocalVelocity().Length(),
                                            GetLocalVelocity().Length2D());
                g_pMomentumTimer->CalculateTickIntervalOffset(this, ZONE_TYPE_STOP, zoneNum);
                const float fCurrentZoneEnterTime = g_pMomentumTimer->CalculateStageTime(zoneNum);
                m_RunStats.SetZoneEnterTime(zoneNum, fCurrentZoneEnterTime);
                m_RunStats.SetZoneTime(zoneNum - 1, fCurrentZoneEnterTime -
                                       g_pMomentumTimer->CalculateStageTime(zoneNum - 1));
            }
        }
    default:
        break;
    }

    CMomRunEntity::OnZoneEnter(pTrigger);
}

void CMomentumPlayer::OnZoneExit(CTriggerZone *pTrigger)
{
    // We only care to go through with this if we're not spectating now
    if (m_iObserverMode == OBS_MODE_NONE)
    {
        // Zone-specific things first
        switch (pTrigger->GetZoneType())
        {
        case ZONE_TYPE_STOP:
            SetLaggedMovementValue(1.0f); // Reset slow motion
            break;
        case ZONE_TYPE_CHECKPOINT:
            break;
        case ZONE_TYPE_START:
            g_pMomentumTimer->CalculateTickIntervalOffset(this, ZONE_TYPE_START, 1);
            g_pMomentumTimer->TryStart(this, true);
            if (m_bShouldLimitPlayerSpeed)
            {
                Vector vecNewVelocity = GetAbsVelocity();

                const float flMaxSpeed = 350.0f;

                if (vecNewVelocity.Length2D() > flMaxSpeed)
                {
                    VectorNormalizeFast(vecNewVelocity);

                    vecNewVelocity *= flMaxSpeed;
                    SetAbsVelocity(vecNewVelocity);
                }
            }
            m_bShouldLimitPlayerSpeed = false;
            // No break here, we want to fall through; this handles both the start and stage triggers
        case ZONE_TYPE_STAGE:
            {
                const auto zoneNum = pTrigger->GetZoneNumber();
                // Timer won't be running if it's the start trigger
                if ((zoneNum == 1 || g_pMomentumTimer->IsRunning()) && !m_bHasPracticeMode)
                {
                    const auto enterVel3D = GetLocalVelocity().Length(),
                        enterVel2D = GetLocalVelocity().Length2D();
                    m_RunStats.SetZoneEnterSpeed(zoneNum, enterVel3D, enterVel2D);
                    if (zoneNum == 1)
                        m_RunStats.SetZoneEnterSpeed(0, enterVel3D, enterVel2D);
                }
            }
        default:
            break;
        }

        CMomRunEntity::OnZoneExit(pTrigger);
    }
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

void CMomentumPlayer::UpdateRunStats()
{
    // If we're in practicing mode, don't update.
    if (!m_bHasPracticeMode)
    {
        // ---- Jumps and Strafes ----
        UpdateJumpStrafes();

        //  ---- MAX VELOCITY ----
        UpdateMaxVelocity();
        // ----------

        //  ---- STRAFE SYNC -----
        UpdateRunSync();
        // ----------
    }

    // this might be used in a later update
    // m_flLastVelocity = velocity;

    // think once per tick
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
}

void CMomentumPlayer::UpdateRunSync()
{
    if (g_pMomentumTimer->IsRunning() || (ConVarRef("mom_hud_strafesync_draw").GetInt() == 2))
    {
        if (!(GetFlags() & (FL_ONGROUND | FL_INWATER)) && GetMoveType() != MOVETYPE_LADDER)
        {
            float dtAngle = EyeAngles().y - LastEyeAngles().y;
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
            m_Data.m_flStrafeSync = (float(m_nPerfectSyncTicks) / float(m_nStrafeTicks)) * 100.0f;
            // ticks gaining speed / ticks strafing
            m_Data.m_flStrafeSync2 = (float(m_nAccelTicks) / float(m_nStrafeTicks)) * 100.0f;
        }

        SetLastEyeAngles(EyeAngles());
    }
}

void CMomentumPlayer::UpdateJumpStrafes()
{
    if (!g_pMomentumTimer->IsRunning())
        return;

    int currentZone = m_Data.m_iCurrentZone;
    if (!m_bPrevTimerRunning) // timer started on this tick
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

    m_bPrevTimerRunning = g_pMomentumTimer->IsRunning();
    m_nPrevButtons = m_nButtons;
}

void CMomentumPlayer::UpdateMaxVelocity()
{
    if (!g_pMomentumTimer->IsRunning())
        return;

    int currentZone = m_Data.m_iCurrentZone;
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
    m_Data.m_flStrafeSync = 0;
    m_Data.m_flStrafeSync2 = 0;
    m_RunStats.Init(g_pMomentumTimer->GetZoneCount());
}
void CMomentumPlayer::CalculateAverageStats()
{
    if (g_pMomentumTimer->IsRunning())
    {
        int currentZone = m_Data.m_iCurrentZone;

        m_flZoneTotalSync[currentZone] += m_Data.m_flStrafeSync;
        m_flZoneTotalSync2[currentZone] += m_Data.m_flStrafeSync2;
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
        m_flZoneTotalSync[0] += m_Data.m_flStrafeSync;
        m_flZoneTotalSync2[0] += m_Data.m_flStrafeSync2;
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
            CMomentumOnlineGhostEntity *pEntity = dynamic_cast<CMomentumOnlineGhostEntity *>(target);
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
        // Don't allow user teleports when spectating. Savelocs can be created, but the
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
    return BaseClass::GetNextObserverSearchStartPoint(bReverse);
    /*int iDir = bReverse ? -1 : 1;

    int startIndex;
    g_pMomentumGhostClient->GetOnlineGhostEntityFromID();
    g_ReplaySystem.m_pPlaybackReplay;
    if (m_hObserverTarget)
    {
        // start using last followed player
        CMomentumGhostBaseEntity *entity = dynamic_cast<CMomentumGhostBaseEntity*>(m_hObserverTarget.Get());
        if (entity)
        {
            if (entity->IsReplayGhost())
            {
                // If we're spectating the replay ghost, check our online map
                CUtlMap<uint64, CMomentumOnlineGhostEntity*> *onlineGhosts = g_pMomentumGhostClient->GetOnlineGhostMap();
                if (onlineGhosts->Count() > 0)
                {
                    
                }
            }
            else
            {
                
            }
        }
        startIndex = m_hObserverTarget->entindex();
    }
    else
    {
        // start using own player index
        startIndex = this->entindex();
    }
    startIndex += iDir;
    startIndex = WrapClamp(startIndex, 1, gEntList.NumberOfEntities());

    return startIndex;*/
}

CBaseEntity *CMomentumPlayer::FindNextObserverTarget(bool bReverse)
{
    CUtlMap<uint64, CMomentumOnlineGhostEntity*> *onlineGhosts = g_pMomentumGhostClient->GetOnlineGhostMap();
    if (m_hObserverTarget)
    {
        // start using last followed player
        CMomentumGhostBaseEntity *entity = GetGhostEnt();
        if (entity)
        {
            if (entity->IsReplayGhost())
            {
                // If we're spectating the replay ghost, check our online map
                if (onlineGhosts->Count() > 0)
                {
                    return bReverse ? onlineGhosts->Element(onlineGhosts->LastInorder()) : onlineGhosts->Element(onlineGhosts->FirstInorder());
                }
            }
            else if (entity->IsOnlineGhost())
            {
                // Was an online ghost, find its index
                CMomentumOnlineGhostEntity *onlineEnt = dynamic_cast<CMomentumOnlineGhostEntity*>(entity);

                CMomentumGhostBaseEntity *pCurrentReplayEnt = nullptr;
                if (g_ReplaySystem.GetPlaybackReplay())
                {
                    pCurrentReplayEnt = g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
                }

                if (onlineEnt)
                {
                    unsigned short indx = onlineGhosts->Find(onlineEnt->GetGhostSteamID().ConvertToUint64());
                    if (onlineGhosts->Count() > 1)
                    {
                        if (indx == onlineGhosts->LastInorder())
                        {
                            if (bReverse)
                            {
                                return onlineGhosts->Element(onlineGhosts->PrevInorder(indx));
                            }
                            // Check the replay ghost, if not there, go to head of map
                            if (pCurrentReplayEnt)
                            {
                                return pCurrentReplayEnt;
                            }

                            return onlineGhosts->Element(onlineGhosts->FirstInorder());
                        }
                        if (indx == onlineGhosts->FirstInorder())
                        {
                            if (bReverse)
                            {
                                // Check the replay ghost, if not there, go to head of map
                                if (pCurrentReplayEnt)
                                {
                                    return pCurrentReplayEnt;
                                }
                                return onlineGhosts->Element(onlineGhosts->LastInorder());
                            }
                            return onlineGhosts->Element(onlineGhosts->NextInorder(indx));
                        }
                        // in the middle of the list, iterate it like normal
                        return bReverse ? onlineGhosts->Element(onlineGhosts->PrevInorder(indx)) : onlineGhosts->Element(onlineGhosts->NextInorder(indx));
                    }

                    // Check the replay ghost, if not there, don't do anything, we're already spectating
                    if (pCurrentReplayEnt)
                    {
                        return pCurrentReplayEnt;
                    }
                }
            }
        }
    }
    else
    {
        if (g_ReplaySystem.GetPlaybackReplay())
        {
            return g_ReplaySystem.GetPlaybackReplay()->GetRunEntity();
        }
        else
        {
            if (onlineGhosts->Count() > 0)
            {
                return onlineGhosts->Element(onlineGhosts->FirstInorder());
            }
        }
    }

    return nullptr;
}

// Overridden for custom IN_EYE check
void CMomentumPlayer::CheckObserverSettings()
{
    // make sure our last mode is valid
    if (m_iObserverLastMode < OBS_MODE_FIXED)
    {
        m_iObserverLastMode = OBS_MODE_ROAMING;
    }

    // check if our spectating target is still a valid one
    if (m_iObserverMode == OBS_MODE_IN_EYE || m_iObserverMode == OBS_MODE_CHASE || m_iObserverMode == OBS_MODE_FIXED 
        || m_iObserverMode == OBS_MODE_POI)
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
        }
    }
}

void CMomentumPlayer::ValidateCurrentObserverTarget()
{
    if (!IsValidObserverTarget(m_hObserverTarget.Get()))
    {
        // our target is not valid, try to find new target
        CBaseEntity *target = FindNextObserverTarget(false);
        if (target)
        {
            // switch to new valid target
            SetObserverTarget(target);
        }
        else
        {
            // roam player view right where it is
            ForceObserverMode(OBS_MODE_ROAMING);
            m_hObserverTarget.Set(nullptr); // no target to follow

            // Let the client know too!
            IGameEvent *event = gameeventmanager->CreateEvent("spec_target_updated");
            if (event)
            {
                gameeventmanager->FireEventClientSide(event);
            }
        }
    }
}

void CMomentumPlayer::TravelSpectateTargets(bool bReverse)
{
    if (GetObserverMode() > OBS_MODE_FIXED)
    {
        // set new spectator mode
        CBaseEntity *target = FindNextObserverTarget(bReverse);
        if (target)
        {
            if (m_bForcedObserverMode)
            {
                // If we found a valid target
                m_bForcedObserverMode = false;        // disable forced mode
                SetObserverMode(m_iObserverLastMode); // switch to last mode
            }
            // goto target
            SetObserverTarget(target);
        }
    }
    else if (GetObserverMode() == OBS_MODE_FREEZECAM)
    {
        AttemptToExitFreezeCam();
    }
}


void CMomentumPlayer::TweenSlowdownPlayer()
{
    // slowdown when map is finished
    if (m_Data.m_bMapFinished)
        // decrease our lagged movement value by 10% every tick
        m_flTweenVelValue *= 0.9f;
    else
        m_flTweenVelValue = GetLaggedMovementValue(); // Reset the tweened value back to normal

    SetLaggedMovementValue(m_flTweenVelValue);

    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "TWEEN");
}

CMomentumGhostBaseEntity *CMomentumPlayer::GetGhostEnt() const
{
    return dynamic_cast<CMomentumGhostBaseEntity *>(m_hObserverTarget.Get());
}

bool CMomentumPlayer::StartObserverMode(int mode)
{
    if (m_iObserverMode == OBS_MODE_NONE)
    {
        SaveCurrentRunState();
    }

    return BaseClass::StartObserverMode(mode);
}

void CMomentumPlayer::StopSpectating()
{
    CMomentumGhostBaseEntity *pGhost = GetGhostEnt();
    if (pGhost)
        pGhost->RemoveSpectator();

    m_bWasSpectating = true;

    StopObserverMode();
    m_hObserverTarget.Set(nullptr);
    ForceRespawn();
    SetMoveType(MOVETYPE_WALK);

    FIRE_GAME_WIDE_EVENT("spec_stop");

    // Update the lobby/server if there is one
    m_sSpecTargetSteamID.Clear(); // reset steamID when we stop spectating
    g_pMomentumGhostClient->SetSpectatorTarget(m_sSpecTargetSteamID, false);
}

void CMomentumPlayer::SaveCurrentRunState()
{
    m_nSavedButtons = m_nButtons;
    m_Data.m_iOldTrack = m_Data.m_iCurrentTrack;
    m_Data.m_iOldZone = m_Data.m_iCurrentZone;
    m_vecLastPos = GetAbsOrigin();
    m_angLastAng = GetAbsAngles();
    m_vecLastVelocity = GetAbsVelocity();
    m_fLastViewOffset = GetViewOffset().z;
    m_nSavedAccelTicks = m_nAccelTicks;
    m_nSavedPerfectSyncTicks = m_nPerfectSyncTicks;
    m_nSavedStrafeTicks = m_nStrafeTicks;
}

void CMomentumPlayer::RestoreRunState()
{
    m_nButtons = m_nSavedButtons;
    m_Data.m_iCurrentTrack = m_Data.m_iOldTrack;
    m_Data.m_iCurrentZone = m_Data.m_iOldZone;
    // Teleport calls SnapEyeAngles, don't worry
    Teleport(&m_vecLastPos, &m_angLastAng, &m_vecLastVelocity);
    SetViewOffset(Vector(0, 0, m_fLastViewOffset));
    SetLastEyeAngles(m_angLastAng);

    m_nAccelTicks = m_nSavedAccelTicks;
    m_nPerfectSyncTicks = m_nSavedPerfectSyncTicks;
    m_nStrafeTicks = m_nSavedStrafeTicks;
}

void CMomentumPlayer::PostThink()
{
    // Update previous origins
    NewPreviousOrigin(GetLocalOrigin());
    BaseClass::PostThink();
}