#include "cbase.h"
#include "mom_gamerules.h"
#include "mathlib/mathlib.h"
#include "mom_shareddefs.h"
#include "voice_gamemgr.h"
#include "weapon/weapon_base_gun.h"
#include "filesystem.h"
#include "movevars_shared.h"
#include "mom_system_gamemode.h"

#ifndef CLIENT_DLL
#include "momentum/mapzones.h"
#include "momentum/mom_player.h"
#endif

#include "tier0/memdbgon.h"

REGISTER_GAMERULES_CLASS(CMomentumGameRules);

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains) (0.002285 * (grains) / 16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains) lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION 1

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)                                                                               \
    ((ftpersec)*12 * BULLET_MASS_GRAINS_TO_KG(grains) * BULLET_IMPULSE_EXAGGERATION)

static CAmmoDef ammoDef;

CAmmoDef *GetAmmoDef()
{
    static bool bInitted = false;

    if (!bInitted)
    {
        bInitted = true;

        ammoDef.AddAmmoType(BULLET_PLAYER_50AE, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_50AE_max",
                            2400 * BULLET_IMPULSE_EXAGGERATION, 0, 10, 14);
        ammoDef.AddAmmoType(BULLET_PLAYER_762MM, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_762mm_max",
                            2400 * BULLET_IMPULSE_EXAGGERATION, 0, 10, 14);
        ammoDef.AddAmmoType(BULLET_PLAYER_556MM, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_556mm_max",
                            2400 * BULLET_IMPULSE_EXAGGERATION, 0, 10, 14);
        ammoDef.AddAmmoType(BULLET_PLAYER_556MM_BOX, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_556mm_box_max",
                            2400 * BULLET_IMPULSE_EXAGGERATION, 0, 10, 14);
        ammoDef.AddAmmoType(BULLET_PLAYER_338MAG, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_338mag_max",
                            2800 * BULLET_IMPULSE_EXAGGERATION, 0, 12, 16);
        ammoDef.AddAmmoType(BULLET_PLAYER_9MM, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_9mm_max",
                            2000 * BULLET_IMPULSE_EXAGGERATION, 0, 5, 10);
        ammoDef.AddAmmoType(BULLET_PLAYER_BUCKSHOT, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_buckshot_max",
                            600 * BULLET_IMPULSE_EXAGGERATION, 0, 3, 6);
        ammoDef.AddAmmoType(BULLET_PLAYER_45ACP, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_45acp_max",
                            2100 * BULLET_IMPULSE_EXAGGERATION, 0, 6, 10);
        ammoDef.AddAmmoType(BULLET_PLAYER_357SIG, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_357sig_max",
                            2000 * BULLET_IMPULSE_EXAGGERATION, 0, 4, 8);
        ammoDef.AddAmmoType(BULLET_PLAYER_57MM, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_57mm_max",
                            2000 * BULLET_IMPULSE_EXAGGERATION, 0, 4, 8);
        ammoDef.AddAmmoType(AMMO_TYPE_HEGRENADE, DMG_BLAST, TRACER_LINE, 0, 0, 1 /*max carry*/, 1, 0);
        ammoDef.AddAmmoType(AMMO_TYPE_FLASHBANG, 0, TRACER_LINE, 0, 0, 2 /*max carry*/, 1, 0);
        ammoDef.AddAmmoType(AMMO_TYPE_SMOKEGRENADE, 0, TRACER_LINE, 0, 0, 1 /*max carry*/, 1, 0);
        ammoDef.AddAmmoType(AMMO_TYPE_PAINT, DMG_BULLET, TRACER_LINE, 0, 0, "ammo_paint_max",
                            3000 * BULLET_IMPULSE_EXAGGERATION, 0);
        ammoDef.AddAmmoType(AMMO_TYPE_ROCKET, DMG_BLAST, TRACER_LINE, 0, 0, "ammo_rocket_max", 1, 0, 146, 146);
    }

    return &ammoDef;
}

ConVar ammo_50AE_max("ammo_50AE_max", "-2", FCVAR_REPLICATED);
ConVar ammo_762mm_max("ammo_762mm_max", "-2", FCVAR_REPLICATED);
ConVar ammo_556mm_max("ammo_556mm_max", "-2", FCVAR_REPLICATED);
ConVar ammo_556mm_box_max("ammo_556mm_box_max", "-2", FCVAR_REPLICATED);
ConVar ammo_338mag_max("ammo_338mag_max", "-2", FCVAR_REPLICATED);
ConVar ammo_9mm_max("ammo_9mm_max", "-2", FCVAR_REPLICATED);
ConVar ammo_buckshot_max("ammo_buckshot_max", "-2", FCVAR_REPLICATED);
ConVar ammo_45acp_max("ammo_45acp_max", "-2", FCVAR_REPLICATED);
ConVar ammo_357sig_max("ammo_357sig_max", "-2", FCVAR_REPLICATED);
ConVar ammo_57mm_max("ammo_57mm_max", "-2", FCVAR_REPLICATED);
ConVar ammo_hegrenade_max("ammo_hegrenade_max", "1", FCVAR_REPLICATED);
ConVar ammo_flashbang_max("ammo_flashbang_max", "2", FCVAR_REPLICATED);
ConVar ammo_smokegrenade_max("ammo_smokegrenade_max", "1", FCVAR_REPLICATED);
ConVar ammo_paint_max("ammo_paint_max", "-2", FCVAR_REPLICATED);
ConVar ammo_rocket_max("ammo_rocket_max", "-2", FCVAR_REPLICATED);

