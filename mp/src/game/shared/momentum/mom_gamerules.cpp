#include "cbase.h"
#include "mom_gamerules.h"
#include "mathlib/mathlib.h"
#include "mom_shareddefs.h"
#include "voice_gamemgr.h"
#include "weapon/weapon_base_gun.h"
#include "filesystem.h"
#include "movevars_shared.h"
#include "mom_system_gamemode.h"
#include "run/mom_run_safeguards.h"

#ifdef CLIENT_DLL
#include "MessageboxPanel.h"
#include "gameui/BaseMenuPanel.h"
#else
#include "momentum/mapzones.h"
#include "momentum/mom_player.h"
#include "momentum/mom_system_tricks.h"
#endif

#include "tier0/memdbgon.h"

REGISTER_GAMERULES_CLASS(CMomentumGameRules);

CMomentumGameRules::CMomentumGameRules()
{
}

CMomentumGameRules::~CMomentumGameRules() {}

static CViewVectors g_ViewVectorsMom(Vector(0, 0, 64), // eye position
                                     Vector(-16, -16, 0), // hull min
                                     Vector(16, 16, 62),  // hull max

                                     Vector(-16, -16, 0), // duck hull min
                                     Vector(16, 16, 45),  // duck hull max
                                     Vector(0, 0, 47),    // duck view

                                     Vector(-10, -10, -10), // observer hull min
                                     Vector(10, 10, 10),    // observer hull max

                                     Vector(0, 0, 14) // dead view height
);

static CViewVectors g_ViewVectorsTF2(Vector(0, 0, 68),      // eye position
                                     Vector(-24, -24, 0),   // hull min
                                     Vector(24, 24, 82),    // hull max

                                     Vector(-24, -24, 0),   // duck hull min
                                     Vector(24, 24, 62),    // duck hull max
                                     Vector(0, 0, 45),      // duck view

                                     Vector(-10, -10, -10), // observer hull min
                                     Vector(10, 10, 10),    // observer hull max

                                     Vector(0, 0, 14)       // dead view height
);

static CViewVectors g_ViewVectorsAhop(Vector(0, 0, 64),      // eye position
                                      Vector(-16, -16, 0),   // hull min
                                      Vector(16, 16, 72),    // hull max

                                      Vector(-16, -16, 0),   // duck hull min
                                      Vector(16, 16, 36),    // duck hull max
                                      Vector(0, 0, 28),      // duck view

                                      Vector(-10, -10, -10), // observer hull min
                                      Vector(10, 10, 10),    // observer hull max

                                      Vector(0, 0, 14)       // dead view height
);

static CViewVectors g_ViewVectorsParkour(Vector(0, 0, 60),      // eye position
                                         Vector(-16, -16, 0),   // hull min
                                         Vector(16, 16, 72),    // hull max

                                         Vector(-16, -16, 0),   // duck hull min
                                         Vector(16, 16, 47),    // duck hull max
                                         Vector(0, 0, 38),      // duck view

                                         Vector(-10, -10, -10), // observer hull min
                                         Vector(10, 10, 10),    // observer hull max

                                         Vector(0, 0, 14)       // dead view height
);

static CViewVectors g_ViewVectorsConc(Vector(0, 0, 64),      // eye position
                                      Vector(-16, -16, 0),   // hull min
                                      Vector(16, 16, 72),    // hull max

                                      Vector(-16, -16, 0),   // duck hull min
                                      Vector(16, 16, 36),    // duck hull max
                                      Vector(0, 0, 24),      // duck view

                                      Vector(-10, -10, -10), // observer hull min
                                      Vector(10, 10, 10),    // observer hull max

                                      Vector(0, 0, 14)       // dead view height
);

const CViewVectors *CMomentumGameRules::GetViewVectors() const
{
    if (g_pGameModeSystem->IsTF2BasedMode())
        return &g_ViewVectorsTF2;

    if(g_pGameModeSystem->GameModeIs(GAMEMODE_CONC))
        return &g_ViewVectorsConc;

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_AHOP))
        return &g_ViewVectorsAhop;

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_PARKOUR))
        return &g_ViewVectorsParkour;

    return &g_ViewVectorsMom;
}

