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
#define SF_PLAYER_START_MASTER 1

    CBaseEntity *pStart = gEntList.FindEntityByClassname(NULL, "info_player_start");
    CBaseEntity *pStartFirst = pStart;
    while (pStart != NULL)
    {
        if (pStart->HasSpawnFlags(SF_PLAYER_START_MASTER))
        {
            g_pLastSpawn = pStart;
            return pStart;
        }

        pStart = gEntList.FindEntityByClassname(pStart, "info_player_start");
    }
    if (!pStartFirst)
    {
        DevMsg("PutClientInServer: no info_player_start on level\n");
        return CBaseEntity::Instance(INDEXENT(0));
    }
    else
    {
        g_pLastSpawn = pStartFirst;
        return pStartFirst;
    }
}


bool CMomentumPlayer::SelectSpawnSpot(const char *pEntClassName, CBaseEntity* &pSpot)
{
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