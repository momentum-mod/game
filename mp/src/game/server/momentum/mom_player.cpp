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
#include "weapon/weapon_mom_paintgun.h"
#include "mom_system_gamemode.h"
#include "mom_system_saveloc.h"
#include "util/mom_util.h"
#include "mom_replay_system.h"
#include "run/mom_replay_base.h"
#include "mapzones.h"
#include "fx_mom_shared.h"
#include "mom_rocket.h"

#include "tier0/memdbgon.h"

#define AVERAGE_STATS_INTERVAL 0.1

static MAKE_TOGGLE_CONVAR(
    mom_practice_safeguard, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Toggles the safeguard for enabling practice mode (not pressing any movement keys to enable). 0 = OFF, 1 = ON.\n");

CON_COMMAND_F(
    mom_practice,
    "Toggle. Allows player to fly around in noclip during a run, teleports the player back upon untoggling.\n"
    "Only activates when player is not pressing any movement inputs if the timer is running and mom_practice_safeguard "
    "is 1.\n",
    FCVAR_CLIENTCMD_CAN_EXECUTE)
{
    CMomentumPlayer *pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    pPlayer->TogglePracticeMode();
}

CON_COMMAND(
    mom_eyetele,
    "Teleports the player to the solid that they are looking at.\n")
{
    CMomentumPlayer *pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    if (pPlayer->m_bHasPracticeMode || !g_pMomentumTimer->IsRunning())
    {
        trace_t tr;
        Vector pos = pPlayer->EyePosition();
        Vector ang;
        pPlayer->EyeVectors(&ang);

        int mask = CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE | CONTENTS_WINDOW;
        UTIL_TraceLine(pos, pos + ang * MAX_COORD_RANGE, mask, pPlayer, COLLISION_GROUP_NONE, &tr);

        if (!CloseEnough(tr.fraction, 1.0f) && tr.DidHit())
        {
            Vector hit = tr.endpos;
            if (enginetrace->PointOutsideWorld(hit))
            {
                hit += (hit - pos).Normalized() * 64.0f;

                UTIL_TraceLine(hit, hit + ang * MAX_COORD_RANGE, mask, pPlayer, COLLISION_GROUP_NONE, &tr);

                if (tr.DidHit())
                    hit = tr.endpos;
            }
            Vector nrm = tr.plane.normal;

            if (CloseEnough(nrm.z, 1.0f))
            {
                if (pPlayer->GetMoveType() == MOVETYPE_NOCLIP) 
                    hit.z += 32.0f;
            }
            else
            {
                hit += (hit - pos).Normalized() * -32.0f;
            }

            QAngle new_ang = pPlayer->GetAbsAngles();
            if (new_ang.x > 45.0f && enginetrace->PointOutsideWorld(pos))
                new_ang.x = 0.0f;

            g_pMomentumTimer->SetCanStart(false);
            pPlayer->Teleport(&hit, &new_ang, nullptr);
        }
    }
    else
    {
        Warning("Eyetele can only be used when the timer is not running or in practice mode!\n");
    }
}

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
SendPropArray3(SENDINFO_ARRAY3(m_iZoneCount), SendPropInt(SENDINFO_ARRAY(m_iZoneCount), 7, SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_iLinearTracks), SendPropInt(SENDINFO_ARRAY(m_iLinearTracks), 1, SPROP_UNSIGNED)),
SendPropDataTable(SENDINFO_DT(m_Data), &REFERENCE_SEND_TABLE(DT_MomRunEntityData)),
SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MomRunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumPlayer)
    DEFINE_THINKFUNC(PlayerThink),
    DEFINE_THINKFUNC(CalculateAverageStats),
    /*DEFINE_THINKFUNC(LimitSpeedInStartZone),*/
END_DATADESC();

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);

void AppearanceCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();

    if (pPlayer)
    {
        pPlayer->LoadAppearance(false);
    }
}

static MAKE_CONVAR_C(mom_ghost_bodygroup, "11", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Appearance bodygroup (shape)\n",
                     APPEARANCE_BODYGROUP_MIN, APPEARANCE_BODYGROUP_MAX, AppearanceCallback);

static ConVar mom_ghost_color("mom_ghost_color", "FF00FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                              "Set the ghost's color. Accepts HEX color value in format RRGGBBAA. if RRGGBB is supplied, Alpha is set to 0x4B",
                              AppearanceCallback);

static ConVar mom_trail_color("mom_trail_color", "FF00FFFF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                              "Set the player's trail color. Accepts HEX color value in format RRGGBBAA",
                              AppearanceCallback);

static MAKE_CONVAR_C(mom_trail_length, "4", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Length of the player's trail (in seconds)\n",
                     APPEARANCE_TRAIL_LEN_MIN, APPEARANCE_TRAIL_LEN_MAX, AppearanceCallback);

static MAKE_TOGGLE_CONVAR_C(mom_trail_enable, "0", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE, "Paint a faint beam trail on the player. 0 = OFF, 1 = ON\n", AppearanceCallback);

// Equivalent to "tf_damagescale_self_soldier" (default: 0.6) in TF2
// Used for scaling damage when not on ground and not in water
#define MOM_DAMAGESCALE_SELF_ROCKET 0.6f