bool CMomentumGameRules::ShouldCollide(int collisionGroup0, int collisionGroup1)
{
    if (collisionGroup0 > collisionGroup1)
    {
        // swap so that lowest is always first
        V_swap(collisionGroup0, collisionGroup1);
    }

    if ((collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT))
    {
        // Don't stand on COLLISION_GROUP_WEAPONs
        if (collisionGroup1 == COLLISION_GROUP_WEAPON)
            return false;

        // We get pushed away from pushaways
        if (collisionGroup1 == COLLISION_GROUP_PUSHAWAY)
            return false;

        // Do not stand on projectiles
        if (collisionGroup1 == COLLISION_GROUP_PROJECTILE)
            return false;
    }

    if (collisionGroup0 == COLLISION_GROUP_DEBRIS && collisionGroup1 == COLLISION_GROUP_PUSHAWAY)
    {
        // let debris and multiplayer objects collide
        return true;
    }

    return BaseClass::ShouldCollide(collisionGroup0, collisionGroup1);
}

#ifdef CLIENT_DLL

bool CMomentumGameRules::PreventDisconnectAttempt()
{
    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_QUIT_TO_MENU))
    {
        g_pMessageBox->CreateConfirmationBox(g_pBasePanel->GetMainMenu(), "#MOM_MB_Safeguard_Map_Quit_ToMenu_Title", "#MOM_MB_Safeguard_Map_Quit_ToMenu_Msg", new KeyValues("ConfirmDisconnect"), nullptr, "#GameUI_Disconnect", "#GameUI_Cancel");

        return true;
    }

    return false;
}

#else
LINK_ENTITY_TO_CLASS(info_player_terrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_counterterrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_logo, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_teamspawn, CPointEntity);

static MAKE_TOGGLE_CONVAR(__map_change_ok, "0", FCVAR_HIDDEN | FCVAR_CLIENTCMD_CAN_EXECUTE, "");

bool CMomentumGameRules::IsManualMapChangeOkay(const char **pszReason)
{
    if (g_pRunSafeguards->IsSafeguarded(RUN_SAFEGUARD_MAP_CHANGE) && !__map_change_ok.GetBool())
    {
        CSingleUserRecipientFilter filter(UTIL_GetLocalPlayer());
        filter.MakeReliable();

        UserMessageBegin(filter, "MB_Safeguard_Map_Change");
        MessageEnd();

        *pszReason = "You currently safeguard against changing maps while the timer is running.";

        return false;
    }

    __map_change_ok.SetValue(0);

    return true;
}

Vector CMomentumGameRules::DropToGround(CBaseEntity *pMainEnt, const Vector &vPos, const Vector &vMins,
                                        const Vector &vMaxs)
{
    trace_t trace;
    UTIL_TraceHull(vPos, vPos + Vector(0, 0, -500), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace);
    return trace.endpos;
}

CBaseEntity *CMomentumGameRules::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
    // get valid spawn point
    if (pPlayer)
    {
        CBaseEntity *pSpawnSpot = pPlayer->EntSelectSpawnPoint();
        if (pSpawnSpot)
        {
            // drop down to ground
            Vector GroundPos = DropToGround(pPlayer, pSpawnSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX);

            // Move the player to the place it said.
            pPlayer->Teleport(&pSpawnSpot->GetAbsOrigin(), &pSpawnSpot->GetLocalAngles(), &vec3_origin);
            pPlayer->m_Local.m_vecPunchAngle = vec3_angle;
            return pSpawnSpot;
        }
    }
    return nullptr;
}

