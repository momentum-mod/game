#include "cbase.h"
#include "mom_gamerules.h"

#include "tier0/memdbgon.h"

REGISTER_GAMERULES_CLASS(CMomentum);


#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS(info_player_terrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_counterterrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_logo, CPointEntity);

CMomentum::CMomentum()
{

}

CMomentum::~CMomentum()
{

}

Vector CMomentum::DropToGround(
    CBaseEntity *pMainEnt,
    const Vector &vPos,
    const Vector &vMins,
    const Vector &vMaxs)
{
    trace_t trace;
    UTIL_TraceHull(vPos, vPos + Vector(0, 0, -500), vMins, vMaxs, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace);
    return trace.endpos;
}

CBaseEntity *CMomentum::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
    // gat valid spwan point
    CBaseEntity *pSpawnSpot = pPlayer->EntSelectSpawnPoint();

    // drop down to ground
    Vector GroundPos = DropToGround(pPlayer, pSpawnSpot->GetAbsOrigin(), VEC_HULL_MIN, VEC_HULL_MAX);

    // Move the player to the place it said.
    pPlayer->Teleport(&pSpawnSpot->GetAbsOrigin(), &pSpawnSpot->GetLocalAngles(), &vec3_origin);
    pPlayer->m_Local.m_vecPunchAngle = vec3_angle;

    return pSpawnSpot;
}

// checks if the spot is clear of players
bool CMomentum::IsSpawnPointValid(CBaseEntity *pSpot, CBasePlayer *pPlayer)
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

bool CMomentum::ClientCommand(CBaseEntity *pEdict, const CCommand &args)
{
    if (BaseClass::ClientCommand(pEdict, args))
        return true;

    CMomentumPlayer *pPlayer = (CMomentumPlayer *) pEdict;

    return pPlayer->ClientCommand(args);
}
#endif