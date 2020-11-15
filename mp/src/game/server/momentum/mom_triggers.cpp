#include "cbase.h"

#include "mom_triggers.h"
#include "in_buttons.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "mom_replay_entity.h"
#include "run/mom_run_entity.h"
#include "mom_system_gamemode.h"
#include "mom_system_progress.h"
#include "mom_stickybomb.h"
#include "fmtstr.h"
#include "mom_timer.h"
#include "mom_modulecomms.h"
#include "trigger_trace_enums.h"
#include "movevars_shared.h"
#include "mom_system_tricks.h"

#include "dt_utlvector_send.h"

#include "tier0/memdbgon.h"

static MAKE_TOGGLE_CONVAR(mom_triggers_overlay_bbox_enable, "0", FCVAR_DEVELOPMENTONLY,
                          "Toggles showing the bounding boxes for momentum triggers, needs map restart if changed!\n");

static MAKE_TOGGLE_CONVAR(mom_triggers_overlay_text_enable, "0", FCVAR_DEVELOPMENTONLY,
                          "Toggles showing the entity text for momentum triggers, needs map restart if changed!\n");

CMomentumTriggerSystem g_MomentumTriggerSystem;

void CMomentumTriggerSystem::FrameUpdatePostEntityThink()
{
    DoVariablePushes();
}

// ------------- Base Trigger ------------------------------------
BEGIN_DATADESC(CBaseMomentumTrigger)
    DEFINE_KEYFIELD(m_iTrackNumber, FIELD_INTEGER, "track_number")
END_DATADESC();

CBaseMomentumTrigger::CBaseMomentumTrigger()
{
    m_iTrackNumber = TRACK_ALL;
}

void CBaseMomentumTrigger::Spawn()
{
    BaseClass::Spawn();
    InitTrigger();

    m_debugOverlays |= ((OVERLAY_BBOX_BIT * mom_triggers_overlay_bbox_enable.GetBool()) |
                        (OVERLAY_TEXT_BIT * mom_triggers_overlay_text_enable.GetBool()));
}

bool CBaseMomentumTrigger::PassesTriggerFilters(CBaseEntity* pOther)
{
    CMomRunEntity *pEnt = dynamic_cast<CMomRunEntity*>(pOther);
    if (pEnt && pEnt->GetRunEntData())
    {
        if (m_iTrackNumber != TRACK_ALL && pEnt->GetRunEntData()->m_iCurrentTrack != m_iTrackNumber)
            return false;
    }

    // Otherwise we need to pass it through
    return BaseClass::PassesTriggerFilters(pOther);
}

int CBaseMomentumTrigger::DrawDebugTextOverlays()
{
    int text_offset = BaseClass::DrawDebugTextOverlays();

    char tempstr[255];
    Q_snprintf(tempstr, sizeof(tempstr), "Track: %d", m_iTrackNumber.Get());
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    return text_offset;
}

// -------------- FilterTrackNumber --------------------------
LINK_ENTITY_TO_CLASS(filter_momentum_track_number, CFilterTrackNumber);

BEGIN_DATADESC(CFilterTrackNumber)
    DEFINE_KEYFIELD(m_iTrackNumber, FIELD_INTEGER, "track_number"),
END_DATADESC();

CFilterTrackNumber::CFilterTrackNumber()
{
    m_iTrackNumber = -1;
}

bool CFilterTrackNumber::KeyValue(const char *szKeyName, const char *szValue)
{
    if (FStrEq(szKeyName, "track_number"))
    {
        m_iTrackNumber = clamp<int>(Q_atoi(szValue), -1, 255);
        return true;
    }
    return BaseClass::KeyValue(szKeyName, szValue);
}

bool CFilterTrackNumber::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
    CMomRunEntity *pEnt = dynamic_cast<CMomRunEntity*>(pEntity);
    return (m_iTrackNumber > -1 && pEnt && pEnt->GetRunEntData()->m_iCurrentTrack == m_iTrackNumber);
}

// -------------- FilterPlayerState --------------------------
LINK_ENTITY_TO_CLASS(filter_momentum_player_state, CFilterPlayerState);

BEGIN_DATADESC(CFilterPlayerState)
DEFINE_KEYFIELD(m_iPlayerState, FIELD_INTEGER, "player_state"),
END_DATADESC();

CFilterPlayerState::CFilterPlayerState()
{
    m_iPlayerState = -1;
}

bool CFilterPlayerState::PassesFilterImpl(CBaseEntity* pCaller, CBaseEntity* pEntity)
{
    if (!pEntity || !pEntity->IsPlayer())
        return false;

    const auto pPlayer = static_cast<CMomentumPlayer*>(pEntity);

    // 0 for any of these means they are the current interaction
    int floor = pPlayer->GetInteractionIndex(SurfInt::TYPE_FLOOR);      // Player is sliding on the floor
    int land = pPlayer->GetInteractionIndex(SurfInt::TYPE_LAND);        // Player landed on the floor such that they can jump off without speed loss (perfect bhop)
    int ground = pPlayer->GetInteractionIndex(SurfInt::TYPE_GROUNDED);  // Player has settled on the ground

    switch (m_iPlayerState)
    {
    case PLAYER_STATE_GROUND:
        return (floor == 0 && pPlayer->GetInteraction(0).trace.plane.normal.z >= 0.7) || land == 0 || ground == 0;

    case PLAYER_STATE_SURF:
        return floor == 0 && pPlayer->GetInteraction(0).trace.plane.normal.z < 0.7; // The floor is a surf ramp

    case PLAYER_STATE_BHOP:
        return land == 0 && pPlayer->m_nButtons & IN_JUMP;

    default:
        return false;
    }
}

// -------------- FilterCollectibles --------------------------
LINK_ENTITY_TO_CLASS(filter_momentum_collectibles, CFilterCollectibles);

BEGIN_DATADESC(CFilterCollectibles)
DEFINE_KEYFIELD(m_iCollectibleCount, FIELD_INTEGER, "player_collectibles"),
END_DATADESC();

CFilterCollectibles::CFilterCollectibles()
{
    m_iCollectibleCount = 0;
}

bool CFilterCollectibles::PassesFilterImpl(CBaseEntity* pCaller, CBaseEntity* pEntity)
{
    if (!pEntity || !pEntity->IsPlayer())
        return false;

    const auto pPlayer = static_cast<CMomentumPlayer*>(pEntity);

    return pPlayer->m_Collectibles.GetCollectibleCount() >= m_iCollectibleCount;
}

// -------------- FilterVelocity --------------------------
LINK_ENTITY_TO_CLASS(filter_momentum_velocity, CFilterVelocity);

BEGIN_DATADESC(CFilterVelocity)
    DEFINE_KEYFIELD(m_iMode, FIELD_INTEGER, "Mode"),
    DEFINE_KEYFIELD(m_bAbove, FIELD_BOOLEAN, "Above"),

    DEFINE_KEYFIELD(m_bVertical, FIELD_BOOLEAN, "EnableVertical"),
    DEFINE_KEYFIELD(m_bHorizontal, FIELD_BOOLEAN, "EnableHorizontal"),
    DEFINE_KEYFIELD(m_flVerticalVelocity, FIELD_FLOAT, "VerticalVelocity"),
    DEFINE_KEYFIELD(m_flHorizontalVelocity, FIELD_FLOAT, "HorizontalVelocity"),

    DEFINE_KEYFIELD(m_bIgnoreSign, FIELD_INTEGER, "IgnoreSign"),
    DEFINE_KEYFIELD(m_vecVelocity, FIELD_VECTOR, "VelocityVector"),
    DEFINE_KEYFIELD(m_vecVelocityAxes, FIELD_VECTOR, "VelocityAxes"),
END_DATADESC();

CFilterVelocity::CFilterVelocity()
{
    m_iMode = VELOCITYFILTER_TOTAL;
    m_bAbove = true;
    m_bVertical = false;
    m_bHorizontal = false;
    m_flVerticalVelocity = 500.0f;
    m_flHorizontalVelocity = 1000.0f;
    m_bIgnoreSign = false;
}

bool CFilterVelocity::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
    if (!pEntity->IsPlayer())
        return false;

    switch (m_iMode)
    {
    case VELOCITYFILTER_TOTAL:
        return CheckTotalVelocity(pEntity);

    case VELOCITYFILTER_PER_AXIS:
        return CheckPerAxisVelocity(pEntity);

    default:
        return false;
    }
}

bool CFilterVelocity::CheckTotalVelocity(CBaseEntity *pEntity)
{
    const auto vel = pEntity->GetAbsVelocity();

    if (!m_bHorizontal && !m_bVertical)
        DevWarning("filter_velocity: Both vertical and horizontal velocity are ignored!\n");

    const bool bHorizontal = m_bHorizontal ? CheckTotalVelocityInternal(vel.Length2D(), true) : true;
    const bool bVertical = m_bVertical ? CheckTotalVelocityInternal(fabsf(vel.z), false) : true;

    return bHorizontal && bVertical;
}

inline bool CFilterVelocity::CheckTotalVelocityInternal(const float flToCheck, bool bIsHorizontal)
{
    const auto speed = bIsHorizontal ? m_flHorizontalVelocity : m_flVerticalVelocity;
    return m_bAbove ? flToCheck > speed : flToCheck < speed;
}

bool CFilterVelocity::CheckPerAxisVelocity(CBaseEntity *pEntity)
{
    auto vel = pEntity->GetAbsVelocity();
    bool check;

    for (int i = 0; i < 3; i++)
    {
        if (m_bIgnoreSign)
            vel[i] = fabsf(vel[i]);

        if (m_vecVelocityAxes[i] == 0.0f)
            continue;

        check = m_bAbove ? vel[i] > m_vecVelocity[i] : vel[i] < m_vecVelocity[i];

        if (!check)
            return false;
    }

    return true;
}

// -------------- BaseMomZoneTrigger ------------------------------
IMPLEMENT_SERVERCLASS_ST(CBaseMomZoneTrigger, DT_BaseMomZoneTrigger)
SendPropInt(SENDINFO(m_iTrackNumber)),
SendPropUtlVector(SENDINFO_UTLVECTOR(m_vecZonePoints), 32, SendPropVector(NULL, 0, sizeof(Vector))),
SendPropFloat(SENDINFO(m_flZoneHeight)),
END_SEND_TABLE();

CBaseMomZoneTrigger::CBaseMomZoneTrigger()
{
    m_iTrackNumber = TRACK_MAIN; // Default zones to the main map.
    m_vecRestartPos = vec3_invalid;
}

void CBaseMomZoneTrigger::Spawn()
{
    Precache();
    AddSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS | SF_TRIGGER_ALLOW_GHOSTS);
    BaseClass::Spawn();
}

void CBaseMomZoneTrigger::Precache()
{
    BaseClass::Precache();
    PrecacheMaterial(MOM_ZONE_DRAW_MATERIAL);
    PrecacheMaterial(MOM_ZONE_DRAW_MATERIAL_OVERLAY);
}

