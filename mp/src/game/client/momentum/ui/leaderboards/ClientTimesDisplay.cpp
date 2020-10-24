#include "cbase.h"

#include "ClientTimesDisplay.h"
#include "LeaderboardsHeader.h"
#include "LeaderboardsStats.h"
#include "LeaderboardsTimes.h"

#include "clientmode.h"

#include "vgui/ISurface.h"
#include "IGameUIFuncs.h"
#include "voice_status.h"
#include <inputsystem/iinputsystem.h>
#include "vgui_controls/AnimationController.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define UPDATE_INTERVAL 15.0f

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientTimesDisplay::CClientTimesDisplay(IViewPort *pViewPort) : EditablePanel(nullptr, PANEL_TIMES)
{
    SetSize(10, 10); // Quiet the "parent not sized yet" spew, actual size in leaderboards.res

    m_bToggledOpen = false;
    m_flNextUpdateTime = 0.0f;

    m_pViewPort = pViewPort;
    // initialize dialog
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    // Create a "popup" so we can get the mouse to detach
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    // set the scheme before any child control is created
    SetScheme(scheme()->LoadSchemeFromFile("resource/LeaderboardsScheme.res", "LeaderboardsScheme"));

    m_pHeader = new CLeaderboardsHeader(this);
    m_pStats = new CLeaderboardsStats(this);
    m_pTimes = new CLeaderboardsTimes(this);

    LoadControlSettings("resource/ui/leaderboards.res");

    m_pTimes->SetKeyBoardInputEnabled(true);

    if (!m_pHeader || !m_pTimes || !m_pStats)
    {
        AssertMsg(0, "Null pointer(s) on scoreboards");
    }

    // update scoreboard instantly if one of these events occur
    ListenForGameEvent("replay_save");
    ListenForGameEvent("run_upload");

    // can be toggled on at start of game launch if starts visible
    SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientTimesDisplay::~CClientTimesDisplay()
{
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CClientTimesDisplay::Reset() { Reset(false); }

void CClientTimesDisplay::Reset(bool bFullReset)
{
    m_flNextUpdateTime = 0.0f;

    // add all the sections
    if (m_pTimes)
    {
        m_pTimes->Reset(bFullReset);
    }

    Update(bFullReset);
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CClientTimesDisplay::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    // light up scoreboard a bit
    SetBgColor(Color(0, 0, 0, 0));
}

void CClientTimesDisplay::PerformLayout()
{
    // resize the images to our resolution
    /*for (int i = 0; i < m_pImageList->GetImageCount(); i++)
    {
         int wide, tall;
         m_pImageList->GetImage(i)->GetSize(wide, tall);
         m_pImageList->GetImage(i)->SetSize(SCALE(wide), SCALE(tall));
    }*/

    // Make it the size of the screen and center
    int screenWide, screenHeight;
    surface()->GetScreenSize(screenWide, screenHeight);
    MoveToCenterOfScreen();
    SetSize(screenWide, screenHeight);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CClientTimesDisplay::ShowPanel(bool bShow)
{
    if (m_bToggledOpen)
    {
        return;
    }

    if (m_pTimes)
        m_pTimes->OnPanelShow(bShow);

    if (BaseClass::IsVisible() == bShow)
        return;

    if (bShow)
    {
        Reset(true);
        SetVisible(true);
        MoveToFront();
        RequestFocus();

        const auto pSpecUI = m_pViewPort->FindPanelByName(PANEL_SPECGUI);
        if (pSpecUI && pSpecUI->IsVisible() && ipanel()->IsMouseInputEnabled(pSpecUI->GetVPanel()))
            SetMouseInputEnabled(true);
    }
    else
    {
        Close();
    }
}

void CClientTimesDisplay::SetMouseInputEnabled(bool bState)
{
    BaseClass::SetMouseInputEnabled(bState);

    if (bState)
    {
        m_bToggledOpen = true;
        g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(GetParent(), "LeaderboardsBgFocusGain");
        engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
        SetKeyBoardInputEnabled(true);
    }
}

void CClientTimesDisplay::SetVisible(bool bState)
{
    BaseClass::SetVisible(bState);

    SetLeaderboardsHideHud(bState);
}

void CClientTimesDisplay::Close()
{
    m_bToggledOpen = false;
    engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
    SetVisible(false);
    SetMouseInputEnabled(false);
    SetKeyBoardInputEnabled(false);

    g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(GetParent(), "LeaderboardsBgFocusLost");
}

void CClientTimesDisplay::OnKeyCodeReleased(KeyCode code)
{
    if (m_bToggledOpen && (code == KEY_ESCAPE || code == gameuifuncs->GetButtonCodeForBind("showtimes")))
    {
        Close();
    }
    else
    {
        BaseClass::OnKeyCodeReleased(code);
    }
}

void CClientTimesDisplay::FireGameEvent(IGameEvent *event)
{
    if (!event)
        return;

    const char *type = event->GetName();

    if (FStrEq(type, "replay_save") && event->GetBool("save"))
    {
        m_pTimes->OnRunSaved();
    }
    else if (FStrEq(type, "run_upload"))
    {
        m_pTimes->OnRunPosted(event->GetBool("run_posted"));
        m_pStats->NeedsUpdate();
    }
}

bool CClientTimesDisplay::NeedsUpdate() { return (m_flNextUpdateTime < gpGlobals->curtime); }

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CClientTimesDisplay::Update() { Update(false); }

void CClientTimesDisplay::Update(bool pFullUpdate)
{
    if (!NeedsUpdate())
        return;

    FillScoreBoard(pFullUpdate);

    MoveToCenterOfScreen();

    m_flNextUpdateTime = gpGlobals->curtime + UPDATE_INTERVAL;
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CClientTimesDisplay::FillScoreBoard()
{
    // update player info
    FillScoreBoard(false);
}

void CClientTimesDisplay::FillScoreBoard(bool bFullUpdate)
{
    if (!engine->IsInGame())
        return;

    if (m_pHeader)
        m_pHeader->LoadData(MapName(), bFullUpdate);

    if (m_pStats)
        m_pStats->LoadData(bFullUpdate);

    if (m_pTimes)
        m_pTimes->FillLeaderboards(bFullUpdate);
}

void CClientTimesDisplay::SetLeaderboardsHideHud(bool bState)
{
    C_BasePlayer* player = C_BasePlayer::GetLocalPlayer();
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
//-----------------------------------------------------------------------------
// Purpose: Center the dialog on the screen.  (vgui has this method on
//          Frame, but we're an EditablePanel, need to roll our own.)
//-----------------------------------------------------------------------------
void CClientTimesDisplay::MoveToCenterOfScreen()
{
    int wx, wy, ww, wt;
    surface()->GetWorkspaceBounds(wx, wy, ww, wt);
    SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

void CClientTimesDisplay::LevelInitPostEntity()
{
    m_pHeader->Reset();
    m_pStats->NeedsUpdate();
    m_pTimes->LevelInit();
}

void CClientTimesDisplay::OnReloadControls()
{
    BaseClass::OnReloadControls();

    LevelInitPostEntity();
}