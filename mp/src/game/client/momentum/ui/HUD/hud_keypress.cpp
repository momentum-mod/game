#include "cbase.h"

#include "hudelement.h"
#include "iclientmode.h"
#include "in_buttons.h"
#include "input.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

#include "c_mom_online_ghost.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "mom_system_gamemode.h"

#include "tier0/memdbgon.h"

#define KEYDRAW_MIN 0.07f

#define STR_M1      L"A1"
#define STR_M2      L"A2"
#define STR_JUMP    L"JUMP"
#define STR_DUCK    L"DUCK"
#define STR_WALK    L"WALK"
#define STR_SPEED   L"SPEED"

using namespace vgui;

extern ConVar cl_yawspeed;
static ConVar showkeys("mom_hud_showkeypresses", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_REPLICATED,
                       "Toggles showing keypresses and strafe/jump counter\n");

class CHudKeyPressDisplay : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(CHudKeyPressDisplay, Panel);

    CHudKeyPressDisplay(const char *pElementName);

    bool ShouldDraw() OVERRIDE;
    void OnThink() OVERRIDE;
    void Paint() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void DrawKeyTemplates();

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
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
    CPanelAnimationVar(Color, m_Disabled, "KeyDisabledColor", "Red");
    CPanelAnimationVarAliasType(float, top_row_ypos, "top_row_ypos", "5", "proportional_float");
    CPanelAnimationVarAliasType(float, mid_row_ypos, "mid_row_ypos", "20", "proportional_float");
    CPanelAnimationVarAliasType(float, lower_row_ypos, "lower_row_ypos", "35", "proportional_float");
    CPanelAnimationVarAliasType(float, jump_row_ypos, "jump_row_ypos", "60", "proportional_float");
    CPanelAnimationVarAliasType(float, duck_row_ypos, "duck_row_ypos", "70", "proportional_float");
    CPanelAnimationVarAliasType(float, walk_row_ypos, "walk_row_ypos", "80", "proportional_float");
    CPanelAnimationVarAliasType(float, sprint_row_ypos, "sprint_row_ypos", "90", "proportional_float");
    CPanelAnimationVarAliasType(float, strafe_count_xpos, "strafe_count_xpos", "80", "proportional_float");
    CPanelAnimationVarAliasType(float, jump_count_xpos, "jump_count_xpos", "80", "proportional_float");

  private:
    int GetTextCenter(HFont font, wchar_t *wstring);

    bool m_bIsDucked;
    int m_nButtons, m_nDisabledButtons, m_nJumps;
    uint32 m_nStrafes;
    bool m_bShouldDrawCounts;
    wchar_t m_pwFwd[BUFSIZESHORT];
    wchar_t m_pwLeft[BUFSIZESHORT];
    wchar_t m_pwBack[BUFSIZESHORT];
    wchar_t m_pwRight[BUFSIZESHORT];
    wchar_t m_pwStrafe[BUFSIZELOCL];
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
    SetHiddenBits(HIDEHUD_LEADERBOARDS);
}

bool CHudKeyPressDisplay::ShouldDraw()
{
    if (!showkeys.GetBool())
        return false;
    // don't show during map finished dialog
    bool shouldDrawLocal = false;
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
        shouldDrawLocal = !pPlayer->GetCurrentUIEntData()->m_bMapFinished;
    return shouldDrawLocal && CHudElement::ShouldDraw();
}

void CHudKeyPressDisplay::Init()
{
    // init wchar with display values
    wcscpy(m_pwFwd, L"W");
    wcscpy(m_pwLeft, L"A");
    wcscpy(m_pwBack, L"S");
    wcscpy(m_pwRight, L"D");
    wcscpy(m_pwStrafe, L"O");

    m_fJumpColorUntil = m_fDuckColorUntil = 0;

    m_nButtons = 0;
    m_nDisabledButtons = 0;
}

// Checks to see if this input was blocked, and if so, paint it red.
#define CHECK_INPUT(flag, color) surface()->DrawSetTextColor((m_nDisabledButtons & flag) ? m_Disabled : color)
// Check for pressed input
#define CHECK_INPUT_P(flag) CHECK_INPUT(flag, m_Normal)
// Check for not pressed input
#define CHECK_INPUT_N(flag) CHECK_INPUT(flag, m_darkGray)