CMomentumGameRules::CMomentumGameRules()
{
}

CMomentumGameRules::~CMomentumGameRules() {}

static CViewVectors g_MOMViewVectors(Vector(0, 0, 64), // eye position
                                     Vector(-16, -16, 0), // hull min
                                     Vector(16, 16, 62),  // hull max

                                     Vector(-16, -16, 0), // duck hull min
                                     Vector(16, 16, 45),  // duck hull max
                                     Vector(0, 0, 47),    // duck view

                                     Vector(-10, -10, -10), // observer hull min
                                     Vector(10, 10, 10),    // observer hull max

                                     Vector(0, 0, 14) // dead view height
);

static CViewVectors g_MOMViewVectorsRJ(Vector(0, 0, 68), // eye position
                                     Vector(-24, -24, 0), // hull min
                                     Vector(24, 24, 82),  // hull max

                                     Vector(-24, -24, 0), // duck hull min
                                     Vector(24, 24, 62),  // duck hull max
                                     Vector(0, 0, 45),    // duck view

                                     Vector(-10, -10, -10), // observer hull min
                                     Vector(10, 10, 10),    // observer hull max

                                     Vector(0, 0, 14) // dead view height
);

const CViewVectors *CMomentumGameRules::GetViewVectors() const
{
    if (g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
        return &g_MOMViewVectorsRJ;

    return &g_MOMViewVectors;
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

#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS(info_player_terrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_counterterrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_logo, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_teamspawn, CPointEntity);

Vector CMomentumGameRules::DropToGround(CBaseEntity *pMainEnt, const Vector &vPos, const Vector &vMins,
                                        const Vector &vMaxs)
{
    trace_t trace;
    UTIL_TraceHull(vPos, vPos + Vector(0, 0, -500), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace);
    return trace.endpos;
}

CBaseEntity *CMomentumGameRules::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
    // gat valid spwan point
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

    if (FStrEq(pKeyValues->GetName(), "NoZones"))
    {
        // Load if they're available in a file
        g_MapZoneSystem.LoadZonesFromFile();
    }
    else if (FStrEq(pKeyValues->GetName(), "ZonesFromSite"))
    {
        if (pKeyValues->FindKey("tracks"))
        {
            KeyValuesAD pTracks(static_cast<KeyValues *>(pKeyValues->GetPtr("tracks")));
            // Zones loaded, pass them through
            g_MapZoneSystem.LoadZonesFromSite(pTracks, CBaseEntity::Instance(pEntity));
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

struct WhiteListedCmd
{
    const char *pName;
    ConVar *pVar;
};

static WhiteListedCmd const g_szWhitelistedCmds[] = {
    { "sv_gravity", &sv_gravity },
    { "sv_maxvelocity", &sv_maxvelocity },
    { "sv_airaccelerate", &sv_airaccelerate },
    { "sv_accelerate", &sv_accelerate },
    { "disconnect", nullptr }
};

void CMomentumGameRules::PointCommandWhitelisted(const char *pCmd)
{
    CUtlVector<char *> vec;
    V_SplitString(pCmd, ";", vec);
    FOR_EACH_VEC(vec, i)
    {
        for (const auto pWl : g_szWhitelistedCmds)
        {
            const auto strLen = V_strlen(pWl.pName);
            if (!V_strnicmp(vec[i], pWl.pName, strLen))
            {
                if (pWl.pVar)
                    pWl.pVar->SetValue(vec[i] + strLen + 1);
                else
                    engine->ServerCommand(vec[i]);
            }
        }
    }

    vec.PurgeAndDeleteElements();
}

static MAKE_TOGGLE_CONVAR(mom_bhop_playblocksound, "1", FCVAR_ARCHIVE, "Makes the door bhop blocks silent or not");

void CMomentumGameRules::PlayerSpawn(CBasePlayer *pPlayer)
{
    if (pPlayer)
    {
        ConVarRef map("host_map");
        const char *pMapName = map.GetString();

        if (gpGlobals->eLoadType == MapLoad_Background || !Q_strcmp(pMapName, "credits"))
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
    // Allow self damage from rockets and generic bombs
    if (pVictim == info.GetAttacker() && (FClassnameIs(info.GetInflictor(), "momentum_rocket") 
                                       || FClassnameIs(info.GetInflictor(), "momentum_generic_bomb")))
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

        if (pEntity == pAttacker && g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
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

    if (pAttacker && g_pGameModeSystem->GameModeIs(GAMEMODE_RJ))
    {
        if (FClassnameIs(pAttacker, "momentum_rocket"))
        {
            flRadius = 121.0f; // Rocket self-damage radius is 121.0f
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
// Purpose: make a body que entry for the given ent so the ent can be respawned elsewhere
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

// Overidden for FOV changes
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

    // NVNT see if this user is still or has began using a haptic device
    const char *pszHH = engine->GetClientConVarValue(pPlayer->entindex(), "hap_HasDevice");
    if (pszHH)
    {
        int iHH = atoi(pszHH);
        pPlayer->SetHaptics(iHH != 0);
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

// override so it we can control who is "spec" from hud_chat instead of the server ...
const char *CMomentumGameRules::GetChatPrefix(bool bTeamOnly, CBasePlayer *pPlayer)
{
    if (pPlayer && pPlayer->IsAlive() == false)
    {
        return "*SPEC*";
    }

    return "";
}
#endif
