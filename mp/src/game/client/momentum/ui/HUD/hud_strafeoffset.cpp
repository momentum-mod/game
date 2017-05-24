#include "cbase.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "c_mom_player.h"
#include "iclientmode.h"
#include "momentum/mom_player_shared.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudStrafeOffset : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudStrafeOffset, CHudNumericDisplay);
    
    CHudStrafeOffset(const char *pElementName);
    bool ShouldDraw() OVERRIDE;
    void Paint() OVERRIDE;
    void OnThink() OVERRIDE;
    
    char m_CurOffset[4];
    char m_History1[4];
    char m_History2[4];
    char m_History3[4];
    char m_History4[4];
    char nullMagic[2];
    
    int m_nHist1, m_nHist2, m_nHist3, m_nHist4;
    
    float m_fAvgOffset;
    float m_fMovingAvg;
    int m_nOffsetCt;
    
    int m_fontTall;
    
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE
    {
        if (!Q_strcmp(pEvent->GetName(), "strafe_offset"))
        {
            //g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel(this);
            //g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("NewStrafeOffset");
            //g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "HistOffset", 24.0, 0.0, 0.08, AnimationController::INTERPOLATOR_DEACCEL, 0);
            float avgTemp = (float)m_nOffsetCt * m_fAvgOffset;
            ++m_nOffsetCt;
            m_fAvgOffset = (avgTemp + (float)(abs(m_pPlayer->m_SrvData.m_strafeOffset)) / (float)m_nOffsetCt;
            m_fMovingAvg = (float)(m_nHist1 + m_nHist2 + m_nHist3 + m_nHist4 + m_pPlayer->m_SrvData.m_strafeOffset) / min(5.0, (float)m_nOffsetCt);
            
            m_nHist4 = m_nHist3;
            m_nHist3 = m_nHist2;
            m_nHist2 = m_nHist1;
            m_nHist1 = abs(m_pPlayer->m_SrvData.m_strafeOffset);
            
            memcpy(m_History4, m_History3, sizeof(m_CurOffset));
            memcpy(m_History3, m_History2, sizeof(m_CurOffset));
            memcpy(m_History2, m_History1, sizeof(m_CurOffset));
            memcpy(m_History1, m_CurOffset, sizeof(m_CurOffset));
            Q_snprintf(m_CurOffset, sizeof m_CurOffset, "%i", m_pPlayer->m_SrvData.m_strafeOffset);
        }
        else //Must be timer_state
        {
            if (pEvent->GetBool("is_running"))
                Reset();
        }
    }
    
    void PaintOffset(char* str, int size);
    void Reset();
    
    C_MomentumPlayer *m_pPlayer;
    
    CPanelAnimationVar(float, m_histOffset, "HistOffset", "0.0");
};

DECLARE_NAMED_HUDELEMENT(CHudStrafeOffset, HudStrafeOffset);

CHudStrafeOffset::CHudStrafeOffset(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudStrafeOffset"),
    m_fontTall(0), m_histOffset(0)
{
    ListenForGameEvent("strafe_offset");
    ListenForGameEvent("timer_state");
    nullMagic[0] = 'z';
    nullMagic[1] = null;
    m_History1[0] = 'z';
    m_History2[0] = 'z';
    m_History3[0] = 'z';
    m_History4[0] = 'z';
}

void CHudStrafeOffset::OnThink()
{

}

void CHudStrafeOffset::Reset()
{
    m_History1[0] = 'z';
    m_History2[0] = 'z';
    m_History3[0] = 'z';
    m_History4[0] = 'z';
    m_CurOffset[0] = '0';
    m_CurOffset[1] = '\0';
    m_nHist1 = 0, m_nHist2 = 0, m_nHist3 = 0, m_nHist4 = 0;
    m_fAvgOffset = 0;
    m_fMovingAvg = 0;
    m_nOffsetCt = 0;
}

bool CHudStrafeOffset::ShouldDraw()
{
    return (CHudElement::ShouldDraw());
}

void CHudStrafeOffset::PaintOffset(char* str, int size)
{
    wchar_t uOffset[4];
    ANSI_TO_UNICODE(str, uOffset);
    int xpos = (GetWide() - UTIL_ComputeStringWidth(m_hNumberFont, uOffset)) / 2;
    surface()->DrawSetTextPos(xpos - m_histOffset, m_fontTall);
    surface()->DrawPrintText(uOffset, 4);
}

void CHudStrafeOffset::Paint()
{
    if(!m_pPlayer)
    {
        m_pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return;
    }

    int histStep = 36;
    
    //if (!m_fontTall)
    m_fontTall = (GetTall() - surface()->GetFontTall(m_hNumberFont)) / 2;
    
    surface()->DrawSetTextFont(m_hNumberFont);
    surface()->DrawSetTextColor(Color(100, 120, 140, 255));
    
    char avgOffsetStr[6], movingAvgOffsetStr[6]; //3 digits, a '.', a '-', and a null
    wchar_t uavgOffsetStr[6]{null}, umovingAvgOffsetStr[6]{null};
    Q_snprintf(avgOffsetStr, sizeof(avgOffsetStr), "%.2f", m_fAvgOffset);
    Q_snprintf(movingAvgOffsetStr, sizeof(movingAvgOffsetStr), "%.2f", m_fMovingAvg);
    ANSI_TO_UNICODE(avgOffsetStr, uavgOffsetStr);
    ANSI_TO_UNICODE(movingAvgOffsetStr, umovingAvgOffsetStr);
    
    int avgwidth = UTIL_ComputeStringWidth(m_hNumberFont, uavgOffsetStr);
    int movingavgwidth = UTIL_ComputeStringWidth(m_hNumberFont, umovingAvgOffsetStr);
    
    surface()->DrawSetTextPos((GetWide() - movingavgwidth) / 2 + 64, m_fontTall);
    surface()->DrawPrintText(umovingAvgOffsetStr, 6);
    surface()->DrawSetTextPos((GetWide() - avgwidth) / 2 + 136, m_fontTall);
    surface()->DrawPrintText(uavgOffsetStr, 6);
    
    Q_snprintf(m_CurOffset, sizeof(m_CurOffset), "%i", m_pPlayer->m_SrvData.m_strafeOffset);
    PaintOffset(m_CurOffset, 4);
    m_histOffset += histStep;
    
    if (Q_strcmp(m_History1, nullMagic))
    {
        PaintOffset(m_History1, 4);
    }
    else
        goto CLEANUP;
    m_histOffset += histStep;
    if (Q_strcmp(m_History2, nullMagic))
    {
        PaintOffset(m_History2, 4);
    }
    else
        goto CLEANUP;
    m_histOffset += histStep;
    if (Q_strcmp(m_History3, nullMagic))
    {
        PaintOffset(m_History3, 4);
    }
    else
        goto CLEANUP;
    m_histOffset += histStep;
    if (Q_strcmp(m_History4, nullMagic))
    {
        PaintOffset(m_History4, 4);
    }
    CLEANUP:
    m_histOffset = 0;
}