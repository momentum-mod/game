#include "cbase.h"
#include "hud_comparisons.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>

#include "baseviewport.h"
#include "mom_player_shared.h"
#include "c_mom_replay_entity.h"
#include "mom_shareddefs.h"
#include "mom_map_cache.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_mapname, "1", FLAG_HUD_CVAR, "Toggles showing the map name. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_author, "0", FLAG_HUD_CVAR, "Toggles showing the map author. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_difficulty, "0", FLAG_HUD_CVAR, "Toggles showing the map difficulty. 0 = OFF, 1 = ON\n");
static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_status, "1", FLAG_HUD_CVAR, "Toggles showing the main status label. 0 = OFF, 1 = ON\n");

class C_HudMapInfo : public CHudElement, public EditablePanel
{
    DECLARE_CLASS_SIMPLE(C_HudMapInfo, EditablePanel);

    C_HudMapInfo(const char *pElementName);
    void OnThink() OVERRIDE;
    void Reset() OVERRIDE;

    void LevelInitPostEntity() OVERRIDE;
    void LevelShutdown() OVERRIDE;
    void FireGameEvent(IGameEvent* event) OVERRIDE;

  private:
    void SetCurrentZoneLabel(C_MomentumPlayer *pPlayer);
    CMomRunEntityData *m_pRunData;

    Label *m_pMainStatusLabel, *m_pMapNameLabel, *m_pMapAuthorLabel, *m_pMapDifficultyLabel;

    wchar_t m_wMapName[BUFSIZELOCL], m_wMapAuthors[BUFSIZELOCL], m_wMapDifficulty[BUFSIZELOCL];
    wchar_t m_wBonus[BUFSIZELOCL], m_wBonusStart[BUFSIZELOCL], m_wBonusEnd[BUFSIZELOCL];
    wchar_t m_wNoZones[BUFSIZELOCL], m_wStage[BUFSIZELOCL], m_wCheckpoint[BUFSIZELOCL], m_wStageStart[BUFSIZELOCL], m_wMainStart[BUFSIZELOCL], m_wMainEnd[BUFSIZELOCL];
    wchar_t m_wMapFinished[BUFSIZELOCL], m_wLinearMap[BUFSIZELOCL];

    bool m_bNeedsUpdate;
};

DECLARE_NAMED_HUDELEMENT(C_HudMapInfo, HudMapInfo);

C_HudMapInfo::C_HudMapInfo(const char *pElementName): CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudMapInfo")
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_pRunData = nullptr;
    m_bNeedsUpdate = true;

    ListenForGameEvent("zone_enter");
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("spec_target_updated");
    ListenForGameEvent("spec_stop");
    ListenForGameEvent("player_spawn");
    ListenForGameEvent("mapfinished_panel_closed");

    m_pMainStatusLabel = new Label(this, "MainStatusLabel", "");
    m_pMapNameLabel = new Label(this, "MapNameLabel", "");
    m_pMapAuthorLabel = new Label(this, "MapAuthorLabel", "");
    m_pMapDifficultyLabel = new Label(this, "MapDifficultyLabel", "");

    LoadControlSettings("resource/ui/HudMapInfo.res");
}

void C_HudMapInfo::OnThink()
{
    BaseClass::OnThink();

    m_pMapNameLabel->SetVisible(mom_hud_mapinfo_show_mapname.GetBool());
    m_pMapAuthorLabel->SetVisible(mom_hud_mapinfo_show_author.GetBool());
    m_pMapDifficultyLabel->SetVisible(mom_hud_mapinfo_show_difficulty.GetBool());
    m_pMainStatusLabel->SetVisible(mom_hud_mapinfo_show_status.GetBool());

    const auto pPlayer = C_MomentumPlayer::GetLocalMomPlayer();
    if (pPlayer)
    {
        m_pRunData = pPlayer->GetCurrentUIEntData();

        if (m_bNeedsUpdate && m_pRunData)
        {
            if (m_pRunData->m_bIsInZone)
            {
                if (m_pRunData->m_iCurrentZone == 1) // Start zone
                {
                    m_pMainStatusLabel->SetText(m_pRunData->m_iCurrentTrack == 0 ?
                                                m_wMainStart :
                                                CConstructLocalizedString(m_wBonusStart, m_pRunData->m_iCurrentTrack.Get()));
                }
                else if (m_pRunData->m_iCurrentZone == 0) // End zone
                {
                    m_pMainStatusLabel->SetText(m_pRunData->m_iCurrentTrack == 0 ?
                                                m_wMainEnd :
                                                CConstructLocalizedString(m_wBonusEnd, m_pRunData->m_iCurrentTrack.Get()));
                }
                else if (m_pRunData->m_bMapFinished) // MOM_TODO does this even need to be here anymore? "Main End Zone" covers this
                {
                    // End zone
                    m_pMainStatusLabel->SetText(m_wMapFinished);
                }
                else if (!pPlayer->m_iLinearTracks.Get(m_pRunData->m_iCurrentTrack)) // Note: The player will never be inside a "checkpoint" zone
                {
                    // Some # stage start
                    m_pMainStatusLabel->SetText(CConstructLocalizedString(m_wStageStart, m_pRunData->m_iCurrentZone.Get()));
                }
                else
                {
                    // It's linear, just draw the current checkpoint
                    SetCurrentZoneLabel(pPlayer);
                }
            }
            else
            {
                if (m_pRunData->m_iCurrentTrack > 0)
                {
                    m_pMainStatusLabel->SetText(CConstructLocalizedString(m_wBonus, m_pRunData->m_iCurrentTrack.Get()));
                }
                else if (pPlayer->m_iZoneCount[m_pRunData->m_iCurrentTrack] > 0)
                {
                    SetCurrentZoneLabel(pPlayer);
                }
                else
                {
                    m_pMainStatusLabel->SetText(m_wNoZones);
                }
            }

            m_bNeedsUpdate = false;
        }
    }
}