void CBaseMomZoneTrigger::InitCustomCollision(CPhysCollide* pPhysCollide, const Vector& vecMins, const Vector& vecMaxs)
{
    // We are able to create a vphysics object just fine, but how physics work IN vphysics is no good for us.
    // We'll have to use the same method triggers use,
    // which is using partitions and waiting for engine->SolidMoved to call StartTouch/EndTouch for us
    // from the object (player in our case)
    // For that we need to insert ourselves to the partition system and
    // do a custom collision test.
    // The default collision test only works if the entity is a proper model or brush.
    // In our case, we're neither.
    objectparams_t params = { 0 };
    params.enableCollisions = true;
    params.pGameData = this;
    params.pName = "";
    params.mass = 1.0f;
    params.volume = 1.0f;

    auto pPhys = physenv->CreatePolyObject(pPhysCollide, 0, GetAbsOrigin(), GetAbsAngles(), &params);
    Assert(pPhys);

    pPhys->EnableMotion(false);
    pPhys->EnableGravity(false);
    pPhys->SetContents(MASK_SOLID);

    VPhysicsDestroyObject();
    VPhysicsSetObject(pPhys);


    if (CollisionProp()->GetPartitionHandle() == PARTITION_INVALID_HANDLE)
        CollisionProp()->CreatePartitionHandle();
    SetSolid(SOLID_VPHYSICS);

    // We need to set the collision bounds manually
    // The collision bound is used by the partition system.
    SetCollisionBounds(vecMins, vecMaxs);


    // If we ever need ray testing uncomment this.
    AddSolidFlags(/*FSOLID_CUSTOMRAYTEST |*/ FSOLID_CUSTOMBOXTEST);
}

bool CBaseMomZoneTrigger::TestCollision(const Ray_t& ray, unsigned mask, trace_t& tr)
{
    const auto pPhys = VPhysicsGetObject();
    Assert(pPhys);

    physcollision->TraceBox(ray, pPhys->GetCollide(), GetAbsOrigin(), GetAbsAngles(), &tr);

    return true;
}

bool CBaseMomZoneTrigger::ToKeyValues(KeyValues *pKvInto)
{
    pKvInto->SetInt("type", GetZoneType());
    return true;
}

bool CBaseMomZoneTrigger::LoadFromKeyValues(KeyValues *pKvFrom)
{
    if (pKvFrom->GetInt("type", ZONE_TYPE_INVALID) != GetZoneType())
        return false;

    return true;
}

int CBaseMomZoneTrigger::GetZoneType()
{
    return ZONE_TYPE_INVALID;
}

bool CBaseMomZoneTrigger::FindStandableGroundBelow(const Vector& traceStartPos, Vector& dropPos)
{
    // Trace for a suitable landing position
    Vector collisionEnd(traceStartPos + Vector(0, 0, -MAX_TRACE_LENGTH));
    int mask = MASK_PLAYERSOLID_BRUSHONLY;
    int group = COLLISION_GROUP_NONE;
    trace_t solidTr;
    UTIL_TraceHull(traceStartPos, collisionEnd, VEC_HULL_MIN, VEC_HULL_MAX, mask, nullptr, group, &solidTr);

    // Check if we would land in a teleport trigger
    Ray_t tpRay;
    tpRay.Init(traceStartPos, solidTr.endpos);
    CTeleportTriggerTraceEnum traceEnum(&tpRay);
    enginetrace->EnumerateEntities(tpRay, true, &traceEnum);

    // Check if one of the following happened:
    // We would land on a trigger_teleport
    // We didn't actually find any ground to stand on
    // We would land on a ramp you cannot stand on
    bool dropOnGround =
        traceEnum.GetTeleportEntity() == nullptr
        && solidTr.DidHit()
        && (!solidTr.allsolid && solidTr.plane.normal.z >= 0.7);
    dropPos = dropOnGround ? solidTr.endpos : traceStartPos;

    return dropOnGround;
}

const Vector& CBaseMomZoneTrigger::GetRestartPosition()
{
    if(m_vecRestartPos == vec3_invalid)
    {
        Vector zoneMaxsRel = CollisionProp()->OBBMaxs();
        Vector zoneMaxs;
        VectorTransform(zoneMaxsRel, CollisionProp()->CollisionToWorldTransform(), zoneMaxs);
        Vector zoneMinsRel = CollisionProp()->OBBMins();
        Vector zoneMins;
        VectorTransform(zoneMinsRel, CollisionProp()->CollisionToWorldTransform(), zoneMins);

        // Fallback restart position in the middle
        Vector zoneCenter(0.5f * (zoneMaxs + zoneMins));
        // Where we actually trace from to find the landing position
        Vector zoneCeilCenter = Vector(zoneCenter.x, zoneCenter.y, zoneMaxs.z);

        Vector dropPos;
        bool foundGround = FindStandableGroundBelow(zoneCeilCenter, dropPos);

        bool groundIsHighEnough = (dropPos.z >= zoneMins.z - 0.9f * (VEC_DUCK_HULL_MAX.z - VEC_DUCK_HULL_MIN.z));
        m_vecRestartPos = (foundGround && groundIsHighEnough) ? dropPos : zoneCenter;
    }
    return m_vecRestartPos;
}

int CBaseMomZoneTrigger::DrawDebugTextOverlays()
{
    int text_offset = BaseClass::DrawDebugTextOverlays();
    
    const char *szZoneType[ZONE_TYPE_COUNT] = {"start", "stop", "stage", "checkpoint"};

    char tempstr[255];
    Q_snprintf(tempstr, sizeof(tempstr), "Zone type: %s", szZoneType[GetZoneType()]);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    return text_offset;
}

// --------- CTriggerZone ----------------------------------------------
BEGIN_DATADESC(CTriggerZone)
    DEFINE_KEYFIELD(m_iZoneNumber, FIELD_INTEGER, "zone_number")
END_DATADESC();

CTriggerZone::CTriggerZone()
{
    m_iZoneNumber = 0; // 0 by default ("end trigger")
}

void CTriggerZone::OnStartTouch(CBaseEntity* pOther)
{
    CMomRunEntity *pEnt = dynamic_cast<CMomRunEntity*>(pOther);
    if (pEnt)
        pEnt->OnZoneEnter(this);

    BaseClass::OnStartTouch(pOther);
}

void CTriggerZone::OnEndTouch(CBaseEntity* pOther)
{
    CMomRunEntity *pEnt = dynamic_cast<CMomRunEntity*>(pOther);
    if (pEnt)
        pEnt->OnZoneExit(this);

    BaseClass::OnEndTouch(pOther);
}

bool CTriggerZone::ToKeyValues(KeyValues* pKvInto)
{
    pKvInto->SetInt("zoneNum", m_iZoneNumber);
    return BaseClass::ToKeyValues(pKvInto);
}

bool CTriggerZone::LoadFromKeyValues(KeyValues* kv)
{
    m_iZoneNumber = kv->GetInt("zoneNum", -1);
    if (m_iZoneNumber >= 0 && m_iZoneNumber < MAX_ZONES)
        return BaseClass::LoadFromKeyValues(kv);

    return false;
}

int CTriggerZone::DrawDebugTextOverlays()
{
    int text_offset = BaseClass::DrawDebugTextOverlays();

    char tempstr[255];
    Q_snprintf(tempstr, sizeof(tempstr), "Zone number: %d", m_iZoneNumber);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    return text_offset;
}

//---------- CTriggerCheckpoint -----------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_checkpoint, CTriggerCheckpoint);

IMPLEMENT_SERVERCLASS_ST(CTriggerCheckpoint, DT_TriggerCheckpoint)
END_SEND_TABLE()

int CTriggerCheckpoint::GetZoneType()
{
    return ZONE_TYPE_CHECKPOINT;
}

//---------- CTriggerStage -----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stage, CTriggerStage);

IMPLEMENT_SERVERCLASS_ST(CTriggerStage, DT_TriggerStage)
END_SEND_TABLE()

int CTriggerStage::GetZoneType()
{
    return ZONE_TYPE_STAGE;
}

//------------------------------------------------------------------------------------------

//---------- CTriggerTimerStart ------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, CTriggerTimerStart);

BEGIN_DATADESC(CTriggerTimerStart)
    DEFINE_KEYFIELD(m_fSpeedLimit, FIELD_FLOAT, "speed_limit"),
    DEFINE_KEYFIELD(m_angLook, FIELD_VECTOR, "look_angles"),
    DEFINE_KEYFIELD(m_bTimerStartOnJump, FIELD_BOOLEAN, "start_on_jump"),
    DEFINE_KEYFIELD(m_iLimitSpeedType, FIELD_INTEGER, "speed_limit_type")
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CTriggerTimerStart, DT_TriggerTimerStart)
END_SEND_TABLE()

CTriggerTimerStart::CTriggerTimerStart()
    : m_angLook(vec3_angle), m_fSpeedLimit(350.0f), m_bTimerStartOnJump(true),
      m_iLimitSpeedType(SPEED_NORMAL_LIMIT)
{
    m_iZoneNumber = 1;
}
bool CTriggerTimerStart::ToKeyValues(KeyValues *pKvInto)
{
    // Structured like this because properties are another DB table for the site
    // (not every trigger has properties)
    const auto pZoneProps = new KeyValues("zoneProps");
    const auto pActualProps = new KeyValues("properties");

    pActualProps->SetFloat("speed_limit", GetSpeedLimit());
    pActualProps->SetBool("limiting_speed", IsLimitingSpeed());
    pActualProps->SetBool("start_on_jump", StartOnJump());
    pActualProps->SetInt("speed_limit_type", GetLimitSpeedType());
    if (HasLookAngles())
    {
        pActualProps->SetFloat("yaw", m_angLook[YAW]);
    }

    pZoneProps->AddSubKey(pActualProps);
    pKvInto->AddSubKey(pZoneProps);

    return BaseClass::ToKeyValues(pKvInto);
};

bool CTriggerTimerStart::LoadFromKeyValues(KeyValues *zoneKV)
{
    if (BaseClass::LoadFromKeyValues(zoneKV))
    {
        const auto pZoneProps = zoneKV->FindKey("zoneProps");
        if (!pZoneProps)
            return false;

        const auto pActualProps = pZoneProps->FindKey("properties");
        if (!pActualProps)
            return false;

        SetSpeedLimit(pActualProps->GetFloat("speed_limit", 350.0f));
        SetIsLimitingSpeed(pActualProps->GetBool("limiting_speed", true));
        SetStartOnJump(pActualProps->GetBool("start_on_jump", true));
        SetLimitSpeedType(pActualProps->GetInt("speed_limit_type", SPEED_NORMAL_LIMIT));

        const float nolook = -190.0f;
        float yaw = pActualProps->GetFloat("yaw", nolook);
        if (!CloseEnough(yaw, nolook))
        {
            SetHasLookAngles(true);
            SetLookAngles(QAngle(0.0f, yaw, 0.0f));
        }
        else
        {
            SetHasLookAngles(false);
        }
        return true;
    }

    return false;
}

int CTriggerTimerStart::GetZoneType()
{
    return ZONE_TYPE_START;
}

void CTriggerTimerStart::Spawn()
{
    m_iZoneNumber = 1;
    // We don't want negative velocities (We're checking against an absolute value)
    m_fSpeedLimit = fabs(m_fSpeedLimit);
    m_angLook.z = 0.0f; // Reset roll since mappers will never stop ruining everything.
    BaseClass::Spawn();

    g_pMomentumTimer->SetStartTrigger(m_iTrackNumber, this);
}
void CTriggerTimerStart::SetSpeedLimit(const float fSpeed) { m_fSpeedLimit = fSpeed; }
void CTriggerTimerStart::SetLookAngles(const QAngle &newang) { m_angLook = newang; }
void CTriggerTimerStart::SetIsLimitingSpeed(const bool bIsLimitSpeed)
{
    if (bIsLimitSpeed)
    {
        if (!HasSpawnFlags(SF_LIMIT_LEAVE_SPEED))
        {
            AddSpawnFlags(SF_LIMIT_LEAVE_SPEED);
        }
    }
    else
    {
        if (HasSpawnFlags(SF_LIMIT_LEAVE_SPEED))
        {
            RemoveSpawnFlags(SF_LIMIT_LEAVE_SPEED);
        }
    }
}
void CTriggerTimerStart::SetHasLookAngles(const bool bHasLook)
{
    if (bHasLook)
    {
        if (!HasSpawnFlags(SF_USE_LOOKANGLES))
        {
            AddSpawnFlags(SF_USE_LOOKANGLES);
        }
    }
    else
    {
        if (HasSpawnFlags(SF_USE_LOOKANGLES))
        {
            RemoveSpawnFlags(SF_USE_LOOKANGLES);
        }
    }
}
//----------------------------------------------------------------------------------------------

