#include "cbase.h"
#include "hud_comparisons.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>

#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "mom_shareddefs.h"
#include "baseviewport.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_mapinfo_show_mapname, "1", FLAG_HUD_CVAR,
                          "Toggles showing the map name. 0 = OFF, 1 = ON");

static MAKE_TOGGLE_CONVAR(mom_mapinfo_show_author, "0", FLAG_HUD_CVAR,
                          "Toggles showing the map author. 0 = OFF, 1 = ON");

static MAKE_TOGGLE_CONVAR(mom_mapinfo_show_difficulty, "0", FLAG_HUD_CVAR,
                          "Toggles showing the map difficulty. 0 = OFF, 1 = ON");

class C_HudMapInfo : public CHudElement, public Panel
{

    DECLARE_CLASS_SIMPLE(C_HudMapInfo, Panel);

    C_HudMapInfo(const char *pElementName);
    void OnThink() OVERRIDE;
    void Init() OVERRIDE;
    void Reset() OVERRIDE;
    void Paint() OVERRIDE;
    bool ShouldDraw() OVERRIDE
    { 
        IViewPortPanel *pLeaderboards = gViewPortInterface->FindPanelByName(PANEL_TIMES);
        return CHudElement::ShouldDraw() && pLeaderboards && !pLeaderboards->IsVisible(); 
    }

    void ApplySchemeSettings(IScheme *pScheme) OVERRIDE
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
    CPanelAnimationVar(Color, m_cTextColor, "TextColor", "MOM.Panel.Fg");
    CPanelAnimationVar(bool, center_status, "centerStatus", "1");

    CPanelAnimationVarAliasType(int, status_xpos, "status_xpos", "0", "proportional_xpos");
    CPanelAnimationVarAliasType(int, status_ypos, "status_ypos", "c+135", "proportional_ypos");
    CPanelAnimationVarAliasType(int, mapinfo_xpos, "mapinfo_xpos", "0", "proportional_xpos");
    CPanelAnimationVarAliasType(int, mapinfo_ypos, "mapinfo_ypos", "0", "proportional_ypos");

  private:
    wchar_t m_pwStageStartString[BUFSIZELOCL], m_pwStageStartLabel[BUFSIZELOCL], m_pwCurrentStages[BUFSIZELOCL],
        m_pwCurrentStatus[BUFSIZELOCL];

    char stageLocalized[BUFSIZELOCL], checkpointLocalized[BUFSIZELOCL], linearLocalized[BUFSIZELOCL],
        startZoneLocalized[BUFSIZELOCL], mapFinishedLocalized[BUFSIZELOCL], m_pszStringStatus[BUFSIZELOCL],
        m_pszStringStages[BUFSIZELOCL], noZonesLocalized[BUFSIZELOCL],
        mapNameLabelLocalized[BUFSIZELOCL], mapAuthorLabelLocalized[BUFSIZELOCL], mapDiffLabelLocalized[BUFSIZELOCL];

    int m_iZoneCount, m_iZoneCurrent;
    bool m_bPlayerInZone, m_bMapFinished, m_bMapLinear;
};

DECLARE_NAMED_HUDELEMENT(C_HudMapInfo, CHudMapInfo);

C_HudMapInfo::C_HudMapInfo(const char *pElementName)
    : CHudElement(pElementName), Panel(g_pClientMode->GetViewport(), pElementName)
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
    m_iZoneCurrent = 0;
    m_iZoneCount = 0;
    m_bPlayerInZone = false;
    m_bMapFinished = false;
}

void C_HudMapInfo::OnThink()
{
    C_MomentumPlayer *pLocal = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());
    if (pLocal && g_MOMEventListener)
    {
        C_MomentumReplayGhostEntity *pGhost = pLocal->GetReplayEnt();
        if (pGhost)
        {
            m_iZoneCurrent = pGhost->m_SrvData.m_RunData.m_iCurrentZone;
            m_bPlayerInZone = pGhost->m_SrvData.m_RunData.m_bIsInZone;
            m_bMapFinished = pGhost->m_SrvData.m_RunData.m_bMapFinished;
        }
        else
        {
            m_iZoneCurrent = pLocal->m_SrvData.m_RunData.m_iCurrentZone;
            m_bPlayerInZone = pLocal->m_SrvData.m_RunData.m_bIsInZone;
            m_bMapFinished = pLocal->m_SrvData.m_RunData.m_bMapFinished;
        }

        m_iZoneCount = g_MOMEventListener->m_iMapZoneCount;
        m_bMapLinear = g_MOMEventListener->m_bMapIsLinear;
    }
}

