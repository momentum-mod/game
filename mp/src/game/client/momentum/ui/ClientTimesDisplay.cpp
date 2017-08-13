//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"

#include <stdio.h>

#include "ClientTimesDisplay.h"
#include "inputsystem/iinputsystem.h"
#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <voice_status.h>

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include <game/client/iviewport.h>

#include "filesystem.h"
#include <util/mom_util.h>
#include "vgui_avatarimage.h"
#include <hud_vote.h>
#include "UtlSortVector.h"
#include <time.h>
#include <util/jsontokv.h>
#include "IMessageboxPanel.h"
#include "run/mom_replay_factory.h"

extern IFileSystem *filesystem;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define RANKSTRING "00000"               // A max of 99999 ranks (too generous)
#define DATESTRING "00/00/0000 00:00:00" // Entire date string
#define TIMESTRING "00:00:00.000"        // Entire time string

bool PNamesMapLessFunc(const uint64 &first, const uint64 &second) { return first < second; }

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientTimesDisplay::CClientTimesDisplay(IViewPort *pViewPort) : 
    EditablePanel(nullptr, PANEL_TIMES),
    m_bLocalTimesLoaded(false),
    m_bLocalTimesNeedUpdate(false),
    m_bOnlineNeedUpdate(false),
     m_bOnlineTimesLoaded(false),
    m_bFriendsNeedUpdate(false),
    m_bFriendsTimesLoaded(false),
    m_bUnauthorizedFriendlist(false)
{
    SetSize(10, 10); // Quiet the "parent not sized yet" spew, actual size in leaderboards.res

    m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");
    m_nCloseKey = BUTTON_CODE_INVALID;

    m_bGetTop10Scores = true;
    m_bLoadedLocalPlayerAvatar = false;

    m_pViewPort = pViewPort;
    // initialize dialog
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    // Create a "popup" so we can get the mouse to detach
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    // set the scheme before any child control is created
    // SetScheme("ClientScheme");
    SetScheme(scheme()->LoadSchemeFromFile("resource/LeaderboardsScheme.res", "LeaderboardsScheme"));

    LoadControlSettings("resource/ui/leaderboards.res");

    m_pHeader = FindChildByName("Header", true);
    m_pMapName = FindControl<Label>("MapName", true);
    m_pMapAuthor = FindControl<Label>("MapAuthor", true);
    m_pMapDetails = FindControl<Label>("MapDetails", true);
    m_pMomentumLogo = FindControl<ImagePanel>("MomentumLogo", true);
    m_pPlayerStats = FindChildByName("PlayerStats", true);
    m_pPlayerAvatar = FindControl<ImagePanel>("PlayerAvatar", true);
    m_pPlayerName = FindControl<Label>("PlayerName", true);
    m_pPlayerMapRank = FindControl<Label>("PlayerMapRank", true);
    m_pPlayerPersonalBest = FindControl<Label>("PlayerPersonalBest", true);
    m_pPlayerGlobalRank = FindControl<Label>("PlayerGlobalRank", true);
    m_pPlayerExperience = FindControl<Label>("PlayerExperience", true);
    m_pLoadingOnlineTimes = FindControl<Label>("LoadingOnlineTimes", true);
    m_pLeaderboards = FindChildByName("Leaderboards", true);
    m_pOnlineLeaderboards = FindControl<SectionedListPanel>("OnlineLeaderboards", true);
    m_pLocalLeaderboards = FindControl<SectionedListPanel>("LocalLeaderboards", true);
    m_pFriendsLeaderboards = FindControl<SectionedListPanel>("FriendsLeaderboards", true);
    m_pGlobalLeaderboardsButton = FindControl<Button>("GlobalLeaderboardsButton", true);
    m_pGlobalTop10Button = FindControl<Button>("GlobalTop10Button", true);
    m_pGlobalAroundButton = FindControl<Button>("GlobalAroundButton", true);
    m_pFriendsLeaderboardsButton = FindControl<Button>("FriendsLeaderboardsButton", true);
    m_pLocalLeaderboardsButton = FindControl<Button>("LocalLeaderboardsButton", true);
    m_pRunFilterButton = FindControl<ToggleButton>("FilterButton", true);
    m_pFilterPanel = FindControl<EditablePanel>("FilterPanel", true);

    m_pFilterPanel->LoadControlSettings("resource/ui/leaderboards_filter.res");

    m_pCurrentLeaderboards = m_pLocalLeaderboards;

    if (!m_pHeader || !m_pMapName || !m_pPlayerStats || !m_pPlayerAvatar || !m_pPlayerName || !m_pMomentumLogo ||
        !m_pPlayerMapRank || !m_pPlayerGlobalRank || !m_pLeaderboards || !m_pOnlineLeaderboards ||
        !m_pLocalLeaderboards || !m_pFriendsLeaderboards || !m_pPlayerPersonalBest || !m_pLoadingOnlineTimes ||
        !m_pPlayerExperience || !m_pGlobalLeaderboardsButton || !m_pFriendsLeaderboardsButton ||
        !m_pGlobalTop10Button || !m_pGlobalAroundButton)
    {
        Assert("Null pointer(s) on scoreboards");
    }

    // Override the parents of the controls (the current parent is this)
    m_pMapName->SetParent(m_pHeader);
    m_pMapAuthor->SetParent(m_pHeader);
    m_pMapDetails->SetParent(m_pHeader);
    m_pPlayerStats->SetParent(m_pLeaderboards);
    m_pMomentumLogo->SetParent(m_pPlayerStats);
    m_pPlayerAvatar->SetParent(m_pPlayerStats);
    m_pPlayerName->SetParent(m_pPlayerStats);
    m_pPlayerMapRank->SetParent(m_pPlayerStats);
    m_pPlayerGlobalRank->SetParent(m_pPlayerStats);
    m_pPlayerPersonalBest->SetParent(m_pPlayerStats);
    m_pPlayerExperience->SetParent(m_pPlayerStats);
    m_pOnlineLeaderboards->SetParent(m_pLeaderboards);
    m_pLoadingOnlineTimes->SetParent(m_pLeaderboards);
    m_pLocalLeaderboards->SetParent(m_pLeaderboards);
    m_pFriendsLeaderboards->SetParent(m_pLeaderboards);
    m_pGlobalLeaderboardsButton->SetParent(m_pLeaderboards);
    m_pGlobalTop10Button->SetParent(m_pLeaderboards);
    m_pGlobalAroundButton->SetParent(m_pLeaderboards);
    m_pFriendsLeaderboardsButton->SetParent(m_pLeaderboards);
    m_pLocalLeaderboardsButton->SetParent(m_pLeaderboards);
    m_pRunFilterButton->SetParent(m_pLeaderboards);

    // Get rid of the scrollbars for the panels
    // MOM_TODO: Do we want the player to be able to explore the ranks?
    m_pOnlineLeaderboards->SetVerticalScrollbar(false);
    m_pLocalLeaderboards->SetVerticalScrollbar(false);
    m_pFriendsLeaderboards->SetVerticalScrollbar(false);

    m_pMomentumLogo->GetImage()->SetSize(scheme()->GetProportionalScaledValue(256),
                                         scheme()->GetProportionalScaledValue(64));

    m_iDesiredHeight = GetTall();

    // update scoreboard instantly if on of these events occur
    ListenForGameEvent("replay_save");
    ListenForGameEvent("run_upload");
    ListenForGameEvent("game_newmap");

    m_pLeaderboardReplayCMenu = new CReplayContextMenu(this);

    m_pImageList = nullptr;
    m_mapAvatarsToImageList.SetLessFunc(DefLessFunc(CSteamID));
    m_mapAvatarsToImageList.RemoveAll();

    m_fLastHeaderUpdate = 0.0f;
    m_flLastOnlineTimeUpdate = 0.0f;
    m_flLastFriendsTimeUpdate = 0.0f;

    m_bFirstHeaderUpdate = true;
    m_bFirstOnlineTimesUpdate = true;
    m_bFirstFriendsTimesUpdate = true;

    m_bMapInfoLoaded = false;

    flaggedRuns = RUNFLAG_NONE;

    m_umMapNames.SetLessFunc(PNamesMapLessFunc);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientTimesDisplay::~CClientTimesDisplay()
{
    m_pCurrentLeaderboards = nullptr;

    if (m_pLeaderboardReplayCMenu)
    {
        m_pLeaderboardReplayCMenu->DeletePanel();
        m_pLeaderboardReplayCMenu = nullptr;
    }
    if (cbGetFriendsTimesCallback.IsActive())
        cbGetFriendsTimesCallback.Cancel();

    if (cbGetMapInfoCallback.IsActive())
        cbGetMapInfoCallback.Cancel();

    if (cbGetOnlineTimesCallback.IsActive())
        cbGetOnlineTimesCallback.Cancel();

    if (cbGetPlayerDataForMapCallback.IsActive())
        cbGetPlayerDataForMapCallback.Cancel();
    // MOM_TODO: Ensure a good destructor
}

//-----------------------------------------------------------------------------
// Call every frame
//-----------------------------------------------------------------------------
void CClientTimesDisplay::OnThink()
{
    BaseClass::OnThink();

    // NOTE: this is necessary because of the way input works.
    // If a key down message is sent to vgui, then it will get the key up message
    // Sometimes the scoreboard is activated by other vgui menus,
    // sometimes by console commands. In the case where it's activated by
    // other vgui menus, we lose the key up message because this panel
    // doesn't accept keyboard input. It *can't* accept keyboard input
    // because another feature of the dialog is that if it's triggered
    // from within the game, you should be able to still run around while
    // the scoreboard is up. That feature is impossible if this panel accepts input.
    // because if a vgui panel is up that accepts input, it prevents the engine from
    // receiving that input. So, I'm stuck with a polling solution.
    //
    // Close key is set to non-invalid when something other than a keybind
    // brings the scoreboard up, and it's set to invalid as soon as the
    // dialog becomes hidden.

    if (m_nCloseKey != BUTTON_CODE_INVALID)
    {
        if (!g_pInputSystem->IsButtonDown(m_nCloseKey))
        {
            m_nCloseKey = BUTTON_CODE_INVALID;
            gViewPortInterface->ShowPanel(PANEL_TIMES, false);
            GetClientVoiceMgr()->StopSquelchMode();
        }
    }
}

//-----------------------------------------------------------------------------
// Called by vgui panels that activate the client scoreboard
//-----------------------------------------------------------------------------
void CClientTimesDisplay::OnPollHideCode(int code) { m_nCloseKey = static_cast<ButtonCode_t>(code); }

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CClientTimesDisplay::Reset() { Reset(false); }

void CClientTimesDisplay::Reset(bool pFullReset)
{
    if (m_pLocalLeaderboards)
    {
        m_pLocalLeaderboards->DeleteAllItems();
        m_pLocalLeaderboards->RemoveAllSections();
    }

    if (m_pOnlineLeaderboards)
    {
        m_pOnlineLeaderboards->DeleteAllItems();
        m_pOnlineLeaderboards->RemoveAllSections();
    }

    if (m_pFriendsLeaderboards)
    {
        m_pFriendsLeaderboards->DeleteAllItems();
        m_pFriendsLeaderboards->RemoveAllSections();
    }

    m_iSectionId = 0;
    m_fNextUpdateTime = 0;
    // add all the sections
    InitScoreboardSections();

    Update(pFullReset);
}

//-----------------------------------------------------------------------------
// Purpose: adds all the team sections to the scoreboard
//-----------------------------------------------------------------------------
void CClientTimesDisplay::InitScoreboardSections()
{
    if (m_pLocalLeaderboards)
    {
        m_pLocalLeaderboards->AddSection(m_iSectionId, "", StaticLocalTimeSortFunc);
        m_pLocalLeaderboards->SetSectionAlwaysVisible(m_iSectionId);
        m_pLocalLeaderboards->SetImageList(m_pImageList, false);
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "time", "#MOM_Time", 0, SCALE(m_aiColumnWidths[2]));
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "date", "#MOM_Date", 0, SCALE(m_aiColumnWidths[0]));
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "flags_input", "", SectionedListPanel::COLUMN_IMAGE, 16);
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "flags_movement", "", SectionedListPanel::COLUMN_IMAGE,
                                                 16);
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "flags_bonus", "", SectionedListPanel::COLUMN_IMAGE, 16);
    }

    if (m_pOnlineLeaderboards)
    {
        m_pOnlineLeaderboards->AddSection(m_iSectionId, "", StaticOnlineTimeSortFunc);
        m_pOnlineLeaderboards->SetSectionAlwaysVisible(m_iSectionId);
        m_pOnlineLeaderboards->SetImageList(m_pImageList, false);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "rank", "#MOM_Rank", SectionedListPanel::COLUMN_CENTER,
                                                  SCALE(m_aiColumnWidths[1]));
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "avatar", "",
                                                  SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER,
                                                  DEFAULT_AVATAR_SIZE);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name",
                                                  SectionedListPanel::COLUMN_CENTER, NAME_WIDTH);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "icon_tm", "", SectionedListPanel::COLUMN_IMAGE, 16);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "icon_vip", "", SectionedListPanel::COLUMN_IMAGE, 16);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "icon_friend", "", SectionedListPanel::COLUMN_IMAGE,
                                                  16);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "time_f", "#MOM_Time", 0, SCALE(m_aiColumnWidths[2]));
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "flags_input", "", SectionedListPanel::COLUMN_IMAGE,
                                                  16);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "flags_movement", "", SectionedListPanel::COLUMN_IMAGE,
                                                  16);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "flags_bonus", "", SectionedListPanel::COLUMN_IMAGE,
                                                  16);
    }

    if (m_pFriendsLeaderboards)
    {
        // We use online timer sort func as it's the same type of data
        m_pFriendsLeaderboards->AddSection(m_iSectionId, "", StaticOnlineTimeSortFunc);
        m_pFriendsLeaderboards->SetSectionAlwaysVisible(m_iSectionId);
        m_pFriendsLeaderboards->SetImageList(m_pImageList, false);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "rank", "#MOM_Rank", SectionedListPanel::COLUMN_CENTER,
                                                   SCALE(m_aiColumnWidths[1]));
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "avatar", "",
                                                   SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER,
                                                   DEFAULT_AVATAR_SIZE);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name",
                                                   SectionedListPanel::COLUMN_CENTER, NAME_WIDTH);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "icon_tm", "", SectionedListPanel::COLUMN_IMAGE, 16);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "icon_vip", "", SectionedListPanel::COLUMN_IMAGE, 16);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "icon_friend", "", SectionedListPanel::COLUMN_IMAGE,
                                                   16);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "time_f", "#MOM_Time",
                                                   SectionedListPanel::COLUMN_CENTER, SCALE(m_aiColumnWidths[2]));
        // Scroll only icon
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "flags_input", "", SectionedListPanel::COLUMN_IMAGE,
                                                   16);
        // HSW/SW/BW/WOnly Icons
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "flags_movement", "", SectionedListPanel::COLUMN_IMAGE,
                                                   16);
        // Bonus Icon
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "flags_bonus", "", SectionedListPanel::COLUMN_IMAGE,
                                                   16);
    }
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CClientTimesDisplay::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    if (m_pImageList)
        delete m_pImageList;
    m_pImageList = new ImageList(true);

    m_mapAvatarsToImageList.RemoveAll();

    for (int index = 0; index < ICON_TOTAL; index++)
    {
        m_IconsIndex[index] = -1;
        IImage *image = nullptr;
        switch (index)
        {
        case ICON_VIP:
            image = scheme()->GetImage("leaderboards_icon_vip", false);
            break;
        case ICON_TEAMMEMBER:
            image = scheme()->GetImage("leaderboards_icon_mom", false);
            break;
        case ICON_FRIEND:
            image = scheme()->GetImage("leaderboards_icon_friends", false);
            break;
        default:
            break;
        }
        if (image)
        {
            image->SetSize(16, 16);
            m_IconsIndex[index] = m_pImageList->AddImage(image);
        }
    }

    m_cFirstPlace = pScheme->GetColor("FirstPlace", Color(240, 210, 147, 50));
    m_cSecondPlace = pScheme->GetColor("SecondPlace", Color(175, 175, 175, 50));
    m_cThirdPlace = pScheme->GetColor("ThirdPlace", Color(205, 127, 50, 50));

    PostApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: Does dialog-specific customization after applying scheme settings.
