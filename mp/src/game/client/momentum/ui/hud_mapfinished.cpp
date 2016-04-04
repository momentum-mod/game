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
#include "mom_shareddefs.h"
#include "util\mom_util.h"
#include "tier0/memdbgon.h"

using namespace vgui;

class CHudMapFinishedDialog : public CHudElement, public Panel
{
    DECLARE_CLASS_SIMPLE(CHudMapFinishedDialog, Panel);

public:
    CHudMapFinishedDialog();
    CHudMapFinishedDialog(const char *pElementName);
    virtual bool ShouldDraw()
    {
        C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return pPlayer && pPlayer->m_bPlayerFinishedMap;
    }
    virtual void Paint();

    virtual void Init();
    virtual void Reset()
    {
        //MOM_TODO : TEMP VALUES.
        m_flAvgSync = 91.257342f;
        m_iTotalStrafes = 5;
        m_iTotalJumps = 2;
        //default value
        strcpy(m_pszRunTime, "00:00:00.000"); 
    }
    virtual void ApplySchemeSettings(IScheme *pScheme)
    {
        Panel::ApplySchemeSettings(pScheme);
        SetBgColor(GetSchemeColor("MOM.Panel.Bg", pScheme));
    }
protected:
    CPanelAnimationVar(HFont, m_hTextFont, "TextFont", "Default");
    CPanelAnimationVarAliasType(float, time_xpos, "time_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, time_ypos, "time_ypos", "5",
        "proportional_float");
    CPanelAnimationVarAliasType(float, strafes_xpos, "strafes_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, strafes_ypos, "strafes_ypos", "25",
        "proportional_float");
    CPanelAnimationVarAliasType(float, jumps_xpos, "jumps_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, jumps_ypos, "jumps_ypos", "45",
        "proportional_float");
    CPanelAnimationVarAliasType(float, sync_xpos, "sync_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, sync_ypos, "sync_ypos", "65",
        "proportional_float");

private:
    wchar_t m_pwTimeLabel[BUFSIZELOCL];
    char m_pszStringTimeLabel[BUFSIZELOCL];
    wchar_t m_pwStrafesLabel[BUFSIZELOCL];
    char m_pszStringStrafesLabel[BUFSIZELOCL];
    wchar_t m_pwJumpsLabel[BUFSIZELOCL];
    char m_pszStringJumpsLabel[BUFSIZELOCL];
    wchar_t m_pwSyncLabel[BUFSIZELOCL];
    char m_pszStringSyncLabel[BUFSIZELOCL];

    char m_pszRunTime[BUFSIZETIME];
    char m_pszAvgSync[BUFSIZELOCL];
    int m_iTotalJumps;
    int m_iTotalStrafes;
    float m_flAvgSync;

};

DECLARE_HUDELEMENT(CHudMapFinishedDialog);

CHudMapFinishedDialog::CHudMapFinishedDialog(const char *pElementName) : 
CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudMapFinishedDialog")
{
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}
void CHudMapFinishedDialog::Init()
{
    Reset();
}
void CHudMapFinishedDialog::Paint()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    //text color
    surface()->DrawSetTextFont(m_hTextFont);
    surface()->DrawSetTextColor(GetFgColor());

    // --- RUN TIME ---
    char timeLocalized[BUFSIZELOCL];
    wchar_t *uTimeUnicode = g_pVGuiLocalize->Find("#MOM_RunTime");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uTimeUnicode ? uTimeUnicode : L"#MOM_RunTime", timeLocalized, BUFSIZELOCL);
    if (pPlayer)
    {
        //copy player's last run time to our local variable
        strcpy(m_pszRunTime, pPlayer->m_pszLastRunTime);
    }
    Q_snprintf(m_pszStringTimeLabel, sizeof(m_pszStringTimeLabel), "%s %s",
        timeLocalized, // run time localization 
        m_pszRunTime    // run time string
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringTimeLabel, m_pwTimeLabel, sizeof(m_pwTimeLabel));
    surface()->DrawSetTextPos(time_xpos, time_ypos);
    surface()->DrawPrintText(m_pwTimeLabel, wcslen(m_pwTimeLabel));
    // ---------------------

    // --- JUMP COUNT ---
    char jumpLocalized[BUFSIZELOCL];
    wchar_t *uJumpUnicode = g_pVGuiLocalize->Find("#MOM_JumpCount");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uJumpUnicode ? uJumpUnicode : L"#MOM_JumpCount", jumpLocalized, BUFSIZELOCL);

    Q_snprintf(m_pszStringJumpsLabel, sizeof(m_pszStringJumpsLabel), "%s %i",
        jumpLocalized, // total jump localization 
        m_iTotalJumps  // total jump int
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringJumpsLabel, m_pwJumpsLabel, sizeof(m_pwJumpsLabel));
    surface()->DrawSetTextPos(jumps_xpos, jumps_ypos);
    surface()->DrawPrintText(m_pwJumpsLabel, wcslen(m_pwJumpsLabel));
    // ---------------------

    // --- STRAFE COUNT ---
    char strafeLocalized[BUFSIZELOCL];
    wchar_t *uStrafeUnicode = g_pVGuiLocalize->Find("#MOM_StrafeCount");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uStrafeUnicode ? uStrafeUnicode : L"#MOM_StrafeCount", strafeLocalized, BUFSIZELOCL);

    Q_snprintf(m_pszStringStrafesLabel, sizeof(m_pszStringStrafesLabel), "%s %i",
        strafeLocalized, // total strafe localization 
        m_iTotalStrafes  //total strafes int
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringStrafesLabel, m_pwStrafesLabel, sizeof(m_pwStrafesLabel));
    surface()->DrawSetTextPos(strafes_xpos, strafes_ypos);
    surface()->DrawPrintText(m_pwStrafesLabel, wcslen(m_pwStrafesLabel));
    // ---------------------

    // --- AVG SYNC ---
    char syncLocalized[BUFSIZELOCL];
    wchar_t *uSyncUnicode = g_pVGuiLocalize->Find("#MOM_AvgSync");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uSyncUnicode ? uSyncUnicode : L"#MOM_AvgSync", syncLocalized, BUFSIZELOCL);

    Q_snprintf(m_pszAvgSync, sizeof(m_pszStringSyncLabel), "%.2f", m_flAvgSync); //convert floating point avg sync to 2 decimal place string
    Q_snprintf(m_pszStringSyncLabel, sizeof(m_pszStringSyncLabel), "%s %s",
        syncLocalized, // avg sync localization 
        m_pszAvgSync    // avg sync float
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringSyncLabel, m_pwSyncLabel, sizeof(m_pwSyncLabel));
    surface()->DrawSetTextPos(sync_xpos, sync_ypos);
    surface()->DrawPrintText(m_pwSyncLabel, wcslen(m_pwSyncLabel));
    // ---------------------

}