// checks if the spot is clear of players
bool CMomentumGameRules::IsSpawnPointValid(CBaseEntity *pSpot, CBasePlayer *pPlayer)
{
    if (!pSpot->IsTriggered(pPlayer))
    {
        return false;
    }

    Vector mins = GetViewVectors()->m_vHullMin;
    Vector maxs = GetViewVectors()->m_vHullMax;

    Vector vTestMins = pSpot->GetAbsOrigin() + mins;
    Vector vTestMaxs = pSpot->GetAbsOrigin() + maxs;

    // First test the starting origin.
    return UTIL_IsSpaceEmpty(pPlayer, vTestMins, vTestMaxs);
}

void CMomentumGameRules::ClientCommandKeyValues(edict_t *pEntity, KeyValues *pKeyValues)
{
    BaseClass::ClientCommandKeyValues(pEntity, pKeyValues);

    if (FStrEq(pKeyValues->GetName(), TRICK_DATA_KEY))
    {
        g_pTrickSystem->LoadTrickDataFromFile(pKeyValues);
    }
    else if (FStrEq(pKeyValues->GetName(), "NoZones"))
    {
        // Load if they're available in a file
        g_MapZoneSystem.LoadZonesFromFile();
    }
    else if (FStrEq(pKeyValues->GetName(), "ZonesFromSite"))
    {
        const auto pTrackPtr = pKeyValues->GetPtr("tracks");
        if (pTrackPtr)
        {
            KeyValuesAD pData(static_cast<KeyValues *>(pTrackPtr));
            g_MapZoneSystem.LoadZonesFromSite(pData, CBaseEntity::Instance(pEntity));
        }
    }
}

bool CMomentumGameRules::ClientCommand(CBaseEntity *pEdict, const CCommand &args)
{
    if (BaseClass::ClientCommand(pEdict, args))
        return true;

    CMomentumPlayer *pPlayer = static_cast<CMomentumPlayer *>(pEdict);

    return pPlayer->ClientCommand(args);
}

struct WhiteListedServerCmd
{
    const char *pName;
    ConVar *pVar;
};

static WhiteListedServerCmd const g_szWhitelistedServerCmds[] = {
    { "sv_gravity", &sv_gravity },
    { "sv_maxvelocity", &sv_maxvelocity },
    { "sv_airaccelerate", &sv_airaccelerate },
    { "sv_accelerate", &sv_accelerate },
    { "disconnect", nullptr }
};

void CMomentumGameRules::RunPointServerCommandWhitelisted(const char *pCmd)
{
    CUtlVector<char *> vec;
    V_SplitString(pCmd, ";", vec);
    FOR_EACH_VEC(vec, i)
    {
        bool bAllowed = false;
        for (const auto pWl : g_szWhitelistedServerCmds)
        {
            const auto strLen = V_strlen(pWl.pName);
            if (!V_strnicmp(vec[i], pWl.pName, strLen))
            {
                bAllowed = true;

                if (pWl.pVar)
                    pWl.pVar->SetValue(vec[i] + strLen + 1);
                else
                    engine->ServerCommand(vec[i]);

                break;
            }
        }

        if (!bAllowed)
            Warning("point_servercommand \"%s\" usage blocked by sv_allow_point_command setting\n", vec[i]);
    }

    vec.PurgeAndDeleteElements();
}

static char* const g_szWhitelistedClientCmds[] = {
    "r_screenoverlay",
    "play",
    "playgamesound"
};

void CMomentumGameRules::RunPointClientCommandWhitelisted(edict_t* pClient, const char* pCmd)
{
    CUtlVector<char*> vec;
    V_SplitString(pCmd, ";", vec);
    FOR_EACH_VEC(vec, i)
    {
        bool bAllowed = false;
        for (const auto pWl : g_szWhitelistedClientCmds)
        {
            const auto strLen = V_strlen(pWl);
            if (!V_strnicmp(vec[i], pWl, strLen))
            {
                bAllowed = true;

                engine->ClientCommand(pClient, "%s\n", vec[i]);

                break;
            }
        }

        if (!bAllowed)
            Warning("point_clientcommand \"%s\" usage blocked by sv_allow_point_command setting\n", vec[i]);
    }

    vec.PurgeAndDeleteElements();
}

