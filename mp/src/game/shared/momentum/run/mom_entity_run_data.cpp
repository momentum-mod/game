#include "cbase.h"
#include "mom_entity_run_data.h"

#include "tier0/memdbgon.h"

CMOMRunEntityData::CMOMRunEntityData()
{
    m_bAutoBhop = false;
    m_iSuccessiveBhops = 0;
    m_flStrafeSync = 0.0f;
    m_flStrafeSync2 = 0.0f;
    m_flLastJumpVel = 0.0f;
    m_flLastJumpTime = 0.0f;
    m_iRunFlags = 0;
    m_bIsInZone = false;
    m_iCurrentZone = 0;
    m_iStartTick = -1;
	m_iStartTickD = -1;
    m_bMapFinished = false;
    m_bTimerRunning = false;
    m_flRunTime = 0.0f;
}