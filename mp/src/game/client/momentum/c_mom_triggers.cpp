#include "cbase.h"
#include "c_mom_triggers.h"
#include "mom_enginedraw.h"
#include "util/mom_util.h"

#include "tier0/memdbgon.h"

static ConVar mom_startzone_outline_enable("mom_startzone_outline_enable", "1",
                                           FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                           "Enable outline for start zone.");

static ConVar mom_endzone_outline_enable("mom_endzone_outline_enable", "1", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                         "Enable outline for end zone.");

static ConVar mom_startzone_color("mom_startzone_color", "00FF00FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                  "Color of the start zone.");

static ConVar mom_endzone_color("mom_endzone_color", "FF0000FF", FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                "Color of the end zone.");

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_start, C_TriggerTimerStart);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStart, DT_TriggerTimerStart, CTriggerTimerStart)
END_RECV_TABLE();

LINK_ENTITY_TO_CLASS(trigger_momentum_timer_stop, C_TriggerTimerStop);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerTimerStop, DT_TriggerTimerStop, CTriggerTimerStop)
END_RECV_TABLE();

bool C_TriggerTimerStart::ShouldDraw() { return true; }

int C_TriggerTimerStart::DrawModel(int flags)
{
    if (mom_startzone_outline_enable.GetBool())
    {
        Color color_startzone;
        if (g_pMomentumUtil->GetColorFromHex(mom_startzone_color.GetString(), color_startzone))
        {
            MOM_DrawBrushOutlineModel(this, color_startzone);

            if (IsEffectActive(EF_NODRAW))
                return 1;
        }
    }

    return BaseClass::DrawModel(flags);
}

bool C_TriggerTimerStop::ShouldDraw() { return true; }

int C_TriggerTimerStop::DrawModel(int flags)
{
    if (mom_endzone_outline_enable.GetBool())
    {
        Color color_endzone;
        if (g_pMomentumUtil->GetColorFromHex(mom_endzone_color.GetString(), color_endzone))
        {
            MOM_DrawBrushOutlineModel(this, color_endzone);

            if (IsEffectActive(EF_NODRAW))
                return 1;
        }
    }

    return BaseClass::DrawModel(flags);
}

LINK_ENTITY_TO_CLASS(trigger_momentum_slide, C_TriggerSlide);

IMPLEMENT_CLIENTCLASS_DT(C_TriggerSlide, DT_TriggerSlide, CTriggerSlide)
RecvPropBool(RECVINFO(m_bStuckOnGround)),
RecvPropBool(RECVINFO(m_bAllowingJump)),
RecvPropBool(RECVINFO(m_bDisableGravity)),
RecvPropBool(RECVINFO(m_bFixUpsideSlope)),
END_RECV_TABLE();
