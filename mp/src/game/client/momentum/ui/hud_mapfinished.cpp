#include "cbase.h"
#include "hud_mapfinished.h"

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT_DEPTH(CHudMapFinishedDialog, 70);

//NOTE: The "CHudMapFinishedDialog" (main panel) control settings are found in HudLayout.res
// whereas the Replay_Icon/etc settings are found in MapFinishedDialog.res
CHudMapFinishedDialog::CHudMapFinishedDialog(const char *pElementName) : 
CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "CHudMapFinishedDialog")
{
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);
    
    LoadControlSettings("resource/UI/MapFinishedDialog.res");
    m_pPlayReplayButton = FindControl<ImagePanel>("Replay_Icon");
    m_pPlayReplayButton->SetParent(this);
    m_pPlayReplayButton->SetMouseInputEnabled(true);
    m_pPlayReplayButton->InstallMouseHandler(this);
    m_pPlayReplayLabel = FindControl<Label>("Replay_Label");
    m_pPlayReplayLabel->SetParent(this);
}

CHudMapFinishedDialog::~CHudMapFinishedDialog()
{
}

bool CHudMapFinishedDialog::ShouldDraw()
{
    bool shouldDrawLocal = false;
    C_MomentumPlayer *pPlayer = ToCMOMPlayer(CBasePlayer::GetLocalPlayer());
    if (pPlayer)
    {
        if (pPlayer->IsWatchingReplay())
        {
            C_MomentumReplayGhostEntity *pEnt = pPlayer->GetReplayEnt();
            shouldDrawLocal = pEnt && pEnt->m_RunData.m_bMapFinished;
        }
        else
        {
            shouldDrawLocal = pPlayer->m_RunData.m_bMapFinished;
        }
    }
    return CHudElement::ShouldDraw() && shouldDrawLocal;
}


void CHudMapFinishedDialog::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(GetSchemeColor("MOM.Panel.Bg", pScheme));
    m_pPlayReplayLabel->SetFont(m_hTextFont);
    m_pPlayReplayLabel->SetTall(surface()->GetFontTall(m_hTextFont) + 2);
}

void CHudMapFinishedDialog::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT)
    {
        if (input()->GetMouseOver() == m_pPlayReplayButton->GetVPanel())
        {
            DevLog("Clicked on the replay icon! Starting replay...\n");
            //MOM_TODO: Play the replay
        }

        //MOM_TODO: Other buttons here ("next page", see stats for entire run)
    }
    else if (code == MOUSE_RIGHT)
    {
        SetMouseInputEnabled(false);//Lock mouse again
    }
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

    //MOM_TODO: Maybe have a "Right click to use the mouse!" string or something?
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

//MOM_TODO: Do we want this to be a think method or update it to a usermsg/event, so it only calls once, and not every frame?
void CHudMapFinishedDialog::OnThink()
{
    C_MomentumPlayer * pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (g_MOMEventListener && pPlayer)
    {
        m_bRunSaved = g_MOMEventListener->m_bTimeDidSave;
        m_bRunUploaded = g_MOMEventListener->m_bTimeDidUpload;
        //MOM_TODO: g_MOMEventListener has a m_szMapUploadStatus, do we want it on this panel?
        //Is it going to be a localized string, except for errors that have to be specific?

        ConVarRef hvel("mom_speedometer_hvel");

        C_MomentumReplayGhostEntity *pGhost = pPlayer->GetReplayEnt();
        RunStats_t *stats = g_MOMEventListener->GetRunStats(pGhost ? pGhost->entindex() : pPlayer->entindex());
        float lastRunTime = pGhost ? pGhost->m_flRunTime : g_MOMEventListener->m_flLastRunTime;

        m_flAvgSpeed = stats->m_flZoneVelocityAvg[0][hvel.GetBool()];
        m_flMaxSpeed = stats->m_flZoneVelocityMax[0][hvel.GetBool()];
        m_flEndSpeed = stats->m_flZoneExitSpeed[0][hvel.GetBool()];
        m_flStartSpeed = stats->m_flZoneEnterSpeed[0][hvel.GetBool()];
        m_flAvgSync2 = stats->m_flZoneStrafeSyncAvg[0];
        m_flAvgSync = stats->m_flZoneStrafeSync2Avg[0];
        m_iTotalJumps = stats->m_iZoneJumps[0];
        m_iTotalStrafes = stats->m_iZoneStrafes[0];
        mom_UTIL->FormatTime(lastRunTime, m_pszRunTime);
    }
}