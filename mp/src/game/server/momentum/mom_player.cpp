#include "cbase.h"
#include "mom_player.h"
#include "mom_triggers.h"
#include "in_buttons.h"
#include "Timer.h"

#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CMomentumPlayer, DT_MOM_Player)
SendPropInt(SENDINFO(m_iShotsFired)),
SendPropInt(SENDINFO(m_iDirection)),
SendPropBool(SENDINFO(m_bResumeZoom)),
SendPropInt(SENDINFO(m_iLastZoom)),
SendPropBool(SENDINFO(m_bAutoBhop)),
SendPropBool(SENDINFO(m_bDidPlayerBhop)),
SendPropBool(SENDINFO(m_iSuccessiveBhops)),
SendPropBool(SENDINFO(m_bPlayerInsideStartZone)),
SendPropBool(SENDINFO(m_bPlayerInsideEndZone)),
SendPropBool(SENDINFO(m_bHasPracticeMode)),
SendPropBool(SENDINFO(m_bPlayerFinishedMap)),
SendPropFloat(SENDINFO(m_flStrafeSync)),
SendPropFloat(SENDINFO(m_flStrafeSync2)),
SendPropFloat(SENDINFO(m_flLastJumpVel)),
SendPropBool(SENDINFO(m_bTimerIsRunning)),
END_SEND_TABLE()

BEGIN_DATADESC(CMomentumPlayer)
DEFINE_THINKFUNC(CheckForBhop),
DEFINE_THINKFUNC(CalculateStrafeSync),
END_DATADESC()

LINK_ENTITY_TO_CLASS(player, CMomentumPlayer);
PRECACHE_REGISTER(player);

CMomentumPlayer::CMomentumPlayer()
{
    m_flPunishTime = -1;
    m_iLastBlock = -1;
}

CMomentumPlayer::~CMomentumPlayer() {}

void CMomentumPlayer::Precache()
{
    // Name of our entity's model
#define ENTITY_MODEL "models/gibs/airboat_broken_engine.mdl"
    PrecacheModel(ENTITY_MODEL);

    BaseClass::Precache();
}

