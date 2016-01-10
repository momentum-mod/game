#include "cbase.h"
#include "mom_player.h"

#include "tier0/memdbgon.h"



IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropInt(SENDINFO(m_iShotsFired)),
SendPropInt(SENDINFO(m_iDirection)),
SendPropBool(SENDINFO(m_bResumeZoom)),
SendPropInt(SENDINFO(m_iLastZoom)),
END_SEND_TABLE()

BEGIN_DATADESC(CMomentumPlayer)

END_DATADESC()

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);

CMomentumPlayer::CMomentumPlayer()
{

}

CMomentumPlayer::~CMomentumPlayer()
{

}

void CMomentumPlayer::SurpressLadderChecks(const Vector& pos, const Vector& normal)
{
    m_ladderSurpressionTimer.Start(1.0f);
    m_lastLadderPos = pos;
    m_lastLadderNormal = normal;
}

bool CMomentumPlayer::CanGrabLadder(const Vector& pos, const Vector& normal)
{
    if (m_ladderSurpressionTimer.GetRemainingTime() <= 0.0f)
    {
        return true;
    }

    const float MaxDist = 64.0f;
    if (pos.AsVector2D().DistToSqr(m_lastLadderPos.AsVector2D()) < MaxDist * MaxDist)
    {
        return false;
    }

    if (normal != m_lastLadderNormal)
    {
        return true;
    }

    return false;
}

//MOM_TODO: clean this method up
CBaseEntity* CMomentumPlayer::EntSelectSpawnPoint()
{
    CBaseEntity *pSpot;
    pSpot = NULL;

    pSpot = g_pLastSpawn;
    if (SelectSpawnSpot("info_player_counterterrorist", pSpot))
    {
        g_pLastSpawn = pSpot;
        goto ReturnSpot;
    }
    else if (SelectSpawnSpot("info_player_terrorist", pSpot))
    {
        g_pLastSpawn = pSpot;
        goto ReturnSpot;
    }

    // If startspot is set, (re)spawn there.
    if (!gpGlobals->startspot || !strlen(STRING(gpGlobals->startspot)))
    {
        pSpot = gEntList.FindEntityByClassname(NULL, "info_player_terrorist");
        if (pSpot)
            goto ReturnSpot;
    }
    else
    {
        pSpot = gEntList.FindEntityByTarget(NULL, STRING(gpGlobals->startspot));
        if (pSpot)
            goto ReturnSpot;
    }

ReturnSpot:
    if (!pSpot)
    {
        Warning("PutClientInServer: no info_player_start on level\n");
        return CBaseEntity::Instance(INDEXENT(0));
    }

    return pSpot;
}


bool CMomentumPlayer::SelectSpawnSpot(const char *pEntClassName, CBaseEntity* &pSpot)
{
    // Find the next spawn spot.
    pSpot = gEntList.FindEntityByClassname(pSpot, pEntClassName);

    if (pSpot == NULL) // skip over the null point
        pSpot = gEntList.FindEntityByClassname(pSpot, pEntClassName);

    CBaseEntity *pFirstSpot = pSpot;
    do
    {
        if (pSpot)
        {
            // check if pSpot is valid
            if (g_pGameRules->IsSpawnPointValid(pSpot, this))
            {
                if (pSpot->GetAbsOrigin() == Vector(0, 0, 0))
                {
                    pSpot = gEntList.FindEntityByClassname(pSpot, pEntClassName);
                    continue;
                }

                // if so, go to pSpot
                return true;
            }
        }
        // increment pSpot
        pSpot = gEntList.FindEntityByClassname(pSpot, pEntClassName);
    } while (pSpot != pFirstSpot); // loop if we're not back to the start

    DevMsg("CCSPlayer::SelectSpawnSpot: couldn't find valid spawn point.\n");

    return false;
}

void CMomentumPlayer::KickBack(float up_base, float lateral_base, float up_modifier,
    float lateral_modifier, float up_max, float lateral_max, int direction_change)
{
    float flKickUp;
    float flKickLateral;

    if (m_iShotsFired == 1) // This is the first round fired
    {
        flKickUp = up_base;
        flKickLateral = lateral_base;
    }
    else
    {
        flKickUp = up_base + m_iShotsFired*up_modifier;
        flKickLateral = lateral_base + m_iShotsFired*lateral_modifier;
    }


    QAngle angle = GetPunchAngle();

    angle.x -= flKickUp;
    if (angle.x < -1 * up_max)
        angle.x = -1 * up_max;

    if (m_iDirection == 1)
    {
        angle.y += flKickLateral;
        if (angle.y > lateral_max)
            angle.y = lateral_max;
    }
    else
    {
        angle.y -= flKickLateral;
        if (angle.y < -1 * lateral_max)
            angle.y = -1 * lateral_max;
    }

    if (!SharedRandomInt("KickBack", 0, direction_change))
        m_iDirection = 1 - m_iDirection;

    SetPunchAngle(angle);
}