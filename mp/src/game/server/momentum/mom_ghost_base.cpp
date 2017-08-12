#include "cbase.h"

#include "mom_ghost_base.h"
#include "util/mom_util.h"

IMPLEMENT_SERVERCLASS_ST(CMomentumGhostBaseEntity, DT_MOM_GHOST_BASE)
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumGhostBaseEntity)
END_DATADESC();

CMomentumGhostBaseEntity::CMomentumGhostBaseEntity(): m_pCurrentSpecPlayer(nullptr), m_eTrail(nullptr)
{
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
    SetRenderMode(kRenderNone); //we don't want to draw the ghost until it can update it's appearance.
    //~~~The magic combo~~~ (collides with triggers, not with players)
    ClearSolidFlags();
    SetCollisionGroup(COLLISION_GROUP_DEBRIS_TRIGGER);
    SetMoveType(MOVETYPE_STEP);
    SetSolid(SOLID_BBOX);
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    // Always call CollisionBounds after you set the model
    SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
    UpdateModelScale();
    SetViewOffset(VEC_VIEW_SCALED(this));

    // Set the appearance to the player's appearance, pending update after spawned in
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (pPlayer)
    {
        SetGhostAppearance(pPlayer->m_playerAppearanceProps);
        //now that we've set our appearance, the ghost should be visible again.
        SetRenderMode(kRenderTransColor);
        if (m_ghostAppearance.GhostTrailEnable)
        {
            CreateTrail();
        }
    }
}
void CMomentumGhostBaseEntity::Think()
{
    BaseClass::Think();
    
    //MOM_TODO: handle recreating trail when ghost teleports.
}
void CMomentumGhostBaseEntity::SetGhostBodyGroup(int bodyGroup)
{
    if (bodyGroup > LAST || bodyGroup < 0)
    {
        Warning("CMomentumGhostBaseEntity::SetGhostBodyGroup() Error: Could not set bodygroup!");
    }
    else
    {
        m_ghostAppearance.GhostModelBodygroup = bodyGroup;
        SetBodygroup(1, bodyGroup);
    }
}
void CMomentumGhostBaseEntity::SetGhostModel(const char *newmodel)
{
    if (newmodel)
    {
        Q_strncpy(m_ghostAppearance.GhostModel, newmodel, strlen(newmodel)+1);
        PrecacheModel(m_ghostAppearance.GhostModel);
        SetModel(m_ghostAppearance.GhostModel);
    }
}
void CMomentumGhostBaseEntity::SetGhostColor(const uint32 newHexColor)
{
    m_ghostAppearance.GhostModelRGBAColorAsHex = newHexColor;
    Color *newColor = g_pMomentumUtil->GetColorFromHex(newHexColor);
    if (newColor)
    {
        SetRenderColor(newColor->r(), newColor->g(), newColor->b(), newColor->a());
    }
}
void CMomentumGhostBaseEntity::SetGhostTrailProperties(const uint32 newHexColor, int newLen, bool enable)
{
    m_ghostAppearance.GhostTrailEnable = enable;
    m_ghostAppearance.GhostTrailRGBAColorAsHex = newHexColor;
    m_ghostAppearance.GhostTrailLength = newLen;
    CreateTrail();
}

void CMomentumGhostBaseEntity::SetGhostFlashlight(bool isOn)
{
    if (isOn)
    {
        AddEffects(EF_DIMLIGHT);
        EmitSound(SND_FLASHLIGHT_ON); // MOM_TODO: Make this quieter than the player
    }
    else
    {
        RemoveEffects(EF_DIMLIGHT);
        EmitSound(SND_FLASHLIGHT_OFF);
    }
}

void CMomentumGhostBaseEntity::StartTimer(int m_iStartTick)
{
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        g_pMomentumTimer->DispatchTimerStateMessage(m_pCurrentSpecPlayer, true);
    }
}
void CMomentumGhostBaseEntity::StopTimer()
{
    if (m_pCurrentSpecPlayer && m_pCurrentSpecPlayer->GetGhostEnt() == this)
    {
        g_pMomentumTimer->DispatchTimerStateMessage(m_pCurrentSpecPlayer, false);
    }
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
void CMomentumGhostBaseEntity::SetGhostAppearance(ghostAppearance_t newApp)
{
    // only set things that NEED TO BE CHANGED!!
    if (m_ghostAppearance.GhostModelBodygroup != newApp.GhostModelBodygroup)
    {
        SetGhostBodyGroup(newApp.GhostModelBodygroup);
    }
    if (Q_strcmp(m_ghostAppearance.GhostModel, newApp.GhostModel) != 0)
    {
        SetGhostModel(newApp.GhostModel);
    }
    if (m_ghostAppearance.GhostModelRGBAColorAsHex != newApp.GhostModelRGBAColorAsHex)
    {
        SetGhostColor(newApp.GhostModelRGBAColorAsHex);
    }
    if (m_ghostAppearance.GhostTrailRGBAColorAsHex != newApp.GhostTrailRGBAColorAsHex || 
        m_ghostAppearance.GhostTrailLength != newApp.GhostTrailLength || 
        m_ghostAppearance.GhostTrailEnable != newApp.GhostTrailEnable)
    {
        SetGhostTrailProperties(newApp.GhostTrailRGBAColorAsHex,
            newApp.GhostTrailLength, newApp.GhostTrailEnable);
    }
    
    SetGhostFlashlight(newApp.FlashlightOn);
}
void CMomentumGhostBaseEntity::CreateTrail()
{
    RemoveTrail();

    if (!m_ghostAppearance.GhostTrailEnable) return;

    // Ty GhostingMod
    m_eTrail = CreateEntityByName("env_spritetrail");
    m_eTrail->SetAbsOrigin(GetAbsOrigin());
    m_eTrail->SetParent(this);
    m_eTrail->KeyValue("rendermode", "5");
    m_eTrail->KeyValue("spritename", "materials/sprites/laser.vmt");
    m_eTrail->KeyValue("startwidth", "9.5");
    m_eTrail->KeyValue("endwidth", "1.05");
    m_eTrail->KeyValue("lifetime", m_ghostAppearance.GhostTrailLength);
    Color *newColor = g_pMomentumUtil->GetColorFromHex(m_ghostAppearance.GhostTrailRGBAColorAsHex);
    if (newColor)
    {
        m_eTrail->SetRenderColor(newColor->r(), newColor->g(), newColor->b(), newColor->a());
        m_eTrail->KeyValue("renderamt", newColor->a());
    }
    DispatchSpawn(m_eTrail);
}
void CMomentumGhostBaseEntity::RemoveTrail()
{
    UTIL_RemoveImmediate(m_eTrail);
    m_eTrail = nullptr;
}