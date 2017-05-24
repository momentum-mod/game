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
    
    void FireGameEvent(IGameEvent *pEvent) OVERRIDE
    {
        if (!Q_strcmp(pEvent->GetName(), "strafe_offset"))
        {
            g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "HistOffset", 24.0, 0.0, 0.08, AnimationController::INTERPOLATOR_DEACCEL, 0);
        }
    }
    
    C_MomentumPlayer *m_pPlayer;
    
    char m_History1[4];
    char m_History2[4];
    char m_History3[4];
    char m_History4[4];
    
    CPanelAnimationVar(float, m_histOffset, "HistOffset", "0.0");
};

DECLARE_NAMED_HUDELEMENT(CHudStrafeOffset, HudStrafeOffset);

CHudStrafeOffset::CHudStrafeOffset(const char *pElementName)
    : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudStrafeOffset")
{
    ListenForGameEvent("strafe_offset");
}

void CHudStrafeOffset::OnThink()
{

}


bool CHudStrafeOffset::ShouldDraw()
{
    return (CHudElement::ShouldDraw());
}

void CHudStrafeOffset::Paint()
{
    if(!m_pPlayer)
    {
        m_pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return;
    }
    
    char cOffset[4];   //Enough to hold 2 digits, a "-" and a null
    wchar_t uOffset[4];
    Q_snprintf(cOffset, sizeof(cOffset), "%i", m_pPlayer->m_SrvData.m_strafeOffset);
    ANSI_TO_UNICODE(cOffset, uOffset);
    int xpos = (GetWide() - UTIL_ComputeStringWidth(m_hNumberFont, uOffset)) / 2;
    int ypos = (GetTall() - surface()->GetFontTall(m_hNumberFont)) / 2;
    surface()->DrawSetTextPos(xpos - m_histOffset, ypos);
    surface()->DrawSetTextFont(m_hNumberFont);
    surface()->DrawSetTextColor(Color(100, 120, 140, 255));
    surface()->DrawPrintText(uOffset, wcslen(uOffset));
}