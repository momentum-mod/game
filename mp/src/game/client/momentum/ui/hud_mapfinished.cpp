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

    bool ShouldDraw() override
    {
        C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
        return pPlayer && pPlayer->m_bMapFinished;
    }

    void Paint() override;
    void OnThink() override;
    void Init() override;
    void Reset() override;

    void ApplySchemeSettings(IScheme *pScheme) override
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
    wchar_t m_pwRunNotSavedLabel[BUFSIZELOCL];
    wchar_t m_pwRunUploadedLabel[BUFSIZELOCL];
    wchar_t m_pwRunNotUploadedLabel[BUFSIZELOCL];

    char m_pszRunTime[BUFSIZETIME];
    char m_pszAvgSync[BUFSIZELOCL], m_pszAvgSync2[BUFSIZELOCL];
    int m_iTotalJumps, m_iTotalStrafes;
    float m_flAvgSync, m_flAvgSync2;
    float m_flStartSpeed, m_flEndSpeed, m_flAvgSpeed, m_flMaxSpeed;

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
    
    //Stats
    LOCALIZE_TOKEN(RunTime, "#MOM_RunTime", timeLocalized);
    LOCALIZE_TOKEN(JumpCount, "#MOM_JumpCount", jumpLocalized);
    LOCALIZE_TOKEN(StrafeCount, "#MOM_StrafeCount", strafeLocalized);
    LOCALIZE_TOKEN(AvgSync, "#MOM_AvgSync", syncLocalized);
    LOCALIZE_TOKEN(AvgSync2, "#MOM_AvgSync2", sync2Localized);
    LOCALIZE_TOKEN(StartVel, "#MOM_StartVel", startVelLocalized);
    LOCALIZE_TOKEN(EndVel, "#MOM_EndVel", endVelLocalized);
    LOCALIZE_TOKEN(AvgVel, "#MOM_AvgVel", avgVelLocalized);
    LOCALIZE_TOKEN(MaxVel, "#MOM_MaxVel", maxVelLocalized);
    //Saving/Uploading
    FIND_LOCALIZATION(m_pwRunSavedLabel, "#MOM_RunSaved");
    FIND_LOCALIZATION(m_pwRunNotSavedLabel, "#MOM_RunNotSaved");
    FIND_LOCALIZATION(m_pwRunUploadedLabel, "#MOM_RunUploaded");
    FIND_LOCALIZATION(m_pwRunNotUploadedLabel, "#MOM_RunNotUploaded");
}

void CHudMapFinishedDialog::Reset()
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

