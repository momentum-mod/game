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

static ConVar strafeoffset_draw("mom_strafeoffset_draw", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
                                "Toggles displaying the strafeoffset data. (1 = only timer , 2 = always (except practice mode))\n",
                                true, 0, true, 2);

class CHudStrafeOffset : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudStrafeOffset, CHudNumericDisplay);
    
    CHudStrafeOffset(const char *pElementName);
    bool ShouldDraw() OVERRIDE;
    void Paint() OVERRIDE;
    
    //Numerical offset history.
    // 0 = current, 1-4 = history, with 4 being oldest
    CUtlVectorFixed<int, 5> m_vecHistInts;
    
    float m_fAvgOffset;
    float m_fMovingAvg;
    int m_nOffsetCt;
    
    int m_NormFontY;
    int m_SmallFontY;

    CPanelAnimationVar(float, m_flHistOffset, "m_flHistOffset", "0.0");
    
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE
    {
        if (!Q_strcmp(pEvent->GetName(), "strafe_offset"))
        {
            C_MomentumReplayGhostEntity *pGhost = m_pPlayer->GetReplayEnt();
            int offset = pGhost ? pGhost->m_SrvData.m_strafeOffset : m_pPlayer->m_SrvData.m_strafeOffset;

            //g_pClientMode->GetViewportAnimationController()->CancelAnimationsForPanel(this);
            //g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("NewStrafeOffset");
            m_pController->RunAnimationCommand(this, "m_flHistOffset", 24.0, 0.0, 0.08, AnimationController::INTERPOLATOR_DEACCEL, 0);
            float avgTemp = static_cast<float>(m_nOffsetCt) * m_fAvgOffset;
            ++m_nOffsetCt;
            m_fAvgOffset = (avgTemp + static_cast<float>(abs(offset))) / static_cast<float>(m_nOffsetCt);
            m_fMovingAvg = static_cast<float>(m_vecHistInts[0] + m_vecHistInts[1] + m_vecHistInts[2] + m_vecHistInts[3] + offset) / min(5.0, (float) m_nOffsetCt);
            
            // Discard oldest history
            m_vecHistInts.Remove(4);
            // Add current offset to history
            m_vecHistInts.AddToHead(offset);
        }
        else //Must be timer_state
        {
            if (pEvent->GetBool("is_running"))
                Reset();
        }
    }
    
    void PaintOffset(int offset, int index);
    void Reset() OVERRIDE;
    void OnThink() OVERRIDE{ if (m_pController) m_pController->UpdateAnimations(engine->Time()); }
    
    C_MomentumPlayer *m_pPlayer;
    AnimationController *m_pController;
};

DECLARE_NAMED_HUDELEMENT(CHudStrafeOffset, HudStrafeOffset);

CHudStrafeOffset::CHudStrafeOffset(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudStrafeOffset"),
    m_NormFontY(0), m_vecHistInts(0, 5)
{
    m_pController = new AnimationController(this);
    ListenForGameEvent("strafe_offset");
    ListenForGameEvent("timer_state");
    for (int i = 0; i < 5; i++)
        m_vecHistInts.AddToHead(0);
    m_NormFontY = (GetTall() - surface()->GetFontTall(m_hNumberFont)) / 2;
}

void CHudStrafeOffset::Reset()
{
    m_vecHistInts.RemoveAll();
    for (int i = 0; i < 5; i++)
        m_vecHistInts.AddToHead(0);
    m_fAvgOffset = 0;
    m_fMovingAvg = 0;
    m_nOffsetCt = 0;
}

bool CHudStrafeOffset::ShouldDraw()
{
    if (!strafeoffset_draw.GetInt())
        return false;

    if(!m_pPlayer)
    {
        m_pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        if (!m_pPlayer)
            return false;
    }
    
    C_MomentumReplayGhostEntity *pGhost = m_pPlayer->GetReplayEnt();
    C_MOMRunEntityData *pData = pGhost ? &pGhost->m_SrvData.m_RunData : &m_pPlayer->m_SrvData.m_RunData;

    return CHudElement::ShouldDraw() && (strafeoffset_draw.GetInt() == 2 || pData->m_bTimerRunning);
}

void CHudStrafeOffset::PaintOffset(int offset, int indx)
{
    wchar_t uOffset[64];
    V_snwprintf(uOffset, 64, L"%i", m_vecHistInts[indx]);
    int xpos = (GetWide() - UTIL_ComputeStringWidth(m_hNumberFont, uOffset)) / 2;
    surface()->DrawSetTextPos(xpos - offset - static_cast<int>(m_flHistOffset), m_NormFontY);
    surface()->DrawSetTextColor(100, 120, 140, 255 - (30 * indx));
    surface()->DrawPrintText(uOffset, wcslen(uOffset));
}

void CHudStrafeOffset::Paint()
{
    int histStep = 44;
    int histOffset = 0;
    
    surface()->DrawSetTextFont(m_hNumberFont);
    surface()->DrawSetTextColor(100, 120, 140, 255); // MOM_TODO: Change this to be defined in HudLayout
    PaintOffset(0, 0);
    histOffset += histStep;
    
    char avgOffsetStr[6], movingAvgOffsetStr[6]; //3 digits, a '.', a '-', and a null
    wchar_t uavgOffsetStr[6]{null}, umovingAvgOffsetStr[6]{null};
    Q_snprintf(avgOffsetStr, sizeof(avgOffsetStr), "%.2f", m_fAvgOffset);
    Q_snprintf(movingAvgOffsetStr, sizeof(movingAvgOffsetStr), "%.2f", m_fMovingAvg);
    ANSI_TO_UNICODE(avgOffsetStr, uavgOffsetStr);
    ANSI_TO_UNICODE(movingAvgOffsetStr, umovingAvgOffsetStr);
    
    int avgwidth = UTIL_ComputeStringWidth(m_hNumberFont, uavgOffsetStr);
    int movingavgwidth = UTIL_ComputeStringWidth(m_hNumberFont, umovingAvgOffsetStr);
    
    surface()->DrawSetTextPos((GetWide() - movingavgwidth) / 2 + 64, m_NormFontY);
    surface()->DrawPrintText(umovingAvgOffsetStr, wcslen(umovingAvgOffsetStr));
    surface()->DrawSetTextPos((GetWide() - avgwidth) / 2 + 144, m_NormFontY);
    surface()->DrawPrintText(uavgOffsetStr, Q_wcslen(uavgOffsetStr));
    
    for (int i = 0; i < 5; i++)
    {
        PaintOffset(histOffset, i);
        histOffset += histStep;
    }
}