// Equivalent to "tf_damageforcescale_self_soldier_rj" (default 10.0) in TF2
// Used for scaling force when not on ground
#define MOM_DAMAGEFORCESCALE_SELF_ROCKET_AIR 10.0f

// Equivalent to "tf_damageforcescale_self_soldier_badrj" (default: 5.0) in TF2
// Used for scaling force when on ground
#define MOM_DAMAGEFORCESCALE_SELF_ROCKET 5.0f



static CMomentumPlayer *s_pPlayer = nullptr;

CMomentumPlayer::CMomentumPlayer()
    : m_duckUntilOnGround(false), m_flStamina(0.0f),
      m_flLastVelocity(0.0f), m_nPerfectSyncTicks(0), m_nStrafeTicks(0), m_nAccelTicks(0),
      m_nPrevButtons(0), m_flTweenVelValue(1.0f), m_bInAirDueToJump(false), m_iProgressNumber(-1)
{
    m_bAllowUserTeleports = true;
    m_flPunishTime = -1;
    m_iLastBlock = -1;
    m_iOldTrack = 0;
    m_iOldZone = 0;
    m_fLerpTime = 0.0f;

    m_bWasSpectating = false;

    m_flNextPaintTime = gpGlobals->curtime;

    m_CurrentSlideTrigger = nullptr;

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

    m_RunStats.Init();

    ListenForGameEvent("mapfinished_panel_closed");
    ListenForGameEvent("lobby_join");

    for (int i = 0; i < MAX_TRACKS; i++)
    {
        m_pStartZoneMarks[i] = nullptr;
    }

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
    {
        gEntList.AddListenerEntity(this);
    }
}

CMomentumPlayer::~CMomentumPlayer()
{
    if (this == s_pPlayer)
        s_pPlayer = nullptr;

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
    {
        gEntList.RemoveListenerEntity(this);
        m_vecRockets.RemoveAll();
    }

    RemoveAllOnehops();

    // Clear our spectating status just in case we leave the map while spectating
    StopObserverMode();
    g_pMomentumGhostClient->SetSpectatorTarget(k_steamIDNil, false, true);
}

CMomentumPlayer* CMomentumPlayer::CreatePlayer(const char *className, edict_t *ed)
{
    s_PlayerEdict = ed;
    const auto toRet = static_cast<CMomentumPlayer *>(CreateEntityByName(className));
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
    if (FStrEq(pEvent->GetName(), "mapfinished_panel_closed"))
    {
        // Hide the mapfinished panel and reset our speed to normal
        m_Data.m_bMapFinished = false;
        ResetMovementProperties();

        // Fix for the replay system not being able to listen to events
        if (g_ReplaySystem.GetPlaybackReplay() && !pEvent->GetBool("restart"))
        {
            g_ReplaySystem.UnloadPlayback();
        }
    }
    else if (FStrEq(pEvent->GetName(), "lobby_join"))
    {
        SendAppearance();
    }
}

void CMomentumPlayer::ItemPostFrame()
{
    BaseClass::ItemPostFrame();
    if (m_nButtons & IN_PAINT)
        DoPaint();
}

//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon.
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CMomentumPlayer::BumpWeapon(CBaseCombatWeapon *pWeapon)
{
    // Get the weapon that we currently have at that slot
    const auto pCurrWeapon = Weapon_GetSlotAndPosition(pWeapon->GetSlot(), pWeapon->GetPosition());
    if (pCurrWeapon)
    {
        // Switch to that weapon for convenience
        Weapon_Switch(pCurrWeapon);
        return false;
    }
    // Otherwise we can try to pick up that weapon
    return BaseClass::BumpWeapon(pWeapon);
}

void CMomentumPlayer::FlashlightTurnOn()
{
    // Emit sound by default
    FlashlightToggle(true, true);
}

void CMomentumPlayer::FlashlightTurnOff()
{
    // Emit sound by default
    FlashlightToggle(false, true);
}

void CMomentumPlayer::FlashlightToggle(bool bOn, bool bEmitSound)
{
    if (bEmitSound)
        EmitSound(bOn ? SND_FLASHLIGHT_ON : SND_FLASHLIGHT_OFF);

    bOn ? AddEffects(EF_DIMLIGHT) : RemoveEffects(EF_DIMLIGHT);

    m_AppearanceData.m_bFlashlightEnabled = bOn;

    SendAppearance();
}

void CMomentumPlayer::LoadAppearance(bool bForceUpdate)
{
    AppearanceData_t newData;
    uint32 newHexColor = MomUtil::GetHexFromColor(mom_trail_color.GetString());
    newData.m_iTrailRGBAColorAsHex = newHexColor;
    newData.m_iTrailLength = mom_trail_length.GetInt();
    newData.m_bTrailEnabled = mom_trail_enable.GetBool();

    newHexColor = MomUtil::GetHexFromColor(mom_ghost_color.GetString());
    newData.m_iModelRGBAColorAsHex = newHexColor;
    
    const auto bodyGroup = mom_ghost_bodygroup.GetInt();
    newData.m_iBodyGroup = bodyGroup;

    newData.m_bFlashlightEnabled = IsEffectActive(EF_DIMLIGHT);

    if (SetAppearanceData(newData, bForceUpdate))
    {
        SendAppearance();
    }
}