//----------- CTriggerTimerStop ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, CTriggerTimerStop);

BEGIN_DATADESC(CTriggerTimerStop)
    DEFINE_KEYFIELD(m_bCancel, FIELD_BOOLEAN, "cancel")
END_DATADESC();

IMPLEMENT_SERVERCLASS_ST(CTriggerTimerStop, DT_TriggerTimerStop)
END_SEND_TABLE()

CTriggerTimerStop::CTriggerTimerStop()
{
    m_bCancel = false;
}

void CTriggerTimerStop::Spawn()
{
    m_iZoneNumber = 0; // Hard code our zone number to 0.

    BaseClass::Spawn();
}

int CTriggerTimerStop::GetZoneType()
{
    return ZONE_TYPE_STOP;
}

//----------------------------------------------------------------------------------------------

//----------- CTriggerTrickZone ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_trick, CTriggerTrickZone);

IMPLEMENT_SERVERCLASS_ST(CTriggerTrickZone, DT_TriggerTrickZone)
SendPropInt(SENDINFO(m_iID), -1, SPROP_UNSIGNED),
SendPropString(SENDINFO(m_szZoneName)),
SendPropInt(SENDINFO(m_iDrawState), 3, SPROP_UNSIGNED),
END_SEND_TABLE();

CTriggerTrickZone::CTriggerTrickZone()
{
    m_iTrackNumber = TRACK_ALL;
    m_iID = -1;
    m_szZoneName.GetForModify()[0] = '\0';
    m_iDrawState = TRICK_DRAW_NONE;
}

void CTriggerTrickZone::Spawn()
{
    BaseClass::Spawn();

    g_pTrickSystem->AddZone(this);
}

int CTriggerTrickZone::GetZoneType()
{
    return ZONE_TYPE_TRICK;
}

bool CTriggerTrickZone::LoadFromKeyValues(KeyValues* pKvFrom)
{
    m_iID = Q_atoi(pKvFrom->GetName());
    Q_strncpy(m_szZoneName.GetForModify(), pKvFrom->GetString("name", "NOT NAMED!!!!! ERROR!"), 32);

    return BaseClass::LoadFromKeyValues(pKvFrom);
}

bool CTriggerTrickZone::ToKeyValues(KeyValues* pKvInto)
{
    pKvInto->SetName(CFmtStr("%i", m_iID.Get()));
    pKvInto->SetString("name", m_szZoneName);

    return BaseClass::ToKeyValues(pKvInto);
}

void CTriggerTrickZone::OnStartTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer())
    {
        const auto pMomPlayer = ToCMOMPlayer(pOther);
        g_pTrickSystem->OnTrickZoneEnter(this, pMomPlayer);
    }
}

void CTriggerTrickZone::OnEndTouch(CBaseEntity *pOther)
{
    if (pOther->IsPlayer())
    {
        const auto pMomPlayer = ToCMOMPlayer(pOther);
        g_pTrickSystem->OnTrickZoneExit(this, pMomPlayer);
    }
}

//----------------------------------------------------------------------------------------------

bool MomTeleportEntity(CBaseEntity* pTeleportTo, 
                    CBaseEntity* pEntToTeleport, 
                    CBaseEntity* pLandmark = nullptr, 
                    int iMode = 0, 
                    Vector vecVelocityScaler = Vector(1.0f, 1.0f, 1.0f), 
                    bool bResetAngles = true, 
                    bool bReorientLandmark = false)
{
    if (!(pTeleportTo && pEntToTeleport))
        return false;

    pEntToTeleport->SetGroundEntity(nullptr);

    QAngle* pAngles = const_cast<QAngle*>(&pTeleportTo->GetAbsAngles());
    Vector* pOrigin = const_cast<Vector*>(&pTeleportTo->GetAbsOrigin());
    Vector* pVelocity = const_cast<Vector*>(&pEntToTeleport->GetAbsVelocity());

    VectorMultiply(*pVelocity, vecVelocityScaler, *pVelocity);

    switch (iMode)
    {
        // Redundant due to the velocity scaler but kept just in case anyone wants a straightforward way to reset velocity
        case TELEPORT_RESET:
        {
            pVelocity->Init();
            break;
        }
        case TELEPORT_KEEP_NEGATIVE_Z:
        {
            pVelocity->x = pVelocity->y = 0;

            if (pVelocity->z > 0.0f)
                pVelocity->z = 0;

            break;
        }
        case TELEPORT_SNAP_TO_DESTINATION:
        {
            matrix3x4_t matMyModelToWorld;
            VMatrix matMyInverse, matRemoteTransform;
            QAngle angVelocityAngle;

            // Build a transformation from the teleportee's velocity vector
            VectorAngles(pEntToTeleport->GetAbsVelocity(), angVelocityAngle);
            AngleMatrix(angVelocityAngle, matMyModelToWorld);
            MatrixInverseGeneral(matMyModelToWorld, matMyInverse);
            matRemoteTransform = pTeleportTo->EntityToWorldTransform();

            // Reorient the velocity
            *pVelocity = matMyInverse.ApplyRotation(*pVelocity);
            *pVelocity = matRemoteTransform.ApplyRotation(*pVelocity);
            break;
        }
        case TELEPORT_LANDMARK:
        {
            if (!pLandmark)
            {
                DevWarning("TeleportEntity: Invalid landmark entity while in landmark mode!\n");
                return false;
            }

            Vector vecNewOrigin;

            // Old landmark behavior is just use the origin offset from the landmark without any transformations
            if (!bReorientLandmark)
            {
                vecNewOrigin = pTeleportTo->GetAbsOrigin() + (pEntToTeleport->GetAbsOrigin() - pLandmark->GetAbsOrigin());

                pEntToTeleport->Teleport(&vecNewOrigin, nullptr, nullptr);

                return true;
            }
            
            Vector vecNewVelocity;
            QAngle angNewAngles;

            matrix3x4_t pTransformMatrix;
            matrix3x4_t pLocalLandmarkMatrix;
            matrix3x4_t pRemoteLandmarkMatrix = pTeleportTo->EntityToWorldTransform();

            MatrixInvert(pLandmark->EntityToWorldTransform(), pLocalLandmarkMatrix);
            ConcatTransforms(pRemoteLandmarkMatrix, pLocalLandmarkMatrix, pTransformMatrix);

            angNewAngles = TransformAnglesToWorldSpace(pEntToTeleport->GetAbsAngles(), pTransformMatrix);
            VectorTransform(pEntToTeleport->GetAbsOrigin(), pTransformMatrix, vecNewOrigin);
            VectorRotate(pEntToTeleport->GetAbsVelocity(), pTransformMatrix, vecNewVelocity);

            pEntToTeleport->Teleport(&vecNewOrigin, &angNewAngles, &vecNewVelocity);

            return true;
        }
        default:
        {
            // vphysics objects get stopped if pVelocity is null
            if (pEntToTeleport->GetMoveType() != MOVETYPE_VPHYSICS)
                pVelocity = nullptr;
        }
    };

    if (!bResetAngles && iMode != TELEPORT_SNAP_TO_DESTINATION)
        pAngles = nullptr;

    pEntToTeleport->Teleport(pOrigin, pAngles, pVelocity);

    return true;
}

//----------- CTriggerTeleport -----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerMomentumTeleport);

BEGIN_DATADESC(CTriggerMomentumTeleport)
    DEFINE_KEYFIELD(m_iMode, FIELD_INTEGER, "mode"),
    DEFINE_KEYFIELD(m_vecVelocityScaler, FIELD_VECTOR, "velocityscale"),
    DEFINE_KEYFIELD(m_bResetAngles, FIELD_BOOLEAN, "resetang"),
    DEFINE_KEYFIELD(m_bFail, FIELD_BOOLEAN, "fail"),
    DEFINE_KEYFIELD(m_Landmark, FIELD_STRING, "landmark"),
    DEFINE_KEYFIELD(m_bReorientLandmark, FIELD_BOOLEAN, "reorient_landmark"),
END_DATADESC()

CTriggerMomentumTeleport::CTriggerMomentumTeleport()
{
    m_iMode = TELEPORT_DEFAULT;
    m_vecVelocityScaler.Init(1.0f, 1.0f, 1.0f);
    m_bResetAngles = true;
    m_bReorientLandmark = false;
    m_bFail = false;
}

void CTriggerMomentumTeleport::Touch(CBaseEntity* pOther)
{
    if (!PassesTriggerFilters(pOther))
        return;

    if (pOther && !HasSpawnFlags(SF_TELE_ONEXIT))
    {
        HandleTeleport(pOther);
    }
}

void CTriggerMomentumTeleport::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);

    if (pOther && HasSpawnFlags(SF_TELE_ONEXIT))
    {
        HandleTeleport(pOther);
    }
}

void CTriggerMomentumTeleport::HandleTeleport(CBaseEntity *pOther)
{
    if (!pOther)
        return;

    if (!m_hDestinationEnt.Get())
    {
        if (m_target != NULL_STRING)
        {
            m_hDestinationEnt = gEntList.FindEntityByName(nullptr, m_target, nullptr, pOther, pOther);
        }
        else
        {
            DevWarning("trigger_teleport: invalid destination entity!\n");
            return;
        }
    }

    CBaseEntity *pLandmark = nullptr;

    if (m_Landmark != NULL_STRING)
    {
        pLandmark = gEntList.FindEntityByName(nullptr, m_Landmark, nullptr, pOther, pOther);

        // Legacy trigger_teleports assume landmark mode if the landmark entity is valid,
        // and someone who specifies a landmark almost certainly intends to make it a landmark teleport
        if (pLandmark)
        {
            m_iMode = TELEPORT_LANDMARK;
        }
    }

    // Deprecated spawnflag (SF_TELEPORT_PRESERVE_ANGLES = 1 << 5) from the old trigger_teleport
    const bool bResetAngles = HasSpawnFlags(1 << 5) ? false : m_bResetAngles;

    MomTeleportEntity(m_hDestinationEnt.Get(), pOther, pLandmark, m_iMode, m_vecVelocityScaler, bResetAngles, m_bReorientLandmark);

    if (m_bFail)
        OnFailTeleport(pOther);
}

void CTriggerMomentumTeleport::OnFailTeleport(CBaseEntity *pEntTeleported)
{
    const auto pPlayer = ToCMOMPlayer(pEntTeleported);
    if (!pPlayer)
        return;

    pPlayer->m_nButtonsToggled = 0;
}

