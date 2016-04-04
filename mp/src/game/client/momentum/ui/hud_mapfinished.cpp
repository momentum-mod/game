#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include "menu.h"
#include "time.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>

#include "vgui_helpers.h"
#include "mom_shareddefs.h"
#include "mom_player_shared.h"
#include "util\mom_util.h"
#include "tier0/memdbgon.h"

using namespace vgui;
#define BUFSIZELOCL (73)
#define BUFSIZETIME (sizeof("00:00:00.000")+1)

class CHudMapFinishedDialog : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(CHudMapFinishedDialog, Panel);

public:
    CHudMapFinishedDialog(const char *pElementName);
    virtual bool ShouldDraw()
    {
        C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return pPlayer && pPlayer->m_bPlayerFinishedMap;
    }
    virtual void Paint();

    virtual void Init();
    virtual void OnThink();
    virtual void Reset()
    {

    }
    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetBgColor(GetSchemeColor("MOM.Panel.Bg", pScheme));
    }
protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVarAliasType(float, time_xpos, "time_xpos", "50",
        "proportional_float");
    CPanelAnimationVarAliasType(float, time_ypos, "time_ypos", "2",
        "proportional_float");
    CPanelAnimationVarAliasType(float, strafes_xpos, "strafes_xpos", "50",
        "proportional_float");
    CPanelAnimationVarAliasType(float, strafes_ypos, "strafes_ypos", "22",
        "proportional_float");
    CPanelAnimationVarAliasType(float, jumps_xpos, "jumps_xpos", "50",
        "proportional_float");
    CPanelAnimationVarAliasType(float, jumps_ypos, "jumps_ypos", "42",
        "proportional_float");
    CPanelAnimationVarAliasType(float, sync_xpos, "sync_xpos", "50",
        "proportional_float");
    CPanelAnimationVarAliasType(float, sync_ypos, "sync_ypos", "62",
        "proportional_float");

private:
    float m_flOpenCloseTime;
    float m_flShutoffTime = 0.5f;

    wchar_t m_pwTime[BUFSIZELOCL];
    char m_pszStringTime[BUFSIZELOCL];
    wchar_t m_pwStrafes[BUFSIZELOCL];
    char m_pszStringStrafes[BUFSIZELOCL];
    wchar_t m_pwJumps[BUFSIZELOCL];
    char m_pszStringJumps[BUFSIZELOCL];
    wchar_t m_pwSync[BUFSIZELOCL];
    char m_pszStringSync[BUFSIZELOCL];

    char m_pszRunTime[BUFSIZETIME];
    //MOM_TODO: temp values, change when we have run stats
    int m_iTotalJumps = 5;
    int m_iTotalStrafes = 14;
    int m_flAvgSync = 92.35f;

};

DECLARE_HUDELEMENT(CHudMapFinishedDialog);

CHudMapFinishedDialog::CHudMapFinishedDialog(const char *pElementName) : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudMapFinishedDialog")
{
    SetScheme("ClientScheme");

    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
}
void CHudMapFinishedDialog::Init()
{
    Reset();
}
void CHudMapFinishedDialog::Paint()
{
    //text color
    surface()->DrawSetTextFont(m_hTextFont);
    surface()->DrawSetTextColor(GetFgColor());

    // --- RUN TIME ---
    char timeLocalized[BUFSIZELOCL];
    wchar_t *uTimeUnicode = g_pVGuiLocalize->Find("#MOM_RunTime");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uTimeUnicode ? uTimeUnicode : L"#MOM_RunTime", timeLocalized, BUFSIZELOCL);

        Q_snprintf(m_pszStringTime, sizeof(m_pszStringTime), "%s %s",
        timeLocalized, // run time localization 
        m_pszRunTime    // run time string
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringTime, m_pwTime, sizeof(m_pwTime));
    surface()->DrawSetTextPos(time_xpos, time_ypos);
    surface()->DrawPrintText(m_pwTime, wcslen(m_pwTime));
    // ---------------------

    // --- JUMP COUNT ---
    char jumpLocalized[BUFSIZELOCL];
    wchar_t *uJumpUnicode = g_pVGuiLocalize->Find("#MOM_JumpCount");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uJumpUnicode ? uJumpUnicode : L"#MOM_JumpCount", jumpLocalized, BUFSIZELOCL);

    Q_snprintf(m_pszStringJumps, sizeof(m_pszStringJumps), "%s %i",
        jumpLocalized, // total jump localization 
        m_iTotalJumps  // total jump int
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringJumps, m_pwJumps, sizeof(m_pwJumps));
    surface()->DrawSetTextPos(jumps_xpos, jumps_ypos);
    surface()->DrawPrintText(m_pwJumps, wcslen(m_pwJumps));
    // ---------------------

    // --- STRAFE COUNT ---
    char strafeLocalized[BUFSIZELOCL];
    wchar_t *uStrafeUnicode = g_pVGuiLocalize->Find("#MOM_StrafeCount");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uStrafeUnicode ? uStrafeUnicode : L"#MOM_StrafeCount", strafeLocalized, BUFSIZELOCL);

    Q_snprintf(m_pszStringStrafes, sizeof(m_pszStringStrafes), "%s %i",
        strafeLocalized, // total strafe localization 
        m_iTotalStrafes  //total strafes int
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringStrafes, m_pwStrafes, sizeof(m_pwStrafes));
    surface()->DrawSetTextPos(strafes_xpos, strafes_ypos);
    surface()->DrawPrintText(m_pwStrafes, wcslen(m_pwStrafes));
    // ---------------------

    // --- AVG SYNC ---
    char syncLocalized[BUFSIZELOCL];
    wchar_t *uSyncUnicode = g_pVGuiLocalize->Find("#MOM_AvgSync");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uSyncUnicode ? uSyncUnicode : L"#MOM_AvgSync", syncLocalized, BUFSIZELOCL);

    Q_snprintf(m_pszStringSync, sizeof(m_pszStringSync), "%s %f",
        syncLocalized, // avg sync localization 
        m_flAvgSync    // avg sync float
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringSync, m_pwSync, sizeof(m_pwSync));
    surface()->DrawSetTextPos(sync_xpos, sync_ypos);
    surface()->DrawPrintText(m_pwSync, wcslen(m_pwSync));
    // ---------------------

}
void CHudMapFinishedDialog::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        //copy player's last run time to our local variable
        strcpy(m_pszRunTime, pPlayer->m_pszLastRunTime);
    }
}