//-----------------------------------------------------------------------------
void CClientTimesDisplay::PostApplySchemeSettings(IScheme *pScheme)
{
    // resize the images to our resolution
    /*for (int i = 0; i < m_pImageList->GetImageCount(); i++)
    {
    int wide, tall;
    m_pImageList->GetImage(i)->GetSize(wide, tall);
    m_pImageList->GetImage(i)->SetSize(SCALE(wide), SCALE(tall));
    }*/

    const char *columnNames[] = {DATESTRING, RANKSTRING, TIMESTRING};

    HFont font = pScheme->GetFont("Default", true);
    for (int i = 0; i < 3; i++)
    {
        const char *currName = columnNames[i];
        const int len = Q_strlen(currName);
        int pixels = 0;
        for (int currentChar = 0; currentChar < len; currentChar++)
        {
            pixels += surface()->GetCharacterWidth(font, currName[currentChar]);
        }
        m_aiColumnWidths[i] = pixels;
    }
    // DevLog("Widths %i %i %i \n", m_aiColumnWidths[0], m_aiColumnWidths[1], m_aiColumnWidths[2]);

    if (m_pMapName)
        m_pMapName->SetVisible(true);

    // Center the "WAITING FOR API RESPONSE" text in the leaderboards
    int textWidth, textHeight;
    m_pLoadingOnlineTimes->GetTextImage()->GetContentSize(textWidth, textHeight);
    int xPos = m_pLocalLeaderboards->GetWide() / 2 - (textWidth / 2);
    int yPos = m_pLocalLeaderboards->GetTall() / 2 - textHeight / 2;
    m_pLoadingOnlineTimes->SetPos(xPos, yPos);

    // Make it the size of the screen and center
    int screenWide, screenHeight;
    surface()->GetScreenSize(screenWide, screenHeight);
    MoveToCenterOfScreen();
    SetSize(screenWide, screenHeight);

    // Place and size the Filter panel properly
    xPos = m_pLeaderboards->GetXPos() + m_pLeaderboards->GetWide();
    yPos = m_pLeaderboards->GetYPos();
    m_pFilterPanel->SetPos(xPos + 4, yPos);
    m_pFilterPanel->SetSize(scheme()->GetProportionalScaledValue(150), m_pFilterPanel->GetTall());

    // light up scoreboard a bit
    SetBgColor(Color(0, 0, 0, 0));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CClientTimesDisplay::ShowPanel(bool bShow)
{
    // Catch the case where we call ShowPanel before ApplySchemeSettings, eg when
    // going from windowed <-> fullscreen
    if (!m_pImageList && bShow)
    {
        InvalidateLayout(true, true);
    }

    if (m_pLeaderboardReplayCMenu)
    {
        // Close the menu
        m_pLeaderboardReplayCMenu->OnKillFocus();
        m_pLeaderboardReplayCMenu->DeletePanel();
    }
    m_pLeaderboardReplayCMenu = nullptr;

    if (!bShow)
    {
        m_nCloseKey = BUTTON_CODE_INVALID;
    }

    if (BaseClass::IsVisible() == bShow)
        return;

    if (bShow)
    {
        Reset(true);
        SetVisible(true);
        // SetEnabled(true);
        MoveToFront();
    }
    else
    {
        SetVisible(false);
        SetMouseInputEnabled(false); // Turn mouse off
        SetKeyBoardInputEnabled(false);
    }
}

void CClientTimesDisplay::FireGameEvent(IGameEvent *event)
{
    if (!event)
        return;

    const char *type = event->GetName();

    if (FStrEq(type, "replay_save") && event->GetBool("save"))
    {
        // this updates the local times file, needing a reload of it
        m_bLocalTimesNeedUpdate = true;
    }
    else if (FStrEq(type, "run_upload"))
    {
        m_bFriendsNeedUpdate = m_bOnlineNeedUpdate = event->GetBool("run_posted");
    }
    else if (FStrEq(type, "game_newmap"))
    {
        m_bLocalTimesLoaded = false;
        m_bMapInfoLoaded = false;
        m_bFriendsNeedUpdate = true;
        m_bOnlineNeedUpdate = true;
    }

    // MOM_TODO: there's a crash here if you uncomment it,
    // if (IsVisible())
    //    Update(true);
}

bool CClientTimesDisplay::NeedsUpdate(void) { return (m_fNextUpdateTime < gpGlobals->curtime); }

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CClientTimesDisplay::Update(void) { Update(false); }

void CClientTimesDisplay::Update(bool pFullUpdate)
{
    // m_pPlayerList->DeleteAllItems();

    FillScoreBoard(pFullUpdate);

    MoveToCenterOfScreen();

    // update every X seconds
    // we don't need to update this too often. (Player is not finishing a run every second, so...)
    m_fNextUpdateTime = gpGlobals->curtime + DELAY_NEXT_UPDATE;

    // This starts as true on the constructor.
    m_bFirstHeaderUpdate = false;
    m_bFirstOnlineTimesUpdate = false;
    m_bFirstFriendsTimesUpdate = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CClientTimesDisplay::UpdatePlayerInfo(KeyValues *kv, bool fullUpdate)
{
    m_iSectionId = 0; // 0'th row is a header

    // add the player to the list
    KeyValues *playerData = new KeyValues("data");
    UpdatePlayerAvatarStandalone();

    player_info_t pi;
    engine->GetPlayerInfo(engine->GetLocalPlayer(), &pi);
    const char *oldName = playerData->GetString("name", pi.name);

    char newName[MAX_PLAYER_NAME_LENGTH];
    UTIL_MakeSafeName(oldName, newName, MAX_PLAYER_NAME_LENGTH);
    playerData->SetString("name", newName);
    // What this if is:
    // We want to do a full update if (we ask for it with fullUpdate boolean AND (the minimum time has passed OR it is
    // the first update)) OR the maximum time has passed
    if ((fullUpdate &&
         (gpGlobals->curtime - m_fLastHeaderUpdate >= MIN_ONLINE_UPDATE_INTERVAL || m_bFirstHeaderUpdate)) ||
        gpGlobals->curtime - m_fLastHeaderUpdate >= MAX_ONLINE_UPDATE_INTERVAL)
    {
        char p_sCalculating[BUFSIZELOCL];
        char p_sWaitingResponse[BUFSIZELOCL];
        LOCALIZE_TOKEN(p_wcCalculating, "MOM_Calculating", p_sCalculating);
        LOCALIZE_TOKEN(p_wcWaitingResponse, "MOM_API_WaitingForResponse", p_sWaitingResponse);

        char p_sMapRank[BUFSIZELOCL];
        char p_sGlobalRank[BUFSIZELOCL];
        char p_sPersonalBest[BUFSIZELOCL];
        char p_sExperiencePoints[BUFSIZELOCL];

        char mrLocalized[BUFSIZELOCL];
        char grLocalized[BUFSIZELOCL];
        char pbLocalized[BUFSIZELOCL];
        char xpLocalized[BUFSIZELOCL];

        LOCALIZE_TOKEN(p_wcMapRank, "MOM_MapRank", p_sMapRank);
        LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
        LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);
        LOCALIZE_TOKEN(p_wcExperiencePoints, "MOM_ExperiencePoints", p_sExperiencePoints);

        Q_snprintf(mrLocalized, BUFSIZELOCL, "%s: %s", p_sMapRank, p_sCalculating);
        Q_snprintf(grLocalized, BUFSIZELOCL, "%s: %s", p_sGlobalRank, p_sCalculating);
        Q_snprintf(pbLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sWaitingResponse);
        Q_snprintf(xpLocalized, BUFSIZELOCL, "%s: %s", p_sExperiencePoints, p_sWaitingResponse);

        m_pPlayerMapRank->SetText(mrLocalized);
        m_pPlayerGlobalRank->SetText(grLocalized);
        m_pPlayerPersonalBest->SetText(pbLocalized);
        m_pPlayerExperience->SetText(xpLocalized);

        char requrl[MAX_PATH];
        // Mapname, tickrate, rank, radius
        Q_snprintf(requrl, MAX_PATH, "%s/getusermaprank/%s/%llu", MOM_APIDOMAIN, g_pGameRules->MapName(),
                   GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64());
        CreateAndSendHTTPReq(requrl, &cbGetPlayerDataForMapCallback, &CClientTimesDisplay::GetPlayerDataForMapCallback);
        m_fLastHeaderUpdate = gpGlobals->curtime;
    }

    kv->AddSubKey(playerData);
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientTimesDisplay::AddHeader()
{
    if (m_pMapName)
    {
        m_pMapName->SetText(g_pGameRules->MapName());
        // Set the author label to be at the end of this label
        int wide, tall;
        m_pMapName->GetContentSize(wide, tall);
        m_pMapAuthor->SetPos(m_pMapName->GetXPos() + wide + SCALE(4),
                             m_pMapName->GetYPos() + tall - SCALE(surface()->GetFontTall(m_pMapAuthor->GetFont())));
    }

    if (m_pMapDetails && !m_bMapInfoLoaded)
    {
        char requrl[MAX_PATH];
        Q_snprintf(requrl, MAX_PATH, "%s/getmapinfo/%s", MOM_APIDOMAIN, g_pGameRules->MapName());
        CreateAndSendHTTPReq(requrl, &cbGetMapInfoCallback, &CClientTimesDisplay::GetMapInfoCallback);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting local times
//-----------------------------------------------------------------------------
bool CClientTimesDisplay::StaticLocalTimeSortFunc(SectionedListPanel *list, int itemID1, int itemID2)
{
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);

    float t1 = it1->GetFloat("time_f");
    float t2 = it2->GetFloat("time_f");
    // Ascending order
    if (t1 < t2)
        return true; // this time is faster, place it up higher
    if (t1 > t2)
        return false;

    // If the same, use IDs
    return itemID1 < itemID2;
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientTimesDisplay::StaticOnlineTimeSortFunc(SectionedListPanel *list, int itemID1, int itemID2)
{
    // Uses rank insetad of time (Momentum page will handle players with same times)
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);
    int t1 = it1->GetFloat("rank");
    int t2 = it2->GetFloat("rank");
    // Ascending order
    if (t1 < t2)
        return true; // this time is faster, place it up higher
    if (t1 > t2)
        return false;
    // We will *almost* never need this, but just in case...

    float s1 = it1->GetFloat("time");
    float s2 = it2->GetFloat("time");
    // Ascending order
    if (s1 < s2)
        return true; // this time is faster, place it up higher
    if (s1 > s2)
        return false;
    return itemID1 < itemID2;
}

void CClientTimesDisplay::LoadLocalTimes(KeyValues *kv)
{
    /*steamapicontext->SteamFriends()->RequestUserInformation()*/
    if (!m_bLocalTimesLoaded || m_bLocalTimesNeedUpdate)
    {
        // Clear the local times for a refresh
        m_vLocalTimes.PurgeAndDeleteElements();

        const char *mapName = g_pGameRules->MapName();
        char path[MAX_PATH];
        Q_snprintf(path, MAX_PATH, "%s/%s*%s", RECORDING_PATH, mapName, EXT_RECORDING_FILE);
        V_FixSlashes(path);

        FileFindHandle_t found;
        const char *pFoundFile = filesystem->FindFirstEx(path, "MOD", &found);
        while (pFoundFile)
        {
            // NOTE: THIS NEEDS TO BE MANUALLY CLEANED UP!
            char pReplayPath[MAX_PATH];
            V_ComposeFileName(RECORDING_PATH, pFoundFile, pReplayPath, MAX_PATH);

            CMomReplayBase *pBase = g_ReplayFactory.LoadReplayFile(pReplayPath, false);
            assert(pBase != nullptr);
            
            if (pBase)
                m_vLocalTimes.InsertNoSort(pBase);

            pFoundFile = filesystem->FindNext(found);
        }

        filesystem->FindClose(found);

        if (!m_vLocalTimes.IsEmpty())
        {
            m_vLocalTimes.RedoSort();
            m_bLocalTimesLoaded = true;
            m_bLocalTimesNeedUpdate = false;
        }
    }

    // Convert
    if (!m_vLocalTimes.IsEmpty())
        ConvertLocalTimes(kv);
}

void CClientTimesDisplay::ConvertLocalTimes(KeyValues *kvInto)
{
    FOR_EACH_VEC(m_vLocalTimes, i)
    {
        CMomReplayBase *t = m_vLocalTimes[i];

        KeyValues *kvLocalTimeFormatted = new KeyValues("localtime");
        char filename[MAX_PATH], runTime[MAX_PATH], runDate[MAX_PATH];

        // Don't ask why, but these need to be formatted in their own strings.
        Q_snprintf(runDate, MAX_PATH, "%li", t->GetRunDate());
        Q_snprintf(runTime, MAX_PATH, "%.3f", t->GetRunTime());
        // It's weird.

        Q_snprintf(filename, MAX_PATH, "%s-%s-%s%s", t->GetMapName(), runDate, runTime, EXT_RECORDING_FILE);
        kvLocalTimeFormatted->SetString("fileName", filename);

        kvLocalTimeFormatted->SetFloat("time_f", t->GetRunTime()); // Used for static compare
        kvLocalTimeFormatted->SetInt("date_t", t->GetRunDate());       // Used for finding

        char timeString[BUFSIZETIME];
        g_pMomentumUtil->FormatTime(t->GetRunTime(), timeString);
        kvLocalTimeFormatted->SetString("time", timeString); // Used for display
        
        char dateString[64];
        tm *local;
        time_t date = t->GetRunDate();
        local = localtime(&date);
        if (local)
        {
            strftime(dateString, sizeof(dateString), "%d/%m/%Y %H:%M:%S", local);
            kvLocalTimeFormatted->SetString("date", dateString);
        }
        else
            kvLocalTimeFormatted->SetInt("date", date);

        // MOM_TODO: Convert the run flags to pictures

        kvInto->AddSubKey(kvLocalTimeFormatted);
    }
}

void CClientTimesDisplay::ConvertOnlineTimes(KeyValues *kv, float seconds)
{
    char timeString[BUFSIZETIME];

    g_pMomentumUtil->FormatTime(seconds, timeString);
    kv->SetString("time_f", timeString);
}

void CClientTimesDisplay::LoadOnlineTimes()
{
    if (!m_bOnlineTimesLoaded || m_bOnlineNeedUpdate)
    {
        char requrl[BUFSIZ], format[BUFSIZ];
        // Mapname, tickrate, rank, radius

        Q_strcpy(format, "%s/getscores/%i/%s/10/%d");

        if (!m_bGetTop10Scores)
            Q_strcat(format, "/%llu", sizeof("/%llu"));

        Q_snprintf(requrl, BUFSIZ, format, MOM_APIDOMAIN, m_bGetTop10Scores ? 1 : 2, g_pGameRules->MapName(),
                   flaggedRuns, GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64());

        // This url is not real, just for testing purposes. It returns a json list with the serialization of the scores
        CreateAndSendHTTPReq(requrl, &cbGetOnlineTimesCallback, &CClientTimesDisplay::GetOnlineTimesCallback);
        m_bOnlineNeedUpdate = false;
        m_flLastOnlineTimeUpdate = gpGlobals->curtime;

        m_pLoadingOnlineTimes->SetVisible(m_pOnlineLeaderboards->IsVisible() || m_pFriendsLeaderboards->IsVisible());
    }
}

void CClientTimesDisplay::LoadFriendsTimes()
{
    if ((!m_bFriendsTimesLoaded || m_bFriendsNeedUpdate) && !m_bUnauthorizedFriendlist)
    {
        char requrl[BUFSIZ];
        Q_snprintf(requrl, BUFSIZ, "%s/getfriendscores/%llu/10/1/%s/%d", MOM_APIDOMAIN,
                   GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64(), g_pGameRules->MapName(),
                   flaggedRuns);
        CreateAndSendHTTPReq(requrl, &cbGetFriendsTimesCallback, &CClientTimesDisplay::GetFriendsTimesCallback);
        m_bFriendsNeedUpdate = false;
        m_flLastFriendsTimeUpdate = gpGlobals->curtime;
        m_pLoadingOnlineTimes->SetVisible(m_pOnlineLeaderboards->IsVisible() || m_pFriendsLeaderboards->IsVisible());
    }
}

void CClientTimesDisplay::CreateAndSendHTTPReq(const char *szURL,
                                               CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t> *callback,
                                               CCallResult<CClientTimesDisplay, HTTPRequestCompleted_t>::func_t func)
{
    if (steamapicontext && steamapicontext->SteamHTTP())
    {
        HTTPRequestHandle handle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, szURL);
        SteamAPICall_t apiHandle;

        if (steamapicontext->SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            Warning("%s - Callback set for %s.\n", __FUNCTION__, szURL);
            callback->Set(apiHandle, this, func);
        }
        else
        {
            Warning("Failed to send HTTP Request to post scores online!\n");
            steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("Steampicontext failure.\n");
    }
}

void CClientTimesDisplay::ParseTimesCallback(HTTPRequestCompleted_t* pCallback, bool bIOFailure, bool bFriendsTimes)
{
    // MOM_TODO: Tell the player the reason this list isn't loading
    // Look into making the "SetEmptyListText" method from ListPanel into a custom SectionedListPanel class

    Warning("%s - Callback received.\n", __FUNCTION__);
    if (bIOFailure)
    {
        Warning("%s - bIOFailure is true!\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode404NotFound)
    {
        Warning("%s - k_EHTTPStatusCode404NotFound !\n", __FUNCTION__);
        return;
    }
    if (pCallback->m_eStatusCode == k_EHTTPStatusCode409Conflict)
    {
        if (bFriendsTimes)
        {
            Warning("%s - Could not fetch player frindlist!\n", __FUNCTION__);
            m_bUnauthorizedFriendlist = true;
            return;
        }

        Warning("%s - No runs found for the map!\n", __FUNCTION__);
        return;
    }
   
    if (pCallback->m_eStatusCode == k_EHTTPStatusCode4xxUnknown)
    {
        Warning("%s - No friends found on this map. You must be a teapot!\n", __FUNCTION__);
        m_bUnauthorizedFriendlist = true;
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode500InternalServerError)
    {
        Warning("%s - INTERNAL SERVER ERROR!\n", __FUNCTION__);
        if (bFriendsTimes) m_bUnauthorizedFriendlist = true;
        return;
    }

    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - 0 body size!\n", __FUNCTION__);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            CKeyValuesDumpContextAsDevMsg dump;
            pResponse->Dump(&dump);
            KeyValues::AutoDelete ad(pResponse);
            KeyValues *pRuns = pResponse->FindKey("runs");

            if (pRuns && !pRuns->IsEmpty())
            {
                // By now we're pretty sure everything will be ok, so we can do this
                (bFriendsTimes ? m_vFriendsTimes : m_vOnlineTimes).PurgeAndDeleteElements();

                if (m_pLoadingOnlineTimes)
                    m_pLoadingOnlineTimes->SetVisible(true);

                // Iterate through each loaded run
                FOR_EACH_SUBKEY(pRuns, pRun)
                {
                    KeyValues *kvEntry = new KeyValues("Entry");
                    // Time is handled by the converter
                    kvEntry->SetFloat("time", pRun->GetFloat("time"));

                    // SteamID, Avatar, and Persona Name
                    uint64 steamID = Q_atoui64(pRun->GetString("steamid"));
                    kvEntry->SetUint64("steamid", steamID);
                    ISteamFriends *steamFriends = steamapicontext->SteamFriends();
                    if (steamFriends && steamapicontext->SteamUser())
                    {
                        if (ShowAvatars())
                        {
                            uint64 localSteamID = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
                            // These handle setting "avatar" for kvEntry
                            if (localSteamID == steamID)
                            {
                                UpdatePlayerAvatarStandaloneOnline(kvEntry);
                            }
                            else
                            {
                                UpdateLeaderboardPlayerAvatar(steamID, kvEntry);
                            }
                        }

                        // persona name
                        if (!steamFriends->RequestUserInformation(CSteamID(steamID), true))
                        {
                            kvEntry->SetString("personaname", steamFriends->GetFriendPersonaName(CSteamID(steamID)));
                        }
                        else
                        {
                            kvEntry->SetString("personaname", "Unknown");
                        }
                    }

                    // Persona name for the time they accomplished the run
                    kvEntry->SetString("personaname_onruntime", pRun->GetString("personaname_t"));

                    // Rank
                    kvEntry->SetInt("rank", static_cast<int>(pRun->GetFloat("rank")));
                    // MOM_TODO: Implement the other end of this (rank is not a number)

                    // Tickrate
                    kvEntry->SetInt("rate", static_cast<int>(pRun->GetFloat("rate")));

                    // Date
                    kvEntry->SetString("date", pRun->GetString("date"));

                    // ID
                    kvEntry->SetInt("id", static_cast<int>(pRun->GetFloat("id")));

                    // Is part of the momentum team?
                    kvEntry->SetBool("tm", pRun->GetBool("tm"));

                    // Is vip?
                    kvEntry->SetBool("vip", pRun->GetBool("vip"));

                    // Add this baby to the online times vector
                    TimeOnline *ot = new TimeOnline(kvEntry);
                    // Convert the time
                    ConvertOnlineTimes(ot->m_kv, ot->time_sec);
                    (bFriendsTimes ? m_vFriendsTimes : m_vOnlineTimes).AddToTail(ot);
                }

                // If we're here and no errors happened, then we can assume times were loaded
                (bFriendsTimes ? m_bFriendsTimesLoaded : m_bOnlineTimesLoaded) = true;
                (bFriendsTimes ? m_bFriendsNeedUpdate : m_bOnlineNeedUpdate) = false;
                (bFriendsTimes ? m_flLastFriendsTimeUpdate : m_flLastOnlineTimeUpdate) = gpGlobals->curtime;

                Update();
            }
            else
            {
                (bFriendsTimes ? m_bFriendsTimesLoaded : m_bOnlineTimesLoaded) = false;
            }
        }
    }
    else
    {
        (bFriendsTimes ? m_bFriendsTimesLoaded : m_bOnlineTimesLoaded) = false;
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void CClientTimesDisplay::GetOnlineTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    ParseTimesCallback(pCallback, bIOFailure, false);
}

void CClientTimesDisplay::GetFriendsTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    ParseTimesCallback(pCallback, bIOFailure, true);
}

void CClientTimesDisplay::GetPlayerDataForMapCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    Warning("%s - Callback received.\n", __FUNCTION__);
    if (bIOFailure)
        return;

    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - size is 0!\n", __FUNCTION__);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            KeyValues::AutoDelete ad(pResponse);

            int mrank = -1;
            int mtotal = -1;

            int grank = -1;
            int gtotal = -1;
            int gexp = -1;

            float seconds = 0.0f;

            KeyValues *pRun = pResponse->FindKey("run");
            if (pRun)
            {
                mrank = static_cast<int>(pRun->GetFloat("rank"));
                seconds = pRun->GetFloat("time");
            }

            KeyValues *pMap = pResponse->FindKey("mapranking");
            if (pMap)
            {
                mtotal = static_cast<int>(pMap->GetFloat("total", -1.0f));
            }

            KeyValues *pExperience = pResponse->FindKey("globalranking");
            if (pExperience)
            {
                grank = static_cast<int>(pExperience->GetFloat("rank"));
                gtotal = static_cast<int>(pExperience->GetFloat("total"));
                gexp = static_cast<int>(pExperience->GetFloat("experience"));
            }

            if (mrank > -1 && mtotal > -1)
            {
                char p_sMapRank[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcMapRank, "MOM_MapRank", p_sMapRank);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sMapRank, mrank, mtotal);
                m_pPlayerMapRank->SetText(p_sLocalized);
            }
            if (seconds > 0.0f)
            {
                char p_sPersonalBestTime[BUFSIZETIME];
                char p_sPersonalBest[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                g_pMomentumUtil->FormatTime(seconds, p_sPersonalBestTime);
                LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sPersonalBestTime);
                m_pPlayerPersonalBest->SetText(p_sLocalized);
            }

            if (grank > -1 && gtotal > -1)
            {
                char p_sGlobalRank[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sGlobalRank, grank, gtotal);
                m_pPlayerGlobalRank->SetText(p_sLocalized);

                char p_sExperience[BUFSIZELOCL];
                char p_sLocalized2[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcExperience, "MOM_ExperiencePoints", p_sExperience);
                Q_snprintf(p_sLocalized2, BUFSIZELOCL, "%s: %i", p_sExperience, gexp);
                m_pPlayerExperience->SetText(p_sLocalized2);
            }
            m_fLastHeaderUpdate = gpGlobals->curtime;
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    alloc.deallocate();
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void CClientTimesDisplay::GetMapInfoCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    Warning("%s - Callback received.\n", __FUNCTION__);
    if (bIOFailure)
    {
        UpdateMapInfoLabel(); // Default param is nullptr, so it hides it
        Warning("%s - bIOFailure is true!\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode409Conflict ||
        pCallback->m_eStatusCode == k_EHTTPStatusCode404NotFound)
    {
        char locl[BUFSIZELOCL];
        LOCALIZE_TOKEN(staged, "MOM_API_Unavailable", locl);
        UpdateMapInfoLabel(locl);
        Warning("%s - Map not found on server!\n", __FUNCTION__);
        return;
    }

    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        UpdateMapInfoLabel();
        Warning("%s - size is 0!\n", __FUNCTION__);
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    steamapicontext->SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

    JsonValue val; // Outer object
    JsonAllocator alloc;
    char *pDataPtr = reinterpret_cast<char *>(pData);
    char *endPtr;
    int status = jsonParse(pDataPtr, &endPtr, &val, alloc);

    if (status == JSON_OK)
    {
        DevLog("JSON Parsed!\n");
        if (val.getTag() == JSON_OBJECT) // Outer should be a JSON Object
        {
            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            KeyValues::AutoDelete ad(pResponse);
            if (pResponse)
            {
                const char *author = pResponse->GetString("submitter", "Unknown");
                const int tier = pResponse->GetInt("difficulty", -1);
                const int bonus = pResponse->GetInt("bonus", -1);
                char layout[BUFSIZELOCL];
                if (pResponse->GetBool("linear", false))
                {
                    LOCALIZE_TOKEN(linear, "MOM_Linear", layout);
                }
                else
                {
                    Q_snprintf(layout, BUFSIZELOCL, "%i STAGES", pResponse->GetInt("zones", -1));
                }

                UpdateMapInfoLabel(author, tier, layout, bonus);
                m_bMapInfoLoaded = true; // Stop this info from being fetched again
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    delete[] pData;
    pData = nullptr;
    alloc.deallocate();
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

//-----------------------------------------------------------------------------
// Purpose: Updates the leaderboard lists
//-----------------------------------------------------------------------------
bool CClientTimesDisplay::GetPlayerTimes(KeyValues *kv, bool fullUpdate)
{
    ConVarRef gm("mom_gamemode");
    if (!kv || gm.GetInt() == MOMGM_ALLOWED)
        return false;

    KeyValues *pLeaderboards = new KeyValues("leaderboards");

    // Fill local times:
    KeyValues *pLocal = new KeyValues("local");
    LoadLocalTimes(pLocal);
    pLeaderboards->AddSubKey(pLocal);

    m_bOnlineNeedUpdate =
        (fullUpdate && (gpGlobals->curtime - m_flLastOnlineTimeUpdate >= MIN_ONLINE_UPDATE_INTERVAL ||
                        m_bFirstOnlineTimesUpdate) ||
         (gpGlobals->curtime - m_flLastOnlineTimeUpdate >= MAX_ONLINE_UPDATE_INTERVAL || m_bOnlineNeedUpdate));

    m_bFriendsNeedUpdate =
        (fullUpdate && (gpGlobals->curtime - m_flLastFriendsTimeUpdate >= MIN_FRIENDS_UPDATE_INTERVAL ||
                        m_bFirstFriendsTimesUpdate) ||
         (gpGlobals->curtime - m_flLastFriendsTimeUpdate >= MAX_FRIENDS_UPDATE_INTERVAL || m_bFriendsNeedUpdate));

    // Fill online times only if needed
    LoadOnlineTimes();
    LoadFriendsTimes();

    kv->AddSubKey(pLeaderboards);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CClientTimesDisplay::UpdatePlayerAvatarStandaloneOnline(KeyValues *kv)
{
    // Load it if it hasn't been already
    if (!m_bLoadedLocalPlayerAvatar)
        UpdatePlayerAvatarStandalone();

    // Update their avatar
    int iImageIndex = -1;
    if (steamapicontext->SteamUser())
    {
        CSteamID steamIDForPlayer = steamapicontext->SteamUser()->GetSteamID();

        // See if we already have that avatar in our list
        int iMapIndex = m_mapAvatarsToImageList.Find(steamIDForPlayer);
        
        if (iMapIndex == m_mapAvatarsToImageList.InvalidIndex())
        {
            iImageIndex = m_pImageList->AddImage(m_pPlayerAvatar->GetImage());
            m_mapAvatarsToImageList.Insert(steamIDForPlayer, iImageIndex);
        }
        else
            iImageIndex = m_mapAvatarsToImageList[iMapIndex];
    }

    kv->SetBool("is_friend", false);
    kv->SetInt("avatar", iImageIndex);
}

void CClientTimesDisplay::UpdatePlayerAvatarStandalone()
{
    // Update their avatar
    if (ShowAvatars())
    {
        if (steamapicontext && steamapicontext->SteamUser())
        {
            if (!m_bLoadedLocalPlayerAvatar)
            {
                CSteamID steamIDForPlayer = steamapicontext->SteamUser()->GetSteamID();

                CAvatarImage *pImage = new CAvatarImage();
                // 64 is enough up to full HD resolutions.
                pImage->SetAvatarSteamID(steamIDForPlayer, k_EAvatarSize64x64);

                pImage->SetDrawFriend(false);
                pImage->SetAvatarSize(64, 64); // Deliberately non scaling, the ImagePanel does that for us

                // Get rid of the other image if it was there
                m_pPlayerAvatar->EvictImage();

                m_pPlayerAvatar->SetImage(pImage);
                m_bLoadedLocalPlayerAvatar = true;
            }
        }
        else
        {
            m_pPlayerAvatar->SetImage("default_steam");
        }
    }
}

void CClientTimesDisplay::UpdateLeaderboardPlayerAvatar(uint64 steamid, KeyValues *kv)
{
    // Update their avatar
    if (steamapicontext->SteamFriends())
    {
        kv->SetBool("is_friend", steamapicontext->SteamFriends()->HasFriend(steamid, k_EFriendFlagImmediate));
        kv->SetInt("avatar", TryAddAvatar(CSteamID(steamid)));
    }
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CClientTimesDisplay::FillScoreBoard()
{
    // update player info
    FillScoreBoard(false);
}

void CClientTimesDisplay::SetPlaceColors(SectionedListPanel *panel) const
{
    panel->SetItemBgColor(panel->GetItemIDFromRow(0), m_cFirstPlace);
    int itemCount = panel->GetItemCount();
    if (itemCount > 1)
    {
        panel->SetItemBgColor(panel->GetItemIDFromRow(1), m_cSecondPlace);

        if (itemCount > 2)
            panel->SetItemBgColor(panel->GetItemIDFromRow(2), m_cThirdPlace);
    }
}

void CClientTimesDisplay::FillScoreBoard(bool pFullUpdate)
{

    KeyValues *m_kvPlayerData = new KeyValues("playdata");
    UpdatePlayerInfo(m_kvPlayerData, pFullUpdate);
    if (pFullUpdate)
        AddHeader();

    // Player Stats panel:
    if (m_pPlayerStats && m_pPlayerAvatar && m_pPlayerName && m_pPlayerGlobalRank && m_pPlayerMapRank &&
        m_kvPlayerData && !m_kvPlayerData->IsEmpty())
    {
        m_pPlayerStats->SetVisible(false); // Hidden so it is not seen being changed

        KeyValues *playdata = m_kvPlayerData->FindKey("data");
        if (playdata)
            m_pPlayerName->SetText(playdata->GetString("name", "Unknown"));

        m_pPlayerStats->SetVisible(true); // And seen again!
    }

    // Leaderboards
    // @Gocnak: If we use event-driven updates and caching, it shouldn't overload the API.
    // The idea is to cache things to show, which will be called every FillScoreBoard call,
    // but it will only call the methods to update the data if booleans are set to.
    // For example, if a local time was saved, the event "timer_timesaved" or something is passed here,
    // and on the GetPlayerTimes passthrough, we'd update the local times, which then gets stored in
    // the Panel object until next update

    GetPlayerTimes(m_kvPlayerData, pFullUpdate);

    if (m_pLeaderboards && m_pOnlineLeaderboards && m_pLocalLeaderboards && m_pFriendsLeaderboards && m_kvPlayerData &&
        !m_kvPlayerData->IsEmpty())
    {
        m_pLeaderboards->SetVisible(false);

        if (m_pCurrentLeaderboards == m_pLocalLeaderboards)
        {
            KeyValues *kvLeaderboards = m_kvPlayerData->FindKey("leaderboards");
            KeyValues *kvLocalTimes = kvLeaderboards->FindKey("local");
            if (kvLocalTimes && !kvLocalTimes->IsEmpty())
            {
                FOR_EACH_SUBKEY(kvLocalTimes, kvLocalTime)
                {
                    int itemID = FindItemIDForLocalTime(kvLocalTime);
                    if (itemID == -1)
                        m_pLocalLeaderboards->AddItem(m_iSectionId, kvLocalTime);
                    else
                        m_pLocalLeaderboards->ModifyItem(itemID, m_iSectionId, kvLocalTime);
                }

                SetPlaceColors(m_pLocalLeaderboards);
            }
        }
        // Online works slightly different, we use the vector content, not the ones from m_kvPlayerData (because
        // online
        // times are not stored there)
        else if (m_pCurrentLeaderboards == m_pOnlineLeaderboards)
        {
            OnlineTimesVectorToLeaderboards(ONLINE_LEADERBOARDS);
        }
        else if (m_pCurrentLeaderboards == m_pFriendsLeaderboards)
        {
            OnlineTimesVectorToLeaderboards(FRIENDS_LEADERBOARDS);
        }

        m_pLeaderboards->SetVisible(true);
    }
    m_kvPlayerData->deleteThis();
}

void CClientTimesDisplay::OnlineTimesVectorToLeaderboards(LEADERBOARDS pLeaderboard)
{
    CUtlVector<TimeOnline *> *pVector;
    SectionedListPanel *pList;
    switch (pLeaderboard)
    {
    case FRIENDS_LEADERBOARDS:
        pVector = &m_vFriendsTimes;
        pList = m_pFriendsLeaderboards;
        break;
    case ONLINE_LEADERBOARDS:
        pVector = &m_vOnlineTimes;
        pList = m_pOnlineLeaderboards;
        break;
    default:
        return;
    }
    if (pVector && pVector->Count() > 0 && pList)
    {
        FOR_EACH_VEC(*pVector, entry)
        {
            // We set the current personaname before anything...
            // Find method is not being nice, so we craft our own, which _might_ be faster
            TimeOnline *runEntry = pVector->Element(entry);
            FOR_EACH_MAP_FAST(m_umMapNames, mIter)
            {
                if (m_umMapNames.Key(mIter) == runEntry->steamid)
                {
                    const char *personaname = m_umMapNames.Element(mIter);
                    if (Q_strcmp(personaname, "") != 0)
                    {
                        runEntry->m_kv->SetString("personaname", personaname);
                    }
                    break;
                }
            }

            int itemID = FindItemIDForOnlineTime(runEntry->id, pLeaderboard);

            runEntry->m_kv->SetInt("icon_tm", runEntry->momember ? m_IconsIndex[ICON_TEAMMEMBER] : -1);
            runEntry->m_kv->SetInt("icon_vip", runEntry->vip ? m_IconsIndex[ICON_VIP] : -1);
            runEntry->m_kv->SetInt("icon_friend", runEntry->is_friend ? m_IconsIndex[ICON_FRIEND] : -1);

            if (itemID == -1)
            {
                itemID = pList->AddItem(m_iSectionId, runEntry->m_kv);
            }
            else
            {
                pList->ModifyItem(itemID, m_iSectionId, runEntry->m_kv);
            }
        }

        SetPlaceColors(pList);

        if (m_pLoadingOnlineTimes)
        {
            m_pLoadingOnlineTimes->SetVisible(false);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: searches for the player in the scoreboard
//-----------------------------------------------------------------------------
int CClientTimesDisplay::FindItemIDForLocalTime(KeyValues *kvRef)
{
    for (int i = 0; i <= m_pLocalLeaderboards->GetHighestItemID(); i++)
    {
        if (m_pLocalLeaderboards->IsItemIDValid(i))
        {
            KeyValues *kv = m_pLocalLeaderboards->GetItemData(i);
            if (kv && (kv->GetInt("date_t") == kvRef->GetInt("date_t")))
            {
                return i;
            }
        }
    }
    return -1;
}

int CClientTimesDisplay::FindItemIDForOnlineTime(int runID, LEADERBOARDS leaderboard)
{
    SectionedListPanel *pLeaderboard;
    switch (leaderboard)
    {
    case FRIENDS_LEADERBOARDS:
        pLeaderboard = m_pFriendsLeaderboards;
        break;
    case ONLINE_LEADERBOARDS:
        pLeaderboard = m_pOnlineLeaderboards;
        break;
    default:
        return -1;
    }
    for (int i = 0; i <= pLeaderboard->GetHighestItemID(); i++)
    {
        if (pLeaderboard->IsItemIDValid(i))
        {
            KeyValues *kv = pLeaderboard->GetItemData(i);
            if (kv && (kv->GetInt("id", -1) == runID))
            {
                return i;
            }
        }
    }
    return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CClientTimesDisplay::MoveLabelToFront(const char *textEntryName)
{
    Label *entry = FindControl<Label>(textEntryName, true);
    if (entry)
    {
        entry->MoveToFront();
    }
}

//-----------------------------------------------------------------------------
// Purpose: Center the dialog on the screen.  (vgui has this method on
//			Frame, but we're an EditablePanel, need to roll our own.)
//-----------------------------------------------------------------------------
void CClientTimesDisplay::MoveToCenterOfScreen()
{
    int wx, wy, ww, wt;
    surface()->GetWorkspaceBounds(wx, wy, ww, wt);
    SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}

CReplayContextMenu *CClientTimesDisplay::GetLeaderboardReplayContextMenu(Panel *pParent)
{
    // create a drop down for this object's states
    // This will stop being created after the second time you open the leaderboards?
    if (m_pLeaderboardReplayCMenu)
        delete m_pLeaderboardReplayCMenu;

    m_pLeaderboardReplayCMenu = new CReplayContextMenu(this);
    m_pLeaderboardReplayCMenu->SetAutoDelete(false);
    m_pLeaderboardReplayCMenu->SetParent(pParent ? pParent : this);
    m_pLeaderboardReplayCMenu->AddActionSignalTarget(pParent ? pParent : nullptr);
    m_pLeaderboardReplayCMenu->SetVisible(false);

    return m_pLeaderboardReplayCMenu;
}

void CClientTimesDisplay::OnContextWatchReplay(const char *runName)
{
    if (runName)
    {
        char command[MAX_PATH];
        Q_snprintf(command, MAX_PATH, "mom_replay_play %s", runName);
        engine->ServerCmd(command);
        ShowPanel(false);
    }
}

void CClientTimesDisplay::OnContextDeleteReplay(const char* runName)
{
    if (runName)
    {
        char file[MAX_PATH];
        V_ComposeFileName(RECORDING_PATH, runName, file, MAX_PATH);

        messageboxpanel->CreateConfirmationBox(this, "#MOM_Leaderboards_DeleteReplay", 
            "#MOM_MB_DeleteRunConfirmation", new KeyValues("ConfirmDeleteReplay", "file", file),
             nullptr, "#MOM_Leaderboards_DeleteReplay");
    }
}

void CClientTimesDisplay::OnConfirmDeleteReplay(KeyValues* data)
{
    if (data)
    {
        const char * file = data->GetString("file", nullptr);
        if (file)
        {
            g_pFullFileSystem->RemoveFile(file, "MOD");
            m_bLocalTimesNeedUpdate = true;
            FillScoreBoard();
        }
    }
}


inline bool CheckParent(Panel *pPanel, SectionedListPanel *pParentToCheck, int itemID)
{
    return pPanel->GetParent() == pParentToCheck && pParentToCheck->IsItemIDValid(itemID);
}

void CClientTimesDisplay::OnItemContextMenu(KeyValues *pData)
{
    int itemID = pData->GetInt("itemID", -1);
    Panel *pPanel = static_cast<Panel *>(pData->GetPtr("SubPanel", nullptr));
    if (pPanel && pPanel->GetParent())
    {
        if (CheckParent(pPanel, m_pLocalLeaderboards, itemID))
        {
            KeyValues *selectedRun = m_pLocalLeaderboards->GetItemData(itemID);

            const char *pFileName = selectedRun->GetString("fileName");

            CReplayContextMenu *pContextMenu = GetLeaderboardReplayContextMenu(pPanel->GetParent());
            pContextMenu->AddMenuItem("StartMap", "#MOM_Leaderboards_WatchReplay", new KeyValues("ContextWatchReplay", "runName", pFileName), this);
            pContextMenu->AddSeparator();
            pContextMenu->AddMenuItem("DeleteRun", "#MOM_Leaderboards_DeleteReplay", new KeyValues("ContextDeleteReplay", "runName", pFileName), this);
            pContextMenu->ShowMenu();
        } 
        else if (CheckParent(pPanel, m_pFriendsLeaderboards, itemID) || CheckParent(pPanel, m_pOnlineLeaderboards, itemID))
        {
            SectionedListPanel *pLeaderboard = static_cast<SectionedListPanel *>(pPanel->GetParent());
            CReplayContextMenu *pContextMenu = GetLeaderboardReplayContextMenu(pLeaderboard);
            KeyValues *pKv = new KeyValues("ContextVisitProfile");
            pKv->SetUint64("profile", pLeaderboard->GetItemData(itemID)->GetUint64("steamid"));
            pContextMenu->AddMenuItem("VisitProfile", "#MOM_Leaderboards_SteamProfile", pKv, this);
            pContextMenu->ShowMenu();
        }
    }
}

void CClientTimesDisplay::OnContextVisitProfile(uint64 profile)
{
    if (profile != 0 && steamapicontext && steamapicontext->SteamFriends())
    {
        steamapicontext->SteamFriends()->ActivateGameOverlayToUser("steamid", CSteamID(profile));
    }
}

void CClientTimesDisplay::OnPersonaStateChange(PersonaStateChange_t *pParam)
{
    if (pParam->m_nChangeFlags & k_EPersonaChangeNameFirstSet || pParam->m_nChangeFlags & k_EPersonaChangeName)
    {
        m_umMapNames.InsertOrReplace(
            pParam->m_ulSteamID, steamapicontext->SteamFriends()->GetFriendPersonaName(CSteamID(pParam->m_ulSteamID)));

        OnlineTimesVectorToLeaderboards(ONLINE_LEADERBOARDS);

        OnlineTimesVectorToLeaderboards(FRIENDS_LEADERBOARDS);
    }
}

int CClientTimesDisplay::TryAddAvatar(const CSteamID &steamid)
{
    // Update their avatar
    if (ShowAvatars() && steamapicontext->SteamFriends() && steamapicontext->SteamUtils())
    {
        // See if we already have that avatar in our list

        int iMapIndex = m_mapAvatarsToImageList.Find(steamid);
        int iImageIndex;
        if (iMapIndex == m_mapAvatarsToImageList.InvalidIndex())
        {
            CAvatarImage *pImage = new CAvatarImage();
            // 64 is enough up to full HD resolutions.
            pImage->SetAvatarSteamID(steamid, k_EAvatarSize64x64);

            pImage->SetDrawFriend(false);
            pImage->SetAvatarSize(32, 32);
            iImageIndex = m_pImageList->AddImage(pImage);
            m_mapAvatarsToImageList.Insert(steamid, iImageIndex);
        }
        else
        {
            iImageIndex = m_mapAvatarsToImageList[iMapIndex];
        }
        return iImageIndex;
    }
    return -1;
}

void CClientTimesDisplay::OnCommand(const char *pCommand)
{
    BaseClass::OnCommand(pCommand);
    // MOM_TODO: Implement run tags
    // Leaderboards type
    bool isTop10 = FStrEq(pCommand, "GlobalTypeTop10");
    bool isAround = FStrEq(pCommand, "GlobalTypeAround");
    bool isLocal = FStrEq(pCommand, "ShowLocal");
    bool isFriends = FStrEq(pCommand, "ShowFriends");
    bool isGlobal = FStrEq(pCommand, "ShowGlobal");
    bool isFilter = FStrEq(pCommand, "ShowFilter");
    bool isReset = FStrEq(pCommand, "ResetFlags");
    bool isFlagScrollOnly = FStrEq(pCommand, "ToggleScrollOnly");
    bool isFlagWOnly = FStrEq(pCommand, "ToggleWOnly");
    bool isFlagHSW = FStrEq(pCommand, "ToggleHSW");
    bool isFlagSideways = FStrEq(pCommand, "ToggleSideways");
    bool isFlagBackwards = FStrEq(pCommand, "ToggleBackwards");
    bool isFlagBonus = FStrEq(pCommand, "ToggleBonus");
    if (isTop10 || isAround)
    {
        m_pFriendsLeaderboardsButton->SetEnabled(true);

        bool wasTop10Scores = m_bGetTop10Scores;
        m_bGetTop10Scores = isTop10;

        m_pGlobalTop10Button->SetEnabled(!isTop10);
        m_pGlobalAroundButton->SetEnabled(isTop10);

        if (wasTop10Scores != m_bGetTop10Scores && m_pOnlineLeaderboards)
        {
            m_vOnlineTimes.PurgeAndDeleteElements();
            m_pOnlineLeaderboards->RemoveAll();
            m_bOnlineNeedUpdate = true;
            Update();
        }
    }
    else if (isLocal || isFriends || isGlobal)
    {
        // Show the right type of leaderboards
        m_pCurrentLeaderboards->SetVisible(false);
        m_pCurrentLeaderboards =
            isGlobal ? m_pOnlineLeaderboards : isLocal ? m_pLocalLeaderboards : m_pFriendsLeaderboards;
        m_pCurrentLeaderboards->SetVisible(true);

        m_pGlobalLeaderboardsButton->SetEnabled(!isGlobal && !isFriends);
        m_pFriendsLeaderboardsButton->SetEnabled(!isFriends);
        m_pLocalLeaderboardsButton->SetEnabled(!isLocal);

        m_pGlobalTop10Button->SetVisible(isGlobal || isFriends);
        m_pGlobalAroundButton->SetVisible(isGlobal || isFriends);
        m_pFriendsLeaderboardsButton->SetVisible(isGlobal || isFriends);

        if (isLocal)
            m_pLoadingOnlineTimes->SetVisible(false);

        else if (isFriends)
        {
            m_pGlobalAroundButton->SetEnabled(true);
            m_pGlobalTop10Button->SetEnabled(true);
        }
    }
    else if (isFilter)
    {
        m_pFilterPanel->SetVisible(m_pRunFilterButton->IsSelected());
    }
    else if (isReset)
    {
        flaggedRuns = RUNFLAG_NONE;
        for (int i = 0; i < m_pFilterPanel->GetChildCount(); i++)
        {
            ToggleButton *pChild = dynamic_cast<ToggleButton*>(m_pFilterPanel->GetChild(i));
            if (pChild)
            {
                pChild->ForceDepressed(false);
                pChild->SetSelected(false);
            }
        }
    }
    else if (isFlagScrollOnly)
    {
        flaggedRuns = static_cast<RUN_FLAG>(flaggedRuns ^ RUNFLAG_SCROLL);
    }
    else if (isFlagWOnly)
    {
        flaggedRuns = static_cast<RUN_FLAG>(flaggedRuns ^ RUNFLAG_W_ONLY);
    }
    else if (isFlagHSW)
    {
        flaggedRuns = static_cast<RUN_FLAG>(flaggedRuns ^ RUNFLAG_HSW);
    }
    else if (isFlagSideways)
    {
        flaggedRuns = static_cast<RUN_FLAG>(flaggedRuns ^ RUNFLAG_SW);
    }
    else if (isFlagBackwards)
    {
        flaggedRuns = static_cast<RUN_FLAG>(flaggedRuns ^ RUNFLAG_BW);
    }
    else if (isFlagBonus)
    {
        flaggedRuns = static_cast<RUN_FLAG>(flaggedRuns ^ RUNFLAG_BONUS);
    }
    else
    {
        DevLog("Caught an unhandled command: %s\n", pCommand);
    }
}

void CClientTimesDisplay::UpdateMapInfoLabel(const char *author, const int tier, const char *layout, const int bonus)
{
    if (m_pMapDetails && m_pMapAuthor)
    {
        char mapAuthor[MAX_PLAYER_NAME_LENGTH + 3];
        Q_snprintf(mapAuthor, MAX_PLAYER_NAME_LENGTH + 3, "By %s", author);
        m_pMapAuthor->SetText(mapAuthor);

        char mapDetails[BUFSIZ];
        Q_snprintf(mapDetails, BUFSIZ, "TIER %i - %s - %i BONUS", tier, layout, bonus);
        UpdateMapInfoLabel(mapDetails);
    }
}

void CClientTimesDisplay::UpdateMapInfoLabel(const char *text)
{
    if (m_pMapDetails)
    {
        m_pMapDetails->SetText(text);
        if (text == nullptr)
        {
            m_pMapDetails->SetVisible(false);
        }
    }
}