int CTriggerMomentumTeleport::DrawDebugTextOverlays(void) 
{
    int text_offset = BaseClass::DrawDebugTextOverlays();
    static const char *szTeleportMode[TELEPORT_COUNT] = { "default", "reset", "keep downwards speed", "redirect", "landmark teleport" };

    char tempstr[255];

    if (m_target != NULL_STRING) 
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Destination: %s", m_target.ToCStr());
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    if (m_iMode >= 0 && m_iMode < TELEPORT_COUNT)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Teleport mode: %s", szTeleportMode[m_iMode]);
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    Q_snprintf(tempstr, sizeof(tempstr), "Velocity scale: %.2f %.2f %.2f", m_vecVelocityScaler.x, m_vecVelocityScaler.y, m_vecVelocityScaler.z);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    if (m_Landmark != NULL_STRING) 
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Landmark: %s", m_Landmark.ToCStr());
        EntityText(text_offset, tempstr, 0);
        text_offset++;

        Q_snprintf(tempstr, sizeof(tempstr), "Reorient landmark: %s", m_bReorientLandmark ? "yes" : "no");
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    if (m_iMode < TELEPORT_SNAP_TO_DESTINATION)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Reset angles: %s", m_bResetAngles ? "yes" : "no");
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    Q_snprintf(tempstr, sizeof(tempstr), "Fail teleport: %s", m_bFail ? "yes" : "no");
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    return text_offset;
}

//----------------------------------------------------------------------------------------------

//---------- CEnvSurfaceTeleport ---------------------------------------------------------------
LINK_ENTITY_TO_CLASS(env_momentum_surface_teleport, CEnvSurfaceTeleport);

void CEnvSurfaceTeleport::PlayerSurfaceChanged(CBasePlayer *pPlayer, char gameMaterial)
{
    if (m_bDisabled)
        return;

    // Do our thing only if it involves the target material
    if ( gameMaterial != m_iCurrentGameMaterial &&
         ( gameMaterial == m_iTargetGameMaterial || m_iCurrentGameMaterial == m_iTargetGameMaterial ) )
    {
        DevMsg(2, "Player changed material to %d (was %d)\n", gameMaterial, m_iCurrentGameMaterial);

        m_iCurrentGameMaterial = gameMaterial;

        // Teleporting the player while still processing movement isn't nice
        SetThink(&CEnvPlayerSurfaceTrigger::UpdateMaterialThink);
        SetNextThink(gpGlobals->curtime);
    }
}

void CEnvSurfaceTeleport::UpdateMaterialThink()
{
    if (m_iCurrentGameMaterial != m_iTargetGameMaterial)
        return;

    CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

    if (!m_hDestinationEnt.Get())
    {
        if (m_target != NULL_STRING)
        {
            m_hDestinationEnt = gEntList.FindEntityByName(nullptr, m_target, nullptr, pPlayer, pPlayer);
        }
        else
        {
            DevWarning("CEnvSurfaceTeleport cannot teleport, pDestinationEnt and m_target are null!\n");
            return;
        }
    }

    MomTeleportEntity(m_hDestinationEnt.Get(), pPlayer, nullptr, TELEPORT_DEFAULT, Vector(0.0f, 0.0f, 0.0f));
}
//----------------------------------------------------------------------------------------------

//---------- CTriggerProgress ----------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_progress, CTriggerProgress);

BEGIN_DATADESC(CTriggerProgress)
    DEFINE_KEYFIELD(m_iProgressNumber, FIELD_INTEGER, "progress_number"),
    DEFINE_OUTPUT(m_ResetOnehops, "OnResetOnehops")
END_DATADESC()

void CTriggerProgress::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        m_ResetOnehops.FireOutput(pPlayer, this);
        pPlayer->SetCurrentProgressTrigger(this);
    }
}

//----------------------------------------------------------------------------------------------

//------------- CFilterProgress --------------------------------------------------------------
LINK_ENTITY_TO_CLASS(filter_momentum_progress, CFilterProgress);

BEGIN_DATADESC(CFilterProgress)
    DEFINE_KEYFIELD(m_iProgressNum, FIELD_INTEGER, "progress_check")
END_DATADESC()

bool CFilterProgress::PassesFilterImpl(CBaseEntity *pCaller, CBaseEntity *pEntity)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pEntity);
    return (pPlayer && pPlayer->m_iProgressNumber >= m_iProgressNum);
}
//----------------------------------------------------------------------------------------------

//----------- CTriggerTeleportCheckpoint -------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_teleport_progress, CTriggerTeleportProgress);

void CTriggerTeleportProgress::Touch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
        SetDestinationEnt(pPlayer->GetCurrentProgressTrigger());
    BaseClass::Touch(pOther);
}

//-----------------------------------------------------------------------------------------------

//---------- CTriggerMultihop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_multihop, CTriggerMultihop);

BEGIN_DATADESC(CTriggerMultihop)
    DEFINE_KEYFIELD(m_fMaxHoldSeconds, FIELD_FLOAT, "hold")
END_DATADESC()

CTriggerMultihop::CTriggerMultihop() : m_fMaxHoldSeconds(0.5f)
{
    SetDefLessFunc(m_mapOnStartTouchedTimes);
}

void CTriggerMultihop::OnStartTouch(CBaseEntity *pOther)
{
    CBaseMomentumTrigger::OnStartTouch(pOther);

    if (pOther->IsPlayer())
    {
        m_mapOnStartTouchedTimes.InsertOrReplace(pOther->entindex(), gpGlobals->curtime);
    }
}

void CTriggerMultihop::OnEndTouch(CBaseEntity *pOther)
{
    CBaseMomentumTrigger::OnEndTouch(pOther);

    m_mapOnStartTouchedTimes.Remove(pOther->entindex());
}

