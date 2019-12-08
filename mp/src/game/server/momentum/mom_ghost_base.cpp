#include "cbase.h"

#include "mom_ghost_base.h"
#include "util/mom_util.h"
#include "mom_player_shared.h"
#include "mom_timer.h"
#include "mom_system_gamemode.h"
#include "in_buttons.h"

#include "tier0/memdbgon.h"

extern void SendProxy_CropFlagsToPlayerFlagBitsLength(const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID);

IMPLEMENT_SERVERCLASS_ST(CMomentumGhostBaseEntity, DT_MOM_GHOST_BASE)
SendPropInt(SENDINFO(m_nGhostButtons)),
SendPropInt(SENDINFO(m_iDisabledButtons)),
SendPropBool(SENDINFO(m_bBhopDisabled)),
SendPropString(SENDINFO(m_szGhostName)),
SendPropBool(SENDINFO(m_bSpectated)),
SendPropInt(SENDINFO(m_fFlags), PLAYER_FLAG_BITS, SPROP_UNSIGNED|SPROP_CHANGES_OFTEN, SendProxy_CropFlagsToPlayerFlagBitsLength),
SendPropDataTable(SENDINFO_DT(m_Data), &REFERENCE_SEND_TABLE(DT_MomRunEntityData)),
SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MomRunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumGhostBaseEntity)
END_DATADESC();

CMomentumGhostBaseEntity::CMomentumGhostBaseEntity(): m_pCurrentSpecPlayer(nullptr)
{
    m_nGhostButtons = 0;
    m_iDisabledButtons = 0;
    m_szGhostName.GetForModify()[0] = '\0';
    m_RunStats.Init();
}

void CMomentumGhostBaseEntity::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
    if (m_takedamage)
    {
        SpawnBlood(ptr->endpos, vecDir, BloodColor(), info.GetDamage());// a little surface blood.
        TraceBleed(info.GetDamage(), vecDir, ptr, info.GetDamageType());
    }
}

void CMomentumGhostBaseEntity::Precache()
{
    BaseClass::Precache();
    PrecacheModel(GHOST_MODEL);
}

void CMomentumGhostBaseEntity::Spawn()
{
    Precache();
    BaseClass::Spawn();
    SetModel(GHOST_MODEL);
    SetBodygroup(1, BODY_PROLATE_ELLIPSE);
    RemoveEffects(EF_NODRAW);
    //~~~The magic combo~~~ (collides with triggers, not with players)
    ClearSolidFlags();
    AddFlag(FL_CLIENT | FL_GODMODE);
    SetCollisionGroup(COLLISION_GROUP_DEBRIS_TRIGGER);
    SetMoveType(MOVETYPE_STEP);
    SetSolid(SOLID_BBOX);
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    // Always call CollisionBounds after you set the model
    SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
    UpdateModelScale();
    SetViewOffset(VEC_VIEW_SCALED(this));
    UnHideGhost();
    m_takedamage = DAMAGE_EVENTS_ONLY;
}

void CMomentumGhostBaseEntity::HideGhost()
{
    // don't render the model when we're in first person mode
    if (GetRenderMode() != kRenderNone)
    {
        SetRenderMode(kRenderNone);
        AddEffects(EF_NOSHADOW);
    }
}

void CMomentumGhostBaseEntity::UnHideGhost()
{
    if (GetRenderMode() != kRenderTransColor)
    {
        SetRenderMode(kRenderTransColor);
        RemoveEffects(EF_NOSHADOW);
    }
}

void CMomentumGhostBaseEntity::SetButtonsEnabled(int iButtonFlags, bool bEnable)
{
    if (bEnable)
        m_iDisabledButtons &= ~iButtonFlags;
    else 
        m_iDisabledButtons |= iButtonFlags;
}

void CMomentumGhostBaseEntity::SetBhopEnabled(bool bEnable)
{
    m_bBhopDisabled = !bEnable;
}

bool CMomentumGhostBaseEntity::GetBhopEnabled() const
{
    return !m_bBhopDisabled;
}

bool CMomentumGhostBaseEntity::ShouldCollide(int collisionGroup, int contentsMask) const
{
    if (collisionGroup == COLLISION_GROUP_PROJECTILE)
        return false; // MOM_TODO allow if it's trikz gamemode

    return BaseClass::ShouldCollide(collisionGroup, contentsMask);
}

void CMomentumGhostBaseEntity::StartTimer(int m_iStartTick)
{
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        g_pMomentumTimer->DispatchTimerEventMessage(m_pCurrentSpecPlayer, entindex(), TIMER_EVENT_STARTED);
    }
}

void CMomentumGhostBaseEntity::FinishTimer()
{
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        g_pMomentumTimer->DispatchTimerEventMessage(m_pCurrentSpecPlayer, entindex(), TIMER_EVENT_FINISHED);
    }
}

void CMomentumGhostBaseEntity::SetSpectator(CMomentumPlayer *player)
{
    m_pCurrentSpecPlayer = player;
    m_bSpectated = true;
}

void CMomentumGhostBaseEntity::RemoveSpectator()
{
    m_bSpectated = false;
    m_pCurrentSpecPlayer = nullptr;
    UnHideGhost();
}

// Ripped from gamemovement for slightly better collision
bool CMomentumGhostBaseEntity::CanUnduck()
{
    trace_t trace;
    Vector newOrigin;

    VectorCopy(GetAbsOrigin(), newOrigin);

    if (GetGroundEntity() != nullptr)
    {
        newOrigin += VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
    }
    else
    {
        // If in air an letting go of crouch, make sure we can offset origin to make
        //  up for uncrouching
        Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
        Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

        newOrigin += -VIEW_SCALE * (hullSizeNormal - hullSizeCrouch);
    }

    UTIL_TraceHull(GetAbsOrigin(), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, this,
                   COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

    if (trace.startsolid || (trace.fraction != 1.0f))
        return false;

    return true;
}

void CMomentumGhostBaseEntity::HandleDucking()
{
    const auto isDucking = (GetFlags() & FL_DUCKING) != 0;
    if (m_nGhostButtons & IN_DUCK)
    {
        if (!isDucking)
        {
            SetCollisionBounds(VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
            AddFlag(FL_DUCKING);
        }
    }
    else if (isDucking)
    {
        if (CanUnduck())
        {
            SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
            RemoveFlag(FL_DUCKING);
        }
    }
}