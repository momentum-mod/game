#pragma once

#include "cbase.h"
#include "mom_gamemovement.h"

class CMomentumGameMovement;
extern CMomentumGameMovement *g_pMomentumGameMovement;

class C_BaseMomentumTrigger : public C_BaseEntity
{
    DECLARE_CLASS(C_BaseMomentumTrigger, C_BaseEntity);

  public:
    void DrawOutlineOBBs(const Color &color);
};

class C_TriggerTimerStart : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStart, C_BaseEntity);
    DECLARE_CLIENTCLASS();
    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerTimerStop : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerTimerStop, C_BaseEntity);
    DECLARE_CLIENTCLASS();
    bool ShouldDraw(void) OVERRIDE;
    int DrawModel(int flags) OVERRIDE;
};

class C_TriggerSlide : public C_BaseMomentumTrigger
{
  public:
    DECLARE_CLASS(C_TriggerSlide, C_BaseEntity);
    DECLARE_CLIENTCLASS();
    CNetworkVar(bool, m_bTouching);

    void PostDataUpdate(DataUpdateType_t) OVERRIDE;
};