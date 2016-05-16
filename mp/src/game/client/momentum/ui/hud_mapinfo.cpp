#include "cbase.h"
#include "hud_comparisons.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "menu.h"
#include "utlvector.h"
#include "vgui_helpers.h"
#include "view.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Panel.h>

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "momentum/util/mom_util.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_mapinfo_show_mapname, "0", FLAG_HUD_CVAR,
                          "Toggles showing the map name. 0 = OFF, 1 = ON");

class C_HudMapInfo : public CHudElement, public Panel
{

    DECLARE_CLASS_SIMPLE(C_HudMapInfo, Panel);

    C_HudMapInfo(const char *pElementName);
    void OnThink() override;
    void Init() override;
    void Reset() override;
    void Paint() override;
    bool ShouldDraw() override { return CHudElement::ShouldDraw(); }

    void ApplySchemeSettings(IScheme *pScheme) override
    {
        Panel::ApplySchemeSettings(pScheme);
        int wide, tall;
        surface()->GetScreenSize(wide, tall);
        SetPos(0, 0);        // Top left corner
        SetSize(wide, tall); // Scale the screen
    }

  protected:
    CPanelAnimationVar(HFont, m_hStatusFont, "StatusFont", "Default");
    CPanelAnimationVar(HFont, m_hMapInfoFont, "MapInfoFont", "Default");

    CPanelAnimationVarAliasType(bool, center_status, "centerStatus", "1", "BOOL");
    CPanelAnimationVarAliasType(int, status_ypos, "status_ypos", "c+135", "proportional_ypos");
    CPanelAnimationVarAliasType(int, status_xpos, "status_xpos", "0", "proportional_xpos");

  private:
    wchar_t m_pwStageStartString[BUFSIZELOCL], m_pwStageStartLabel[BUFSIZELOCL], m_pwCurrentStages[BUFSIZELOCL],
        m_pwCurrentStatus[BUFSIZELOCL];

    char stageLocalized[BUFSIZELOCL], checkpointLocalized[BUFSIZELOCL], linearLocalized[BUFSIZELOCL], 
        startZoneLocalized[BUFSIZELOCL], mapFinishedLocalized[BUFSIZELOCL], m_pszStringStatus[BUFSIZELOCL], 
        m_pszStringStages[BUFSIZELOCL], noStagesLocalized[BUFSIZELOCL], noCPLocalized[BUFSIZELOCL];

    int m_iStageCount, m_iStageCurrent;
    bool m_bPlayerInZone, m_bMapFinished, m_bMapLinear;
};

DECLARE_NAMED_HUDELEMENT(C_HudMapInfo, CHudMapInfo);

C_HudMapInfo::C_HudMapInfo(const char *pElementName)
    : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), "CHudMapInfo")
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_iStageCurrent = 0;
    m_bPlayerInZone = false;
    m_bMapFinished = false;
}

void C_HudMapInfo::OnThink()
{
    C_MomentumPlayer *pLocal = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pLocal && g_MOMEventListener)
    {
        m_iStageCurrent = pLocal->m_iCurrentStage;
        m_bPlayerInZone = pLocal->m_bIsInZone;
        m_bMapFinished = pLocal->m_bMapFinished;
        m_iStageCount = g_MOMEventListener->m_iMapCheckpointCount;
        m_bMapLinear = g_MOMEventListener->m_bMapIsLinear;
    }
}

void C_HudMapInfo::Init()
{
    SetBgColor(Color(0, 0, 0, 0));
    // LOCALIZE STUFF HERE:
    FIND_LOCALIZATION(m_pwStageStartString, "#MOM_Stage_Start");
    LOCALIZE_TOKEN(Stage, "#MOM_Stage", stageLocalized);
    LOCALIZE_TOKEN(Checkpoint, "#MOM_Checkpoint", checkpointLocalized);
    LOCALIZE_TOKEN(Linear, "#MOM_Linear", linearLocalized);
    LOCALIZE_TOKEN(InsideStart, "#MOM_InsideStartZone", startZoneLocalized);
    LOCALIZE_TOKEN(MapFinished, "#MOM_MapFinished", mapFinishedLocalized);
    LOCALIZE_TOKEN(NoCheckpoint, "#MOM_Status_NoCheckpoints", noCPLocalized);
    LOCALIZE_TOKEN(NoStages, "#MOM_Status_NoStages", noStagesLocalized);
}

