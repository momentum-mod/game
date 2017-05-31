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

#define HISTWIDTH 4

class CHudStrafeOffset : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudStrafeOffset, CHudNumericDisplay);
    
    CHudStrafeOffset(const char *pElementName);
    bool ShouldDraw() OVERRIDE;
    void Paint() OVERRIDE;
    void OnThink() OVERRIDE;
    
    //Text versions of history data, for rendering.
    char m_CurOffset[HISTWIDTH];
    char m_History1[HISTWIDTH];
    char m_History2[HISTWIDTH];
    char m_History3[HISTWIDTH];
    char m_History4[HISTWIDTH];
    
    //Numerical offset history.
    int m_nHist1, m_nHist2, m_nHist3, m_nHist4;
    
    float m_fAvgOffset;
    float m_fMovingAvg;
    int m_nOffsetCt;
    
    int m_NormFontY;
    int m_SmallFontY;
    
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE
    {
        if (!Q_strcmp(pEvent->GetName(), "strafe_offset"))
        {
            //g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel(this);
            //g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("NewStrafeOffset");
            //g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "HistOffset", 24.0, 0.0, 0.08, AnimationController::INTERPOLATOR_DEACCEL, 0);
            float avgTemp = (float)m_nOffsetCt * m_fAvgOffset;
            ++m_nOffsetCt;
            m_fAvgOffset = (avgTemp + (float)(abs(m_pPlayer->m_SrvData.m_strafeOffset))) / (float)m_nOffsetCt;
            m_fMovingAvg = (float)(m_nHist1 + m_nHist2 + m_nHist3 + m_nHist4 + m_pPlayer->m_SrvData.m_strafeOffset) / min(5.0, (float)m_nOffsetCt);
            
            m_nHist4 = m_nHist3;
            m_nHist3 = m_nHist2;
            m_nHist2 = m_nHist1;
            m_nHist1 = abs(m_pPlayer->m_SrvData.m_strafeOffset);
            
            memcpy(m_History4, m_History3, HISTWIDTH);
            memcpy(m_History3, m_History2, HISTWIDTH);
            memcpy(m_History2, m_History1, HISTWIDTH);
            memcpy(m_History1, m_CurOffset, HISTWIDTH);
            
            C_MomentumReplayGhostEntity *pGhost = m_pPlayer->GetReplayEnt();
            int offset = pGhost ? pGhost->m_SrvData.m_strafeOffset : m_pPlayer->m_SrvData.m_strafeOffset;
            Q_snprintf(m_CurOffset, HISTWIDTH, "%i", offset);
        }
        else //Must be timer_state
        {
            if (pEvent->GetBool("is_running"))
                Reset();
        }
    }
    
    void PaintOffset(int offset, char* str);
    void Reset();
    
    C_MomentumPlayer *m_pPlayer;
};

DECLARE_NAMED_HUDELEMENT(CHudStrafeOffset, HudStrafeOffset);

CHudStrafeOffset::CHudStrafeOffset(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudStrafeOffset"),
    m_NormFontY(0)
{
    ListenForGameEvent("strafe_offset");
    ListenForGameEvent("timer_state");
    m_History1[0] = 'z';
    m_History2[0] = 'z';
    m_History3[0] = 'z';
    m_History4[0] = 'z';
    m_NormFontY = (GetTall() - surface()->GetFontTall(m_hNumberFont)) / 2;
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

void CHudStrafeOffset::PaintOffset(int offset, char* str)
{
    wchar_t uOffset[HISTWIDTH];
    ANSI_TO_UNICODE(str, uOffset);
    int xpos = (GetWide() - UTIL_ComputeStringWidth(m_hNumberFont, uOffset)) / 2;
    surface()->DrawSetTextPos(xpos - offset, m_NormFontY);
    surface()->DrawPrintText(uOffset, HISTWIDTH);
}

void CHudStrafeOffset::Paint()
{
    if(!m_pPlayer)
    {
        m_pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return;
    }

    int histStep = 36;
    int histOffset = 0;
    
    surface()->DrawSetTextFont(m_hNumberFont);
    PaintOffset(0, m_CurOffset);
    histOffset += histStep;
    
    surface()->DrawSetTextColor(Color(100, 120, 140, 255));
    
    char avgOffsetStr[6], movingAvgOffsetStr[6]; //3 digits, a '.', a '-', and a null
    wchar_t uavgOffsetStr[6]{null}, umovingAvgOffsetStr[6]{null};
    Q_snprintf(avgOffsetStr, sizeof(avgOffsetStr), "%.2f", m_fAvgOffset);
    Q_snprintf(movingAvgOffsetStr, sizeof(movingAvgOffsetStr), "%.2f", m_fMovingAvg);
    ANSI_TO_UNICODE(avgOffsetStr, uavgOffsetStr);
    ANSI_TO_UNICODE(movingAvgOffsetStr, umovingAvgOffsetStr);
    
    int avgwidth = UTIL_ComputeStringWidth(m_hNumberFont, uavgOffsetStr);
    int movingavgwidth = UTIL_ComputeStringWidth(m_hNumberFont, umovingAvgOffsetStr);
    
    surface()->DrawSetTextPos((GetWide() - movingavgwidth) / 2 + 64, m_NormFontY);
    surface()->DrawPrintText(umovingAvgOffsetStr, 6);
    surface()->DrawSetTextPos((GetWide() - avgwidth) / 2 + 136, m_NormFontY);
    surface()->DrawPrintText(uavgOffsetStr, 6);
    
    //surface()->DrawSetTextColor(Color(100, 120, 140, 245));
    if (m_History1[0] != 'z')
    {
        PaintOffset(histOffset, m_History1);
    }
    else
        goto CLEANUP;
    histOffset += histStep;
    //surface()->DrawSetTextColor(Color(100, 120, 140, 220));
    if (m_History2[0] != 'z')
    {
        PaintOffset(histOffset, m_History2);
    }
    else
        goto CLEANUP;
    histOffset += histStep;
    //surface()->DrawSetTextColor(Color(100, 120, 140, 185));
    if (m_History3[0] != 'z')
    {
        PaintOffset(histOffset, m_History3);
    }
    else
        goto CLEANUP;
    histOffset += histStep;
    //surface()->DrawSetTextColor(Color(100, 120, 140, 110));
    if (m_History4[0] != 'z')
    {
        PaintOffset(histOffset, m_History4);
    }
    CLEANUP:
    histOffset = 0;
}