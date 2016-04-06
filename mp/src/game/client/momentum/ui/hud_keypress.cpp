#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "in_buttons.h"
#include "input.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "mom_shareddefs.h"
#include "mom_player_shared.h"
#include "mom_event_listener.h"

using namespace vgui; 

static ConVar showkeys("mom_showkeypresses", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
    "Toggles showing keypresses on screen\n");
#define BUFSIZESHORT 10

class CHudKeyPressDisplay : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(CHudKeyPressDisplay, Panel);

public:
    CHudKeyPressDisplay();
    CHudKeyPressDisplay(const char *pElementName);
    virtual bool ShouldDraw()
    {
        return showkeys.GetBool() && !m_EventListener->m_bMapFinished; //don't show during map finished dialog
    }
    virtual void OnThink();
    virtual void Paint();
    virtual void Init();
    virtual void Reset();
    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetBgColor(Color::Color(0,0,0,1)); //empty background, 1 alpha (out of 255) so game text doesnt obscure our text
        SetFgColor(GetSchemeColor("MOM.Panel.Fg", pScheme));
    }
protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVar(HFont, m_hWordTextFont, "WordTextFont", "Default");
    CPanelAnimationVar(HFont, m_hCounterTextFont, "CounterTextFont", "Default");
    CPanelAnimationVarAliasType(float, top_row_ypos, "top_row_ypos", "5",
        "proportional_float");
    CPanelAnimationVarAliasType(float, mid_row_ypos, "mid_row_ypos", "20",
        "proportional_float");
    CPanelAnimationVarAliasType(float, lower_row_ypos, "lower_row_ypos", "35",
        "proportional_float");
    CPanelAnimationVarAliasType(float, bottom_row_ypos, "bottom_row_ypos", "50",
        "proportional_float");    
    CPanelAnimationVarAliasType(float, strafe_count_xpos, "strafe_count_xpos", "80",
        "proportional_float");
    CPanelAnimationVarAliasType(float, jump_count_xpos, "jump_count_xpos", "80",
        "proportional_float");
private:
    int m_nButtons, m_nStrafes, m_nJumps;
    bool m_bShouldDrawCounts; 
    wchar_t m_pwfwd[BUFSIZESHORT];
    wchar_t m_pwleft[BUFSIZESHORT];
    wchar_t m_pwback[BUFSIZESHORT];
    wchar_t m_pwright[BUFSIZESHORT];
    wchar_t m_pwjump[BUFSIZESHORT];
    wchar_t m_pwduck[BUFSIZESHORT];
    C_Momentum_EventListener *m_EventListener = new C_Momentum_EventListener();
};

DECLARE_HUDELEMENT(CHudKeyPressDisplay);

CHudKeyPressDisplay::CHudKeyPressDisplay(const char *pElementName) :
CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudKeyPressDisplay")
{
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}
void CHudKeyPressDisplay::Init()
{
    //init wchar with display values 
    wcscpy(m_pwfwd, L"W");
    wcscpy(m_pwleft, L"A");
    wcscpy(m_pwback, L"S");
    wcscpy(m_pwright, L"D");
    wcscpy(m_pwjump, L"JUMP");
    wcscpy(m_pwduck, L"DUCK");
}
void CHudKeyPressDisplay::Paint()
{
    surface()->DrawSetTextFont(m_hTextFont);

    if (m_nButtons & IN_FORWARD)
    {
        int text_center = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, m_pwfwd) / 2;  
        surface()->DrawSetTextPos(text_center, top_row_ypos);
        surface()->DrawPrintText(m_pwfwd, wcslen(m_pwfwd));
    }
    if (m_nButtons & IN_MOVELEFT)
    {
        int text_center = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, m_pwleft) / 2;
        int text_left = text_center - UTIL_ComputeStringWidth(m_hTextFont, m_pwleft);
        surface()->DrawSetTextPos(text_left, mid_row_ypos);
        surface()->DrawPrintText(m_pwleft, wcslen(m_pwleft));
    }
    if (m_nButtons & IN_BACK)
    {
        int text_center = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, m_pwback) / 2; 
        surface()->DrawSetTextPos(text_center, mid_row_ypos);
        surface()->DrawPrintText(m_pwback, wcslen(m_pwback));
    }
    if (m_nButtons & IN_MOVERIGHT)
    {
        int text_center = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, m_pwback) / 2;
        int text_right = text_center + UTIL_ComputeStringWidth(m_hTextFont, m_pwright);
        surface()->DrawSetTextPos(text_right, mid_row_ypos);
        surface()->DrawPrintText(m_pwright, wcslen(m_pwright));
    }
    //reset text font for jump/duck
    surface()->DrawSetTextFont(m_hWordTextFont);

    if (m_nButtons & IN_JUMP)
    {
        int text_center = GetWide() / 2 - UTIL_ComputeStringWidth(m_hWordTextFont, m_pwjump) / 2;
        surface()->DrawSetTextPos(text_center, lower_row_ypos);
        surface()->DrawPrintText(m_pwjump, wcslen(m_pwjump));
    }
    if (m_nButtons & IN_DUCK)
    {
        int text_center = GetWide() / 2 - UTIL_ComputeStringWidth(m_hWordTextFont, m_pwduck) / 2;
        surface()->DrawSetTextPos(text_center, bottom_row_ypos);
        surface()->DrawPrintText(m_pwduck, wcslen(m_pwduck));
    }
    // ---------- 
    if (m_bShouldDrawCounts)
    {
        surface()->DrawSetTextFont(m_hCounterTextFont);

        wchar_t strafes[BUFSIZESHORT];
        char cstr_strafes[BUFSIZESHORT];
        Q_snprintf(cstr_strafes, sizeof(cstr_strafes), "( %i )", m_nStrafes);
        g_pVGuiLocalize->ConvertANSIToUnicode(cstr_strafes, strafes, sizeof(strafes));

        surface()->DrawSetTextPos(strafe_count_xpos, mid_row_ypos);
        surface()->DrawPrintText(strafes, wcslen(strafes));

        wchar_t jumps[BUFSIZESHORT];
        char cstr_jumps[BUFSIZESHORT];
        Q_snprintf(cstr_jumps, sizeof(cstr_jumps), "( %i )", m_nJumps);
        g_pVGuiLocalize->ConvertANSIToUnicode(cstr_jumps, jumps, sizeof(jumps));

        surface()->DrawSetTextPos(jump_count_xpos, lower_row_ypos);
        surface()->DrawPrintText(jumps, wcslen(jumps));
    }
}
void CHudKeyPressDisplay::OnThink()
{
    m_nButtons = ::input->GetButtonBits(0);
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        m_nStrafes = pPlayer->m_nTotalStrafes;
        m_nJumps = pPlayer->m_nTotalJumps;
    }
    if (m_EventListener)
    {   //we should only draw the strafe/jump counters when the timer is running
        m_bShouldDrawCounts = m_EventListener->m_bTimerIsRunning;
    }
}
void CHudKeyPressDisplay::Reset()
{
    //reset buttons member in case a button gets stuck
    m_nButtons = NULL;
}