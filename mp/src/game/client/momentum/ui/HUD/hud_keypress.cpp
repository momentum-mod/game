#include "cbase.h"
#include "hud_macros.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "in_buttons.h"
#include "input.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

#include "c_mom_replay_entity.h"
#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"

#include "tier0/memdbgon.h"

#define KEYDRAW_MIN 0.07f

using namespace vgui;

static ConVar showkeys("mom_showkeypresses", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                       "Toggles showing keypresses and strafe/jump counter\n");

class CHudKeyPressDisplay : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(CHudKeyPressDisplay, Panel);

    CHudKeyPressDisplay(const char *pElementName);

    bool ShouldDraw() override
    {
        C_MomentumPlayer *pMom = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
        // don't show during map finished dialog
        return showkeys.GetBool() && pMom && !pMom->m_RunData.m_bMapFinished && CHudElement::ShouldDraw();
    }

    void OnThink() override;
    void Paint() override;
    void Init() override;
    void Reset() override;
    void DrawKeyTemplates();

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        SetBgColor(Color(0, 0, 0, 1)); // empty background, 1 alpha (out of 255) so game text doesnt obscure our text
    }

  protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVar(HFont, m_hWordTextFont, "WordTextFont", "Default");
    CPanelAnimationVar(HFont, m_hCounterTextFont, "CounterTextFont", "Default");
    CPanelAnimationVar(Color, m_Normal, "KeyPressedColor", "Panel.Fg");
    CPanelAnimationVar(Color, m_darkGray, "KeyOutlineColor", "Dark Gray");
    CPanelAnimationVarAliasType(float, top_row_ypos, "top_row_ypos", "5", "proportional_float");
    CPanelAnimationVarAliasType(float, mid_row_ypos, "mid_row_ypos", "20", "proportional_float");
    CPanelAnimationVarAliasType(float, lower_row_ypos, "lower_row_ypos", "35", "proportional_float");
    CPanelAnimationVarAliasType(float, jump_row_ypos, "jump_row_ypos", "60", "proportional_float");
    CPanelAnimationVarAliasType(float, duck_row_ypos, "duck_row_ypos", "70", "proportional_float");
    CPanelAnimationVarAliasType(float, strafe_count_xpos, "strafe_count_xpos", "80", "proportional_float");
    CPanelAnimationVarAliasType(float, jump_count_xpos, "jump_count_xpos", "80", "proportional_float");

  private:
    int GetTextCenter(HFont font, wchar_t *wstring);

    int m_nButtons, m_nStrafes, m_nJumps;
    bool m_bShouldDrawCounts;
    wchar_t m_pwFwd[BUFSIZESHORT];
    wchar_t m_pwLeft[BUFSIZESHORT];
    wchar_t m_pwBack[BUFSIZESHORT];
    wchar_t m_pwRight[BUFSIZESHORT];
    wchar_t m_pwJump[BUFSIZELOCL];
    wchar_t m_pwDuck[BUFSIZELOCL];
    float m_fJumpColorUntil;
    float m_fDuckColorUntil;
};

DECLARE_HUDELEMENT(CHudKeyPressDisplay);