void CHudKeyPressDisplay::Paint()
{
    // create local variable so we can mutate it without worry
    int nButtons = m_nButtons;
    // do we need to invert the +left/+right due to negative yawspeed?
    if (cl_yawspeed.GetInt() < 0 && !(nButtons & IN_STRAFE))
    {
        // in_right = in_left
        nButtons = (m_nButtons & IN_LEFT) ? (nButtons | IN_RIGHT) : (nButtons & (~IN_RIGHT));
        // in_left = in_right
        nButtons = (m_nButtons & IN_RIGHT) ? (nButtons | IN_LEFT) : (nButtons & (~IN_LEFT));
    }
    else if (cl_yawspeed.GetInt() == 0)
    {
        nButtons = nButtons & (~(IN_LEFT | IN_RIGHT));
    }
    // first, semi-transparent key templates
    DrawKeyTemplates();
    // then, color the key in if it's pressed
    surface()->DrawSetTextColor(m_Normal);
    surface()->DrawSetTextFont(m_hTextFont);
    if (nButtons & IN_FORWARD)
    {
        CHECK_INPUT_P(IN_FORWARD);
        surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwFwd), top_row_ypos);
        surface()->DrawPrintText(m_pwFwd, wcslen(m_pwFwd));
    }
    if (nButtons & IN_MOVELEFT)
    {
        CHECK_INPUT_P(IN_MOVELEFT);
        int text_left = GetTextCenter(m_hTextFont, m_pwLeft) - UTIL_ComputeStringWidth(m_hTextFont, m_pwLeft);
        surface()->DrawSetTextPos(text_left, mid_row_ypos);
        surface()->DrawPrintText(m_pwLeft, wcslen(m_pwLeft));
    }
    // Turning left with turnbind
    if (nButtons & IN_LEFT)
    {
        CHECK_INPUT_P(IN_RIGHT);
        int text_left = GetTextCenter(m_hTextFont, m_pwLeft) - (UTIL_ComputeStringWidth(m_hTextFont, m_pwLeft) * 2);
        surface()->DrawSetTextPos(text_left, mid_row_ypos);
        surface()->DrawPrintText(m_pwLeft, wcslen(m_pwLeft));
    }
    if (nButtons & IN_BACK)
    {
        CHECK_INPUT_P(IN_BACK);
        surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwBack), lower_row_ypos);
        surface()->DrawPrintText(m_pwBack, wcslen(m_pwBack));
    }
    if (nButtons & IN_MOVERIGHT)
    {
        CHECK_INPUT_P(IN_MOVERIGHT);
        int text_right = GetTextCenter(m_hTextFont, m_pwRight) + UTIL_ComputeStringWidth(m_hTextFont, m_pwRight);
        surface()->DrawSetTextPos(text_right, mid_row_ypos);
        surface()->DrawPrintText(m_pwRight, wcslen(m_pwRight));
    }
    // Turning right with turnbind
    if (nButtons & IN_RIGHT)
    {
        CHECK_INPUT_P(IN_RIGHT);
        int text_right = GetTextCenter(m_hTextFont, m_pwRight) + (UTIL_ComputeStringWidth(m_hTextFont, m_pwRight) * 2);
        surface()->DrawSetTextPos(text_right, mid_row_ypos);
        surface()->DrawPrintText(m_pwRight, wcslen(m_pwRight));
    }

    if (nButtons & IN_STRAFE)
    {
        CHECK_INPUT_P(IN_STRAFE);
        surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwStrafe), (lower_row_ypos + top_row_ypos) / 2.0f);
        surface()->DrawPrintText(m_pwStrafe, wcslen(m_pwStrafe));
    }

    // reset text font for jump/duck
    surface()->DrawSetTextFont(m_hWordTextFont);

    if (nButtons & IN_JUMP || gpGlobals->curtime < m_fJumpColorUntil)
    {
        if (nButtons & IN_JUMP)
        {
            m_fJumpColorUntil = gpGlobals->curtime + KEYDRAW_MIN;
        }

        surface()->DrawSetTextColor((m_nDisabledButtons & IN_JUMP || m_nDisabledButtons & IN_BHOPDISABLED) ? m_Disabled : m_Normal);
        surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_JUMP), jump_row_ypos);
        surface()->DrawPrintText(STR_JUMP, 4);
    }

    if (nButtons & IN_DUCK || m_bIsDucked || gpGlobals->curtime < m_fDuckColorUntil)
    {
        if (nButtons & IN_DUCK)
        {
            m_fDuckColorUntil = gpGlobals->curtime + KEYDRAW_MIN;
        }
        CHECK_INPUT_P(IN_DUCK);
        surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_DUCK), duck_row_ypos);
        surface()->DrawPrintText(STR_DUCK, 4);
    }

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_AHOP))
    {
        if (nButtons & IN_WALK)
        {
            CHECK_INPUT_P(IN_WALK);
            surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_WALK), walk_row_ypos);
            surface()->DrawPrintText(STR_WALK, 4);
        }
        if (nButtons & IN_SPEED)
        {
            CHECK_INPUT_P(IN_SPEED);
            surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_SPEED), sprint_row_ypos);
            surface()->DrawPrintText(STR_SPEED, 5);
        }
    }

    // Add M1 and M2 buttons
    if (nButtons & IN_ATTACK)
    {
        CHECK_INPUT_P(IN_ATTACK);
        int xposM1 = GetTextCenter(m_hWordTextFont, STR_M1) - (UTIL_ComputeStringWidth(m_hWordTextFont, STR_M1)) * 1.5;
        surface()->DrawSetTextPos(xposM1, top_row_ypos);
        surface()->DrawPrintText(STR_M1, 2);
    }
    if (nButtons & IN_ATTACK2)
    {
        CHECK_INPUT_P(IN_ATTACK2);
        int xposM2 = GetTextCenter(m_hWordTextFont, STR_M2) + (UTIL_ComputeStringWidth(m_hWordTextFont, STR_M2)) * 1.5;
        surface()->DrawSetTextPos(xposM2, top_row_ypos);
        surface()->DrawPrintText(STR_M2, 2);
    }
    // ----------
    if (m_bShouldDrawCounts)
    {
        surface()->DrawSetTextColor(m_Normal); // Back to normal, counts don't get disabled
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
    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        const auto pUIEnt = pPlayer->GetCurrentUIEntity();

        if (pUIEnt->GetEntType() >= RUN_ENT_GHOST)
        {
            const auto pGhost = static_cast<C_MomentumGhostBaseEntity *>(pUIEnt);
            if (pUIEnt->GetEntType() == RUN_ENT_REPLAY)
            {
                m_nStrafes = pGhost->m_RunStats.m_iZoneStrafes[0];
                m_nJumps = pGhost->m_RunStats.m_iZoneJumps[0];
            }

            m_nButtons = pGhost->m_nGhostButtons;
            m_nDisabledButtons = pGhost->m_iDisabledButtons;
            m_bIsDucked = pGhost->GetFlags() & FL_DUCKING;

            m_bShouldDrawCounts = pUIEnt->GetEntType() == RUN_ENT_REPLAY;
        }
        else
        {
            m_nButtons = ::input->GetButtonBits(engine->IsPlayingDemo());
            m_nDisabledButtons = pPlayer->m_afButtonDisabled;
            m_bIsDucked = pPlayer->GetFlags() & FL_DUCKING;
            // we should only draw the strafe/jump counters when the timer is running
            m_bShouldDrawCounts = pPlayer->m_Data.m_bTimerRunning;
            if (m_bShouldDrawCounts)
            {
                m_nStrafes = pPlayer->m_RunStats.m_iZoneStrafes[0];
                m_nJumps = pPlayer->m_RunStats.m_iZoneJumps[0];
            }
        }
    }
}
void CHudKeyPressDisplay::Reset()
{
    // reset buttons member in case a button gets stuck
    m_nButtons = 0;
    m_nDisabledButtons = 0;
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
    CHECK_INPUT_N(IN_FORWARD);
    surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwFwd), top_row_ypos);
    surface()->DrawPrintText(m_pwFwd, wcslen(m_pwFwd));
    // left
    CHECK_INPUT_N(IN_MOVELEFT);
    int text_left = GetTextCenter(m_hTextFont, m_pwLeft) - UTIL_ComputeStringWidth(m_hTextFont, m_pwLeft);
    surface()->DrawSetTextPos(text_left, mid_row_ypos);
    surface()->DrawPrintText(m_pwLeft, wcslen(m_pwLeft));
    // back
    CHECK_INPUT_N(IN_BACK);
    surface()->DrawSetTextPos(GetTextCenter(m_hTextFont, m_pwBack), lower_row_ypos);
    surface()->DrawPrintText(m_pwBack, wcslen(m_pwBack));
    // right
    CHECK_INPUT_N(IN_MOVERIGHT);
    int text_right = GetTextCenter(m_hTextFont, m_pwRight) + UTIL_ComputeStringWidth(m_hTextFont, m_pwRight);
    surface()->DrawSetTextPos(text_right, mid_row_ypos);
    surface()->DrawPrintText(m_pwRight, wcslen(m_pwRight));

    // reset text font for jump/duck
    surface()->DrawSetTextFont(m_hWordTextFont);

    surface()->DrawSetTextColor((m_nDisabledButtons & IN_JUMP || m_nDisabledButtons & IN_BHOPDISABLED) ? m_Disabled : m_darkGray);
    surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_JUMP), jump_row_ypos);
    surface()->DrawPrintText(STR_JUMP, 4);

    CHECK_INPUT_N(IN_DUCK);
    surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_DUCK), duck_row_ypos);
    surface()->DrawPrintText(STR_DUCK, 4);

    if (g_pGameModeSystem->GameModeIs(GAMEMODE_AHOP))
    {
        CHECK_INPUT_N(IN_WALK);
        surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_WALK), walk_row_ypos);
        surface()->DrawPrintText(STR_WALK, 4);

        CHECK_INPUT_N(IN_SPEED);
        surface()->DrawSetTextPos(GetTextCenter(m_hWordTextFont, STR_SPEED), sprint_row_ypos);
        surface()->DrawPrintText(STR_SPEED, 5);
    }

    CHECK_INPUT_N(IN_ATTACK);
    int xposM1 = GetTextCenter(m_hWordTextFont, STR_M1) - (UTIL_ComputeStringWidth(m_hWordTextFont, STR_M1)) * 1.5;
    surface()->DrawSetTextPos(xposM1, top_row_ypos);
    surface()->DrawPrintText(STR_M1, 2);

    CHECK_INPUT_N(IN_ATTACK2);
    int xposM2 = GetTextCenter(m_hWordTextFont, STR_M2) + (UTIL_ComputeStringWidth(m_hWordTextFont, STR_M2)) * 1.5;
    surface()->DrawSetTextPos(xposM2, top_row_ypos);
    surface()->DrawPrintText(STR_M2, 2);
}