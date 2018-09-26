#pragma once

#include "cbase.h"

#define MAX_TRIGGER_NAME 128

class C_BaseMomentumTrigger : public C_BaseEntity
{
    DECLARE_CLASS(C_BaseMomentumTrigger, C_BaseEntity);

  public:

	C_BaseMomentumTrigger();

    void InitTrigger();
    void Spawn(void) OVERRIDE;
    bool PointIsWithin(const Vector &vecPoint);

    // Make networked?
    bool m_bDisabled;
    string_t m_iFilterName;
    CHandle<class CBaseFilter> m_hFilter;
};

class C_TriggerTimerStart : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();
    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerTimerStop : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();
    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerSlide : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();

    CNetworkVar(bool, m_bStuckOnGround);
    CNetworkVar(bool, m_bAllowingJump);
    CNetworkVar(bool, m_bDisableGravity);
    CNetworkVar(bool, m_bFixUpsideSlope);
};

class C_TriggerTeleport : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTeleport, C_BaseMomentumTrigger);
    DECLARE_CLIENTCLASS();

    void Spawn(void) OVERRIDE;
    void StartTouch(CBaseEntity *pOther) OVERRIDE;
    void Touch(CBaseEntity *pOther) OVERRIDE;

  private:
    CNetworkString(m_iszModel, MAX_TRIGGER_NAME);
};