CHudKeyPressDisplay::CHudKeyPressDisplay(const char *pElementName)
    : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudKeyPressDisplay")
{
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}
void CHudKeyPressDisplay::Init()
{
    // init wchar with display values
    wcscpy(m_pwFwd, L"W");
    wcscpy(m_pwLeft, L"A");
    wcscpy(m_pwBack, L"S");
    wcscpy(m_pwRight, L"D");

    // localize jump and duck strings
    FIND_LOCALIZATION(m_pwJump, "#MOM_Jump");
    FIND_LOCALIZATION(m_pwDuck, "#MOM_Duck");

    m_fJumpColorUntil = m_fDuckColorUntil = 0;

    m_nButtons = 0;
}
void CHudKeyPressDisplay::Paint()
{
    // first, semi-transparent key templates
    DrawKeyTemplates();
    // then, color the key in if it's pressed
    surface()->DrawSetTextColor(m_Normal);
    surface()->DrawSetTextFont(m_hTextFont);
    if (m_nButtons & IN_FORWARD)
    {
        surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwFwd), top_row_ypos);
        surface()->DrawPrintText(m_pwFwd, wcslen(m_pwFwd));
    }
    if (m_nButtons & IN_MOVELEFT)
    {
        int text_left = GetTextCenter(m_hTextFont, m_pwLeft) - UTIL_ComputeStringWidth(m_hTextFont, m_pwLeft);
        surface()->DrawSetTextPos(text_left, mid_row_ypos);
        surface()->DrawPrintText(m_pwLeft, wcslen(m_pwLeft));
    }
    if (m_nButtons & IN_BACK)
    {
        surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwBack), lower_row_ypos);
        surface()->DrawPrintText(m_pwBack, wcslen(m_pwBack));
    }
    if (m_nButtons & IN_MOVERIGHT)
    {
        int text_right = GetTextCenter(m_hTextFont, m_pwRight) + UTIL_ComputeStringWidth(m_hTextFont, m_pwRight);
        surface()->DrawSetTextPos(text_right, mid_row_ypos);
        surface()->DrawPrintText(m_pwRight, wcslen(m_pwRight));
    }
    // reset text font for jump/duck
    surface()->DrawSetTextFont(m_hWordTextFont);

    if (m_nButtons & IN_JUMP || gpGlobals->curtime < m_fJumpColorUntil)
    {
        if (m_nButtons & IN_JUMP)
        {
            m_fJumpColorUntil = gpGlobals->curtime + KEYDRAW_MIN;
        }
        surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, m_pwJump), jump_row_ypos);
        surface()->DrawPrintText(m_pwJump, wcslen(m_pwJump));
    }
    if (m_nButtons & IN_DUCK || gpGlobals->curtime < m_fDuckColorUntil)
    {
        if (m_nButtons & IN_DUCK)
        {
            m_fDuckColorUntil = gpGlobals->curtime + KEYDRAW_MIN;
        }
        surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, m_pwDuck), duck_row_ypos);
        surface()->DrawPrintText(m_pwDuck, wcslen(m_pwDuck));
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

        surface()->DrawSetTextPos(jump_count_xpos, jump_row_ypos);
        surface()->DrawPrintText(jumps, wcslen(jumps));
    }
}
void CHudKeyPressDisplay::OnThink()
{
    CMomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        C_MomentumReplayGhostEntity *pReplayEnt = pPlayer->GetReplayEnt();
        if (pReplayEnt)
        {
            m_bShouldDrawCounts = pReplayEnt->m_RunData.m_bTimerRunning;
            m_nButtons = pReplayEnt->m_nReplayButtons;
            m_nStrafes = pReplayEnt->m_iTotalStrafes;
            m_nJumps = pReplayEnt->m_iTotalJumps;
        }
        else
        {
            m_nButtons = ::input->GetButtonBits(engine->IsPlayingDemo());
            if (g_MOMEventListener)
            {
                // we should only draw the strafe/jump counters when the timer is running
                m_bShouldDrawCounts = pPlayer->m_RunData.m_bTimerRunning;
                if (m_bShouldDrawCounts)
                {
                    CMomRunStats *stats = &pPlayer->m_RunStats;
                    m_nStrafes = stats->GetZoneStrafes(0);
                    m_nJumps = stats->GetZoneJumps(0);
                }
            }
        }
    }
}
void CHudKeyPressDisplay::Reset()
{
    // reset buttons member in case a button gets stuck
    m_nButtons = 0;
    m_fDuckColorUntil = 0;
    m_fJumpColorUntil = 0;
}
int CHudKeyPressDisplay::GetTextCenter(HFont font, wchar_t *wstring)
{
    return GetWide() / 2 - UTIL_ComputeStringWidth(font, wstring) / 2;
}
void CHudKeyPressDisplay::DrawKeyTemplates()
{
    // first draw all keys on screen in a dark gray
    surface()->DrawSetTextColor(m_darkGray);
    surface()->DrawSetTextFont(m_hTextFont);
    // fwd
    surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwFwd), top_row_ypos);
    surface()->DrawPrintText(m_pwFwd, wcslen(m_pwFwd));
    // left
    int text_left = GetTextCenter(m_hTextFont, m_pwLeft) - UTIL_ComputeStringWidth(m_hTextFont, m_pwLeft);
    surface()->DrawSetTextPos(text_left, mid_row_ypos);
    surface()->DrawPrintText(m_pwLeft, wcslen(m_pwLeft));
    // back
    surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwBack), lower_row_ypos);
    surface()->DrawPrintText(m_pwBack, wcslen(m_pwBack));
    // right
    int text_right = GetTextCenter(m_hTextFont, m_pwRight) + UTIL_ComputeStringWidth(m_hTextFont, m_pwRight);
    surface()->DrawSetTextPos(text_right, mid_row_ypos);
    surface()->DrawPrintText(m_pwRight, wcslen(m_pwRight));
    // reset text font for jump/duck
    surface()->DrawSetTextFont(m_hWordTextFont);
    // jump
    surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, m_pwJump), jump_row_ypos);
    surface()->DrawPrintText(m_pwJump, wcslen(m_pwJump));
    // duck
    surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, m_pwDuck), duck_row_ypos);
    surface()->DrawPrintText(m_pwDuck, wcslen(m_pwDuck));
}