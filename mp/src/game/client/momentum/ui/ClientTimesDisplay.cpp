//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"

#include <stdio.h>

#include "ClientTimesDisplay.h"
#include "IGameUIFuncs.h" // for key bindings
#include "inputsystem/iinputsystem.h"
#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>
#include <voice_status.h>

#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include <game/client/iviewport.h>
#include <igameresources.h>

#include "filesystem.h"
#include "util\mom_util.h"
#include "vgui_avatarimage.h"
#include <time.h>
#include <util/jsontokv.h>

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
CClientTimesDisplay::CClientTimesDisplay(IViewPort *pViewPort) : EditablePanel(nullptr, PANEL_TIMES)
{
    m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");
    m_nCloseKey = BUTTON_CODE_INVALID;

    m_pViewPort = pViewPort;
    // initialize dialog
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    // Create a "popup" so we can get the mouse to detach
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    // set the scheme before any child control is created
    SetScheme("ClientScheme");

    m_pPlayerList = new SectionedListPanel(this, "PlayerList");
    m_pPlayerList->SetVerticalScrollbar(false);

    LoadControlSettings("resource/ui/timesdisplay.res");

    m_pHeader = FindControl<Panel>("Header", true);
    m_lMapSummary = FindControl<Label>("MapSummary", true);
    m_lMapDetails = FindControl<Label>("MapDetails", true);
    m_pMomentumLogo = FindControl<ImagePanel>("MomentumLogo", true);
    m_pPlayerStats = FindControl<Panel>("PlayerStats", true);
    m_pPlayerAvatar = FindControl<ImagePanel>("PlayerAvatar", true);
    m_lPlayerName = FindControl<Label>("PlayerName", true);
    m_lPlayerMapRank = FindControl<Label>("PlayerMapRank", true);
    m_lPlayerPersonalBest = FindControl<Label>("PlayerPersonalBest", true);
    m_lPlayerGlobalRank = FindControl<Label>("PlayerGlobalRank", true);
    m_lPlayerExperience = FindControl<Label>("PlayerExperience", true);
    m_lLoadingOnlineTimes = FindControl<Label>("LoadingOnlineTimes", true);
    m_pLeaderboards = FindControl<Panel>("Leaderboards", true);
    m_pOnlineLeaderboards = FindControl<SectionedListPanel>("OnlineLeaderboards", true);
    m_pLocalLeaderboards = FindControl<SectionedListPanel>("LocalLeaderboards", true);
    m_pFriendsLeaderboards = FindControl<SectionedListPanel>("FriendsLeaderboards", true);
    m_pGlobalLeaderboardsButton = FindControl<Button>("GlobalLeaderboarsButton", true);
    m_pFriendsLeaderboardsButton = FindControl<Button>("FriendsLeaderboardsButton", true);

    if (!m_pHeader || !m_lMapSummary || !m_pPlayerStats || !m_pPlayerAvatar || !m_lPlayerName || !m_pMomentumLogo ||
        !m_lPlayerMapRank || !m_lPlayerGlobalRank || !m_pLeaderboards || !m_pOnlineLeaderboards ||
        !m_pLocalLeaderboards || !m_pFriendsLeaderboards || !m_lPlayerPersonalBest || !m_lLoadingOnlineTimes || !m_lPlayerExperience ||
        !m_pGlobalLeaderboardsButton || !m_pFriendsLeaderboardsButton)
    {
        Assert("Null pointer(s) on scoreboards");
    }
    SetSize(scheme()->GetProportionalScaledValue(480), scheme()->GetProportionalScaledValue(480));

    m_pHeader->SetParent(this);
    m_pPlayerStats->SetParent(this);
    m_pLeaderboards->SetParent(this);
    m_lMapSummary->SetParent(m_pHeader);
    m_lMapDetails->SetParent(m_pHeader);
    m_pMomentumLogo->SetParent(m_pPlayerStats);
    m_pPlayerAvatar->SetParent(m_pPlayerStats);
    m_lPlayerName->SetParent(m_pPlayerStats);
    m_lPlayerMapRank->SetParent(m_pPlayerStats);
    m_lPlayerGlobalRank->SetParent(m_pPlayerStats);
    m_lPlayerPersonalBest->SetParent(m_pPlayerStats);
    m_lPlayerExperience->SetParent(m_pPlayerStats);
    m_pOnlineLeaderboards->SetParent(m_pLeaderboards);
    m_lLoadingOnlineTimes->SetParent(m_pLeaderboards);
    m_pLocalLeaderboards->SetParent(m_pLeaderboards);
    m_pFriendsLeaderboards->SetParent(m_pLeaderboards);
    m_pGlobalLeaderboardsButton->SetParent(m_pLeaderboards);
    m_pFriendsLeaderboardsButton->SetParent(m_pLeaderboards);

    m_pGlobalLeaderboardsButton->SetMouseInputEnabled(true);
    m_pGlobalLeaderboardsButton->AddActionSignalTarget(this);
    m_pFriendsLeaderboardsButton->SetMouseInputEnabled(true);
    m_pFriendsLeaderboardsButton->AddActionSignalTarget(this);
    m_pGlobalLeaderboardsButton->SetPos(86, 3);
    m_pFriendsLeaderboardsButton->SetPos(3, 3);
    m_pGlobalLeaderboardsButton->SetCommand(new KeyValues("ToggleLeaderboard", "leaderboard", "global"));
    m_pFriendsLeaderboardsButton->SetCommand(new KeyValues("ToggleLeaderboard", "leaderboard", "friend"));
    m_pGlobalLeaderboardsButton->SetEnabled(false);
    m_pOnlineLeaderboards->SetVisible(true);
    m_pFriendsLeaderboards->SetVisible(false);

    m_pOnlineLeaderboards->SetVerticalScrollbar(false);
    m_pLocalLeaderboards->SetVerticalScrollbar(false);
    m_pFriendsLeaderboards->SetVerticalScrollbar(false);

    m_pLeaderboards->SetMouseInputEnabled(true);

    m_pLocalLeaderboards->SetMouseInputEnabled(true);
    m_pOnlineLeaderboards->SetMouseInputEnabled(true);
    m_pFriendsLeaderboards->SetMouseInputEnabled(true);

    m_pLocalLeaderboards->SetKeyBoardInputEnabled(true);
    m_pLocalLeaderboards->SetClickable(true);
    m_pLocalLeaderboards->AddActionSignalTarget(this);

    m_pOnlineLeaderboards->SetKeyBoardInputEnabled(true);
    m_pOnlineLeaderboards->SetClickable(true);
    m_pOnlineLeaderboards->AddActionSignalTarget(this);

    m_pFriendsLeaderboards->SetKeyBoardInputEnabled(true);
    m_pFriendsLeaderboards->SetClickable(true);
    m_pFriendsLeaderboards->AddActionSignalTarget(this);

    m_pOnlineLeaderboards->SetPaintBorderEnabled(true);
    m_pFriendsLeaderboards->SetPaintBorderEnabled(true);

    m_pMomentumLogo->GetImage()->SetSize(scheme()->GetProportionalScaledValue(256),
                                         scheme()->GetProportionalScaledValue(64));

    m_iDesiredHeight = GetTall();
    m_pPlayerList->SetVisible(false); // hide this until we load the images in applyschemesettings
    m_lLoadingOnlineTimes->SetVisible(false);

    // update scoreboard instantly if on of these events occur
    ListenForGameEvent("run_save");
    ListenForGameEvent("run_upload");
    ListenForGameEvent("game_newmap");

    m_pLeaderboardReplayCMenu = new CReplayContextMenu(this);

    m_pImageList = nullptr;
    m_mapAvatarsToImageList.SetLessFunc(DefLessFunc(CSteamID));
    m_mapAvatarsToImageList.RemoveAll();

    m_fLastHeaderUpdate = 0.0f;
    m_flLastOnlineTimeUpdate = 0.0f;

    m_bFirstHeaderUpdate = true;
    m_bFirstOnlineTimesUpdate = true;

    m_umMapNames.SetLessFunc(PNamesMapLessFunc);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientTimesDisplay::~CClientTimesDisplay()
{
    if (m_pImageList)
    {
        delete m_pImageList;
        m_pImageList = nullptr;
    }

    if (m_pLeaderboardReplayCMenu)
    {
        delete m_pLeaderboardReplayCMenu;
        m_pLeaderboardReplayCMenu = nullptr;
    }
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
void CClientTimesDisplay::OnPollHideCode(int code) { m_nCloseKey = (ButtonCode_t)code; }

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CClientTimesDisplay::Reset() { Reset(false); }

void CClientTimesDisplay::Reset(bool pFullReset)
{
    // clear
    m_pPlayerList->DeleteAllItems();
    m_pPlayerList->RemoveAllSections();

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
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "time", "#MOM_Time", 0, SCALE(m_aiColumnWidths[2]));
        m_pLocalLeaderboards->AddColumnToSection(m_iSectionId, "date", "#MOM_Date", 0, SCALE(m_aiColumnWidths[0]));
    }

    if (m_pOnlineLeaderboards)
    {
        m_pOnlineLeaderboards->AddSection(m_iSectionId, "", StaticOnlineTimeSortFunc);
        m_pOnlineLeaderboards->SetSectionAlwaysVisible(m_iSectionId);
        m_pOnlineLeaderboards->SetImageList(m_pImageList, false);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "rank", "#MOM_Rank", 0, SCALE(m_aiColumnWidths[1]));
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "avatar", "", 2, 64);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name", 0, NAME_WIDTH);
        m_pOnlineLeaderboards->AddColumnToSection(m_iSectionId, "time_f", "#MOM_Time", 0, SCALE(m_aiColumnWidths[2]));
    }

    if (m_pFriendsLeaderboards)
    {
        // We use online timer sort func as it's the same type of data
        m_pFriendsLeaderboards->AddSection(m_iSectionId, "", StaticOnlineTimeSortFunc);
        m_pFriendsLeaderboards->SetSectionAlwaysVisible(m_iSectionId);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "rank", "#MOM_Rank", 0, SCALE(m_aiColumnWidths[1]));
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "avatar", "", 2, 64);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name", 0, NAME_WIDTH);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "time_f", "#MOM_Time", 0, SCALE(m_aiColumnWidths[2]));
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
    m_pImageList = new ImageList(false);

    m_mapAvatarsToImageList.RemoveAll();

    PostApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: Does dialog-specific customization after applying scheme settings.
