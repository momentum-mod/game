#include "cbase.h"
#include "mom_ghost_base.h"
#include "util/mom_util.h"

static ConVar mom_replay_ghost_bodygroup("mom_replay_ghost_bodygroup", "11",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Replay ghost's body group (model)", true, 0, true, 14);
static ConCommand mom_replay_ghost_color("mom_replay_ghost_color", CMomentumGhostBaseEntity::SetGhostColorCC,
    "Set the ghost's color. Accepts HEX color value in format RRGGBBAA. if RRGGBB is supplied, Alpha is set to 0x4B",
    FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE);

Color CMomentumGhostBaseEntity::m_NewGhostColor;


IMPLEMENT_SERVERCLASS_ST(CMomentumGhostBaseEntity, DT_MOM_GHOST_BASE)
END_SEND_TABLE();

BEGIN_DATADESC(CMomentumGhostBaseEntity)
END_DATADESC();

void CMomentumGhostBaseEntity::Precache()
{
    BaseClass::Precache();
    PrecacheModel(GHOST_MODEL);
    SetGhostColor(COLOR_GREEN);
}
void CMomentumGhostBaseEntity::Spawn()
{
    Precache();
    BaseClass::Spawn();

    RemoveEffects(EF_NODRAW);
    SetRenderMode(kRenderTransColor);
    SetRenderColor(m_GhostColor.r(), m_GhostColor.g(), m_GhostColor.b(), m_GhostColor.a());
    //~~~The magic combo~~~ (collides with triggers, not with players)
    ClearSolidFlags();
    SetCollisionGroup(COLLISION_GROUP_DEBRIS_TRIGGER);
    SetMoveType(MOVETYPE_STEP);
    SetSolid(SOLID_BBOX);
    RemoveSolidFlags(FSOLID_NOT_SOLID);

    SetModel(GHOST_MODEL);
    // Always call CollisionBounds after you set the model
    SetCollisionBounds(VEC_HULL_MIN, VEC_HULL_MAX);
    SetGhostBodyGroup(mom_replay_ghost_bodygroup.GetInt());
    UpdateModelScale();
    SetViewOffset(VEC_VIEW_SCALED(this));
}
void CMomentumGhostBaseEntity::Think()
{
    // update color, bodygroup, and other params if they change
    if (mom_replay_ghost_bodygroup.GetInt() != m_iBodyGroup)
    {
        SetGhostBodyGroup(mom_replay_ghost_bodygroup.GetInt());
    }
    if (m_GhostColor != m_NewGhostColor)
    {
        m_GhostColor = m_NewGhostColor;
        SetRenderColor(m_GhostColor.r(), m_GhostColor.g(), m_GhostColor.b(), m_GhostColor.a());
    }
}
void CMomentumGhostBaseEntity::SetGhostBodyGroup(int bodyGroup)
{
    if (bodyGroup > ghostModelBodyGroup::LAST || bodyGroup < 0)
    {
        Warning("CMomentumGhostBaseEntity::SetGhostBodyGroup() Error: Could not set bodygroup!");
    }
    else
    {
        m_iBodyGroup = bodyGroup;
        SetBodygroup(1, bodyGroup);
        mom_replay_ghost_bodygroup.SetValue(bodyGroup);
    }
}
void CMomentumGhostBaseEntity::SetGhostColorCC(const CCommand &args)
{
    Color *pColor = g_pMomentumUtil->GetColorFromHex(args.ArgS());
    if (pColor)
    {
        m_NewGhostColor = *pColor;
    }
}
void CMomentumGhostBaseEntity::SetGhostModel(const char *newmodel)
{
    if (newmodel)
    {
        Q_strncpy(m_pszModel, newmodel, sizeof(m_pszModel));
        PrecacheModel(m_pszModel);
        SetModel(m_pszModel);
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