void CMomentumPlayer::SendAppearance() { g_pMomentumGhostClient->SendAppearanceData(m_AppearanceData); }

void CMomentumPlayer::Spawn()
{
    SetModel(ENTITY_MODEL);
    SetBodygroup(1, 11); // BODY_PROLATE_ELLIPSE
    // BASECLASS SPAWN MUST BE AFTER SETTING THE MODEL, OTHERWISE A NULL HAPPENS!
    BaseClass::Spawn();
    
    m_takedamage = DAMAGE_EVENTS_ONLY;

    // this removes the flag that was added while switching to spectator mode which prevented the player from activating
    // triggers
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    m_bAllowUserTeleports = true;

    // Handle resetting only if we weren't spectating nor have practice mode
    if (m_bWasSpectating)
    {
        RestoreRunState(false);
        m_bWasSpectating = false;
    }
    else if (!m_bHasPracticeMode)
    {
        m_Data.m_bIsInZone = false;
        m_Data.m_bMapFinished = false;
        m_Data.m_iCurrentZone = 0;
        m_bPreventPlayerBhop = false;
        m_iLandTick = 0;
        ResetRunStats();

        for (int i = 0; i < MAX_TRACKS; i++)
        {
            ClearStartMark(i);
        }

        g_MapZoneSystem.DispatchMapInfo(this);

        // Reset current checkpoint trigger upon spawn
        m_CurrentProgress.Term();
    }

    SetPracticeModeState();

    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("THINK_AVERAGE_STATS");
    // RegisterThinkContext("CURTIME_FOR_START");
    RegisterThinkContext("TWEEN");
    SetContextThink(&CMomentumPlayer::PlayerThink, gpGlobals->curtime + gpGlobals->interval_per_tick,
                    "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CalculateAverageStats, gpGlobals->curtime + AVERAGE_STATS_INTERVAL,
                    "THINK_AVERAGE_STATS");
    // SetContextThink(&CMomentumPlayer::LimitSpeedInStartZone, gpGlobals->curtime, "CURTIME_FOR_START");
    SetContextThink(&CMomentumPlayer::TweenSlowdownPlayer, gpGlobals->curtime, "TWEEN");

    LoadAppearance(false);

    SetNextThink(gpGlobals->curtime);

    g_pGameModeSystem->GetGameMode()->OnPlayerSpawn(this);
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
    const char *pSpawns[] = {"info_player_start", "info_player_counterterrorist", "info_player_terrorist", "info_player_teamspawn"};
    for (auto pSpawn : pSpawns)
    {
        if (SelectSpawnSpot(pSpawn, pStart))
            return pStart;
    }

    Warning("No valid spawn point found!\n");
    return Instance(INDEXENT(0));
}

void CMomentumPlayer::SetAutoBhopEnabled(bool bEnable)
{
    m_bAutoBhop = bEnable;
    DevLog("%s autobhop\n", bEnable ? "Enabled" : "Disabled");
}

void CMomentumPlayer::OnJump()
{
    // OnCheckBhop code
    m_bDidPlayerBhop = gpGlobals->tickcount - m_iLandTick < NUM_TICKS_TO_BHOP;
    if (!m_bDidPlayerBhop)
        m_iSuccessiveBhops = 0;

    m_Data.m_flLastJumpVel = GetLocalVelocity().Length2D();
    m_Data.m_flLastJumpZPos = GetLocalOrigin().z;
    m_iSuccessiveBhops++;

    // Set our runstats jump count
    if (g_pMomentumTimer->IsRunning())
    {
        const int currentZone = m_Data.m_iCurrentZone;
        m_RunStats.SetZoneJumps(0, m_RunStats.GetZoneJumps(0) + 1); // Increment total jumps
        m_RunStats.SetZoneJumps(currentZone, m_RunStats.GetZoneJumps(currentZone) + 1); // Increment zone jumps
    }
    else if (m_Data.m_bIsInZone && m_Data.m_iCurrentZone == 1 && m_bStartTimerOnJump)
    {
        // Jumps are incremented in this method
        g_pMomentumTimer->TryStart(this, false);
    }
}