static MAKE_TOGGLE_CONVAR(mom_bhop_playblocksound, "1", FCVAR_ARCHIVE, "Makes the door bhop blocks silent or not");

void CMomentumGameRules::PlayerSpawn(CBasePlayer *pPlayer)
{
    if (pPlayer)
    {
        if (gpGlobals->eLoadType == MapLoad_Background)
        {
            // Hide HUD on background maps
            pPlayer->m_Local.m_iHideHUD |= HIDEHUD_ALL;
        }
        else
        {
            // Turn them back on
            pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_ALL;
        }

       // Handle game_player_equip ents
        CBaseEntity *pWeaponEntity = nullptr;
        while ((pWeaponEntity = gEntList.FindEntityByClassname(pWeaponEntity, "game_player_equip")) != nullptr)
        {
            pWeaponEntity->Touch(pPlayer);
        }

        // MOM_TODO: could this change to gamemode != ALLOWED ?
    }
}

bool CMomentumGameRules::AllowDamage(CBaseEntity *pVictim, const CTakeDamageInfo &info)
{
    // Allow self damage from rockets, generic bombs and stickies
    if (pVictim == info.GetAttacker() && (FClassnameIs(info.GetInflictor(), "momentum_rocket") ||
                                          FClassnameIs(info.GetInflictor(), "momentum_generic_bomb") ||
                                          FClassnameIs(info.GetInflictor(), "momentum_stickybomb")) ||
                                          FClassnameIs(info.GetInflictor(), "momentum_concgrenade"))
        return true;

    return !pVictim->IsPlayer();
}

void CMomentumGameRules::RadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore)
{
    CBaseEntity *pEntity = nullptr;
    CBaseEntity *pAttacker = info.GetAttacker();

    float flFalloff = 0.5f;

    const auto initialRadiusSqr = flRadius * flRadius;
    // iterate on all entities in the vicinity.
    for (CEntitySphereQuery sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != nullptr; sphere.NextEntity())
    {
        if (pEntity == pEntityIgnore || pEntity->m_takedamage == DAMAGE_NO)
        {
            continue;
        }

        // UNDONE: this should check a damage mask, not an ignore
        if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
        {
            continue;
        }

        if (pEntity == pAttacker && g_pGameModeSystem->IsTF2BasedMode())
        {
            // Skip attacker, we will handle them separately (below)
            continue;
        }

        Vector nearestPoint;
        pEntity->CollisionProp()->CalcNearestPoint(vecSrc, &nearestPoint);
        if ((vecSrc - nearestPoint).LengthSqr() > initialRadiusSqr)
            continue;

        ApplyRadiusDamage(pEntity, info, vecSrc, flRadius, flFalloff);
    }

    if (pAttacker)
    {
        CBaseEntity *pInflictor = info.GetInflictor();

        if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
        {
            if (FClassnameIs(pInflictor, "momentum_rocket"))
            {
                flRadius = 121.0f; // Rocket self-damage radius is 121.0f
            }
        }

        if (g_pGameModeSystem->GameModeIs(GAMEMODE_CONC))
        {
            if(FClassnameIs(pInflictor, "momentum_concgrenade"))
            {
                flRadius = 280.0f;
            }
        }

        Vector nearestPoint;
        pAttacker->CollisionProp()->CalcNearestPoint(vecSrc, &nearestPoint);

        if ((vecSrc - nearestPoint).LengthSqr() <= (flRadius * flRadius))
        {
            ApplyRadiusDamage(pAttacker, info, vecSrc, flRadius, flFalloff);
        }
    }
}