void CHudMapFinishedDialog::Paint()
{
    //text color
    surface()->DrawSetTextFont(m_hTextFont);
    surface()->DrawSetTextColor(GetFgColor());

    // --- RUN TIME ---
    Q_snprintf(m_pszStringTimeLabel, BUFSIZELOCL, "%s %s",
        timeLocalized, // run time localization 
        m_pszRunTime    // run time string
        );

    ANSI_TO_UNICODE(m_pszStringTimeLabel, m_pwTimeLabel);

    surface()->DrawSetTextPos(time_xpos, time_ypos);
    surface()->DrawPrintText(m_pwTimeLabel, wcslen(m_pwTimeLabel));
    // ---------------------

    // --- JUMP COUNT ---
    Q_snprintf(m_pszStringJumpsLabel, BUFSIZELOCL, "%s %i",
        jumpLocalized, // total jump localization 
        m_iTotalJumps  // total jump int
        );

    ANSI_TO_UNICODE(m_pszStringJumpsLabel, m_pwJumpsLabel);

    surface()->DrawSetTextPos(jumps_xpos, jumps_ypos);
    surface()->DrawPrintText(m_pwJumpsLabel, wcslen(m_pwJumpsLabel));
    // ---------------------

    // --- STRAFE COUNT ---
    Q_snprintf(m_pszStringStrafesLabel, BUFSIZELOCL, "%s %i",
        strafeLocalized, // total strafe localization 
        m_iTotalStrafes  //total strafes int
        );

    ANSI_TO_UNICODE(m_pszStringStrafesLabel, m_pwStrafesLabel);

    surface()->DrawSetTextPos(strafes_xpos, strafes_ypos);
    surface()->DrawPrintText(m_pwStrafesLabel, wcslen(m_pwStrafesLabel));
    // ---------------------

    // --- AVG SYNC ---
    Q_snprintf(m_pszAvgSync, BUFSIZELOCL, "%.2f", m_flAvgSync); //convert floating point avg sync to 2 decimal place string
    Q_snprintf(m_pszStringSyncLabel, BUFSIZELOCL, "%s %s",
        syncLocalized, // avg sync localization 
        m_pszAvgSync    // avg sync float
        );

    ANSI_TO_UNICODE(m_pszStringSyncLabel, m_pwSyncLabel);

    surface()->DrawSetTextPos(sync_xpos, sync_ypos);
    surface()->DrawPrintText(m_pwSyncLabel, wcslen(m_pwSyncLabel));
    // ---------------------

    // --- AVG SYNC 2---
    Q_snprintf(m_pszAvgSync2, BUFSIZELOCL, "%.2f", m_flAvgSync2); //convert floating point avg sync to 2 decimal place string
    Q_snprintf(m_pszStringSync2Label, BUFSIZELOCL, "%s %s",
        sync2Localized, // avg sync localization 
        m_pszAvgSync2    // avg sync float
        );

    ANSI_TO_UNICODE(m_pszStringSync2Label, m_pwSync2Label);

    surface()->DrawSetTextPos(sync2_xpos, sync2_ypos);
    surface()->DrawPrintText(m_pwSync2Label, wcslen(m_pwSync2Label));
    // ---------------------

    // --- STARTING VELOCITY---
    Q_snprintf(m_pszStartSpeedLabel, BUFSIZELOCL, "%s %f",
        startVelLocalized,
        m_flStartSpeed
        );
    ANSI_TO_UNICODE(m_pszStartSpeedLabel, m_pwStartSpeedLabel);

    surface()->DrawSetTextPos(startvel_xpos, startvel_ypos);
    surface()->DrawPrintText(m_pwStartSpeedLabel, wcslen(m_pwStartSpeedLabel));
    // ---------------------

    // --- ENDING VELOCITY---
    Q_snprintf(m_pszEndSpeedLabel, BUFSIZELOCL, "%s %f", endVelLocalized, m_flEndSpeed);
    ANSI_TO_UNICODE(m_pszEndSpeedLabel, m_pwEndSpeedLabel);

    surface()->DrawSetTextPos(endvel_xpos, endvel_ypos);
    surface()->DrawPrintText(m_pwEndSpeedLabel, wcslen(m_pwEndSpeedLabel));
    // ---------------------

    // --- AVG VELOCITY---
    Q_snprintf(m_pszAvgSpeedLabel, BUFSIZELOCL, "%s %f",
        avgVelLocalized,
        m_flAvgSpeed
        );
    ANSI_TO_UNICODE(m_pszAvgSpeedLabel, m_pwAvgSpeedLabel);

    surface()->DrawSetTextPos(avgvel_xpos, avgvel_ypos);
    surface()->DrawPrintText(m_pwAvgSpeedLabel, wcslen(m_pwAvgSpeedLabel));
    // ---------------------

    // --- MAX VELOCITY---
    Q_snprintf(m_pszMaxSpeedLabel, BUFSIZELOCL, "%s %f",
        maxVelLocalized,
        m_flMaxSpeed
        );
    
    ANSI_TO_UNICODE(m_pszMaxSpeedLabel, m_pwMaxSpeedLabel);

    surface()->DrawSetTextPos(maxvel_xpos, maxvel_ypos);
    surface()->DrawPrintText(m_pwMaxSpeedLabel, wcslen(m_pwAvgSpeedLabel));
    // ---------------------

    // ---- RUN SAVING AND UPLOADING ----

    // -- run save --
    wchar_t *runSaveUni = m_bRunSaved ? m_pwRunSavedLabel : m_pwRunNotSavedLabel;

    // -- run upload --
    wchar_t *runUploadUni = m_bRunUploaded ? m_pwRunUploadedLabel : m_pwRunNotUploadedLabel;

    // -- draw run save --
    int save_text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, runSaveUni) / 2; //center label

    surface()->DrawSetTextPos(save_text_xpos, runsave_ypos);
    surface()->DrawSetTextColor(m_bRunSaved ? GetFgColor() : COLOR_RED);//MOM_TODO: should the success color be green?
    surface()->DrawPrintText(runSaveUni, wcslen(runSaveUni));

    // -- draw run upload --
    int upload_text_xpos = GetWide() / 2 - UTIL_ComputeStringWidth(m_hTextFont, runUploadUni) / 2; //center label

    surface()->DrawSetTextPos(upload_text_xpos, runupload_ypos);
    surface()->DrawSetTextColor(m_bRunUploaded ? GetFgColor() : COLOR_RED);//MOM_TODO: should the success color be green?
    surface()->DrawPrintText(runUploadUni, wcslen(runUploadUni));
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
        //MOM_TODO: g_MOMEventListener has a m_szMapUploadStatus, do we want it on this panel?
        //Is it going to be a localized string, except for errors that have to be specific?

        ConVarRef hvel("mom_speedometer_hvel");
        m_flAvgSpeed = hvel.GetBool() ? g_MOMEventListener->m_flStageVelocityAvg[0][1] : g_MOMEventListener->m_flStageVelocityAvg[0][0];
        m_flMaxSpeed = hvel.GetBool() ? g_MOMEventListener->m_flStageVelocityMax[0][1] : g_MOMEventListener->m_flStageVelocityMax[0][0];
        m_flEndSpeed = hvel.GetBool() ? g_MOMEventListener->m_flStageExitSpeed[0][1] : g_MOMEventListener->m_flStageExitSpeed[0][0];
        m_flStartSpeed = hvel.GetBool() ? g_MOMEventListener->m_flStageStartSpeed[0][1] : g_MOMEventListener->m_flStageStartSpeed[0][0];
        m_flAvgSync2 = g_MOMEventListener->m_flStageStrafeSyncAvg[0];
        m_flAvgSync = g_MOMEventListener->m_flStageStrafeSync2Avg[0];
        m_iTotalJumps = g_MOMEventListener->m_iStageJumps[0];
        m_iTotalStrafes = g_MOMEventListener->m_iStageStrafes[0];
    }
    if (pPlayer != nullptr)
        mom_UTIL->FormatTime(pPlayer->m_flLastRunTime, m_pszRunTime);
}