void CTriggerMultihop::Touch(CBaseEntity *pOther)
{
    if (PassesTriggerFilters(pOther) && pOther->IsPlayer())
    {
        const auto pPlayer = static_cast<CMomentumPlayer*>(pOther);
        if (pPlayer && m_mapOnStartTouchedTimes.IsValidIndex(m_mapOnStartTouchedTimes.Find(pPlayer->entindex())))
        {
            const auto fEnterTime = m_mapOnStartTouchedTimes[m_mapOnStartTouchedTimes.Find(pPlayer->entindex())];
            if (gpGlobals->curtime - fEnterTime >= m_fMaxHoldSeconds)
            {
                SetDestinationEnt(pPlayer->GetCurrentProgressTrigger());
                HandleTeleport(pPlayer);
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------

//------------ CTriggerOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_onehop, CTriggerOnehop);

BEGIN_DATADESC(CTriggerOnehop)
    DEFINE_OUTPUT(m_hopNoLongerJumpable, "OnHopNoLongerJumpable")
END_DATADESC()

CTriggerOnehop::CTriggerOnehop() : m_bhopNoLongerJumpableFired(false)
{
}

void CTriggerOnehop::OnStartTouch(CBaseEntity *pOther)
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        if (pPlayer->FindOnehopOnList(this))
        {
            SetDestinationEnt(pPlayer->GetCurrentProgressTrigger());
            HandleTeleport(pPlayer);
        }
        else
        {
            pPlayer->AddOnehop(this);

            if (!m_bhopNoLongerJumpableFired)
            {
                m_hopNoLongerJumpable.FireOutput(pPlayer, this);
                m_bhopNoLongerJumpableFired = true;
            }

            BaseClass::OnStartTouch(pPlayer);
        }
    }
}

//-----------------------------------------------------------------------------------------------

//------- CTriggerResetOnehop -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_resetonehop, CTriggerResetOnehop);

BEGIN_DATADESC(CTriggerResetOnehop)
    DEFINE_OUTPUT(m_ResetOnehops, "OnResetOnehops")
END_DATADESC()

void CTriggerResetOnehop::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
    CMomentumPlayer *pPlayer = ToCMOMPlayer(pOther);
    if (pPlayer)
    {
        m_ResetOnehops.FireOutput(pPlayer, this);
        pPlayer->RemoveAllOnehops();
    }
}

//-----------------------------------------------------------------------------------------------

//--------- CTriggerUserInput -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_userinput, CTriggerUserInput);

BEGIN_DATADESC(CTriggerUserInput)
    DEFINE_KEYFIELD(m_eKey, FIELD_INTEGER, "lookedkey"),
    DEFINE_OUTPUT(m_OnKeyPressed, "OnKeyPressed"),
    DEFINE_OUTPUT(m_OnKeyHeld, "OnKeyHeld"),
    DEFINE_OUTPUT(m_OnKeyReleased, "OnKeyReleased"),
END_DATADESC();

CTriggerUserInput::CTriggerUserInput()
{
    m_eKey = KEY_FORWARD;
    m_ButtonRep = IN_FORWARD;
}

void CTriggerUserInput::Spawn()
{
    switch (m_eKey)
    {
    case KEY_FORWARD:
        m_ButtonRep = IN_FORWARD;
        break;
    case KEY_BACK:
        m_ButtonRep = IN_BACK;
        break;
    case KEY_MOVELEFT:
        m_ButtonRep = IN_MOVELEFT;
        break;
    case KEY_MOVERIGHT:
        m_ButtonRep = IN_MOVERIGHT;
        break;
    case KEY_JUMP:
        m_ButtonRep = IN_JUMP;
        break;
    case KEY_DUCK:
        m_ButtonRep = IN_DUCK;
        break;
    case KEY_ATTACK:
        m_ButtonRep = IN_ATTACK;
        break;
    case KEY_ATTACK2:
        m_ButtonRep = IN_ATTACK2;
        break;
    case KEY_RELOAD:
        m_ButtonRep = IN_RELOAD;
        break;
    default:
        DevWarning("Passed unhandled key press");
        m_ButtonRep = 0;
        break;
    }

    BaseClass::Spawn();
}

void CTriggerUserInput::Touch(CBaseEntity *pOther)
{
    if (PassesTriggerFilters(pOther))
    {
        const auto pPlayer = static_cast<CBasePlayer*>(pOther);
        if (pPlayer)
        {
            if (pPlayer->m_afButtonPressed & m_ButtonRep)
                m_OnKeyPressed.FireOutput(pPlayer, this);

            if (pPlayer->m_nButtons & m_ButtonRep)
                m_OnKeyHeld.FireOutput(pPlayer, this);

            if (pPlayer->m_afButtonReleased & m_ButtonRep)
                m_OnKeyReleased.FireOutput(pPlayer, this);
        }
    }
}

//-----------------------------------------------------------------------------------------------

//--------- CTriggerLimitMovement -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_limitmovement, CTriggerLimitMovement);

void CTriggerLimitMovement::Touch(CBaseEntity *pOther)
{
    if (!PassesTriggerFilters(pOther))
        return;

    CMomRunEntity *pEnt = dynamic_cast<CMomRunEntity*>(pOther);
    if (pEnt)
        ToggleButtons(pEnt, false);

    BaseClass::Touch(pOther);
}

void CTriggerLimitMovement::OnEndTouch(CBaseEntity *pOther)
{
    CMomRunEntity *pEnt = dynamic_cast<CMomRunEntity*>(pOther);
    if (pEnt)
        ToggleButtons(pEnt, true);

    BaseClass::OnEndTouch(pOther);
}

void CTriggerLimitMovement::ToggleButtons(CMomRunEntity* pEnt, bool bEnable)
{
    if (m_spawnflags & SF_LIMIT_FORWARD)
        pEnt->SetButtonsEnabled(IN_FORWARD, bEnable);
    if (m_spawnflags & SF_LIMIT_LEFT)
        pEnt->SetButtonsEnabled(IN_MOVELEFT, bEnable);
    if (m_spawnflags & SF_LIMIT_RIGHT)
        pEnt->SetButtonsEnabled(IN_MOVERIGHT, bEnable);
    if (m_spawnflags & SF_LIMIT_BACK)
        pEnt->SetButtonsEnabled(IN_BACK, bEnable);
    if (m_spawnflags & SF_LIMIT_JUMP)
        pEnt->SetButtonsEnabled(IN_JUMP, bEnable);
    if (m_spawnflags & SF_LIMIT_CROUCH)
        pEnt->SetButtonsEnabled(IN_DUCK, bEnable);
    if (m_spawnflags & SF_LIMIT_WALK)
        pEnt->SetButtonsEnabled(IN_WALK, bEnable);
    if (m_spawnflags & SF_LIMIT_SPRINT)
        pEnt->SetButtonsEnabled(IN_SPEED, bEnable);
    if (m_spawnflags & SF_LIMIT_BHOP)
        pEnt->SetBhopEnabled(bEnable);
}

//-----------------------------------------------------------------------------------------------

CUtlVector<VariablePush> g_VariablePushes;

void InitVariablePush(CBaseEntity *pOther, Vector vecForce, float flDuration, float flBias, bool bIncreasing)
{
    VariablePush push;

    push.m_pEntity = pOther;
    push.m_vecPushForce = vecForce;
    push.m_iNumTicks = Ceil2Int(flDuration / gpGlobals->interval_per_tick);
    push.m_iElapsedTicks = 0;
    push.m_flDuration = flDuration;
    push.m_flBias = flBias;
    push.m_bIncreasing = bIncreasing;

    g_VariablePushes.AddToTail(push);
}

void DoVariablePushes()
{
    FOR_EACH_VEC(g_VariablePushes, i)
    {
        VariablePush *pPush = &g_VariablePushes[i];

        if (pPush->m_iElapsedTicks == 0)
        {
            pPush->m_flStartTime = (float)Plat_FloatTime();
        }

        if (pPush->m_iElapsedTicks == pPush->m_iNumTicks)
        {
            DevLog("Variable push: %d ticks in %.4f seconds, average %.4f seconds per tick\n", 
                pPush->m_iElapsedTicks, Plat_FloatTime() - pPush->m_flStartTime, (Plat_FloatTime() - pPush->m_flStartTime) / pPush->m_iElapsedTicks);
        
            g_VariablePushes.Remove(i);
            continue;
        }

        // We start on tick 0 hence the -1 here
        float flFactor = pPush->m_iElapsedTicks / (float)(pPush->m_iNumTicks - 1);

        if (!pPush->m_bIncreasing)
        {
            flFactor = 1.0f - flFactor;
        }

        Vector vecForce = pPush->m_vecPushForce * Bias(flFactor, pPush->m_flBias);

        pPush->m_pEntity->SetAbsVelocity(vecForce + pPush->m_pEntity->GetAbsVelocity());
        pPush->m_iElapsedTicks++;
    }
}

void PushEntity(CBaseEntity *pEntity, Vector vecPush, int iMode, float flVariableDuration = 1.0f, float flVariableBias = 0.5f, bool bVariableIncreasing = false)
{
    switch (iMode)
    {
    case PUSH_ADD:
        vecPush += pEntity->GetAbsVelocity();
        break;
    case PUSH_SET_IF_LOWER:
        if (vecPush.LengthSqr() < pEntity->GetAbsVelocity().LengthSqr())
            vecPush = pEntity->GetAbsVelocity();
        break;
    case PUSH_ADD_IF_LOWER:
        if (vecPush.LengthSqr() < pEntity->GetAbsVelocity().LengthSqr())
            vecPush += pEntity->GetAbsVelocity();
        break;
    case PUSH_BASEVELOCITY:
        pEntity->SetBaseVelocity(vecPush);
        return;
    case PUSH_VARIABLE:
        InitVariablePush(pEntity, vecPush, flVariableDuration, flVariableBias, bVariableIncreasing);
        return;
    default:
        DevWarning("PushEntity: invalid mode %d, defaulting to set velocity\n", iMode);
        break;
    }

    pEntity->SetAbsVelocity(vecPush);
}

//---------- CFuncShootBoost --------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(func_shootboost, CFuncShootBoost);

BEGIN_DATADESC(CFuncShootBoost)
    DEFINE_KEYFIELD(m_vPushDir, FIELD_VECTOR, "pushdir"),
    DEFINE_KEYFIELD(m_flPushForce, FIELD_FLOAT, "force"),
    DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase"),
    DEFINE_KEYFIELD(m_flVariablePushDuration, FIELD_FLOAT, "varpushduration"),
    DEFINE_KEYFIELD(m_flVariablePushBias, FIELD_FLOAT, "varpushbias"),
    DEFINE_KEYFIELD(m_bVariablePushIncreasing, FIELD_BOOLEAN, "varpushincrease")
END_DATADESC()

CFuncShootBoost::CFuncShootBoost(): m_flPushForce(300.0f), m_iIncrease(3), m_flVariablePushDuration(1.0f), m_flVariablePushBias(0.5), m_bVariablePushIncreasing(false)
{
    m_vPushDir.Init();
}

void CFuncShootBoost::Spawn()
{
    BaseClass::Spawn();

    // Convert pushdir from angles to a vector (copied straight from CTriggerPush::Spawn)
    Vector vecAbsDir;
    QAngle angPushDir = QAngle(m_vPushDir.x, m_vPushDir.y, m_vPushDir.z);
    AngleVectors(angPushDir, &vecAbsDir);

    // Transform the vector into entity space
    VectorIRotate(vecAbsDir, EntityToWorldTransform(), m_vPushDir);

    // We don't need health here
    SetMaxHealth(0);

    // Enable "damage" so OnTakeDamage gets called
    m_takedamage = DAMAGE_YES;

    m_debugOverlays |= ((OVERLAY_BBOX_BIT * mom_triggers_overlay_bbox_enable.GetBool()) |
                        (OVERLAY_TEXT_BIT * mom_triggers_overlay_text_enable.GetBool()));

    if (m_target != NULL_STRING)
        m_hEntityCheck = gEntList.FindEntityByName(nullptr, m_target);
}

int CFuncShootBoost::OnTakeDamage(const CTakeDamageInfo &info)
{
    const auto pInflictor = info.GetAttacker();
    if (pInflictor)
    {
        if (m_hEntityCheck.Get())
        {
            const auto pTrigger = dynamic_cast<CBaseTrigger*>(m_hEntityCheck.Get());
            if (pTrigger && !pTrigger->IsTouching(pInflictor))
                return info.GetDamage();
        }

        // Transform the vector back to world space
        Vector vecAbsDir;
        VectorRotate(m_vPushDir, EntityToWorldTransform(), vecAbsDir);

        Vector finalVel = HasSpawnFlags(SF_PUSH_DIRECTION_AS_FINAL_FORCE) ? vecAbsDir : vecAbsDir.Normalized() * m_flPushForce;

        PushEntity(pInflictor, finalVel, m_iIncrease, m_flVariablePushDuration, m_flVariablePushBias, m_bVariablePushIncreasing);
    }
    // As we don't want to break it, we don't call BaseClass::OnTakeDamage(info);
    // OnTakeDamage returns the damage dealt
    return info.GetDamage();
}

int CFuncShootBoost::DrawDebugTextOverlays(void) 
{
    int text_offset = BaseClass::DrawDebugTextOverlays();

    char tempstr[255];
    static const char *szBoostType[PUSH_COUNT] = { "set", "add", "set if lower", "add if lower", "basevelocity", "variable push" };

    if (m_iIncrease >= 0 && m_iIncrease < PUSH_COUNT)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Push type: %s", szBoostType[m_iIncrease]);
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }
    
    Q_snprintf(tempstr, sizeof(tempstr), "Force: %.2f", m_flPushForce);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    Vector vecFinalVel;
    VectorRotate(m_vPushDir, EntityToWorldTransform(), vecFinalVel);

    if (!HasSpawnFlags(SF_PUSH_DIRECTION_AS_FINAL_FORCE))
        vecFinalVel = vecFinalVel.Normalized() * m_flPushForce;

    if (m_iIncrease == PUSH_VARIABLE)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Variable push duration: %.2f", m_flVariablePushDuration);
        EntityText(text_offset, tempstr, 0);
        text_offset++;

        const int iNumTicks = Ceil2Int(m_flVariablePushDuration / gpGlobals->interval_per_tick);

        float flFactor;
        Vector vecTotalForce(0.0f, 0.0f, 0.0f);

        // Calculate the total velocity given over the duration
        // This is pretty dumb but it's the most accurate method, and we're only doing this when printing debug text anyway
        for (int i = 0; i < iNumTicks; i++)
        {
            flFactor = i / (float)(iNumTicks - 1);

            if (!m_bVariablePushIncreasing)
            {
                flFactor = 1.0f - flFactor;
            }

            vecTotalForce += vecFinalVel * Bias(1.0f - flFactor, m_flVariablePushBias);
        }

        vecFinalVel = vecTotalForce;

        Q_snprintf(tempstr, sizeof(tempstr), "Variable push total force: %.2f", vecFinalVel.Length());
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    Q_snprintf(tempstr, sizeof(tempstr), "Push force vector: %.2f %.2f %.2f", vecFinalVel.x, vecFinalVel.y, vecFinalVel.z);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    if (m_target != NULL_STRING) 
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Trigger: %s", m_target.ToCStr());
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    return text_offset;
}
//-----------------------------------------------------------------------------------------------

//---------- CTriggerMomentumPush ---------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_push, CTriggerMomentumPush);

BEGIN_DATADESC(CTriggerMomentumPush)
    DEFINE_KEYFIELD(m_vPushDir, FIELD_VECTOR, "pushdir"),
    DEFINE_KEYFIELD(m_flPushForce, FIELD_FLOAT, "force"),
    DEFINE_KEYFIELD(m_iIncrease, FIELD_INTEGER, "increase"),
    DEFINE_KEYFIELD(m_flVariablePushDuration, FIELD_FLOAT, "varpushduration"),
    DEFINE_KEYFIELD(m_flVariablePushBias, FIELD_FLOAT, "varpushbias"),
    DEFINE_KEYFIELD(m_bVariablePushIncreasing, FIELD_BOOLEAN, "varpushincrease")
END_DATADESC()

CTriggerMomentumPush::CTriggerMomentumPush(): m_flPushForce(300.0f), m_iIncrease(3), m_flVariablePushDuration(1.0f), m_flVariablePushBias(0.5), m_bVariablePushIncreasing(false)
{
    m_vPushDir.Init();
}

void CTriggerMomentumPush::Spawn()
{
    BaseClass::Spawn();

    // Convert pushdir from angles to a vector (copied straight from CTriggerPush::Spawn)
    Vector vecAbsDir;
    QAngle angPushDir = QAngle(m_vPushDir.x, m_vPushDir.y, m_vPushDir.z);
    AngleVectors(angPushDir, &vecAbsDir);

    // Transform the vector into entity space
    VectorIRotate(vecAbsDir, EntityToWorldTransform(), m_vPushDir);
}

void CTriggerMomentumPush::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);
    if (pOther && HasSpawnFlags(SF_PUSH_ONSTART) && pOther->IsPlayer())
        OnSuccessfulTouch(pOther);
}

void CTriggerMomentumPush::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);
    if (pOther && HasSpawnFlags(SF_PUSH_ONEND) && pOther->IsPlayer())
        OnSuccessfulTouch(pOther);
}

void CTriggerMomentumPush::OnSuccessfulTouch(CBaseEntity *pOther)
{
    if (pOther)
    {
        // Transform the vector back to world space
        Vector vecAbsDir;
        VectorRotate(m_vPushDir, EntityToWorldTransform(), vecAbsDir);

        Vector finalVel = HasSpawnFlags(SF_PUSH_DIRECTION_AS_FINAL_FORCE) ? vecAbsDir : vecAbsDir.Normalized() * m_flPushForce;

        PushEntity(pOther, finalVel, m_iIncrease, m_flVariablePushDuration, m_flVariablePushBias, m_bVariablePushIncreasing);
    }
} 