void C_HudMapInfo::Reset()
{
    FIND_LOCALIZATION(m_wMapName, "#MOM_Map_Name");
    FIND_LOCALIZATION(m_wMapAuthors, "#MOM_Map_Authors");
    FIND_LOCALIZATION(m_wMapDifficulty, "#MOM_Map_Difficulty");
    FIND_LOCALIZATION(m_wStageStart, "#MOM_Stage_Start");
    FIND_LOCALIZATION(m_wNoZones, "#MOM_Status_NoZones");
    FIND_LOCALIZATION(m_wStage, "#MOM_Stage");
    FIND_LOCALIZATION(m_wCheckpoint, "#MOM_Checkpoint");
    FIND_LOCALIZATION(m_wMainStart, "#MOM_InsideStartZone");
    FIND_LOCALIZATION(m_wMainEnd, "#MOM_InsideEndZone");
    FIND_LOCALIZATION(m_wBonus, "#MOM_Bonus");
    FIND_LOCALIZATION(m_wBonusStart, "#MOM_Bonus_Start");
    FIND_LOCALIZATION(m_wBonusEnd, "#MOM_Bonus_End");
    FIND_LOCALIZATION(m_wLinearMap, "#MOM_Linear");
    FIND_LOCALIZATION(m_wMapFinished, "#MOM_MapFinished");

    m_bNeedsUpdate = true;
}

void C_HudMapInfo::LevelInitPostEntity()
{
    KeyValuesAD mapInfo("MapInfo");

    MapData *pData = g_pMapCache->GetCurrentMapData();
    if (pData)
    {
        mapInfo->SetString("mapName", pData->m_szMapName);
        CUtlString authors;
        if (pData->GetCreditString(&authors, CREDIT_AUTHOR))
        {
            mapInfo->SetString("authors", authors.Get());
            m_pMapAuthorLabel->SetText(CConstructLocalizedString(m_wMapAuthors, (KeyValues*)mapInfo));
        }
        mapInfo->SetInt("difficulty", pData->m_MainTrack.m_iDifficulty);
        m_pMapDifficultyLabel->SetText(CConstructLocalizedString(m_wMapDifficulty, (KeyValues*)mapInfo));
    }
    else
    {
        mapInfo->SetString("mapName", g_pGameRules->MapName());
        m_pMapAuthorLabel->SetText("");
        m_pMapDifficultyLabel->SetText("");
    }

    m_pMapNameLabel->SetText(CConstructLocalizedString(m_wMapName, (KeyValues*)mapInfo));
}

void C_HudMapInfo::LevelShutdown()
{
    m_pRunData = nullptr;
}

void C_HudMapInfo::FireGameEvent(IGameEvent* event)
{
    m_bNeedsUpdate = true;
}

void C_HudMapInfo::SetCurrentZoneLabel(C_MomentumPlayer *pPlayer)
{
    // Current stage(checkpoint)/total stages(checkpoints)
    const wchar_t *pCurrent = CConstructLocalizedString(L"%s1/%s2", m_pRunData->m_iCurrentZone.Get(), pPlayer->m_iZoneCount.Get(m_pRunData->m_iCurrentTrack));
    m_pMainStatusLabel->SetText(CConstructLocalizedString(pPlayer->m_iLinearTracks.Get(m_pRunData->m_iCurrentTrack) ? m_wCheckpoint : m_wStage, pCurrent));
}