void CMomentumGameRules::ApplyRadiusDamage(CBaseEntity *pEntity, const CTakeDamageInfo &info, const Vector &vecSrc, float flRadius, float falloff)
{
    const int MASK_RADIUS_DAMAGE = MASK_SHOT & (~CONTENTS_HITBOX);
    trace_t tr;

    // Check that the explosion can 'see' this entity, trace through players.
    Vector vecSpot = pEntity->BodyTarget(vecSrc, false);
    UTIL_TraceLine(vecSrc, vecSpot, MASK_RADIUS_DAMAGE, info.GetInflictor(), COLLISION_GROUP_PROJECTILE, &tr);
    
    if (tr.fraction != 1.0 && tr.m_pEnt != pEntity)
    {
        return;
    }

    float flDistanceToEntity;

    if (pEntity->IsPlayer())
    {
        // Use whichever is closer, absorigin or worldspacecenter
        float flToWorldSpaceCenter = (vecSrc - pEntity->WorldSpaceCenter()).Length();
        float flToOrigin = (vecSrc - pEntity->GetAbsOrigin()).Length();

        flDistanceToEntity = min(flToWorldSpaceCenter, flToOrigin);
    }
    else
    {
        flDistanceToEntity = (vecSrc - tr.endpos).Length();
    }

    // Adjust the damage - apply falloff.
    float flAdjustedDamage = RemapValClamped(flDistanceToEntity, 0.0f, flRadius, info.GetDamage(), info.GetDamage() * falloff);

    if (flAdjustedDamage <= 0)
        return;

    // the explosion can 'see' this entity, so hurt them!
    if (tr.startsolid)
    {
        // if we're stuck inside them, fixup the position and distance
        tr.endpos = vecSrc;
        tr.fraction = 0.0f;
    }

    CTakeDamageInfo adjustedInfo = info;    
    adjustedInfo.SetDamage(flAdjustedDamage);

    Vector dir = vecSpot - vecSrc;
    VectorNormalize(dir);

    if (adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin)
    {
        CalculateExplosiveDamageForce(&adjustedInfo, dir, vecSrc);
    }
    else
    {
        // Assume the force passed in is the maximum force. Decay it based on falloff.
        float flForce = adjustedInfo.GetDamageForce().Length() * falloff;
        adjustedInfo.SetDamageForce(dir * flForce);
        adjustedInfo.SetDamagePosition(vecSrc);
    }

    if (!CloseEnough(tr.fraction, 1.0f) && pEntity == tr.m_pEnt)
    {
        ClearMultiDamage();
        pEntity->DispatchTraceAttack(adjustedInfo, dir, &tr);
        ApplyMultiDamage();
    }
    else
    {
        pEntity->TakeDamage(adjustedInfo);
        if (pEntity->IsPlayer())
        {
            CMomentumPlayer *pPlayer = static_cast<CMomentumPlayer*>(pEntity);
            CSingleUserRecipientFilter user(pPlayer);
            user.MakeReliable();
            UserMessageBegin(user, "DamageIndicator");
            WRITE_BYTE((int)adjustedInfo.GetDamage());
            WRITE_VEC3COORD(vecSrc);
            MessageEnd();
        }
    }

    // Now hit all triggers along the way that respond to damage...
    pEntity->TraceAttackToTriggers(adjustedInfo, vecSrc, tr.endpos, dir);
}

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
  public:
    bool CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity) OVERRIDE { return true; }
};
CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

//-----------------------------------------------------------------------------
// Purpose: MULTIPLAYER BODY QUE HANDLING
//-----------------------------------------------------------------------------
class CCorpse : public CBaseAnimating
{
  public:
    DECLARE_CLASS(CCorpse, CBaseAnimating);
    DECLARE_SERVERCLASS();

    int ObjectCaps(void) OVERRIDE { return FCAP_DONT_SAVE; }

  public:
    CNetworkVar(int, m_nReferencePlayer);
};

IMPLEMENT_SERVERCLASS_ST(CCorpse, DT_Corpse)
SendPropInt(SENDINFO(m_nReferencePlayer), 10, SPROP_UNSIGNED) END_SEND_TABLE()

    LINK_ENTITY_TO_CLASS(bodyque, CCorpse);

