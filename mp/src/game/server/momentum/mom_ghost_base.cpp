#include "cbase.h"

#include "mom_ghost_base.h"
#include "util/mom_util.h"
#include "ghost_client.h"
#include "mom_player_shared.h"
#include "mom_timer.h"

#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CMomentumGhostBaseEntity, DT_MOM_GHOST_BASE)
SendPropInt(SENDINFO(m_nGhostButtons)),
SendPropInt(SENDINFO(m_iDisabledButtons)),
SendPropBool(SENDINFO(m_bBhopDisabled)),
SendPropString(SENDINFO(m_szGhostName)),
SendPropBool(SENDINFO(m_bSpectated)),
SendPropDataTable(SENDINFO_DT(m_Data), &REFERENCE_SEND_TABLE(DT_MomRunEntityData)),
SendPropDataTable(SENDINFO_DT(m_RunStats), &REFERENCE_SEND_TABLE(DT_MomRunStats)),
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumGhostBaseEntity)
END_DATADESC();

static void RefreshGhostData(IConVar *var, const char *pValue, float oldValue)
{
    g_pMomentumGhostClient->ResetOtherAppearanceData();
}

static MAKE_TOGGLE_CONVAR_C(mom_ghost_online_alpha_override_enable, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE, 
    "Toggle overriding other player's ghost alpha values to the one defined in \"mom_ghost_online_color_alpha_override\".\n", RefreshGhostData);