void C_HudMapInfo::Init()
{
    // LOCALIZE STUFF HERE:
    FIND_LOCALIZATION(m_pwStageStartString, "#MOM_Stage_Start");
    LOCALIZE_TOKEN(Stage, "#MOM_Stage", stageLocalized);
    LOCALIZE_TOKEN(Checkpoint, "#MOM_Checkpoint", checkpointLocalized);
    LOCALIZE_TOKEN(Linear, "#MOM_Linear", linearLocalized);
    LOCALIZE_TOKEN(InsideStart, "#MOM_InsideStartZone", startZoneLocalized);
    LOCALIZE_TOKEN(MapFinished, "#MOM_MapFinished", mapFinishedLocalized);
    LOCALIZE_TOKEN(NoCheckpoint, "#MOM_Status_NoZones", noZonesLocalized);
    LOCALIZE_TOKEN(MapName, "#MOM_Map_Name", mapNameLabelLocalized);
    LOCALIZE_TOKEN(MapAuthor, "#MOM_Map_Author", mapAuthorLabelLocalized);
    LOCALIZE_TOKEN(MapDifficulty, "#MOM_Map_Difficulty", mapDiffLabelLocalized);
}

void C_HudMapInfo::Reset()
{
    m_iZoneCount = 0;
    m_iZoneCurrent = 0;
    m_bMapLinear = false;
    m_bPlayerInZone = false;
    m_bMapFinished = false;
}

void C_HudMapInfo::Paint()
{
    if (m_iZoneCount > 0)
    {
        // Current stage(checkpoint)/total stages(checkpoints)
        Q_snprintf(m_pszStringStages, sizeof(m_pszStringStages), "%s %i/%i",
                   m_bMapLinear ? checkpointLocalized : stageLocalized, // "Stage" / "Checkpoint"
                   m_iZoneCurrent,                                     // Current stage/checkpoint
                   m_iZoneCount                                        // Total number of stages/checkpoints
                   );
    }
    else
    { // No stages/checkpoints found
        Q_strncpy(m_pszStringStages, noZonesLocalized, sizeof(m_pszStringStages));
    }

    ANSI_TO_UNICODE(m_pszStringStages, m_pwCurrentStages);

    // No matter what, we always want the player's status printed out, if they're in a zone
    if (m_bPlayerInZone)
    {
        if (m_iZoneCurrent == 1)
        {
            // Start zone
            Q_strncpy(m_pszStringStatus, startZoneLocalized, sizeof(m_pszStringStatus));
            ANSI_TO_UNICODE(m_pszStringStatus, m_pwStageStartLabel);
        }
        else if (m_bPlayerInZone && m_bMapFinished) // don't check for zone # in case the player skipped one somehow
        {
            // End zone
            Q_strncpy(m_pszStringStatus, mapFinishedLocalized, sizeof(m_pszStringStatus));
            ANSI_TO_UNICODE(m_pszStringStatus, m_pwStageStartLabel);
        }
        else
        {
            //Note: The player will never be inside a "checkpoint" zone
            // Stage # Start
            wchar_t stageCurrent[128]; // 00'\0' and max stages is 64

            V_snwprintf(stageCurrent, ARRAYSIZE(stageCurrent), L"%d", m_iZoneCurrent);
            // Fills the "Stage %s1 Start" string
            g_pVGuiLocalize->ConstructString(m_pwStageStartLabel, sizeof(m_pwStageStartLabel), m_pwStageStartString, 1,
                                             stageCurrent);
        }
    }

    // We need our text color, otherwise the following text can be hijacked
    surface()->DrawSetTextColor(m_cTextColor);

    if (center_status)
    {
        // Center the status string above the timer.
        // MOM_TODO: perhaps check the timer's position using gHUD? A user could change the status_ypos...
        int dummy, totalWide;
        // Draw current time.
        surface()->GetScreenSize(totalWide, dummy);
        // GetSize(totalWide, dummy);
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

    int yPos = mapinfo_ypos;
    int toIncrement = surface()->GetFontTall(m_hMapInfoFont) + 2;
    surface()->DrawSetTextFont(m_hMapInfoFont);
    IViewPortPanel *pSpecGUI = gViewPortInterface->FindPanelByName(PANEL_SPECGUI);
    if (mom_mapinfo_show_mapname.GetBool() && pSpecGUI && !pSpecGUI->IsVisible())
    {
        const char *pMapName = g_pGameRules->MapName();
        if (pMapName)
        {
            char mapStringANSI[BUFSIZELOCL];
            wchar_t mapStringUnicode[BUFSIZELOCL];
            Q_snprintf(mapStringANSI, sizeof(mapStringANSI), "%s%s", mapNameLabelLocalized, pMapName);
            ANSI_TO_UNICODE(mapStringANSI, mapStringUnicode);
            surface()->DrawSetTextPos(mapinfo_xpos, yPos);
            surface()->DrawPrintText(mapStringUnicode, wcslen(mapStringUnicode));
            yPos += toIncrement;
        }
    }

    if (mom_mapinfo_show_author.GetBool())
    {
        // MOM_TODO: Map author(s)

        // char mapAuthorANSI[BUFSIZELOCL];
        // wchar_t mapAuthorUnicode[BUFSIZELOCL];
        // surface()->DrawSetTextPos(mapinfo_xpos, yPos);
        // surface()->DrawPrintText(mapAuthorUnicode, wcslen(mapAuthorUnicode));
    }

    if (mom_mapinfo_show_difficulty.GetBool())
    {
        // MOM_TODO: We need to determine difficulty of a map.
    }
}