int CTriggerMomentumPush::DrawDebugTextOverlays(void) 
{
    int text_offset = BaseClass::DrawDebugTextOverlays();

    char tempstr[255];
    static const char *szBoostType[PUSH_COUNT] = { "set", "add", "set if lower", "add if lower", "basevelocity", "variable push" };

    if (m_iIncrease >= 0 && m_iIncrease < PUSH_COUNT)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Push type: %s", szBoostType[m_iIncrease]);
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }
    
    Q_snprintf(tempstr, sizeof(tempstr), "Force: %.2f", m_flPushForce);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    Vector vecFinalVel;
    VectorRotate(m_vPushDir, EntityToWorldTransform(), vecFinalVel);

    if (!HasSpawnFlags(SF_PUSH_DIRECTION_AS_FINAL_FORCE))
        vecFinalVel = vecFinalVel.Normalized() * m_flPushForce;

    if (m_iIncrease == PUSH_VARIABLE)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Variable push duration: %.2f", m_flVariablePushDuration);
        EntityText(text_offset, tempstr, 0);
        text_offset++;

        const int iNumTicks = Ceil2Int(m_flVariablePushDuration / gpGlobals->interval_per_tick);

        float flFactor;
        Vector vecTotalForce(0.0f, 0.0f, 0.0f);

        // Calculate the total velocity given over the duration
        // This is pretty dumb but it's the most accurate method, and we're only doing this when printing debug text anyway
        for (int i = 0; i < iNumTicks; i++)
        {
            flFactor = i / (float)(iNumTicks - 1);

            if (!m_bVariablePushIncreasing)
            {
                flFactor = 1 - flFactor;
            }

            vecTotalForce += vecFinalVel * Bias(1 - flFactor, m_flVariablePushBias);
        }

        vecFinalVel = vecTotalForce;

        Q_snprintf(tempstr, sizeof(tempstr), "Variable push total force: %.2f", vecFinalVel.Length());
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    Q_snprintf(tempstr, sizeof(tempstr), "Push force vector: %.2f %.2f %.2f", vecFinalVel.x, vecFinalVel.y, vecFinalVel.z);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    return text_offset;
}
//-----------------------------------------------------------------------------------------------

//--------- CTriggerSlide -------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, CTriggerSlide);

BEGIN_DATADESC(CTriggerSlide)
    DEFINE_KEYFIELD(m_bStuckOnGround, FIELD_BOOLEAN, "StuckOnGround"),
    DEFINE_KEYFIELD(m_bAllowingJump, FIELD_BOOLEAN, "AllowingJump"),
    DEFINE_KEYFIELD(m_bDisableGravity, FIELD_BOOLEAN, "DisableGravity"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CTriggerSlide, DT_TriggerSlide)
SendPropBool(SENDINFO(m_bStuckOnGround)),
SendPropBool(SENDINFO(m_bAllowingJump)),
SendPropBool(SENDINFO(m_bDisableGravity)),
END_SEND_TABLE();

void CTriggerSlide::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    if (pOther->IsPlayer())
    {
        const auto pPlayer = ToCMOMPlayer(pOther);
        if (pPlayer)
        {
            pPlayer->m_vecSlideTriggers.AddToHead(this);
            pPlayer->m_CurrentSlideTrigger = this;
        }
    }
}