void CMomentumPlayer::Spawn()
{
    SetModel(ENTITY_MODEL);
    BaseClass::Spawn();
    AddFlag(FL_GODMODE);
    // do this here because we can't get a local player in the timer class
    ConVarRef gm("mom_gamemode");
    switch (gm.GetInt())
    {
    case MOMGM_BHOP:
    case MOMGM_SURF:
    case MOMGM_UNKNOWN:
    default:
        EnableAutoBhop();
        break;
    case MOMGM_SCROLL:
        DisableAutoBhop();
        break;
    }
    SetNextThink(gpGlobals->curtime);
    RegisterThinkContext("THINK_EVERY_TICK");
    RegisterThinkContext("CURTIME");
    SetContextThink(&CMomentumPlayer::CalculateStrafeSync, gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
    SetContextThink(&CMomentumPlayer::CheckForBhop, gpGlobals->curtime, "CURTIME");
}

void CMomentumPlayer::SurpressLadderChecks(const Vector &pos, const Vector &normal)
{
    m_ladderSurpressionTimer.Start(1.0f);
    m_lastLadderPos = pos;
    m_lastLadderNormal = normal;
}

bool CMomentumPlayer::CanGrabLadder(const Vector &pos, const Vector &normal)
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

CBaseEntity *CMomentumPlayer::EntSelectSpawnPoint()
{
    CBaseEntity *pStart;
    pStart = NULL;
    if (SelectSpawnSpot("info_player_counterterrorist", pStart))
    {
        return pStart;
    }
    else if (SelectSpawnSpot("info_player_terrorist", pStart))
    {
        return pStart;
    }
    else if (SelectSpawnSpot("info_player_start", pStart))
    {
        return pStart;
    }
    else
    {
        DevMsg("No valid spawn point found.\n");
        return BaseClass::Instance(INDEXENT(0));
    }
}

bool CMomentumPlayer::SelectSpawnSpot(const char *pEntClassName, CBaseEntity *&pStart)
{
#define SF_PLAYER_START_MASTER 1
    pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    if (pStart == NULL) // skip over the null point
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    CBaseEntity *pLast;
    pLast = NULL;
    while (pStart != NULL)
    {
        if (g_pGameRules->IsSpawnPointValid(pStart, this))
        {
            if (pStart->HasSpawnFlags(SF_PLAYER_START_MASTER))
            {
                g_pLastSpawn = pStart;
                return true;
            }
        }
        pLast = pStart;
        pStart = gEntList.FindEntityByClassname(pStart, pEntClassName);
    }
    if (pLast)
    {
        g_pLastSpawn = pLast;
        pStart = pLast;
        return true;
    }

    return false;
}

void CMomentumPlayer::Touch(CBaseEntity *pOther)
{
    BaseClass::Touch(pOther);

    if (g_MOMBlockFixer->IsBhopBlock(pOther->entindex()))
        g_MOMBlockFixer->PlayerTouch(this, pOther);
}

void CMomentumPlayer::EnableAutoBhop()
{
    m_bAutoBhop = true;
    DevLog("Enabled autobhop\n");
}
void CMomentumPlayer::DisableAutoBhop()
{
    m_bAutoBhop = false;
    DevLog("Disabled autobhop\n");
}
void CMomentumPlayer::CheckForBhop()
{
	if (GetGroundEntity() != NULL)
	{
		m_flTicksOnGround += gpGlobals->interval_per_tick;
		// true is player is on ground for less than 10 ticks, false if they are on ground for more s
		m_bDidPlayerBhop = (m_flTicksOnGround < NUM_TICKS_TO_BHOP * gpGlobals->interval_per_tick) != 0;
		if (!m_bDidPlayerBhop)
			m_iSuccessiveBhops = 0;
		if (m_nButtons & IN_JUMP)
		{
			m_flLastJumpVel = GetLocalVelocity().Length2D();
			m_iSuccessiveBhops++;
		}
    }
    else
        m_flTicksOnGround = 0;
    SetNextThink(gpGlobals->curtime, "CURTIME");
}
void CMomentumPlayer::CalculateStrafeSync()
{
    float velocity = GetLocalVelocity().Length2DSqr();
    if (!(GetFlags() & (FL_ONGROUND | FL_INWATER)) && GetMoveType() != MOVETYPE_LADDER)
    {
        if (EyeAngles().y > m_qangLastAngle.y) //player turned left 
        {
            m_nStrafeTicks++;
            if ((m_nButtons & IN_MOVELEFT) && !(m_nButtons & IN_MOVERIGHT))
                m_nPerfectSyncTicks++;
            if (velocity > m_flLastVelocity)
                m_nAccelTicks++;
        }
        else if (EyeAngles().y < m_qangLastAngle.y) //player turned right 
        {
            m_nStrafeTicks++;
            if ((m_nButtons & IN_MOVERIGHT) && !(m_nButtons & IN_MOVELEFT))
                m_nPerfectSyncTicks++;
            if (velocity > m_flLastVelocity)
                m_nAccelTicks++;
        }
    }
    if (m_nStrafeTicks && m_nAccelTicks && m_nPerfectSyncTicks)
    {
        m_flStrafeSync = ((float)m_nPerfectSyncTicks / (float)m_nStrafeTicks) * 100; // ticks strafing perfectly / ticks strafing
        m_flStrafeSync2 = ((float)m_nAccelTicks / (float)m_nStrafeTicks) * 100; // ticks gaining speed / ticks strafing
    }
    m_qangLastAngle = EyeAngles();
    m_flLastVelocity = velocity;
    //think once per tick   
    SetNextThink(gpGlobals->curtime + gpGlobals->interval_per_tick, "THINK_EVERY_TICK");
}
void CMomentumPlayer::ResetStrafeSync()
{
    m_nPerfectSyncTicks = 0;
    m_nStrafeTicks = 0;
    m_nAccelTicks = 0;
    m_flStrafeSync = 0;
    m_flStrafeSync2 = 0;
}
void CMomentumPlayer::Think()
{
    //USE: send info about timer to client class
    m_bTimerIsRunning = g_Timer.IsRunning();
    SetNextThink(gpGlobals->curtime);
}