#include "cbase.h"

#include "TrickList.h"

#include "fmtstr.h"

#include "LocalTricksTab.h"
#include "MapTeleTab.h"
#include "OfficialTricksTab.h"
#include "CommunityTricksTab.h"

#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui_controls/PropertySheet.h"

#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

using namespace vgui;

TrickList::TrickList(IViewPort *pViewPort) : BaseClass(nullptr, PANEL_TRICK_LIST)
{
    SetSize(5, 5);

    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    const auto hScheme = scheme()->LoadSchemeFromFile("resource/TrickListScheme.res", "TrickListScheme");
    if (hScheme)
    {
        SetScheme(hScheme);
    }

    m_pTrickProgressLabel = new Label(this, "TrickProgressLabel", "0 / 0 (0%)");
    m_pTrickProgressBar = new ContinuousProgressBar(this, "TrickProgressBar");

    m_pTrickTabs = new PropertySheet(this, "TrickTabs");

    LoadControlSettings("resource/ui/tricks/TrickList.res");

    m_pTrickTabs->InvalidateLayout(true, true);

    m_pOfficialTricksTab = new OfficialTricksTab(this);
    m_pCommunityTricksTab = new CommunityTricksTab(this);
    m_pMapTelesTab = new MapTeleTab(this);
    m_pLocalTricksTab = new LocalTricksTab(this);

    m_pTrickTabs->AddPage(m_pOfficialTricksTab, "#MOM_Tricks_Official");
    m_pTrickTabs->AddPage(m_pCommunityTricksTab, "#MOM_Tricks_Community");
    m_pTrickTabs->AddPage(m_pMapTelesTab, "#MOM_Tricks_MapTeleList");
    m_pTrickTabs->AddPage(m_pLocalTricksTab, "#MOM_Tricks_Local");

    ListenForGameEvent("trick_data_loaded");
}

TrickList::~TrickList()
{

}

void TrickList::ShowPanel(bool bState)
{
    SetVisible(bState);

    if (!bState)
    {
        SetMouseInputEnabled(false);
    }

    C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
    if (player)
    {
        if (bState)
        {
            player->m_Local.m_iHideHUD |= HIDEHUD_LEADERBOARDS;
        }
        else
        {
            player->m_Local.m_iHideHUD &= ~HIDEHUD_LEADERBOARDS;
        }
    }
}

void TrickList::FireGameEvent(IGameEvent *event)
{
    ParseTrickList();
}

void TrickList::ParseTrickList()
{
    m_pLocalTricksTab->ParseTrickData();
    m_pMapTelesTab->ParseTrickData();
}