void CMomentumPlayer::OnLand()
{
    m_iLandTick = gpGlobals->tickcount;

    if (m_Data.m_bIsInZone && m_Data.m_iCurrentZone == 1 && GetMoveType() == MOVETYPE_WALK && !m_bHasPracticeMode)
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
        if (!g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
        {
            CWeaponBase *pWeapon = dynamic_cast<CWeaponBase *>(GetActiveWeapon());

            if (pWeapon && pWeapon->GetWeaponID() != WEAPON_GRENADE)
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
    const auto pCurrentZoneTrigger = GetCurrentZoneTrigger();

    if (pCurrentZoneTrigger && pCurrentZoneTrigger->IsTouching(this) && pCurrentZoneTrigger->GetZoneType() == ZONE_TYPE_START)
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

void CMomentumPlayer::CheckChatText(char *p, int bufsize) { g_pMomentumGhostClient->SendChatMessage(p); }

// Overrides Teleport() so we can take care of the trail
void CMomentumPlayer::Teleport(const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity)
{
    if (!m_bAllowUserTeleports || m_Data.m_bMapFinished)
        return;

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
    if (g_pMomentumTimer->IsRunning() && m_bHasPracticeMode)
        return;

    if (m_iObserverMode != OBS_MODE_NONE)
        return;

    // Zone-specific things first
    const auto iZoneType = pTrigger->GetZoneType();
    switch (iZoneType)
    {
    case ZONE_TYPE_START:
        {
            ResetMovementProperties();

            const auto pStartTrigger = static_cast<CTriggerTimerStart*>(pTrigger);

            m_Data.m_iCurrentTrack = pStartTrigger->GetTrackNumber();
            m_bStartTimerOnJump = pStartTrigger->StartOnJump();
            m_iLimitSpeedType = pStartTrigger->GetLimitSpeedType();
            m_bShouldLimitPlayerSpeed = pStartTrigger->IsLimitingSpeed();

            SetCurrentZoneTrigger(pStartTrigger);
            SetCurrentProgressTrigger(pStartTrigger);

            if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
            {
                DestroyRockets();

                // Don't limit speed in rocketjump mode,
                // reset timer on zone enter and start on zone leave.
                g_pMomentumTimer->Reset(this);
                m_bStartTimerOnJump = false;
                m_bShouldLimitPlayerSpeed = false;
                break;
            }

            // Limit to 260 if timer is not running and we're not in practice mode
            if (!(g_pMomentumTimer->IsRunning() || m_bHasPracticeMode))
                LimitSpeed(260.0f, false);

            // When we start on jump, we reset on land (see OnLand)
            // If we're already on ground we can safely reset now
            if (GetFlags() & FL_ONGROUND && GetMoveType() == MOVETYPE_WALK && !m_bHasPracticeMode)
            {
                g_pMomentumTimer->Reset(this);
            }
        }
        break;
    case ZONE_TYPE_STOP:
        {
            // We've reached end zone, stop here
            //auto pStopTrigger = static_cast<CTriggerTimerStop *>(pTrigger);
            m_iOldTrack = m_Data.m_iCurrentTrack;
            m_iOldZone = m_Data.m_iCurrentZone;

            if (g_pMomentumTimer->IsRunning())
            {
                const int zoneNum = m_Data.m_iCurrentZone;

                // This is needed so we have an ending velocity.
                const float endvel = GetLocalVelocity().Length();
                const float endvel2D = GetLocalVelocity().Length2D();

                m_RunStats.SetZoneExitSpeed(zoneNum, endvel, endvel2D);

                // Check to see if we should calculate the timer offset fix
                /*if (pStopTrigger->ContainsPosition(GetPreviousOrigin()))
                    DevLog("PrevOrigin inside of end trigger, not calculating offset!\n");
                else
                {
                    DevLog("Previous origin is NOT inside the trigger, calculating offset...\n");
                    // g_pMomentumTimer->CalculateTickIntervalOffset(this, ZONE_TYPE_STOP, zoneNum);
                }*/

                // This is needed for the final stage
                m_RunStats.SetZoneTicks(zoneNum, g_pMomentumTimer->GetCurrentTime() - m_RunStats.GetZoneEnterTick(zoneNum));

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
                m_Data.m_iRunTime = g_pMomentumTimer->GetLastRunTime();
                // The map is now finished, show the mapfinished panel
                m_Data.m_bMapFinished = true;
            }
        }
        break;
    case ZONE_TYPE_CHECKPOINT:
    case ZONE_TYPE_STAGE:
        {
            const auto bIsStage = iZoneType == ZONE_TYPE_STAGE;
            const int zoneNum = pTrigger->GetZoneNumber();
            if (bIsStage)
            {
                SetCurrentProgressTrigger(pTrigger);
                SetCurrentZoneTrigger(pTrigger);
            }
            else
            {
                const auto enterVel3D = GetLocalVelocity().Length(),
                           enterVel2D = GetLocalVelocity().Length2D();
                m_RunStats.SetZoneEnterSpeed(zoneNum, enterVel3D, enterVel2D);
            }

            if (g_pMomentumTimer->IsRunning())
            {
                const auto locVel = GetLocalVelocity();
                m_RunStats.SetZoneExitSpeed(zoneNum - 1, locVel.Length(), locVel.Length2D());
                // g_pMomentumTimer->CalculateTickIntervalOffset(this, ZONE_TYPE_STOP, zoneNum);

                if (zoneNum > m_Data.m_iCurrentZone)
                {
                    const auto iTime = g_pMomentumTimer->GetCurrentTime();
                    m_RunStats.SetZoneEnterTick(zoneNum, iTime);
                    m_RunStats.SetZoneTicks(zoneNum - 1, iTime - m_RunStats.GetZoneEnterTick(zoneNum - 1));
                }
            }
        }
    default:
        break;
    }

    CMomRunEntity::OnZoneEnter(pTrigger);
}

void CMomentumPlayer::OnZoneExit(CTriggerZone *pTrigger)
{
    if (g_pMomentumTimer->IsRunning() && m_bHasPracticeMode)
        return;

    if (m_iObserverMode != OBS_MODE_NONE)
        return;

    // Zone-specific things first
    switch (pTrigger->GetZoneType())
    {
    case ZONE_TYPE_STOP:
        m_Data.m_iCurrentTrack = m_iOldTrack;
        m_Data.m_iCurrentZone = m_iOldZone;
        ResetMovementProperties();
        break;
    case ZONE_TYPE_CHECKPOINT:
        break;
    case ZONE_TYPE_START:
        // g_pMomentumTimer->CalculateTickIntervalOffset(this, ZONE_TYPE_START, 1);
        g_pMomentumTimer->TryStart(this, true);
        if (m_bShouldLimitPlayerSpeed && !m_bHasPracticeMode && !g_pMOMSavelocSystem->IsUsingSaveLocMenu())
        {
            const auto pStart = static_cast<CTriggerTimerStart*>(pTrigger);

            LimitSpeed(pStart->GetSpeedLimit(), true);
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

void CMomentumPlayer::ResetMovementProperties()
{
    SetLaggedMovementValue(1.0f);
    SetGravity(1.0f);
}

void CMomentumPlayer::Touch(CBaseEntity *pOther)
{
    BaseClass::Touch(pOther);

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_BHOP))
    {
        if (g_MOMBlockFixer->IsBhopBlock(pOther->entindex()))
            g_MOMBlockFixer->PlayerTouch(this, pOther);
    }
}

void CMomentumPlayer::OnEntitySpawned(CBaseEntity *pEntity)
{
    if (pEntity->GetFlags() & FL_GRENADE)
    {
        const auto pRocket = dynamic_cast<CMomRocket *>(pEntity);
        if (pRocket)
        {
            m_vecRockets.AddToTail(pRocket);
        }
    }
}

void CMomentumPlayer::OnEntityDeleted(CBaseEntity *pEntity)
{
    if ((pEntity->GetFlags() & FL_GRENADE) && !m_vecRockets.IsEmpty())
    {
        const auto pRocket = dynamic_cast<CMomRocket *>(pEntity);
        if (pRocket)
        {
            m_vecRockets.FindAndRemove(pRocket);
        }
    }
}

void CMomentumPlayer::PlayerThink()
{
    // If we're in practicing mode, don't update.
    if (!m_bHasPracticeMode)
    {
        // ---- Jumps and Strafes ----
        UpdateStrafes();

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
    static ConVarRef sync("mom_hud_strafesync_draw");
    if (g_pMomentumTimer->IsRunning() || (sync.GetInt() == 2))
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

void CMomentumPlayer::UpdateStrafes()
{
    if (!g_pMomentumTimer->IsRunning())
        return;

    const auto currentZone = m_Data.m_iCurrentZone;
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
    // MOM_TODO: Consider any other resets needed (classname, any flags, etc)

    m_nPerfectSyncTicks = 0;
    m_nStrafeTicks = 0;
    m_nAccelTicks = 0;
    m_Data.m_flStrafeSync = 0;
    m_Data.m_flStrafeSync2 = 0;
    m_RunStats.Init(g_MapZoneSystem.GetZoneCount(m_Data.m_iCurrentTrack));
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

void CMomentumPlayer::LimitSpeed(float flSpeedLimit, bool bSaveZ)
{
    auto vecNewVelocity = GetAbsVelocity();

    if (vecNewVelocity.Length2D() > flSpeedLimit)
    {
        const auto fSavedZ = vecNewVelocity.z;
        vecNewVelocity.z = 0;
        VectorNormalizeFast(vecNewVelocity);

        vecNewVelocity *= flSpeedLimit;

        if (bSaveZ)
            vecNewVelocity.z = fSavedZ;

        SetAbsVelocity(vecNewVelocity);
    }
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

// Override of CBasePlayer::SetObserverTarget that lets us add/remove ourselves as spectators to the ghost
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

    const auto base = BaseClass::SetObserverTarget(target);

    if (pGhostToSpectate && base)
    {
        // Don't allow user teleports when spectating. Savelocs can be created, but the
        // teleporting logic needs to not be allowed.
        m_bAllowUserTeleports = false;

        RemoveTrail();

        // Disable flashlight
        if (FlashlightIsOn())
            FlashlightToggle(false, false); // Don't emit flashlight sound when turned off automatically

        pGhostToSpectate->SetSpectator(this);

        if (pGhostToSpectate->IsOnlineGhost())
        {
            const auto pOnlineEnt = static_cast<CMomentumOnlineGhostEntity*>(target);
            m_sSpecTargetSteamID = pOnlineEnt->GetGhostSteamID();
        }
        else if (pGhostToSpectate->IsReplayGhost())
        {
            m_sSpecTargetSteamID = CSteamID(uint64(1));
        }

        g_pMomentumGhostClient->SetSpectatorTarget(m_sSpecTargetSteamID, pCurrentGhost == nullptr);
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

inline bool TestGhost(CMomentumOnlineGhostEntity *pEnt)
{
    return pEnt && !pEnt->m_bSpectating;
}

inline CBaseEntity *IterateAndFindViableNextGhost(uint16 startIndx, bool bReverse, CBaseEntity *pReplay, bool bTestStart)
{
    const auto pOnlineGhostMap = g_pMomentumGhostClient->GetOnlineGhostMap();
    // Test our starter if we need to
    if (bTestStart && pOnlineGhostMap->IsValidIndex(startIndx) && TestGhost(pOnlineGhostMap->Element(startIndx)))
    {
        return pOnlineGhostMap->Element(startIndx);
    }

    auto index = bReverse ? pOnlineGhostMap->PrevInorder(startIndx) : pOnlineGhostMap->NextInorder(startIndx);

    while (index != startIndx)
    {
        if (pOnlineGhostMap->IsValidIndex(index))
        {
            const auto pTest = pOnlineGhostMap->Element(index);
            if (TestGhost(pTest))
                return pTest;

            index = bReverse ? pOnlineGhostMap->PrevInorder(index) : pOnlineGhostMap->NextInorder(index);
        }
        else if (pReplay)
        {
            return pReplay;
        }
        else
        {
            index = bReverse ? pOnlineGhostMap->LastInorder() : pOnlineGhostMap->FirstInorder();
        }
    }
    return nullptr;
}

CBaseEntity *CMomentumPlayer::FindNextObserverTarget(bool bReverse)
{
    const auto pOnlineGhostMap = g_pMomentumGhostClient->GetOnlineGhostMap();
    CMomentumGhostBaseEntity *pCurrentReplayEnt = g_ReplaySystem.IsPlayingBack() ?
        g_ReplaySystem.GetPlaybackReplay()->GetRunEntity() : nullptr;

    // Start with the current observer target...
    if (m_hObserverTarget.Get())
    {
        // ... but only if it's a ghost
        const auto pGhostEnt = GetGhostEnt();
        if (pGhostEnt)
        {
            if (pGhostEnt->IsReplayGhost())
            {
                // If we're spectating the replay ghost, check our online map
                if (pOnlineGhostMap->Count() > 0)
                {
                    return IterateAndFindViableNextGhost(bReverse ? pOnlineGhostMap->LastInorder() : pOnlineGhostMap->FirstInorder(),
                                                         bReverse, nullptr, true);
                }
            }
            else if (pGhostEnt->IsOnlineGhost())
            {
                const auto pOnlineEnt = static_cast<CMomentumOnlineGhostEntity*>(pGhostEnt);
                if (pOnlineEnt)
                {
                    const auto indx = pOnlineGhostMap->Find(pOnlineEnt->GetGhostSteamID().ConvertToUint64());
                    return IterateAndFindViableNextGhost(indx, bReverse, pCurrentReplayEnt, false);
                }
            }
        }
    }

    // Default to current replay entity if not spectating a target
    if (pCurrentReplayEnt)
    {
        return pCurrentReplayEnt;
    }

    // Parse the list for an online ghost if we have any
    if (pOnlineGhostMap->Count() > 0)
    {
        return IterateAndFindViableNextGhost(bReverse ? pOnlineGhostMap->LastInorder() : pOnlineGhostMap->FirstInorder(),
                                             bReverse, nullptr, true);
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
        SaveCurrentRunState(false);
        FIRE_GAME_WIDE_EVENT("spec_start");

        g_pMomentumGhostClient->SetIsSpectating(true);
    }

    return BaseClass::StartObserverMode(mode);
}

void CMomentumPlayer::StopObserverMode()
{
    if (m_iObserverMode > OBS_MODE_NONE)
    {
        FIRE_GAME_WIDE_EVENT("spec_stop");

        g_pMomentumGhostClient->SetIsSpectating(false);
    }

    BaseClass::StopObserverMode();
}

void CMomentumPlayer::StopSpectating()
{
    if (m_iObserverMode == OBS_MODE_NONE)
        return;

    CMomentumGhostBaseEntity *pGhost = GetGhostEnt();
    if (pGhost)
        pGhost->RemoveSpectator();

    m_bWasSpectating = true;

    StopObserverMode();
    m_hObserverTarget.Set(nullptr);
    ForceRespawn();

    // Update the lobby/server if there is one
    m_sSpecTargetSteamID.Clear(); // reset steamID when we stop spectating
    g_pMomentumGhostClient->SetSpectatorTarget(m_sSpecTargetSteamID, false);
}

void CMomentumPlayer::TimerCommand_Restart(int track)
{
    if (!AllowUserTeleports())
        return;

    g_pMomentumTimer->Stop(this);
    g_pMomentumTimer->SetCanStart(false);

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
    {
        DestroyRockets();
    }

    const auto pStart = g_pMomentumTimer->GetStartTrigger(track);
    if (pStart)
    {
        const auto pStartMark = GetStartMark(track);
        if (pStartMark)
        {
            pStartMark->Teleport(this);
        }
        else
        {
            // Don't set angles if still in start zone.
            QAngle ang = pStart->GetLookAngles();
            Teleport(&pStart->WorldSpaceCenter(), (pStart->HasLookAngles() ? &ang : nullptr), &vec3_origin);
        }

        m_Data.m_iCurrentTrack = track;
        ResetRunStats();
    }
    else
    {
        const auto pStartPoint = EntSelectSpawnPoint();
        if (pStartPoint)
        {
            Teleport(&pStartPoint->GetAbsOrigin(), &pStartPoint->GetAbsAngles(), &vec3_origin);
            ResetRunStats();
        }
    }
}

void CMomentumPlayer::TimerCommand_Reset()
{
    if (AllowUserTeleports())
    {
        const auto pCurrentZone = GetCurrentZoneTrigger();
        if (pCurrentZone)
        {
            if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
            {
                DestroyRockets();
            }

            // MOM_TODO do a trace downwards from the top of the trigger's center to touchable land, teleport the player there
            Teleport(&pCurrentZone->WorldSpaceCenter(), nullptr, &vec3_origin);
        }
        else
        {
            Warning("Cannot reset, you have no current zone!\n");
        }
    }
}

void CMomentumPlayer::DestroyRockets()
{
    FOR_EACH_VEC(m_vecRockets, i)
    {
        const auto pRocket = m_vecRockets[i];
        if (pRocket)
        {
            pRocket->Destroy(true);
        }
    }

    m_vecRockets.RemoveAll();
}

void CMomentumPlayer::TogglePracticeMode()
{
    if (!m_bAllowUserTeleports || m_iObserverMode != OBS_MODE_NONE)
        return;

    if (m_bHasPracticeMode)
        DisablePracticeMode();
    else
        EnablePracticeMode();
}

void CMomentumPlayer::SetPracticeModeState()
{
    if (m_bHasPracticeMode)
    {
        SetParent(nullptr);
        SetMoveType(MOVETYPE_NOCLIP);
        if (!IsEFlagSet(EFL_NOCLIP_ACTIVE))
            ClientPrint(this, HUD_PRINTCONSOLE, "Practice mode ON!\n");
        AddEFlags(EFL_NOCLIP_ACTIVE);
    }
    else
    {
        SetMoveType(MOVETYPE_WALK);
        if (IsEFlagSet(EFL_NOCLIP_ACTIVE))
            ClientPrint(this, HUD_PRINTCONSOLE, "Practice mode OFF!\n");
        RemoveEFlags(EFL_NOCLIP_ACTIVE);
    }
}

void CMomentumPlayer::EnablePracticeMode()
{
    if (m_bHasPracticeMode)
        return;

    if (g_pMomentumTimer->IsRunning() && mom_practice_safeguard.GetBool())
    {
        const auto safeGuard = (m_nButtons & (IN_FORWARD|IN_MOVELEFT|IN_MOVERIGHT|IN_BACK|IN_JUMP|IN_DUCK|IN_WALK)) != 0;
        if (safeGuard)
        {
            Warning("You cannot enable practice mode while moving when the timer is running! Toggle this with "
                    "\"mom_practice_safeguard\"!\n");
            return;
        }
    }

    m_bHasPracticeMode = true;
    SetPracticeModeState();

    // This is outside the isRunning check because if you practice mode -> tele to start -> toggle -> start run,
    // the replay file doesn't have your "last" position, so we just save it regardless of timer state, but only restore
    // it if in a run.
    SaveCurrentRunState(true);

    g_pMomentumTimer->EnablePractice(this);

    IGameEvent *pEvent = gameeventmanager->CreateEvent("practice_mode");
    if (pEvent)
    {
        pEvent->SetBool("enabled", true);
        gameeventmanager->FireEvent(pEvent);
    }
}

void CMomentumPlayer::DisablePracticeMode()
{
    if (!m_bHasPracticeMode)
        return;

    m_bHasPracticeMode = false;
    SetPracticeModeState();

    // Only when timer is running
    if (g_pMomentumTimer->IsRunning())
    {
        if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
        {
            DestroyRockets();
        }

        RestoreRunState(true);
    }
    else if (m_Data.m_bIsInZone && m_Data.m_iCurrentZone == 1)
    {
        LimitSpeed(0.0f, false);
    }

    g_pMomentumTimer->DisablePractice(this);

    IGameEvent *pEvent = gameeventmanager->CreateEvent("practice_mode");
    if (pEvent)
    {
        pEvent->SetBool("enabled", false);
        gameeventmanager->FireEvent(pEvent);
    }
}

void CMomentumPlayer::SaveCurrentRunState(bool bFromPractice)
{
    SavedState_t *pState;
    if (bFromPractice)
    {
        pState = &m_SavedRunState;
    }
    else
    {
        if (m_bHasPracticeMode)
            pState = &m_PracticeModeState;
        else
            pState = &m_SavedRunState;
    }

    pState->m_nButtons = m_nButtons;
    pState->m_vecLastPos = GetAbsOrigin();
    pState->m_angLastAng = GetAbsAngles();
    pState->m_vecLastVelocity = GetAbsVelocity();
    pState->m_fLastViewOffset = GetViewOffset().z;
    pState->m_fNextPrimaryAttack = GetActiveWeapon() ? GetActiveWeapon()->m_flNextPrimaryAttack - gpGlobals->curtime : 0.0f;

    if (bFromPractice || !m_bHasPracticeMode)
    {
        Q_strncpy(pState->m_pszTargetName, GetEntityName().ToCStr(), sizeof(pState->m_pszTargetName));
        Q_strncpy(pState->m_pszClassName, GetClassname(), sizeof(pState->m_pszClassName));
        m_iOldTrack = m_Data.m_iCurrentTrack;
        m_iOldZone = m_Data.m_iCurrentZone;
        pState->m_nSavedAccelTicks = m_nAccelTicks;
        pState->m_nSavedPerfectSyncTicks = m_nPerfectSyncTicks;
        pState->m_nSavedStrafeTicks = m_nStrafeTicks;
    }
}

void CMomentumPlayer::RestoreRunState(bool bFromPractice)
{
    SavedState_t *pState;

    if (bFromPractice)
    {
        pState = &m_SavedRunState;
    }
    else
    {
        if (m_bHasPracticeMode)
            pState = &m_PracticeModeState;
        else
            pState = &m_SavedRunState;
    }

    m_nButtons = pState->m_nButtons;
    // Teleport calls SnapEyeAngles, don't worry
    Teleport(&pState->m_vecLastPos, &pState->m_angLastAng, &pState->m_vecLastVelocity);
    SetViewOffset(Vector(0, 0, pState->m_fLastViewOffset));
    SetLastEyeAngles(pState->m_angLastAng);
    if (GetActiveWeapon())
        GetActiveWeapon()->m_flNextPrimaryAttack = gpGlobals->curtime + pState->m_fNextPrimaryAttack;

    if (bFromPractice || !m_bHasPracticeMode)
    {
        SetName(MAKE_STRING(pState->m_pszTargetName));
        SetClassname(pState->m_pszClassName);
        m_Data.m_iCurrentTrack = m_iOldTrack;
        m_Data.m_iCurrentZone = m_iOldZone;
        m_nAccelTicks = pState->m_nSavedAccelTicks;
        m_nPerfectSyncTicks = pState->m_nSavedPerfectSyncTicks;
        m_nStrafeTicks = pState->m_nSavedStrafeTicks;
    }
}

void CMomentumPlayer::PostThink()
{
    // Update previous origins
    NewPreviousOrigin(GetLocalOrigin());
    BaseClass::PostThink();
}

int CMomentumPlayer::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
    CBaseEntity *pAttacker = info.GetAttacker();
    CBaseEntity *pInflictor = info.GetInflictor();

    // Handle taking self damage from rockets and pumpkin bombs
    if (pAttacker == GetLocalPlayer() &&
        (FClassnameIs(pInflictor, "momentum_rocket") || FClassnameIs(pInflictor, "momentum_generic_bomb")))
    {
        // Grab the vector of the incoming attack.
        // (Pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
        Vector vecDir = vec3_origin;
        if (pInflictor)
        {
            vecDir = info.GetInflictor()->WorldSpaceCenter() - Vector(0.0f, 0.0f, 10.0f) - WorldSpaceCenter();
            VectorNormalize(vecDir);
        }

        // Apply knockback
        ApplyPushFromDamage(info, vecDir);

        // Done
        return 1;
    }

    return BaseClass::OnTakeDamage_Alive(info);
}

// Apply TF2-like knockback when damaging self with rockets
// https://github.com/danielmm8888/TF2Classic/blob/master/src/game/server/tf/tf_player.cpp#L4108
void CMomentumPlayer::ApplyPushFromDamage(const CTakeDamageInfo &info, Vector &vecDir)
{
    if (info.GetDamageType() & DMG_PREVENT_PHYSICS_FORCE)
        return;

    CBaseEntity *pAttacker = info.GetAttacker();

    if (!info.GetInflictor() || GetMoveType() != MOVETYPE_WALK || pAttacker->IsSolidFlagSet(FSOLID_TRIGGER))
        return;

    // Apply different force scale when on ground
    float flScale = 1.0f;

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
    {
        if (GetFlags() & FL_ONGROUND)
        {
            flScale = MOM_DAMAGEFORCESCALE_SELF_ROCKET;
        }
        else
        {
            flScale = MOM_DAMAGEFORCESCALE_SELF_ROCKET_AIR;

            if (!(GetFlags() & FL_INWATER))
            {
                // Not in water
                flScale *= MOM_DAMAGESCALE_SELF_ROCKET;
            }
        }
    }

    // Scale force if we're ducked
    if (GetFlags() & FL_DUCKING)
    {
        // TF2 crouching collision box height used to be 55 units,
        // before it was changed to 62, the old height is still used
        // for calculating force from explosions.
        flScale *= 82.0f / 55.0f;
    }

    // Clamp force to 1000.0f
    float force = Min(info.GetDamage() * flScale, 1000.0f);
    Vector vecForce = -vecDir * force;
    ApplyAbsVelocityImpulse(vecForce);
}

bool CMomentumPlayer::CanPaint() { return m_flNextPaintTime <= gpGlobals->curtime; }

void CMomentumPlayer::DoPaint()
{
    if (!CanPaint())
        return;

    // Fire a paintgun bullet (doesn't actually equip/use the paintgun weapon)
    FX_FireBullets(entindex(), EyePosition(), EyeAngles(), AMMO_TYPE_PAINT, false,
                   GetPredictionRandomSeed() & 255, 0.0f);

    // Delay next time we paint
    m_flNextPaintTime = gpGlobals->curtime + CMomentumPaintGun::GetPrimaryCycleTime();
}