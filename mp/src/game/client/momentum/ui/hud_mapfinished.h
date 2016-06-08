#pragma once

#include "cbase.h"

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "menu.h"
#include "time.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/pch_vgui_controls.h>
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
#include "util/mom_util.h"

using namespace vgui;

class CHudMapFinishedDialog : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(CHudMapFinishedDialog, EditablePanel);

public:
    CHudMapFinishedDialog(const char *pElementName);
    ~CHudMapFinishedDialog();

    bool ShouldDraw() override
    {
        return CHudElement::ShouldDraw();

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

    void Paint() override;
    void OnThink() override;
    void Init() override;
    void Reset() override;

    void OnMousePressed(MouseCode code) override;

    void ApplySchemeSettings(IScheme *pScheme) override;

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

    ImagePanel *m_pPlayReplayButton;
    Label *m_pPlayReplayLabel;

    bool m_bRunSaved, m_bRunUploaded;
};