static MAKE_CONVAR_C(mom_ghost_online_alpha_override, "100", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Overrides ghosts alpha to be this value.\n", 0, 255, RefreshGhostData);
static MAKE_TOGGLE_CONVAR_C(mom_ghost_online_trail_enable, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Toggles drawing other ghost's trails. 0 = OFF, 1 = ON\n", RefreshGhostData);

CMomentumGhostBaseEntity::CMomentumGhostBaseEntity(): m_pCurrentSpecPlayer(nullptr), m_eTrail(nullptr)
{
    m_nGhostButtons = 0;
    m_iDisabledButtons = 0;
    m_szGhostName.GetForModify()[0] = '\0';
    m_RunStats.Init();
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
    SetModel(GHOST_MODEL); //we need a model
    SetGhostBodyGroup(BODY_PROLATE_ELLIPSE);
    RemoveEffects(EF_NODRAW);
    //~~~The magic combo~~~ (collides with triggers, not with players)
    ClearSolidFlags();
    AddFlag(FL_CLIENT);
    SetCollisionGroup(COLLISION_GROUP_DEBRIS_TRIGGER);
    SetMoveType(MOVETYPE_STEP);
    SetSolid(SOLID_BBOX);
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    // Always call CollisionBounds after you set the model
    SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
    UpdateModelScale();
    SetViewOffset(VEC_VIEW_SCALED(this));
    UnHideGhost();
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
        m_iDisabledButtons |= iButtonFlags;
    else 
        m_iDisabledButtons &= ~iButtonFlags;
}

void CMomentumGhostBaseEntity::SetBhopEnabled(bool bEnable)
{
    m_bBhopDisabled = !bEnable;
}

bool CMomentumGhostBaseEntity::GetBhopEnabled() const
{
    return !m_bBhopDisabled;
}

void CMomentumGhostBaseEntity::Think()
{
    BaseClass::Think();
}
void CMomentumGhostBaseEntity::SetGhostBodyGroup(int bodyGroup)
{
    if (bodyGroup > LAST || bodyGroup < 0)
    {
        Warning("CMomentumGhostBaseEntity::SetGhostBodyGroup() Error: Could not set bodygroup!");
    }
    else
    {
        m_ghostAppearance.m_iGhostModelBodygroup = bodyGroup;
        SetBodygroup(1, bodyGroup);
    }
}

void CMomentumGhostBaseEntity::SetGhostColor(const uint32 newHexColor)
{
    m_ghostAppearance.m_iGhostModelRGBAColorAsHex = newHexColor;
    Color newColor;
    if (g_pMomentumUtil->GetColorFromHex(newHexColor, newColor))
    {
        int alpha = mom_ghost_online_alpha_override_enable.GetBool() ? mom_ghost_online_alpha_override.GetInt() : newColor.a();
        SetRenderColor(newColor.r(), newColor.g(), newColor.b(), alpha);
    }
}
void CMomentumGhostBaseEntity::SetGhostTrailProperties(const uint32 newHexColor, int newLen, bool enable)
{
    m_ghostAppearance.m_bGhostTrailEnable = enable;
    m_ghostAppearance.m_iGhostTrailRGBAColorAsHex = newHexColor;
    m_ghostAppearance.m_iGhostTrailLength = clamp<int>(newLen, 1, 10);
    CreateTrail();
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
bool CMomentumGhostBaseEntity::CanUnduck(CMomentumGhostBaseEntity *pGhost)
{
    trace_t trace;
    Vector newOrigin;

    if (pGhost)
    {
        VectorCopy(pGhost->GetAbsOrigin(), newOrigin);

        if (pGhost->GetGroundEntity() != nullptr)
        {
            newOrigin += VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
        }
        else
        {
            // If in air an letting go of croush, make sure we can offset origin to make
            //  up for uncrouching
            Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
            Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

            newOrigin += -0.5f * (hullSizeNormal - hullSizeCrouch);
        }

        UTIL_TraceHull(pGhost->GetAbsOrigin(), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, pGhost,
                       COLLISION_GROUP_PLAYER_MOVEMENT, &trace);

        if (trace.startsolid || (trace.fraction != 1.0f))
            return false;

        return true;
    }
    return false;
}
void CMomentumGhostBaseEntity::SetGhostAppearance(GhostAppearance_t newApp, bool bForceUpdate /* = false*/)
{
    // only set things that NEED TO BE CHANGED!!
    if (m_ghostAppearance.m_iGhostModelBodygroup != newApp.m_iGhostModelBodygroup || bForceUpdate)
    {
        SetGhostBodyGroup(newApp.m_iGhostModelBodygroup);
    }
    if (m_ghostAppearance.m_iGhostModelRGBAColorAsHex != newApp.m_iGhostModelRGBAColorAsHex || bForceUpdate)
    {
        SetGhostColor(newApp.m_iGhostModelRGBAColorAsHex);
    }
    if (m_ghostAppearance.m_iGhostTrailRGBAColorAsHex != newApp.m_iGhostTrailRGBAColorAsHex || 
        m_ghostAppearance.m_iGhostTrailLength != newApp.m_iGhostTrailLength || 
        m_ghostAppearance.m_bGhostTrailEnable != newApp.m_bGhostTrailEnable || bForceUpdate)
    {
        SetGhostTrailProperties(newApp.m_iGhostTrailRGBAColorAsHex,
                                newApp.m_iGhostTrailLength, newApp.m_bGhostTrailEnable);
    }
}
void CMomentumGhostBaseEntity::CreateTrail()
{
    RemoveTrail();

    if (!(m_ghostAppearance.m_bGhostTrailEnable && mom_ghost_online_trail_enable.GetBool())) return;

    // Ty GhostingMod
    m_eTrail = CreateEntityByName("env_spritetrail");
    m_eTrail->SetAbsOrigin(GetAbsOrigin());
    m_eTrail->SetParent(this);
    m_eTrail->KeyValue("rendermode", "5");
    m_eTrail->KeyValue("spritename", "materials/sprites/laser.vmt");
    m_eTrail->KeyValue("startwidth", "9.5");
    m_eTrail->KeyValue("endwidth", "1.05");
    m_eTrail->KeyValue("lifetime", m_ghostAppearance.m_iGhostTrailLength);
    Color newColor;
    if (g_pMomentumUtil->GetColorFromHex(m_ghostAppearance.m_iGhostTrailRGBAColorAsHex, newColor))
    {
        m_eTrail->SetRenderColor(newColor.r(), newColor.g(), newColor.b(), newColor.a());
        m_eTrail->KeyValue("renderamt", newColor.a());
    }

    DispatchSpawn(m_eTrail);
}
void CMomentumGhostBaseEntity::RemoveTrail()
{
    UTIL_RemoveImmediate(m_eTrail);
    m_eTrail = nullptr;
}