CCorpse *g_pBodyQueueHead;

void InitBodyQue(void)
{
    CCorpse *pEntity = static_cast<CCorpse *>(CreateEntityByName("bodyque"));
    pEntity->AddEFlags(EFL_KEEP_ON_RECREATE_ENTITIES);
    g_pBodyQueueHead = pEntity;
    CCorpse *p = g_pBodyQueueHead;

    // Reserve 3 more slots for dead bodies
    for (int i = 0; i < 3; i++)
    {
        CCorpse *next = static_cast<CCorpse *>(CreateEntityByName("bodyque"));
        next->AddEFlags(EFL_KEEP_ON_RECREATE_ENTITIES);
        p->SetOwnerEntity(next);
        p = next;
    }

    p->SetOwnerEntity(g_pBodyQueueHead);
}

//-----------------------------------------------------------------------------
// Purpose: make a body queue entry for the given ent so the ent can be respawned elsewhere
// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
//-----------------------------------------------------------------------------
void CopyToBodyQue(CBaseAnimating *pCorpse)
{
    if (pCorpse->IsEffectActive(EF_NODRAW))
        return;

    CCorpse *pHead = g_pBodyQueueHead;

    pHead->CopyAnimationDataFrom(pCorpse);

    pHead->SetMoveType(MOVETYPE_FLYGRAVITY);
    pHead->SetAbsVelocity(pCorpse->GetAbsVelocity());
    pHead->ClearFlags();
    pHead->m_nReferencePlayer = ENTINDEX(pCorpse);

    pHead->SetLocalAngles(pCorpse->GetAbsAngles());
    UTIL_SetOrigin(pHead, pCorpse->GetAbsOrigin());

    UTIL_SetSize(pHead, pCorpse->WorldAlignMins(), pCorpse->WorldAlignMaxs());
    g_pBodyQueueHead = static_cast<CCorpse *>(pHead->GetOwnerEntity());
}

// Overridden for FOV changes
void CMomentumGameRules::ClientSettingsChanged(CBasePlayer *pPlayer)
{
    const char *pszName = engine->GetClientConVarValue(pPlayer->entindex(), "name");

    const char *pszOldName = pPlayer->GetPlayerName();

    // msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
    // Note, not using FStrEq so that this is case sensitive
    if (pszOldName[0] != 0 && Q_strcmp(pszOldName, pszName))
    {
        char text[256];
        Q_snprintf(text, sizeof(text), "%s changed name to %s\n", pszOldName, pszName);

        UTIL_ClientPrintAll(HUD_PRINTTALK, text);

        IGameEvent *event = gameeventmanager->CreateEvent("player_changename");
        if (event)
        {
            event->SetInt("userid", pPlayer->GetUserID());
            event->SetString("oldname", pszOldName);
            event->SetString("newname", pszName);
            gameeventmanager->FireEvent(event);
        }

        pPlayer->SetPlayerName(pszName);
    }

    const char *pszFov = engine->GetClientConVarValue(pPlayer->entindex(), "fov_desired");
    if (pszFov)
    {
        int iFov = atoi(pszFov);
        // iFov = clamp(iFov, 75, 90);
        pPlayer->SetDefaultFOV(iFov);
    }
}

void FovChanged(IConVar *pVar, const char *pOldValue, float flOldValue)
{
    ConVarRef var(pVar);
    const auto pMomPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pMomPlayer)
    {
        pMomPlayer->SetDefaultFOV(var.GetInt());
        pMomPlayer->SetFOV(pMomPlayer, var.GetInt());
    }
}

ConVar fov_desired("fov_desired", "90", FCVAR_ARCHIVE | FCVAR_USERINFO, "Sets the base field-of-view.\n", true, 1.0,
    true, 179.0, FovChanged);

int CMomentumGameRules::DefaultFOV() { return fov_desired.GetInt(); }
#endif