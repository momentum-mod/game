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
#include "mom_event_listener.h"
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
        return pPlayer && g_MOMEventListener && g_MOMEventListener->m_bMapFinished;
    }
    virtual void Paint();
    virtual void OnThink();
    virtual void Init();
    virtual void Reset()
    {
        //default values
        m_iTotalStrafes = 0;
        m_iTotalJumps = 0;
        m_flAvgSpeed = 0;
        m_flEndSpeed = 0;
        m_flStartSpeed = 0;
        m_flMaxSpeed = 0;
        m_flAvgSync = 0;
        m_flAvgSync2 = 0;
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
    CPanelAnimationVarAliasType(float, sync2_xpos, "sync2_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, sync2_ypos, "sync2_ypos", "85",
        "proportional_float");
    CPanelAnimationVarAliasType(float, startvel_xpos, "startvel_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, startvel_ypos, "startvel_ypos", "65",
        "proportional_float");
    CPanelAnimationVarAliasType(float, endvel_xpos, "endvel_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, endvel_ypos, "endvel_ypos", "65",
        "proportional_float");
    CPanelAnimationVarAliasType(float, avgvel_xpos, "avgvel_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, avgvel_ypos, "avgvel_ypos", "65",
        "proportional_float");
    CPanelAnimationVarAliasType(float, maxvel_xpos, "maxvel_xpos", "30",
        "proportional_float");
    CPanelAnimationVarAliasType(float, maxvel_ypos, "maxvel_ypos", "65",
        "proportional_float");
    CPanelAnimationVarAliasType(float, runsave_ypos, "runsave_ypos", "65",
        "proportional_float");
    CPanelAnimationVarAliasType(float, runupload_ypos, "runupload_ypos", "65",
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
    wchar_t m_pwSync2Label[BUFSIZELOCL];
    char m_pszStringSync2Label[BUFSIZELOCL];
    wchar_t m_pwStartSpeedLabel[BUFSIZELOCL];
    char m_pszStartSpeedLabel[BUFSIZELOCL];
    wchar_t m_pwEndSpeedLabel[BUFSIZELOCL];
    char m_pszEndSpeedLabel[BUFSIZELOCL];
    wchar_t m_pwAvgSpeedLabel[BUFSIZELOCL];
    char m_pszAvgSpeedLabel[BUFSIZELOCL];
    wchar_t m_pwMaxSpeedLabel[BUFSIZELOCL];
    char m_pszMaxSpeedLabel[BUFSIZELOCL];

    wchar_t m_pwRunSavedLabel[BUFSIZELOCL];
    char m_pszRunSavedLabel[BUFSIZELOCL];
    wchar_t m_pwRunNotSavedLabel[BUFSIZELOCL];
    char m_pszRunNotSavedLabel[BUFSIZELOCL];
    wchar_t m_pwRunUploadedLabel[BUFSIZELOCL];
    char m_pszRunUploadedLabel[BUFSIZELOCL];
    wchar_t m_pwRunNotUploadedLabel[BUFSIZELOCL];
    char m_pszRunNotUploadedLabel[BUFSIZELOCL];

    char m_pszRunTime[BUFSIZETIME];
    char m_pszAvgSync[BUFSIZELOCL], m_pszAvgSync2[BUFSIZELOCL];
    int m_iTotalJumps, m_iTotalStrafes;
    float m_flAvgSync, m_flAvgSync2;
    float m_flStartSpeed, m_flEndSpeed, m_flAvgSpeed, m_flMaxSpeed;

    char runSaveLocalized[BUFSIZELOCL], runNotSaveLocalized[BUFSIZELOCL], 
        runUploadLocalized[BUFSIZELOCL], runNotUploadLocalized[BUFSIZELOCL];

    char maxVelLocalized[BUFSIZELOCL], avgVelLocalized[BUFSIZELOCL], endVelLocalized[BUFSIZELOCL], 
        startVelLocalized[BUFSIZELOCL], sync2Localized[BUFSIZELOCL], syncLocalized[BUFSIZELOCL], 
        strafeLocalized[BUFSIZELOCL], jumpLocalized[BUFSIZELOCL], timeLocalized[BUFSIZELOCL];


    bool m_bRunSaved, m_bRunUploaded;
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
    //cache localization files

    wchar_t *uTimeUnicode = g_pVGuiLocalize->Find("#MOM_RunTime");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uTimeUnicode ? uTimeUnicode : L"#MOM_RunTime", timeLocalized, BUFSIZELOCL);

    wchar_t *uJumpUnicode = g_pVGuiLocalize->Find("#MOM_JumpCount");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uJumpUnicode ? uJumpUnicode : L"#MOM_JumpCount", jumpLocalized, BUFSIZELOCL);

    wchar_t *uStrafeUnicode = g_pVGuiLocalize->Find("#MOM_StrafeCount");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uStrafeUnicode ? uStrafeUnicode : L"#MOM_StrafeCount", strafeLocalized, BUFSIZELOCL);

    wchar_t *uSyncUnicode = g_pVGuiLocalize->Find("#MOM_AvgSync");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uSyncUnicode ? uSyncUnicode : L"#MOM_AvgSync", syncLocalized, BUFSIZELOCL);

    wchar_t *uSync2Unicode = g_pVGuiLocalize->Find("#MOM_AvgSync2");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uSync2Unicode ? uSync2Unicode : L"#MOM_AvgSync2", sync2Localized, BUFSIZELOCL);

    wchar_t *uStartVelUnicode = g_pVGuiLocalize->Find("#MOM_StartVel");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uStartVelUnicode ? uStartVelUnicode : L"#MOM_StartVel", startVelLocalized, BUFSIZELOCL);

    wchar_t *uendVelUnicode = g_pVGuiLocalize->Find("#MOM_EndVel");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uendVelUnicode ? uendVelUnicode : L"#MOM_EndVel", endVelLocalized, BUFSIZELOCL);

    wchar_t *uavgVelUnicode = g_pVGuiLocalize->Find("#MOM_AvgVel");
    g_pVGuiLocalize->ConvertUnicodeToANSI(uavgVelUnicode ? uavgVelUnicode : L"#MOM_AvgVel", avgVelLocalized, BUFSIZELOCL);

    wchar_t *umaxVelUnicode = g_pVGuiLocalize->Find("#MOM_MaxVel");
    g_pVGuiLocalize->ConvertUnicodeToANSI(umaxVelUnicode ? umaxVelUnicode : L"#MOM_MaxVel", maxVelLocalized, BUFSIZELOCL);

    // --- RUN SAVING NOTIFICATION ---
    wchar_t *urunSaveUnicode = g_pVGuiLocalize->Find("#MOM_RunSaved");
    g_pVGuiLocalize->ConvertUnicodeToANSI(urunSaveUnicode ? urunSaveUnicode : L"#MOM_RunSaved", runSaveLocalized, BUFSIZELOCL);

    wchar_t *urunNotSaveUnicode = g_pVGuiLocalize->Find("#MOM_RunNotSaved");
    g_pVGuiLocalize->ConvertUnicodeToANSI(urunNotSaveUnicode ? urunNotSaveUnicode : L"#MOM_RunNotSaved", runNotSaveLocalized, BUFSIZELOCL);

    wchar_t *urunUploadUnicode = g_pVGuiLocalize->Find("#MOM_RunUploaded");
    g_pVGuiLocalize->ConvertUnicodeToANSI(urunUploadUnicode ? urunUploadUnicode : L"#MOM_RunUploaded", runUploadLocalized, BUFSIZELOCL);

    wchar_t *urunNotUploadUnicode = g_pVGuiLocalize->Find("#MOM_RunNotUploaded");
    g_pVGuiLocalize->ConvertUnicodeToANSI(urunNotUploadUnicode ? urunNotUploadUnicode : L"#MOM_RunNotUploaded", runNotUploadLocalized, BUFSIZELOCL);
}
void CHudMapFinishedDialog::Paint()
{
    //text color
    surface()->DrawSetTextFont(m_hTextFont);
    surface()->DrawSetTextColor(GetFgColor());

    // --- RUN TIME ---
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

    // --- AVG SYNC 2---
    Q_snprintf(m_pszAvgSync2, sizeof(m_pszStringSync2Label), "%.2f", m_flAvgSync2); //convert floating point avg sync to 2 decimal place string
    Q_snprintf(m_pszStringSync2Label, sizeof(m_pszStringSync2Label), "%s %s",
        sync2Localized, // avg sync localization 
        m_pszAvgSync2    // avg sync float
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStringSync2Label, m_pwSync2Label, sizeof(m_pwSync2Label));

    surface()->DrawSetTextPos(sync2_xpos, sync2_ypos);
    surface()->DrawPrintText(m_pwSync2Label, wcslen(m_pwSync2Label));
    // ---------------------

    // --- STARTING VELOCITY---
    Q_snprintf(m_pszStartSpeedLabel, sizeof(m_pszStartSpeedLabel), "%s %f",
        startVelLocalized,
        m_flStartSpeed
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszStartSpeedLabel, m_pwStartSpeedLabel, sizeof(m_pwStartSpeedLabel));

    surface()->DrawSetTextPos(startvel_xpos, startvel_ypos);
    surface()->DrawPrintText(m_pwStartSpeedLabel, wcslen(m_pwStartSpeedLabel));
    // ---------------------

    // --- ENDING VELOCITY---
    Q_snprintf(m_pszEndSpeedLabel, sizeof(m_pszEndSpeedLabel), "%s %f",
        endVelLocalized,
        m_flEndSpeed
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszEndSpeedLabel, m_pwEndSpeedLabel, sizeof(m_pwEndSpeedLabel));

    surface()->DrawSetTextPos(endvel_xpos, endvel_ypos);
    surface()->DrawPrintText(m_pwEndSpeedLabel, wcslen(m_pwEndSpeedLabel));
    // ---------------------

    // --- AVG VELOCITY---
    Q_snprintf(m_pszAvgSpeedLabel, sizeof(m_pszAvgSpeedLabel), "%s %f",
        avgVelLocalized,
        m_flAvgSpeed
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszAvgSpeedLabel, m_pwAvgSpeedLabel, sizeof(m_pwAvgSpeedLabel));

    surface()->DrawSetTextPos(avgvel_xpos, avgvel_ypos);
    surface()->DrawPrintText(m_pwAvgSpeedLabel, wcslen(m_pwAvgSpeedLabel));
    // ---------------------

    // --- MAX VELOCITY---
    Q_snprintf(m_pszMaxSpeedLabel, sizeof(m_pszMaxSpeedLabel), "%s %f",
        maxVelLocalized,
        m_flMaxSpeed
        );
    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_pszMaxSpeedLabel, m_pwMaxSpeedLabel, sizeof(m_pwMaxSpeedLabel));

    surface()->DrawSetTextPos(maxvel_xpos, maxvel_ypos);
    surface()->DrawPrintText(m_pwMaxSpeedLabel, wcslen(m_pwAvgSpeedLabel));
    // ---------------------

    // ---- RUN SAVING AND UPLOADING ----

    // -- run save --
    Q_snprintf(m_bRunSaved ? m_pszRunSavedLabel : m_pszRunNotSavedLabel,
        m_bRunSaved ? sizeof(m_pszRunSavedLabel) : sizeof(m_pszRunNotSavedLabel), "%s",
        m_bRunSaved ? runSaveLocalized : runNotSaveLocalized);

    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_bRunSaved ? m_pszRunSavedLabel : m_pszRunNotSavedLabel,
        m_bRunSaved ? m_pwRunSavedLabel : m_pwRunNotSavedLabel,
        m_bRunSaved ? sizeof(m_pwRunSavedLabel) : sizeof(m_pwRunNotSavedLabel));

    // -- run upload --
    Q_snprintf(m_bRunUploaded ? m_pszRunUploadedLabel : m_pszRunNotUploadedLabel,
        m_bRunUploaded ? sizeof(m_pszRunUploadedLabel) : sizeof(m_pszRunNotUploadedLabel), "%s",
        m_bRunUploaded ? runUploadLocalized : runNotUploadLocalized);

    g_pVGuiLocalize->ConvertANSIToUnicode(
        m_bRunUploaded ? m_pszRunUploadedLabel : m_pszRunNotUploadedLabel,
        m_bRunUploaded ? m_pwRunUploadedLabel : m_pwRunNotUploadedLabel,
        m_bRunUploaded ? sizeof(m_pwRunUploadedLabel) : sizeof(m_pwRunNotUploadedLabel));

    int save_text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, 
        m_bRunSaved ? m_pwRunSavedLabel : m_pwRunNotSavedLabel) / 2; //center label

    surface()->DrawSetTextPos(save_text_xpos, runsave_ypos);
    surface()->DrawSetTextColor(m_bRunSaved ? GetFgColor() : COLOR_RED);
    surface()->DrawPrintText(m_bRunSaved ? m_pwRunSavedLabel : m_pwRunNotSavedLabel, 
        m_bRunSaved ? wcslen(m_pwRunSavedLabel) : wcslen(m_pwRunNotSavedLabel));

    int upload_text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont,
        m_bRunUploaded ? m_pwRunUploadedLabel : m_pwRunNotUploadedLabel) / 2; //center label

    surface()->DrawSetTextPos(upload_text_xpos, runupload_ypos);
    surface()->DrawSetTextColor(m_bRunUploaded ? GetFgColor() : COLOR_RED);
    surface()->DrawPrintText(m_bRunUploaded ? m_pwRunUploadedLabel : m_pwRunNotUploadedLabel, 
        m_bRunUploaded ? wcslen(m_pwRunUploadedLabel) : wcslen(m_pwRunNotUploadedLabel));
    // ----------------
    // ------------------------------
}
void CHudMapFinishedDialog::OnThink()
{
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());

    if (g_MOMEventListener)
    {
        m_bRunSaved = g_MOMEventListener->m_bTimeDidSave;
        m_bRunUploaded = g_MOMEventListener->m_bTimeDidUpload;
        m_flAvgSpeed = g_MOMEventListener->m_flVelocityAvg;
        m_flMaxSpeed = g_MOMEventListener->m_flVelocityMax;
        m_flStartSpeed = g_MOMEventListener->m_flStartSpeed;
        m_flEndSpeed = g_MOMEventListener->m_flEndSpeed;
        m_flAvgSync2 = g_MOMEventListener->m_flStrafeSync2Avg;
        m_flAvgSync = g_MOMEventListener->m_flStrafeSyncAvg;
        m_iTotalJumps = g_MOMEventListener->m_iTotalJumps;
        m_iTotalStrafes = g_MOMEventListener->m_iTotalStrafes;
    }
    if (pPlayer != nullptr)
        mom_UTIL->FormatTime(pPlayer->m_flLastRunTime, m_pszRunTime);
}