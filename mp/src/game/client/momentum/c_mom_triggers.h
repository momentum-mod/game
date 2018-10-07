#ifndef _C_MOM_TRIGGERS_H_
#define _C_MOM_TRIGGERS_H_

#ifdef WIN32
#pragma once
#endif

#include "cbase.h"
#include "mom_basetrigger.h"
#include "mom_basefilter.h"
#include "prediction.h"

#include "mom_entityoutput.h"

class C_TriggerTimerStart : public CBaseTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, CBaseTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerTimerStop : public CBaseTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, CBaseTrigger);
    DECLARE_CLIENTCLASS();

    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerSlide : public CBaseTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, CBaseTrigger);
    DECLARE_CLIENTCLASS();

    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
    CNetworkVar(bool, m_bFixUpsideSlope);
};

class C_TriggerTeleport : public CBaseTrigger
{
public:
	DECLARE_CLASS(C_TriggerTeleport, CBaseTrigger);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	void Touch(CBaseEntity *pOther) OVERRIDE;

	CNetworkVar(unsigned int, m_iLandmarkCRC);
};

#endif