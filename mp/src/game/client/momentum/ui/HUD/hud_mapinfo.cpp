#include "cbase.h"
#include "hud_comparisons.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "utlvector.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>

#include "baseviewport.h"
#include "mom_event_listener.h"
#include "mom_player_shared.h"
#include "c_mom_replay_entity.h"
#include "mom_shareddefs.h"
#include "mom_map_cache.h"

#include "tier0/memdbgon.h"

using namespace vgui;

static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_mapname, "1", FLAG_HUD_CVAR, "Toggles showing the map name. 0 = OFF, 1 = ON");

static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_author, "0", FLAG_HUD_CVAR, "Toggles showing the map author. 0 = OFF, 1 = ON");

static MAKE_TOGGLE_CONVAR(mom_hud_mapinfo_show_difficulty, "0", FLAG_HUD_CVAR, "Toggles showing the map difficulty. 0 = OFF, 1 = ON");

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
    int m_iSpecEntIndex;
    C_MomentumPlayer *m_pPlayer;

    C_MOMRunEntityData *m_pRunData;

    Label *m_pMainStatusLabel, *m_pMapNameLabel, *m_pMapAuthorLabel, *m_pMapDifficultyLabel;

    wchar_t m_wMapName[BUFSIZELOCL], m_wMapAuthors[BUFSIZELOCL], m_wMapDifficulty[BUFSIZELOCL];
    wchar_t m_wBonus[BUFSIZELOCL], m_wBonusStart[BUFSIZELOCL], m_wBonusEnd[BUFSIZELOCL];
    wchar_t m_wNoZones[BUFSIZELOCL], m_wStage[BUFSIZELOCL], m_wCheckpoint[BUFSIZELOCL], m_wStageStart[BUFSIZELOCL], m_wMainStart[BUFSIZELOCL], m_wMainEnd[BUFSIZELOCL];
    wchar_t m_wMapFinished[BUFSIZELOCL], m_wLinearMap[BUFSIZELOCL];

    int m_iCurrentZone;
    bool m_bNeedsUpdate, m_bInZone;
};

DECLARE_NAMED_HUDELEMENT(C_HudMapInfo, HudMapInfo);

C_HudMapInfo::C_HudMapInfo(const char *pElementName): CHudElement(pElementName), EditablePanel(g_pClientMode->GetViewport(), "HudMapInfo")
{
    SetPaintBackgroundEnabled(false);
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_LEADERBOARDS);

    m_pPlayer = nullptr;
    m_pRunData = nullptr;
    m_bNeedsUpdate = true;
    m_iCurrentZone = 0;
    m_iSpecEntIndex = 0;
    m_bInZone = false;

    ListenForGameEvent("zone_enter");
    ListenForGameEvent("zone_exit");
    ListenForGameEvent("spec_target_updated");
    ListenForGameEvent("spec_stop");
    ListenForGameEvent("player_spawn");

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

    if (!m_pPlayer)
        m_pPlayer = ToCMOMPlayer(C_BasePlayer::GetLocalPlayer());

    if (!m_pRunData && m_pPlayer)
    {
        if (m_iSpecEntIndex)
        {
            C_MomentumReplayGhostEntity *pGhost = m_pPlayer->GetReplayEnt();
            if (pGhost)
            {
                // m_pSpecTarget = pGhost;
                m_pRunData = &pGhost->m_SrvData.m_RunData;
            }
        }
        else
        {
            m_pRunData = &m_pPlayer->m_SrvData.m_RunData;
        }
    }

    if (m_bNeedsUpdate && m_pRunData)
    {
        if (m_bInZone)
        {
            if (m_iCurrentZone == 1) // Start zone
            {
                m_pMainStatusLabel->SetText(m_pRunData->m_iBonusZone == 0 ? 
                                            m_wMainStart :
                                            CConstructLocalizedString(m_wBonusStart, m_pRunData->m_iBonusZone));
            }
            else if (m_iCurrentZone == 0) // End zone
            {
                m_pMainStatusLabel->SetText(m_pRunData->m_iBonusZone == 0 ? 
                                            m_wMainEnd :
                                            CConstructLocalizedString(m_wBonusEnd, m_pRunData->m_iBonusZone));
            }
            else if (m_pRunData->m_bMapFinished) // MOM_TODO does this even need to be here anymore? "Main End Zone" covers this
            {
                // End zone
                m_pMainStatusLabel->SetText(m_wMapFinished);
            }
            else if (!g_MOMEventListener->m_bMapIsLinear) // Note: The player will never be inside a "checkpoint" zone
            {
                // Some # stage start
                m_pMainStatusLabel->SetText(CConstructLocalizedString(m_wStageStart, m_iCurrentZone));
            }
        }
        else
        {
            if (m_pRunData->m_iBonusZone > 0)
            {
                m_pMainStatusLabel->SetText(CConstructLocalizedString(m_wBonus, m_pRunData->m_iBonusZone));
            }
            else if (g_MOMEventListener->m_iMapZoneCount > 0)
            {
                // Current stage(checkpoint)/total stages(checkpoints)
                const wchar_t *pCurrent = CConstructLocalizedString(L"%s1/%s2", m_iCurrentZone, g_MOMEventListener->m_iMapZoneCount);
                m_pMainStatusLabel->SetText(CConstructLocalizedString(g_MOMEventListener->m_bMapIsLinear ? m_wCheckpoint : m_wStage, pCurrent));
            }
            else
            {
                m_pMainStatusLabel->SetText(m_wNoZones);
            }
        }

        m_bNeedsUpdate = false;
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
        mapInfo->SetInt("difficulty", pData->m_Info.m_iDifficulty);
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
    m_pPlayer = nullptr;
    m_iSpecEntIndex = 0;
    m_iCurrentZone = 0;
    m_bInZone = false;
}

void C_HudMapInfo::FireGameEvent(IGameEvent* event)
{
    const char *pName = event->GetName();
    if (FStrEq(pName, "zone_enter") || FStrEq(pName, "zone_exit"))
    {
        const int ent = event->GetInt("ent");
        if ((ent == engine->GetLocalPlayer() || ent == m_iSpecEntIndex))
        {
            m_bInZone = FStrEq(pName, "zone_enter");
            m_iCurrentZone = event->GetInt("num");
            m_bNeedsUpdate = true;
        }
    }
    else if (FStrEq(pName, "spec_target_updated"))
    {
        if (m_pPlayer)
        {
            m_iSpecEntIndex = m_pPlayer->GetSpecEntIndex();
            m_bNeedsUpdate = true;
        }
    }
    else if (FStrEq(pName, "spec_stop"))
    {
        m_iSpecEntIndex = 0;
        m_bNeedsUpdate = true;
    }
    else // "player_spawn"
    {
        if (m_pPlayer)
        {
            m_pRunData = &m_pPlayer->m_SrvData.m_RunData;
            m_bInZone = m_pRunData->m_bIsInZone;
            m_iCurrentZone = m_pRunData->m_iCurrentZone;
        }
    }
}