//-----------------------------------------------------------------------------
void CClientTimesDisplay::PostApplySchemeSettings(vgui::IScheme *pScheme)
{
    // resize the images to our resolution
    for (int i = 0; i < m_pImageList->GetImageCount(); i++)
    {
        int wide, tall;
        m_pImageList->GetImage(i)->GetSize(wide, tall);
        m_pImageList->GetImage(i)->SetSize(SCALE(wide), SCALE(tall));
    }

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

    m_pPlayerList->SetImageList(m_pImageList, false);
    m_pPlayerList->SetVisible(true);

    if (m_lMapSummary)
        m_lMapSummary->SetVisible(true);

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
    if (m_pImageList == nullptr)
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

    if (Q_strcmp(type, "run_save") == 0)
    {
        // this updates the local times file, needing a reload of it
        m_bLocalTimesNeedUpdate = true;
    }
    else if (Q_strcmp(type, "run_upload") == 0)
    {
        // MOM_TODO: this updates your rank (friends/online panel)
        m_bOnlineNeedUpdate = event->GetBool("run_posted");
    }
    else if (Q_strcmp(type, "game_newmap") == 0)
    {
        m_bLocalTimesLoaded = false;
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

    // grow the scoreboard to fit all the players
    int wide, tall;
    m_pPlayerList->GetContentSize(wide, tall);
    tall += GetAdditionalHeight();
    wide = GetWide();
    if (m_iDesiredHeight < tall)
    {
        SetSize(wide, tall);
        m_pPlayerList->SetSize(wide, tall);
    }
    else
    {
        SetSize(wide, m_iDesiredHeight);
        m_pPlayerList->SetSize(wide, m_iDesiredHeight);
    }
    MoveToCenterOfScreen();

    // update every X seconds
    // we don't need to update this too often. (Player is not finishing a run every second, so...)
    m_fNextUpdateTime = gpGlobals->curtime + DELAY_NEXT_UPDATE;

    // This starts as true on the constructor.
    m_bFirstHeaderUpdate = false;
    m_bFirstOnlineTimesUpdate = false;
}

//-----------------------------------------------------------------------------
// Purpose: Sort all the teams
//-----------------------------------------------------------------------------
void CClientTimesDisplay::UpdateTeamInfo()
{
    // TODO: work out a sorting algorithm for team display for TF2
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CClientTimesDisplay::UpdatePlayerInfo(KeyValues *kv, bool fullUpdate)
{
    m_iSectionId = 0; // 0'th row is a header

    // add the player to the list
    KeyValues *playerData = new KeyValues("data");
    UpdatePlayerAvatar(engine->GetLocalPlayer(), playerData);

    player_info_t pi;
    engine->GetPlayerInfo(engine->GetLocalPlayer(), &pi);
    const char *oldName = playerData->GetString("name", pi.name);

    char newName[MAX_PLAYER_NAME_LENGTH];
    UTIL_MakeSafeName(oldName, newName, MAX_PLAYER_NAME_LENGTH);
    playerData->SetString("name", newName);
    // What this if is:
    // We want to do a full update if (we ask for it with fullUpdate boolean AND (the minimum time has passed OR it is the first update)) OR the maximum time has passed
    if ((fullUpdate && (gpGlobals->curtime - m_fLastHeaderUpdate >= MIN_ONLINE_UPDATE_INTERVAL || m_bFirstHeaderUpdate)) 
        || gpGlobals->curtime - m_fLastHeaderUpdate >= MAX_ONLINE_UPDATE_INTERVAL)
    {
        // MOM_TODO: Get real data from the API
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

        m_lPlayerMapRank->SetText(mrLocalized);
        m_lPlayerGlobalRank->SetText(grLocalized);
        m_lPlayerPersonalBest->SetText(pbLocalized);
        m_lPlayerExperience->SetText(xpLocalized);

        char requrl[MAX_PATH];
        // Mapname, tickrate, rank, radius
        Q_snprintf(requrl, MAX_PATH, "%s/getusermaprank/%s/%llu", MOM_APIDOMAIN, g_pGameRules->MapName(), GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64());
        CreateAndSendHTTPReq(requrl, &cbGetGetPlayerDataForMapCallback, &CClientTimesDisplay::GetGetPlayerDataForMapCallback);
    }

    kv->AddSubKey(playerData);
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientTimesDisplay::AddHeader()
{
    if (m_lMapSummary)
        m_lMapSummary->SetText(g_pGameRules->MapName());

    if (m_lMapDetails)
    {
        char mapDetails[BUFSIZ];
        Q_snprintf(mapDetails, BUFSIZ, "By %s\nTIER %i - %s - %i BONUS", "Author", 1, "Linear", 3);
        m_lMapDetails->SetText(mapDetails);
    }
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting local times
//-----------------------------------------------------------------------------
bool CClientTimesDisplay::StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
{
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);

    float t1 = it1->GetFloat("time_f");
    float t2 = it2->GetFloat("time_f");
    // Ascending order
    if (t1 < t2)
        return true; // this time is faster, place it up higher
    else if (t1 > t2)
        return false;

    // If the same, use IDs
    return itemID1 < itemID2;
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientTimesDisplay::StaticOnlineTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
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
    else if (t1 > t2)
        return false;
    else
    {

        // We will *almost* never need this, but just in case...

        float s1 = it1->GetFloat("time");
        float s2 = it2->GetFloat("time");
        // Ascending order
        if (s1 < s2)
            return true; // this time is faster, place it up higher
        else if (s1 > s2)
            return false;
    }
    return itemID1 < itemID2;
}

void CClientTimesDisplay::LoadLocalTimes(KeyValues *kv)
{
    /*steamapicontext->SteamFriends()->RequestUserInformation()*/
    if (!m_bLocalTimesLoaded || m_bLocalTimesNeedUpdate)
    {
        // Clear the local times for a refresh
        m_vLocalTimes.RemoveAll();

        // Load from .tim file
        KeyValues *pLoaded = new KeyValues("local");
        char fileName[MAX_PATH], filePath[MAX_PATH];
        const char *mapName = g_pGameRules->MapName();
        Q_snprintf(fileName, MAX_PATH, "%s%s", mapName ? mapName : "FIXME", EXT_TIME_FILE);
        V_ComposeFileName(MAP_FOLDER, fileName, filePath, MAX_PATH);

        DevLog("Loading from file %s...\n", filePath);
        if (pLoaded->LoadFromFile(filesystem, filePath, "MOD"))
        {
            for (KeyValues *kvLocalTime = pLoaded->GetFirstSubKey(); kvLocalTime;
                 kvLocalTime = kvLocalTime->GetNextKey())
            {
                Time t = Time(kvLocalTime);
                m_vLocalTimes.AddToTail(t);
            }
            m_bLocalTimesLoaded = true;
            m_bLocalTimesNeedUpdate = false;
        }
        pLoaded->deleteThis();
    }

    // Convert
    if (!m_vLocalTimes.IsEmpty())
        ConvertLocalTimes(kv);
}

void CClientTimesDisplay::ConvertLocalTimes(KeyValues *kvInto)
{
    FOR_EACH_VEC(m_vLocalTimes, i)
    {
        Time t = m_vLocalTimes[i];

        KeyValues *kvLocalTimeFormatted = new KeyValues("localtime");
        kvLocalTimeFormatted->SetFloat("time_f", t.time_sec); // Used for static compare
        kvLocalTimeFormatted->SetInt("date_t", t.date);       // Used for finding
        char timeString[BUFSIZETIME];

        mom_UTIL->FormatTime(t.time_sec, timeString);
        kvLocalTimeFormatted->SetString("time", timeString);

        char dateString[64];
        tm *local;
        local = localtime(&t.date);
        if (local)
        {
            strftime(dateString, sizeof(dateString), "%d/%m/%Y %H:%M:%S", local);
            kvLocalTimeFormatted->SetString("date", dateString);
        }
        else
            kvLocalTimeFormatted->SetInt("date", t.date);

        kvInto->AddSubKey(kvLocalTimeFormatted);
    }
}

void CClientTimesDisplay::ConvertOnlineTimes(KeyValues *kv, float seconds)
{
    char timeString[BUFSIZETIME];

    mom_UTIL->FormatTime(seconds, timeString);
    kv->SetString("time_f", timeString);
}

void CClientTimesDisplay::LoadOnlineTimes()
{
    if (!m_bOnlineTimesLoaded || m_bOnlineNeedUpdate)
    {
        char requrl[MAX_PATH];
        // Mapname, tickrate, rank, radius
        Q_snprintf(requrl, MAX_PATH, "%s/getscores/%s/10", MOM_APIDOMAIN, g_pGameRules->MapName());
        // This url is not real, just for testing pourposes. It returns a json list with the serialization of the scores
        CreateAndSendHTTPReq(requrl, &cbGetOnlineTimesCallback, &CClientTimesDisplay::GetOnlineTimesCallback);
        m_bOnlineNeedUpdate = false;
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
            Warning("Callback set.\n");
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

void CClientTimesDisplay::GetOnlineTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    Warning("Callback received.\n");
    if (bIOFailure)
        return;

    uint32 size = 0;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size < 1)
    {
        Warning("GetOnlineTimesCallback: size < 1 !!");
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
            CKeyValuesDumpContextAsDevMsg dev;
            pResponse->Dump(&dev, 1);
            KeyValues::AutoDelete ad(pResponse);
            KeyValues *pRuns = pResponse->FindKey("runs");

            if (pRuns && !pRuns->IsEmpty())
            {
                // By now we're pretty sure everything will be ok, so we can do this
                m_vOnlineTimes.PurgeAndDeleteElements();
                if (m_lLoadingOnlineTimes)
                {
                    m_lLoadingOnlineTimes->SetVisible(true);
                }

                // Iterate through each loaded run
                FOR_EACH_SUBKEY(pRuns, pRun)
                {
                    KeyValues *kvEntry = new KeyValues("Entry");
                    //Time is handled by the converter
                    kvEntry->SetFloat("time", pRun->GetFloat("time"));

                    //SteamID, Avatar, and Persona Name
                    uint64 steamID = Q_atoui64(pRun->GetString("steamid"));
                    kvEntry->SetUint64("steamid", steamID);
                    if (steamapicontext->SteamFriends())
                    {
                        //These handle setting "avatar" for kvEntry
                        if (GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64() == steamID)
                        {
                            UpdatePlayerAvatar(GetLocalPlayerIndex(), kvEntry);
                        }
                        else
                        {
                            UpdateLeaderboardPlayerAvatar(steamID, kvEntry);
                        }

                        //persona name
                        if (!steamapicontext->SteamFriends()->RequestUserInformation(CSteamID(steamID), true))
                        {
                            kvEntry->SetString(
                                "personaname",
                                steamapicontext->SteamFriends()->GetFriendPersonaName(CSteamID(steamID)));
                        }
                    }

                    //Persona name for the time they accomplished the run
                    kvEntry->SetString("personaname_onruntime", pRun->GetString("personaname"));

                    //Rank
                    kvEntry->SetInt("rank", static_cast<int>(pRun->GetFloat("rank")));
                    //MOM_TODO: Implement the other end of this (rank is not a number)

                    //Tickrate
                    kvEntry->SetInt("rate", static_cast<int>(pRun->GetFloat("rate")));

                    //Date
                    kvEntry->SetString("date", pRun->GetString("date"));

                    //ID
                    kvEntry->SetInt("id", static_cast<int>(pRun->GetFloat("id")));

                    //Add this baby to the online times vector
                    TimeOnline *ot = new TimeOnline(kvEntry);
                    //Convert the time
                    ConvertOnlineTimes(ot->m_kv, ot->time_sec);
                    m_vOnlineTimes.AddToTail(ot);
                }

                // If we're here and no errors happened, then we can assume times were loaded
                m_bOnlineTimesLoaded = true;
                m_bOnlineNeedUpdate = false;

                m_flLastOnlineTimeUpdate = gpGlobals->curtime;

                Update();
            }
            else
            {
                m_bOnlineTimesLoaded = false;
            }
        }
    }
    else
    {
        m_bOnlineTimesLoaded = false;
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    alloc.deallocate();
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
}

void CClientTimesDisplay::GetGetPlayerDataForMapCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    Warning("Callback received.\n");
    if (bIOFailure)
        return;

    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
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
                m_lPlayerMapRank->SetText(p_sLocalized);
            }
            if (seconds > 0.0f)
            {
                char p_sPersonalBestTime[BUFSIZETIME];
                char p_sPersonalBest[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                mom_UTIL->FormatTime(seconds, p_sPersonalBestTime);
                LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sPersonalBestTime);
                m_lPlayerPersonalBest->SetText(p_sLocalized);
            }

            if (grank > -1 && gtotal > -1)
            {
                char p_sGlobalRank[BUFSIZELOCL];
                char p_sLocalized[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
                Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sGlobalRank, grank, gtotal);
                m_lPlayerGlobalRank->SetText(p_sLocalized);

                char p_sExperience[BUFSIZELOCL];
                char p_sLocalized2[BUFSIZELOCL];
                LOCALIZE_TOKEN(p_wcExperience, "MOM_ExperiencePoints", p_sExperience);
                Q_snprintf(p_sLocalized2, BUFSIZELOCL, "%s: %i", p_sExperience, gexp);
                m_lPlayerExperience->SetText(p_sLocalized2);
            }
            m_fLastHeaderUpdate = gpGlobals->curtime;
        }
    }
    else
    {
            Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
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
    // MOM_TODO: QUERY THE API AND FILL THE LEADERBOARD LISTS
    KeyValues *pLeaderboards = new KeyValues("leaderboards");

    KeyValues *pLocal = new KeyValues("local");
    // Fill local times:
    LoadLocalTimes(pLocal);

    pLeaderboards->AddSubKey(pLocal);

    m_bOnlineNeedUpdate = (fullUpdate && (gpGlobals->curtime - m_flLastOnlineTimeUpdate >= MIN_ONLINE_UPDATE_INTERVAL || m_bFirstOnlineTimesUpdate)
        || gpGlobals->curtime - m_flLastOnlineTimeUpdate >= MAX_ONLINE_UPDATE_INTERVAL);
    // Fill online times only if needed
    LoadOnlineTimes();

    KeyValues *pFriends = new KeyValues("friends");
    // MOM_TODO: Fill online times (friends)

    pLeaderboards->AddSubKey(pFriends);

    kv->AddSubKey(pLeaderboards);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CClientTimesDisplay::UpdatePlayerAvatar(int playerIndex, KeyValues *kv)
{
    // Update their avatar
    if (kv && ShowAvatars() && steamapicontext->SteamFriends() && steamapicontext->SteamUtils())
    {
        player_info_t pi;
        if (engine->GetPlayerInfo(playerIndex, &pi))
        {
            if (pi.friendsID)
            {
                CSteamID steamIDForPlayer(pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(),
                                          k_EAccountTypeIndividual);

                // See if we already have that avatar in our list
                int iMapIndex = m_mapAvatarsToImageList.Find(steamIDForPlayer);
                int iImageIndex;
                if (iMapIndex == m_mapAvatarsToImageList.InvalidIndex())
                {
                    CAvatarImage *pImage = new CAvatarImage();
                    // 64 is enough up to full HD resolutions.
                    pImage->SetAvatarSteamID(steamIDForPlayer, k_EAvatarSize64x64);
                    pImage->SetAvatarSize(64, 64); // Deliberately non scaling
                    iImageIndex = m_pImageList->AddImage(pImage);

                    m_mapAvatarsToImageList.Insert(steamIDForPlayer, iImageIndex);
                }
                else
                {
                    iImageIndex = m_mapAvatarsToImageList[iMapIndex];
                }

                kv->SetInt("avatar", iImageIndex);

                CAvatarImage *pAvIm = (CAvatarImage *)m_pImageList->GetImage(iImageIndex);
                pAvIm->UpdateFriendStatus();
                // Set friend draw to false, so the offset is not set
                pAvIm->SetDrawFriend(false);
            }
        }
    }
}

void CClientTimesDisplay::UpdateLeaderboardPlayerAvatar(uint64 steamid, KeyValues *kv)
{
    // Update their avatar
    if (ShowAvatars() && steamapicontext->SteamFriends() && steamapicontext->SteamUtils())
    {

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

void CClientTimesDisplay::FillScoreBoard(bool pFullUpdate)
{

    KeyValues *m_kvPlayerData = new KeyValues("playdata");
    UpdatePlayerInfo(m_kvPlayerData, pFullUpdate);
    if (pFullUpdate)
        AddHeader();

    // Player Stats panel:
    if (m_pPlayerStats && m_pPlayerAvatar && m_lPlayerName && m_lPlayerGlobalRank && m_lPlayerMapRank &&
        m_kvPlayerData && !m_kvPlayerData->IsEmpty())
    {
        m_pPlayerStats->SetVisible(false); // Hidden so it is not seen being changed

        KeyValues *playdata = m_kvPlayerData->FindKey("data");
        if (playdata)
        {
            m_lPlayerName->SetText(playdata->GetString("name", "Unknown"));
            if (pFullUpdate)
            {
                int pAvatarIndex = playdata->GetInt("avatar", 0);
                if (pAvatarIndex == 0)
                    m_pPlayerAvatar->SetImage("default_steam");
                else
                    m_pPlayerAvatar->SetImage(m_pImageList->GetImage(pAvatarIndex));
                m_pPlayerAvatar->GetImage()->SetSize(scheme()->GetProportionalScaledValue(32),
                                                     scheme()->GetProportionalScaledValue(32));
            }
        }
        //m_pPlayerStats->SetVisible(true); // And seen again!
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

        // MOM_TODO: switch (currentFilter)
        // case (ONLINE):
        // etc

        // Just doing local times for reference now
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
        }

        // Online works slightly different, we use the vector content, not the ones from m_kvPlayerData (because online
        // times are not stored there)

        // Place m_vOnlineTimes into m_pOnlineLeaderboards
        OnlineTimesVectorToLeaderboards();

        m_pLeaderboards->SetVisible(true);
    }
    m_kvPlayerData->deleteThis();
}

void CClientTimesDisplay::OnlineTimesVectorToLeaderboards()
{
    if (m_vOnlineTimes.Count() > 0 && m_pOnlineLeaderboards)
    {
        FOR_EACH_VEC(m_vOnlineTimes, entry)
        {
            // We set the current personaname before anything...
            // Find method is not being nice, so we craft our own
            // int mId = m_mSIdNames.Find(m_vOnlineTimes.Element(entry).steamid);
            FOR_EACH_MAP_FAST(m_umMapNames, mIter)
            {
                if (m_umMapNames.Key(mIter) == m_vOnlineTimes.Element(entry)->steamid)
                {
                    const char *personaname = m_umMapNames.Element(mIter);
                    if (Q_strcmp(personaname, "") != 0)
                    {
                        m_vOnlineTimes.Element(entry)->m_kv->SetString("personaname", personaname);
                    }
                    break;
                }
            }
            TimeOnline *runEntry = m_vOnlineTimes.Element(entry);
            int itemID = FindItemIDForOnlineTime(runEntry->id);
            if (itemID == -1)
            {
                itemID = m_pOnlineLeaderboards->AddItem(m_iSectionId, runEntry->m_kv);
            }
            else
            {
                m_pOnlineLeaderboards->ModifyItem(itemID, m_iSectionId, runEntry->m_kv);
            }
            switch (runEntry->rank)
            {
            case 1:
                m_pOnlineLeaderboards->SetItemFgColor(itemID, Color(240, 210, 147, 225));
                break;
            case 2:
                m_pOnlineLeaderboards->SetItemFgColor(itemID, Color(175, 175, 175, 225));
                break;
            case 3:
                m_pOnlineLeaderboards->SetItemFgColor(itemID, Color(205, 127, 50, 225));
                break;
            default:
                break;
            }
        }
        if (m_lLoadingOnlineTimes)
        {
            m_lLoadingOnlineTimes->SetVisible(false);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: searches for the player in the scoreboard
//-----------------------------------------------------------------------------
int CClientTimesDisplay::FindItemIDForPlayerIndex(int playerIndex)
{
    // MOM_TODO: make this return an ItemID for another person's time (friend/global)
    for (int i = 0; i <= m_pPlayerList->GetHighestItemID(); i++)
    {
        if (m_pPlayerList->IsItemIDValid(i))
        {
            KeyValues *kv = m_pPlayerList->GetItemData(i);
            kv = kv->FindKey(m_iPlayerIndexSymbol);
            if (kv && kv->GetInt() == playerIndex)
                return i;
        }
    }
    return -1;
}

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

int CClientTimesDisplay::FindItemIDForOnlineTime(int runID)
{
    for (int i = 0; i <= m_pOnlineLeaderboards->GetHighestItemID(); i++)
    {
        if (m_pOnlineLeaderboards->IsItemIDValid(i))
        {
            KeyValues *kv = m_pOnlineLeaderboards->GetItemData(i);
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

    if (!pParent)
    {
        m_pLeaderboardReplayCMenu->SetParent(this);
    }
    else
    {
        m_pLeaderboardReplayCMenu->AddActionSignalTarget(pParent);
        m_pLeaderboardReplayCMenu->SetParent(pParent);
    }

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

void CClientTimesDisplay::OnItemContextMenu(KeyValues *pData)
{
    int itemID = pData->GetInt("itemID", -1);
    Panel *pPanel = static_cast<Panel *>(pData->GetPtr("SubPanel", nullptr));
    if (pPanel && pPanel->GetParent())
    {
        if (pPanel->GetParent() == m_pLocalLeaderboards && m_pLocalLeaderboards->IsItemIDValid(itemID))
        {
            KeyValues *selectedRun = m_pLocalLeaderboards->GetItemData(itemID);
            char recordingName[MAX_PATH];
            Q_snprintf(recordingName, MAX_PATH, "%i-%.3f", selectedRun->GetInt("date_t"),
                       selectedRun->GetFloat("time_f"));

            CReplayContextMenu *pContextMenu = GetLeaderboardReplayContextMenu(pPanel->GetParent());
            pContextMenu->AddMenuItem("StartMap", "#MOM_Leaderboards_WatchReplay",
                                      new KeyValues("ContextWatchReplay", "runName", recordingName), this);
            pContextMenu->ShowMenu();
        }
        else if ((pPanel->GetParent() == m_pOnlineLeaderboards && m_pOnlineLeaderboards->IsItemIDValid(itemID)) || (pPanel->GetParent() == m_pFriendsLeaderboards && m_pFriendsLeaderboards->IsItemIDValid(itemID)))
        {
            SectionedListPanel *pLeaderboard = static_cast<SectionedListPanel*>(pPanel->GetParent());
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
    if (pParam->m_nChangeFlags & k_EPersonaChangeName)
    {
        m_umMapNames.InsertOrReplace(
            pParam->m_ulSteamID, steamapicontext->SteamFriends()->GetFriendPersonaName(CSteamID(pParam->m_ulSteamID)));
    }
}

int CClientTimesDisplay::TryAddAvatar(CSteamID steamid)
{
    // Update their avatar
    if (ShowAvatars() && steamapicontext->SteamFriends() && steamapicontext->SteamUtils())
    {
        CSteamID sID = CSteamID(steamid);
        // See if we already have that avatar in our list

        int iMapIndex = m_mapAvatarsToImageList.Find(sID);
        int iImageIndex;
        if (iMapIndex == m_mapAvatarsToImageList.InvalidIndex())
        {
            CAvatarImage *pImage = new CAvatarImage();
            // 64 is enough up to full HD resolutions.
            pImage->SetAvatarSteamID(sID, k_EAvatarSize64x64);
            pImage->SetAvatarSize(64, 64); // Deliberately non scaling
            pImage->UpdateFriendStatus();
            pImage->SetDrawFriend(false);
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

void CClientTimesDisplay::OnToggleLeaderboard(KeyValues *pData)
{
    if (m_pOnlineLeaderboards && m_pFriendsLeaderboards)
    {
        const char *leaderboard = pData->GetString("leaderboard");
        m_bGlobalsShown = !Q_strcmp(leaderboard, "global");
        m_pOnlineLeaderboards->SetVisible(m_bGlobalsShown);
        m_pGlobalLeaderboardsButton->SetEnabled(!m_bGlobalsShown);
        m_pFriendsLeaderboards->SetVisible(!m_bGlobalsShown);
        m_pFriendsLeaderboardsButton->SetEnabled(m_bGlobalsShown);
    }
}