void CTriggerSlide::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);

    if (pOther->IsPlayer())
    {
        const auto pPlayer = ToCMOMPlayer(pOther);
        if (pPlayer)
        {
            pPlayer->m_vecSlideTriggers.FindAndRemove(this);

            if (pPlayer->m_CurrentSlideTrigger.Get() == this)
            {
                if (!pPlayer->m_vecSlideTriggers.IsEmpty())
                    pPlayer->m_CurrentSlideTrigger = pPlayer->m_vecSlideTriggers[0];
                else
                    pPlayer->m_CurrentSlideTrigger = nullptr;
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(trigger_momentum_reversespeed, CTriggerReverseSpeed);

BEGIN_DATADESC(CTriggerReverseSpeed)
    DEFINE_KEYFIELD(m_bReverseHorizontalSpeed, FIELD_BOOLEAN, "ReverseHorizontal"),
    DEFINE_KEYFIELD(m_bReverseVerticalSpeed, FIELD_BOOLEAN, "ReverseVertical"),
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"),
    DEFINE_KEYFIELD(m_bOnThink, FIELD_BOOLEAN, "OnThink")
END_DATADESC()

CTriggerReverseSpeed::CTriggerReverseSpeed()
{
    m_bReverseHorizontalSpeed = true;
    m_bReverseVerticalSpeed = true;
    m_flInterval = 1.0f;
    m_bOnThink = false;
}

void CTriggerReverseSpeed::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    if (pOther->IsPlayer())
    {
        // Reverse x/y velocity.
        if (m_bReverseHorizontalSpeed)
        {
            ReverseSpeed(pOther ,true);
        }

        // Reverse z velocity.
        if (m_bReverseVerticalSpeed)
        {
            ReverseSpeed(pOther, false);
        }

        vecCalculatedVel = pOther->GetAbsVelocity();

        if (m_bOnThink)
            SetNextThink(gpGlobals->curtime + m_flInterval);
    }
}

void CTriggerReverseSpeed::Think()
{
    if (m_bOnThink)
    {
        FOR_EACH_VEC(m_hTouchingEntities, i)
        {
            const auto pEnt = m_hTouchingEntities[i].Get();
            if (pEnt && pEnt->IsPlayer())
            {
                // Shall we will use the already calculated vel here, if we recalculate we could be stuck into a trigger
                // since it will take the new velocity already Reversed? If the interval is high enough it shouldn't matter.
                // pPlayer->SetAbsVelocity(vecCalculatedVel);

                // Reverse x/y velocity.
                if (m_bReverseHorizontalSpeed)
                {
                    ReverseSpeed(pEnt, true);
                }

                // Reverse z velocity.
                if (m_bReverseVerticalSpeed)
                {
                    ReverseSpeed(pEnt, false);
                }

                SetNextThink(gpGlobals->curtime + m_flInterval);
            }
        }
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }
}

void CTriggerReverseSpeed::ReverseSpeed(CBaseEntity *pEntity, bool bIsHorizontal)
{
    if (!pEntity)
        return;

    if (bIsHorizontal)
    {
        Vector vecVelocity = pEntity->GetAbsVelocity();
        const auto zVelBackup = vecVelocity.z;
        vecVelocity.z = 0.0f;

        const auto flSpeedAmount = vecVelocity.Length2D();

        // We need to compute its direction now to reverse the speed properly.
        QAngle qDirVelocity;
        VectorNormalizeFast(vecVelocity);
        VectorAngles(vecVelocity, qDirVelocity);

        // Revert the direction
        qDirVelocity.y = AngleNormalize(qDirVelocity.y - 180.0f);

        // Apply the speed.
        Vector vecNewVelocity;
        AngleVectors(qDirVelocity, &vecNewVelocity);
        vecNewVelocity.x *= flSpeedAmount;
        vecNewVelocity.y *= flSpeedAmount;
        vecNewVelocity.z = zVelBackup;

        pEntity->SetAbsVelocity(vecNewVelocity);
    }
    else
    {
        Vector vecVelocity = pEntity->GetAbsVelocity();
        vecVelocity.z = -vecVelocity.z;
        pEntity->SetAbsVelocity(vecVelocity);
    }
}

//-----------------------------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(trigger_momentum_setspeed, CTriggerSetSpeed);

BEGIN_DATADESC(CTriggerSetSpeed)
    DEFINE_KEYFIELD(m_bKeepHorizontalSpeed, FIELD_BOOLEAN, "KeepHorizontalSpeed"),
    DEFINE_KEYFIELD(m_bKeepVerticalSpeed, FIELD_BOOLEAN, "KeepVerticalSpeed"),
    DEFINE_KEYFIELD(m_flHorizontalSpeedAmount, FIELD_FLOAT, "HorizontalSpeedAmount"),
    DEFINE_KEYFIELD(m_flVerticalSpeedAmount, FIELD_FLOAT, "VerticalSpeedAmount"),
    DEFINE_KEYFIELD(m_angWishDirection, FIELD_VECTOR, "Direction"),
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"),
    DEFINE_KEYFIELD(m_bOnThink, FIELD_BOOLEAN, "OnThink"),
    DEFINE_KEYFIELD(m_bEveryTick, FIELD_BOOLEAN, "EveryTick"),
END_DATADESC()

CTriggerSetSpeed::CTriggerSetSpeed()
{
    SetDefLessFunc(m_mapCalculatedVelocities);
    m_bKeepHorizontalSpeed = false;
    m_bKeepVerticalSpeed = false;
    m_flHorizontalSpeedAmount = 500.0f;
    m_flVerticalSpeedAmount = 100.0f;
    m_bOnThink = false;
    m_flInterval = 1.0f;
    m_bEveryTick = false;
}

void CTriggerSetSpeed::OnStartTouch(CBaseEntity *pOther)
{
    BaseClass::OnStartTouch(pOther);

    if (pOther->IsPlayer())
    {
        CalculateSpeed(pOther);

        if (m_bOnThink)
            SetNextThink(gpGlobals->curtime + m_flInterval);
    }
}

void CTriggerSetSpeed::OnEndTouch(CBaseEntity *pOther)
{
    BaseClass::OnEndTouch(pOther);

    if (pOther->IsPlayer())
    {
        m_mapCalculatedVelocities.Remove(pOther->entindex());
    }
}

void CTriggerSetSpeed::Think()
{
    if (m_bOnThink)
    {
        FOR_EACH_VEC(m_hTouchingEntities, i)
        {
            const auto pEnt = m_hTouchingEntities[i].Get();
            if (pEnt && pEnt->IsPlayer())
            {
                const auto calcIndx = m_mapCalculatedVelocities.Find(pEnt->entindex());
                if (m_mapCalculatedVelocities.IsValidIndex(calcIndx))
                {
                    pEnt->SetAbsVelocity(m_mapCalculatedVelocities[calcIndx]);
                    SetNextThink(gpGlobals->curtime + m_flInterval);
                }
            }
        }
    }
    else
    {
        SetNextThink(TICK_NEVER_THINK);
    }
}

void CTriggerSetSpeed::Touch(CBaseEntity *pOther)
{
    BaseClass::Touch(pOther);

    if (m_bEveryTick)
    {
        if (!PassesTriggerFilters(pOther))
            return;

        if (pOther->IsPlayer())
        {
            CalculateSpeed(pOther);
            const auto calcIndx = m_mapCalculatedVelocities.Find(pOther->entindex());
            if (m_mapCalculatedVelocities.IsValidIndex(calcIndx))
            {
                pOther->SetAbsVelocity(m_mapCalculatedVelocities[calcIndx]);
            }
        }
    }
}

void CTriggerSetSpeed::CalculateSpeed(CBaseEntity *pOther)
{
    // As far I know, you can get the same direction by just playing with x/y ,
    // for getting the same direction as the z angle, except if there is a gimbal lock on the given angle.
    // I didn't look much about it, but it's pretty interesting. Gotta investigate.

    // Compute velocity direction only from y angle. We ignore these because if the mapper set -90 and 180,
    // the results on x/y axis velocity direction will be close to 0 and result that the horizontal speed 
    // amount won't be set correctly.
    // Since vertical speed can be set manually anyway, we can ignore and zero the x and z axis on the angle.
    m_angWishDirection.x = m_angWishDirection.z = 0.0f;

    Vector vecNewVelocity;
    AngleVectors(m_angWishDirection, &vecNewVelocity);

    Vector vecNewFinalVelocity = pOther->GetAbsVelocity();

    // Apply the speed.
    vecNewVelocity.x *= m_flHorizontalSpeedAmount;
    vecNewVelocity.y *= m_flHorizontalSpeedAmount;
    vecNewVelocity.z = m_flVerticalSpeedAmount;

    if (!m_bKeepVerticalSpeed)
        vecNewFinalVelocity.z = vecNewVelocity.z;

    if (!m_bKeepHorizontalSpeed)
    {
        vecNewFinalVelocity.x = vecNewVelocity.x;
        vecNewFinalVelocity.y = vecNewVelocity.y;
    }

    pOther->SetAbsVelocity(vecNewFinalVelocity);
    m_mapCalculatedVelocities.InsertOrReplace(pOther->entindex(), vecNewFinalVelocity);
}

// --------------------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(func_momentum_brush, CFuncMomentumBrush);

BEGIN_DATADESC(CFuncMomentumBrush)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage"), 
    DEFINE_KEYFIELD(m_iDisabledAlpha, FIELD_CHARACTER, "DisabledAlpha"),
    DEFINE_KEYFIELD(m_bInverted, FIELD_BOOLEAN, "Invert"),
    DEFINE_KEYFIELD(m_bDisableUI, FIELD_BOOLEAN, "DisableUI"),
END_DATADESC();

CFuncMomentumBrush::CFuncMomentumBrush()
{
    m_iWorld = -1;
    m_iStage = 0;
    m_iDisabledAlpha = 102;
    m_bInverted = false;
    m_bDisableUI = false;
}

void CFuncMomentumBrush::Spawn()
{
    // On spawn, we need to check if this brush should be enabled

    SetMoveType(MOVETYPE_PUSH); // so it doesn't get pushed by anything
    SetRenderMode(kRenderTransAlpha); // Allows alpha override

    SetSolid(SOLID_BSP); // Seems to have the best collision for standing/jumping (see the bhop block fix system)
    AddEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);
    AddSolidFlags(FSOLID_TRIGGER); // Allow us to touch the player

    SetModel(STRING(GetModelName()));

    if (g_pMomentumProgress->IsStageBeat(m_iStage, m_iWorld))
        TurnOn();
    else
        TurnOff();

    // If it can't move/go away, it's really part of the world
    if (!GetEntityName() || !m_iParent)
        AddFlag(FL_WORLDBRUSH);
    
    CreateVPhysics();
}

bool CFuncMomentumBrush::IsOn() const
{
    return (m_bInverted ? m_iDisabled != 0 : m_iDisabled == 0);
}

void CFuncMomentumBrush::TurnOn()
{
    // Turning this brush "on" means making it 100% solid and visible
    if (IsOn())
        return;

    // MOM_TODO: The below is probably needed later on, after mappers make the map
    /*if (!g_pMomentumProgress->ShouldEnableBrush(m_iStageEnable))
        return;*/

    if (m_bInverted)
       AddSolidFlags(FSOLID_NOT_SOLID);
    else
       RemoveSolidFlags(FSOLID_NOT_SOLID);

    SetRenderColorA(m_bInverted ? m_iDisabledAlpha : 255);

    m_iDisabled = m_bInverted;
}

void CFuncMomentumBrush::TurnOff()
{
    // Turning this brush "off" means making it non-solid, and only translucent
    if (!IsOn())
        return;

    if (m_bInverted)
        RemoveSolidFlags(FSOLID_NOT_SOLID);    
    else
        AddSolidFlags(FSOLID_NOT_SOLID);

    SetRenderColorA(m_bInverted ? 255 : m_iDisabledAlpha);

    m_iDisabled = !m_bInverted;
}

void CFuncMomentumBrush::StartTouch(CBaseEntity* pOther)
{
    BaseClass::StartTouch(pOther);
    // MOM_TODO: Show a UI that says which stage needs unlocking
    if (!m_bDisableUI)
    {
        if (pOther->IsPlayer())
        {
            if (m_iDisabled)
                ClientPrint((CBasePlayer*)pOther, HUD_PRINTCENTER, 
                            CFmtStr("Beat Stage %i To Make This Solid!", m_iStage).Get());
        }
    }
}

void CFuncMomentumBrush::EndTouch(CBaseEntity* pOther)
{
    BaseClass::EndTouch(pOther);
    // MOM_TODO: Hide the UI
    // if (m_iDisabled && pOther->IsPlayer()) or something
}


LINK_ENTITY_TO_CLASS(filter_momentum_campaign_progress, CFilterCampaignProgress);

BEGIN_DATADESC(CFilterCampaignProgress)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage")
END_DATADESC()

CFilterCampaignProgress::CFilterCampaignProgress()
{
    m_iWorld = -1;
    m_iStage = 0;
}

bool CFilterCampaignProgress::PassesFilterImpl(CBaseEntity* pCaller, CBaseEntity* pEntity)
{
    // So far the only entity that is a player is the local player
    if (pEntity->IsPlayer())
    {
        return g_pMomentumProgress->IsStageBeat(m_iStage, m_iWorld);
    }
    return false;
}

LINK_ENTITY_TO_CLASS(trigger_momentum_campaign_changelevel, CTriggerCampaignChangelevel);

BEGIN_DATADESC(CTriggerCampaignChangelevel)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage"),
    DEFINE_KEYFIELD(m_iGametype, FIELD_INTEGER, "gametype"),
    DEFINE_KEYFIELD(m_MapOverride, FIELD_STRING, "map_name_override")
END_DATADESC()

CTriggerCampaignChangelevel::CTriggerCampaignChangelevel()
{
    m_iWorld = -1;
    m_iStage = 0;
    m_iGametype = 0;
}

void CTriggerCampaignChangelevel::OnStartTouch(CBaseEntity* pOther)
{
    BaseClass::OnStartTouch(pOther);

    if (pOther->IsPlayer())
    {
        if (!m_MapOverride)
        {
            if (m_iWorld == -1)
            {
                // Go back to the Hub
                engine->ClientCommand(pOther->edict(), "map mom_hub\n");
            }
            else
            {
                // Otherwise go to a specific world stage
                const char *pMapPrefix = g_pGameModeSystem->GetGameMode(m_iGametype)->GetMapPrefix();

                engine->ClientCommand(pOther->edict(), CFmtStr("map %sw%i_s%i\n", pMapPrefix, m_iWorld, m_iStage).Get());
            }
        }
        else
        {
            // Go to the specific map
            engine->ClientCommand(pOther->edict(), CFmtStr("map %s\n", m_MapOverride.ToCStr()).Get());
        }
    }
}


LINK_ENTITY_TO_CLASS(info_momentum_map, CMomentumMapInfo);

BEGIN_DATADESC(CMomentumMapInfo)
    DEFINE_KEYFIELD(m_iWorld, FIELD_INTEGER, "World"),
    DEFINE_KEYFIELD(m_iStage, FIELD_INTEGER, "Stage"),
    DEFINE_KEYFIELD(m_iGametype, FIELD_INTEGER, "gametype"),
    DEFINE_KEYFIELD(m_MapAuthor, FIELD_STRING, "author")
END_DATADESC()

CMomentumMapInfo::CMomentumMapInfo(): m_iWorld(-1), m_iStage(0), m_iGametype(0)
{
}

void CMomentumMapInfo::Spawn()
{
    BaseClass::Spawn();

    KeyValues *pKv = new KeyValues("map_info");
    pKv->SetInt("world", m_iWorld);
    pKv->SetInt("stage", m_iStage);
    pKv->SetInt("gametype", m_iGametype);
    pKv->SetString("author", m_MapAuthor.ToCStr());
    g_pModuleComms->FireEvent(pKv);
    // MOM_TODO: Handle this event in Client (UI) and Timer?
}

static CUtlVector<CNoGrenadesZone *> s_vecNoGrenadeZones;

LINK_ENTITY_TO_CLASS(func_nogrenades, CNoGrenadesZone);

BEGIN_DATADESC(CNoGrenadesZone)
    DEFINE_KEYFIELD(m_iExplosivePreventionType, FIELD_INTEGER, "explosion_prevention_type")
END_DATADESC();

CNoGrenadesZone::~CNoGrenadesZone()
{
    s_vecNoGrenadeZones.FindAndFastRemove(this);
}

void CNoGrenadesZone::Spawn()
{
    Precache();
    BaseClass::Spawn();
    InitTrigger();

    AddSpawnFlags(SF_TRIGGER_ALLOW_ALL);
    AddEffects(EF_NODRAW);

    s_vecNoGrenadeZones.AddToTail(this);
}

void CNoGrenadesZone::Precache()
{
    PrecacheModel(NOGRENADE_SPRITE);
}

void CNoGrenadesZone::OnStartTouch(CBaseEntity* pOther)
{
    BaseClass::OnStartTouch(pOther);

    if (!pOther)
        return;

    const auto pExplosive = dynamic_cast<CMomExplosive *>(pOther);
    if (!pExplosive)
        return;

    if (m_iExplosivePreventionType == FIZZLE_ON_ENTRANCE)
    {
        pExplosive->Fizzle();
        return;
    }

    const auto pSticky = dynamic_cast<CMomStickybomb *>(pExplosive);
    if (pSticky && m_iExplosivePreventionType != FIZZLE_ON_LAND)
        pSticky->SetCanExplode(false);
}

void CNoGrenadesZone::OnEndTouch(CBaseEntity* pOther)
{
    BaseClass::OnEndTouch(pOther);

    if (!pOther)
        return;

    const auto pSticky = dynamic_cast<CMomStickybomb *>(pOther);
    if (!pSticky)
        return;

    pSticky->SetCanExplode(true);
}

CNoGrenadesZone* CNoGrenadesZone::IsInsideNoGrenadesZone(CBaseEntity *pOther)
{
    if (!pOther)
        return nullptr;

    FOR_EACH_VEC(s_vecNoGrenadeZones, i)
    {
        const auto pNoGrenadeZone = s_vecNoGrenadeZones[i];

        if (pNoGrenadeZone->m_bDisabled || !pNoGrenadeZone->PointIsWithin(pOther->GetAbsOrigin()))
            continue;

        return pNoGrenadeZone;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------------------------

//--------- CTriggerMomentumCatapult -------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(trigger_momentum_catapult, CTriggerMomentumCatapult);

// Alias tf2 trigger_catapult for backwards compat
LINK_ENTITY_TO_CLASS(trigger_catapult, CTriggerMomentumCatapult);

BEGIN_DATADESC(CTriggerMomentumCatapult)
    DEFINE_KEYFIELD(m_flPlayerSpeed, FIELD_FLOAT, "playerSpeed"),
    DEFINE_KEYFIELD(m_bUseThresholdCheck, FIELD_INTEGER, "useThresholdCheck"),
    DEFINE_KEYFIELD(m_flEntryAngleTolerance, FIELD_FLOAT, "entryAngleTolerance"),
    DEFINE_KEYFIELD(m_iUseExactVelocity, FIELD_INTEGER, "useExactVelocity"),
    DEFINE_KEYFIELD(m_iExactVelocityChoiceType, FIELD_INTEGER, "exactVelocityChoiceType"),
    DEFINE_KEYFIELD(m_flLowerThreshold, FIELD_FLOAT, "lowerThreshold"),
    DEFINE_KEYFIELD(m_flUpperThreshold, FIELD_FLOAT, "upperThreshold"),
    DEFINE_KEYFIELD(m_vLaunchDirection, FIELD_VECTOR, "launchDirection"),
    DEFINE_KEYFIELD(m_target, FIELD_STRING, "launchTarget"),
    DEFINE_KEYFIELD(m_bOnlyCheckVelocity, FIELD_INTEGER, "onlyCheckVelocity"),
    DEFINE_OUTPUT(m_OnCatapulted, "OnCatapulted"),
    DEFINE_KEYFIELD(m_flInterval, FIELD_FLOAT, "Interval"),
    DEFINE_KEYFIELD(m_bOnThink, FIELD_BOOLEAN, "OnThink"),
    DEFINE_KEYFIELD(m_bEveryTick, FIELD_BOOLEAN, "EveryTick"),
    DEFINE_KEYFIELD(m_flHeightOffset, FIELD_FLOAT, "heightOffset"),
END_DATADESC()


CTriggerMomentumCatapult::CTriggerMomentumCatapult()
{
    m_flPlayerSpeed = 450.0f;
    m_bUseThresholdCheck = 0;
    m_flEntryAngleTolerance = 0.0f;
    m_iUseExactVelocity = 0;
    m_iExactVelocityChoiceType = BEST;
    m_flLowerThreshold = 0.15f;
    m_flUpperThreshold = 0.30f;
    m_vLaunchDirection = vec3_angle;
    m_hLaunchTarget = nullptr;
    m_bOnlyCheckVelocity = false;
    m_flInterval = 1.0;
    m_bOnThink = false;
    m_bEveryTick = false;
    m_flHeightOffset = 32.0f;
}

void CTriggerMomentumCatapult::Spawn()
{ 
    BaseClass::Spawn();

    m_flLowerThreshold = clamp(m_flLowerThreshold, 0.0f, 1.0f);
    m_flUpperThreshold = clamp(m_flUpperThreshold, 0.0f, 1.0f);

    m_flEntryAngleTolerance = clamp(m_flEntryAngleTolerance, -1.0f, 1.0f);

    if (!m_hLaunchTarget.Get())
    {
        if (m_target != NULL_STRING)
        {
            m_hLaunchTarget = gEntList.FindEntityByName(nullptr, m_target);
            m_bUseLaunchTarget = true;
        }
        else
        {
            m_bUseLaunchTarget = false;
        }
    }
}

Vector CTriggerMomentumCatapult::CalculateLaunchVelocity(CBaseEntity *pOther)
{
    // Calculated from time ignoring grav, then compensating for gravity later
    // From https://www.gamasutra.com/blogs/KainShin/20090515/83954/Predictive_Aim_Mathematics_for_AI_Targeting.php
    // and setting the target's velocity vector to zero

    Vector vecPlayerOrigin = pOther->GetAbsOrigin();

    vecPlayerOrigin.z += m_flHeightOffset;

    Vector vecAbsDifference = m_hLaunchTarget->GetAbsOrigin() - vecPlayerOrigin;
    float flSpeedSquared = m_flPlayerSpeed * m_flPlayerSpeed;
    float flGravity = GetCurrentGravity();

    float flDiscriminant = 4.0f * flSpeedSquared * vecAbsDifference.Length() * vecAbsDifference.Length();

    flDiscriminant = sqrtf(flDiscriminant);
    float fTime = 0.5f * (flDiscriminant / flSpeedSquared);

    Vector vecLaunchVelocity = (vecAbsDifference / fTime);

    Vector vecGravityComp(0, 0, 0.5f * -flGravity * fTime);
    vecLaunchVelocity -= vecGravityComp;

    return vecLaunchVelocity;
}

Vector CTriggerMomentumCatapult::CalculateLaunchVelocityExact(CBaseEntity* pOther)
{
    // Uses exact trig and gravity

    Vector vecPlayerOrigin = pOther->GetAbsOrigin();

    vecPlayerOrigin.z += m_flHeightOffset;

    Vector vecAbsDifference = m_hLaunchTarget->GetAbsOrigin() - vecPlayerOrigin;
    Vector vecAbsDifferenceXY = Vector(vecAbsDifference.x, vecAbsDifference.y, 0.0f);

    float flSpeedSquared = m_flPlayerSpeed * m_flPlayerSpeed;
    float flSpeedQuad = m_flPlayerSpeed * m_flPlayerSpeed * m_flPlayerSpeed * m_flPlayerSpeed;
    float flAbsX = vecAbsDifferenceXY.Length();
    float flAbsZ = vecAbsDifference.z;
    float flGravity = GetCurrentGravity();

    float flDiscriminant = flSpeedQuad - flGravity * (flGravity * flAbsX * flAbsX + 2.0f * flAbsZ * flSpeedSquared);

    // Maybe not this but some sanity check ofc, then default to non exact case which should always have a solution
    if (m_flPlayerSpeed < sqrtf(flGravity * (flAbsZ + vecAbsDifference.Length())))
    {
        DevWarning("Not enough speed to reach target.\n");
        return CalculateLaunchVelocity(pOther);
    }
    if (flDiscriminant < 0.0f)
    {
        DevWarning("Not enough speed to reach target.\n");
        return CalculateLaunchVelocity(pOther);
    }
    if (CloseEnough(flAbsX, 0.0f))
    {
        DevWarning("Target position cannot be the same as catapult position?\n");
        return CalculateLaunchVelocity(pOther);
    }

    flDiscriminant = sqrtf(flDiscriminant);

    float flLowAng = atanf((flSpeedSquared - flDiscriminant) / (flGravity * flAbsX));
    float flHighAng = atanf((flSpeedSquared + flDiscriminant) / (flGravity * flAbsX));

    Vector fGroundDir = vecAbsDifferenceXY.Normalized();
    Vector vecLowAngVelocity = m_flPlayerSpeed * (fGroundDir * cosf(flLowAng) + Vector(0, 0, sinf(flLowAng)));
    Vector vecHighAngVelocity = m_flPlayerSpeed * (fGroundDir * cosf(flHighAng) + Vector(0, 0, sinf(flHighAng)));
    Vector vecLaunchVelocity = vec3_origin;
    Vector vecPlayerEntryVel = pOther->GetAbsVelocity();

    switch (m_iExactVelocityChoiceType)
    {
    case BEST:
        // "Best" solution seems to minimize angle of entry with respect to launch vector
        vecLaunchVelocity = vecPlayerEntryVel.Dot(vecLowAngVelocity) < vecPlayerEntryVel.Dot(vecHighAngVelocity)
                                ? vecLowAngVelocity
                                : vecHighAngVelocity;
        break;

    case SOLUTION_ONE:
        vecLaunchVelocity = vecLowAngVelocity;
        break;

    case SOLUTION_TWO:
        vecLaunchVelocity = vecHighAngVelocity;
        break;

    default:
        break;
    }

    return vecLaunchVelocity;
}
void CTriggerMomentumCatapult::LaunchAtDirection(CBaseEntity *pOther)
{
    pOther->SetGroundEntity(nullptr);
    Vector vecLaunchDir = vec3_origin;
    AngleVectors(m_vLaunchDirection, &vecLaunchDir);
    pOther->SetAbsVelocity(m_flPlayerSpeed * vecLaunchDir);
    m_OnCatapulted.FireOutput(pOther, this);
}

void CTriggerMomentumCatapult::LaunchAtTarget(CBaseEntity *pOther)
{
    pOther->SetGroundEntity(nullptr);
    Vector vecLaunchVelocity = vec3_origin;

    if (m_iUseExactVelocity)
    {
        vecLaunchVelocity = CalculateLaunchVelocityExact(pOther);
    }
    else
    {
        vecLaunchVelocity = CalculateLaunchVelocity(pOther);
    }

    pOther->SetAbsVelocity(vecLaunchVelocity);
    m_OnCatapulted.FireOutput(pOther, this);
}

void CTriggerMomentumCatapult::Launch(CBaseEntity *pOther)
{
    bool bLaunch = true;

    // Check threshold
    if (m_bUseThresholdCheck)
    {
        Vector vecPlayerVelocity = pOther->GetAbsVelocity();
        float flPlayerSpeed = vecPlayerVelocity.Length();
        bLaunch = false;

        // From VDC
        if (flPlayerSpeed > m_flPlayerSpeed - (m_flPlayerSpeed * m_flLowerThreshold) &&
            flPlayerSpeed < m_flPlayerSpeed + (m_flPlayerSpeed * m_flUpperThreshold))
        {
            float flPlayerEntryAng = 0.0f;

            if (m_bUseLaunchTarget)
            {
                Vector vecAbsDifference = m_hLaunchTarget->GetAbsOrigin() - pOther->GetAbsOrigin();
                flPlayerEntryAng = DotProduct(vecAbsDifference.Normalized(), vecPlayerVelocity.Normalized());

            }
            else
            {
                Vector vecLaunchDir = vec3_origin;
                AngleVectors(m_vLaunchDirection, &vecLaunchDir);
                flPlayerEntryAng = DotProduct(vecLaunchDir.Normalized(), vecPlayerVelocity.Normalized());
            }

            // VDC uses brackets so inclusive??
            if (flPlayerEntryAng >= m_flEntryAngleTolerance)
            {
                if (m_bOnlyCheckVelocity)
                {
                    m_OnCatapulted.FireOutput(pOther, this);
                    return;
                }
                bLaunch = true;
            }
        }
    }

    if (!bLaunch)
    {
        return;
    }

    if (m_bUseLaunchTarget)
    {
        LaunchAtTarget(pOther);
    }
    else
    {
        LaunchAtDirection(pOther);
    }
}

void CTriggerMomentumCatapult::OnStartTouch(CBaseEntity* pOther)
{
    BaseClass::OnStartTouch(pOther);

    // Ignore vphys only allow players
    if (pOther && pOther->IsPlayer())
    {
        Launch(pOther);

        if (m_bOnThink)
            SetNextThink(gpGlobals->curtime + m_flInterval);
    }
}

void CTriggerMomentumCatapult::Touch(CBaseEntity *pOther)
{
    BaseClass::Touch(pOther);

    if (m_bEveryTick)
    {
        if (!PassesTriggerFilters(pOther))
            return;

        if (pOther && pOther->IsPlayer())
        {
            Launch(pOther);
        }
    }
}

void CTriggerMomentumCatapult::Think()
{
    if (!m_bOnThink)
    {
        SetNextThink(TICK_NEVER_THINK);
        return;
    }
    
    FOR_EACH_VEC(m_hTouchingEntities, i)
    {
        const auto pEnt = m_hTouchingEntities[i].Get();
        if (pEnt && pEnt->IsPlayer())
        {
            Launch(pEnt);
            SetNextThink(gpGlobals->curtime + m_flInterval);
        }
    }
}

int CTriggerMomentumCatapult::DrawDebugTextOverlays()
{
    int text_offset = BaseClass::DrawDebugTextOverlays();

    char tempstr[255];

    if (m_target != NULL_STRING)
    {
        Q_snprintf(tempstr, sizeof(tempstr), "Launch target: %s", m_target.ToCStr());
        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }

    Q_snprintf(tempstr, sizeof(tempstr), "Player velocity: %f", m_flPlayerSpeed);
    EntityText(text_offset, tempstr, 0);
    text_offset++;

    Vector vecLaunchVelocity = vec3_origin;
    Vector vecLaunchVelocityExact = vec3_origin;
    if (m_target != NULL_STRING)
    {
        vecLaunchVelocity = CalculateLaunchVelocity(this);
        vecLaunchVelocityExact = CalculateLaunchVelocityExact(this);

        Q_snprintf(tempstr, sizeof(tempstr), "Adjusted player velocity: %f",
                   m_iUseExactVelocity ? (float)vecLaunchVelocity.Length() : (float)vecLaunchVelocityExact.Length());

        EntityText(text_offset, tempstr, 0);
        text_offset++;
    }


    return text_offset;
}

//-----------------------------------------------------------------------------------------------