void C_HudMapInfo::Reset()
{
    // MOM_TODO: Reset all the numbers and stuff here?
}

void C_HudMapInfo::Paint()
{
    // MOM_TODO: this will change to be checkpoints
    if (m_iStageCount > 0)
    {
        // Current stage(checkpoint)/total stages(checkpoints)
        Q_snprintf(m_pszStringStages, sizeof(m_pszStringStages), "%s %i/%i",
            m_bMapLinear ? checkpointLocalized : stageLocalized,     // "Stage" / MOM_TODO: "Checkpoint"
            m_iStageCurrent, // Current stage / MOM_TODO: Current checkpoint
            m_iStageCount    // Total number of stages / MOM_TODO: checkpoints
            );
    } 
    else
    {//No stages/checkpoints found
        Q_snprintf(m_pszStringStages, sizeof(m_pszStringStages), 
            m_bMapLinear ? noCPLocalized : noStagesLocalized);
    }

    ANSI_TO_UNICODE(m_pszStringStages, m_pwCurrentStages);

    // No matter what, we always want the player's status printed out, if they're in a zone
    if (m_bPlayerInZone)
    {
        if (m_iStageCurrent == 1)
        {
            // Start zone
            Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), startZoneLocalized);
            ANSI_TO_UNICODE(m_pszStringStatus, m_pwStageStartLabel);
        }
        else if (m_bPlayerInZone && m_bMapFinished) // don't check for zone # in case the player skipped one somehow
        {
            // End zone
            Q_snprintf(m_pszStringStatus, sizeof(m_pszStringStatus), mapFinishedLocalized);
            ANSI_TO_UNICODE(m_pszStringStatus, m_pwStageStartLabel);
        }
        else
        {
            // Stage # Start
            wchar_t stageCurrent[128]; // 00'\0' and max stages is 64

            V_snwprintf(stageCurrent, ARRAYSIZE(stageCurrent), L"%d", m_iStageCurrent);
            // Fills the "Stage %s1 Start" string
            g_pVGuiLocalize->ConstructString(m_pwStageStartLabel, sizeof(m_pwStageStartLabel), m_pwStageStartString, 1,
                                             stageCurrent);
        }
    }

    if (center_status)
    {
        // Center the status string above the timer.
        // MOM_TODO: perhaps check the timer's position using gHUD? A user could change the status_ypos...
        int dummy, totalWide;
        // Draw current time.
        surface()->GetScreenSize(totalWide, dummy);
        //GetSize(totalWide, dummy);
        int stageWide;

        surface()->GetTextSize(m_hStatusFont, m_bPlayerInZone ? m_pwStageStartLabel : m_pwCurrentStages, stageWide,
                               dummy);
        int offsetToCenter = ((totalWide - stageWide) / 2);
        surface()->DrawSetTextPos(offsetToCenter, status_ypos);
    }
    else
    {
        // User-defined place
        surface()->DrawSetTextPos(status_xpos, status_ypos);
    }

    surface()->DrawSetTextFont(m_hStatusFont);
    // If we're inside a stage trigger, print that stage's start label, otherwise the
    // current number of stages/MOM_TODO: checkpoints
    surface()->DrawPrintText(m_bPlayerInZone ? m_pwStageStartLabel : m_pwCurrentStages,
                             wcslen(m_bPlayerInZone ? m_pwStageStartLabel : m_pwCurrentStages));
}