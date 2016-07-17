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
    m_pMomentumLogo = FindControl<ImagePanel>("MomentumLogo", true);
    m_pPlayerStats = FindControl<Panel>("PlayerStats", true);
    m_pPlayerAvatar = FindControl<ImagePanel>("PlayerAvatar", true);
    m_lPlayerName = FindControl<Label>("PlayerName", true);
    m_lPlayerMapRank = FindControl<Label>("PlayerMapRank", true);
    m_lPlayerGlobalRank = FindControl<Label>("PlayerGlobalRank", true);
    m_lPlayerPersonaBest = FindControl<Label>("PlayerPersonalBest", true);
    m_lLoadingOnlineTimes = FindControl<Label>("LoadingOnlineTimes", true);
    m_pLeaderboards = FindControl<Panel>("Leaderboards", true);
    m_pOnlineLeaderboards = FindControl<SectionedListPanel>("OnlineLeaderboards", true);
    m_pLocalLeaderboards = FindControl<SectionedListPanel>("LocalLeaderboards", true);
    m_pFriendsLeaderboards = FindControl<SectionedListPanel>("FriendsLeaderboards", true);

    if (!m_pHeader || !m_lMapSummary || !m_pPlayerStats || !m_pPlayerAvatar || !m_lPlayerName || !m_pMomentumLogo ||
        !m_lPlayerMapRank || !m_lPlayerGlobalRank || !m_pLeaderboards || !m_pOnlineLeaderboards ||
        !m_pLocalLeaderboards || !m_pFriendsLeaderboards || !m_lPlayerPersonaBest || !m_lLoadingOnlineTimes)
    {
        Assert("Null pointer(s) on scoreboards");
    }
    SetSize(scheme()->GetProportionalScaledValue(640), scheme()->GetProportionalScaledValue(480));

    m_pHeader->SetParent(this);
    m_pPlayerStats->SetParent(this);
    m_pLeaderboards->SetParent(this);
    m_lMapSummary->SetParent(m_pHeader);
    m_pMomentumLogo->SetParent(m_pPlayerStats);
    m_pPlayerAvatar->SetParent(m_pPlayerStats);
    m_lPlayerName->SetParent(m_pPlayerStats);
    m_lPlayerMapRank->SetParent(m_pPlayerStats);
    m_lPlayerGlobalRank->SetParent(m_pPlayerStats);
    m_lPlayerPersonaBest->SetParent(m_pPlayerStats);
    m_pOnlineLeaderboards->SetParent(m_pLeaderboards);
    m_lLoadingOnlineTimes->SetParent(m_pLeaderboards);
    m_pLocalLeaderboards->SetParent(m_pLeaderboards);
    m_pFriendsLeaderboards->SetParent(m_pLeaderboards);

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

    m_fLastHeaderUpdate = 0;
    m_fMaxHeaderUpdateInterval = 45;
    m_fMinHeaderUpdateInterval = 15;

    m_bFirstUpdate = true;

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
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "rank", "#MOM_Rank", 0,
                                                   SCALE(m_aiColumnWidths[1] * 1.8));
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "name", "#MOM_Name", 0, NAME_WIDTH * 1.8);
        m_pFriendsLeaderboards->AddColumnToSection(m_iSectionId, "time", "#MOM_Time", 0,
                                                   SCALE(m_aiColumnWidths[2] * 1.8));
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

    // update every second
    // we don't need to update this too often. (Player is not finishing a run every second, so...)

    m_fNextUpdateTime = gpGlobals->curtime + 3.0f;

    // This starts as true on the constructor.
    m_bFirstUpdate = false;
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
    if ((fullUpdate && (gpGlobals->curtime - m_fLastHeaderUpdate >= m_fMinHeaderUpdateInterval || m_bFirstUpdate)) || gpGlobals->curtime - m_fLastHeaderUpdate >= m_fMaxHeaderUpdateInterval)
    {
        // MOM_TODO: Get real data from the API
        char p_sCalculating[BUFSIZELOCL];
        char p_sWaitingResponse[BUFSIZELOCL];
        LOCALIZE_TOKEN(p_wcCalculating, "MOM_Calculating", p_sCalculating);     
        LOCALIZE_TOKEN(p_wcWaitingResponse, "MOM_API_WaitingForResponse", p_sWaitingResponse);

        char p_sMapRank[BUFSIZELOCL];
        char p_sGlobalRank[BUFSIZELOCL];
        char p_sPersonalBest[BUFSIZELOCL];
        
        char mrLocalized[BUFSIZELOCL];
        char grLocalized[BUFSIZELOCL];
        char pbLocalized[BUFSIZELOCL];

        LOCALIZE_TOKEN(p_wcMapRank, "MOM_MapRank", p_sMapRank);
        LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", p_sGlobalRank);
        LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);

        Q_snprintf(mrLocalized, BUFSIZELOCL, "%s: %s", p_sMapRank, p_sCalculating);
        Q_snprintf(grLocalized, BUFSIZELOCL, "%s: %s", p_sGlobalRank, p_sCalculating);
        Q_snprintf(pbLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sWaitingResponse);
        m_lPlayerMapRank->SetText(mrLocalized);
        m_lPlayerGlobalRank->SetText(grLocalized);
        m_lPlayerPersonaBest->SetText(pbLocalized);

        char requrl[MAX_PATH];
        // Mapname, tickrate, rank, radius
        Q_snprintf(requrl, MAX_PATH, "http://127.0.0.1:5000/getusermaprank/%s/%llu", g_pGameRules->MapName(), GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64());
        CreateAndSendHTTPReq(requrl, &cbGetGetPlayerDataForMapCallback, &CClientTimesDisplay::GetGetPlayerDataForMapCallback);
    }

    kv->AddSubKey(playerData);
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientTimesDisplay::AddHeader(Label *pMapSummary)
{
    // @Rabs:
    // MOM_TODO: This is a very very very ugly hack I had to do because
    // the pointer to m_lMapSummary was not valid inside the function, but it is outside it.
    // We have to get rid of it, or at least discover why it was working for some (Goc) and not to me
    if (pMapSummary)
    {
        char gameMode[5];
        switch (ConVarRef("mom_gamemode").GetInt())
        {
        case MOMGM_SURF:
                Q_strcpy(gameMode, "SURF");
                break;
        case MOMGM_BHOP:
                Q_strcpy(gameMode, "BHOP");
                break;
        case MOMGM_SCROLL:
                Q_strcpy(gameMode, "SCROLL");
                break;
        default:
                Q_strcpy(gameMode, "????");
                break;
        }

        char mapSummary[BUFSIZELOCL];
        Q_snprintf(mapSummary, 64, "%s | %s", gameMode, g_pGameRules->MapName());
        pMapSummary->SetText(mapSummary);
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
        Q_snprintf(requrl, MAX_PATH, "http://127.0.0.1:5000/getscores/%s", g_pGameRules->MapName());
        // This url is not real, just for testing pourposes. It returns a json list with the serialization of the scores
        CreateAndSendHTTPReq(requrl, &cbGetOnlineTimesCallback, &CClientTimesDisplay::GetOnlineTimesCallback);
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
            JsonValue oval = val.toNode()->value;
            if (oval.getTag() == JSON_ARRAY)
            {
                // By now we're pretty sure everything will be ok, so we can do this
                m_vOnlineTimes.RemoveAll();
                if (m_pOnlineLeaderboards && m_lLoadingOnlineTimes)
                {
                    m_pOnlineLeaderboards->SetVisible(false);
                    m_lLoadingOnlineTimes->SetVisible(true);
                }
                for (auto i : oval)
                {
                    if (i->value.getTag() == JSON_OBJECT)
                    {
                        KeyValues *kv = new KeyValues("entry");
                        for (auto j : i->value)
                        {
                            if (!Q_strcmp(j->key, "time") && j->value.getTag() == JSON_NUMBER)
                            {
                                kv->SetFloat("time", j->value.toNumber());
                            }
                            else if (!Q_strcmp(j->key, "steamid") && j->value.getTag() == JSON_STRING)
                            {
                                uint64 steamid = Q_atoui64(j->value.toString());
                                kv->SetUint64("steamid", steamid);
                                if (steamapicontext && steamapicontext->SteamFriends())
                                {
                                    if (GetSteamIDForPlayerIndex(GetLocalPlayerIndex()).ConvertToUint64() == steamid)
                                    {
                                        UpdatePlayerAvatar(GetLocalPlayerIndex(), kv);
                                    }
                                    else
                                    {
                                        UpdateLeaderboardPlayerAvatar(steamid, kv);
                                    }
                                    if (!steamapicontext->SteamFriends()->RequestUserInformation(CSteamID(steamid),
                                                                                                 true))
                                    {
                                        kv->SetString(
                                            "personaname",
                                            steamapicontext->SteamFriends()->GetFriendPersonaName(CSteamID(steamid)));
                                    }
                                }
                            }
                            else if (!Q_strcmp(j->key, "personaname") && j->value.getTag() == JSON_STRING)
                            {
                                // This is the name the player had when the run was completed, not the one currently
                                kv->SetString("personaname_onruntime", j->value.toString());
                            }
                            else if (!Q_strcmp(j->key, "rank"))
                            {
                                if (j->value.getTag() == JSON_NUMBER)
                                {
                                    kv->SetInt("rank", j->value.toNumber());
                                }
                                else
                                {
                                    char p_sCalculating[BUFSIZELOCL];
                                    LOCALIZE_TOKEN(p_wcCalculating, "MOM_Calculating", p_sCalculating);
                                    kv->SetString("rank", p_sCalculating);
                                }
                            }
                            else if (!Q_strcmp(j->key, "rate") && j->value.getTag() == JSON_NUMBER)
                            {
                                kv->SetInt("rate", j->value.toNumber());
                            }
                            else if (!Q_strcmp(j->key, "date"))
                            {
                                // MOM_TODO: Implement.
                            }
                            else if (!Q_strcmp(j->key, "id") && j->value.getTag() == JSON_NUMBER)
                            {
                                kv->SetInt("id", j->value.toNumber());
                            }
                        }
                        TimeOnline ot = TimeOnline(kv);
                        ConvertOnlineTimes(ot.m_kv, ot.time_sec);
                        kv->deleteThis();
                        m_vOnlineTimes.AddToTail(ot);
                    }
                }
                // If we're here and no errors happened, then we can assume times were loaded
                m_bOnlineTimesLoaded = true;
                m_bOnlineNeedUpdate = false;
            }
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
            JsonValue oval = val.toNode()->value;
            if (oval.getTag() == JSON_ARRAY)
            {
                if (JsonNode *i = oval.toNode())
                {
                    int rank = -1;
                    int total = -1;
                    float seconds = 0;
                    if (i->value.getTag() == JSON_OBJECT)
                    {
                        for (auto j : i->value)
                        {
                            if (!Q_strcmp(j->key, "run") && j->value.getTag() == JSON_OBJECT)
                            {
                                for (auto k : j->value)
                                {
                                    if (!Q_strcmp(k->key, "rank") && k->value.getTag() == JSON_NUMBER)
                                    {
                                        rank = k->value.toNumber();
                                    }
                                    else if (!Q_strcmp(k->key, "time") && k->value.getTag() == JSON_NUMBER)
                                    {
                                        seconds = k->value.toNumber();
                                    }
                                }
                            }
                            else if (!Q_strcmp(j->key, "total") && j->value.getTag() == JSON_NUMBER)
                            {
                                total = j->value.toNumber();
                            }
                        } 
                    }
                    if (rank > -1)
                    {
                        char p_sMapRank[BUFSIZELOCL];
                        char p_sLocalized[BUFSIZELOCL];
                        LOCALIZE_TOKEN(p_wcMapRank, "MOM_MapRank", p_sMapRank);
                        Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %i/%i", p_sMapRank, rank, total);
                        m_lPlayerMapRank->SetText(p_sLocalized);
                    }
                    if (seconds > 0)
                    {
                        char p_sPersonalBestTime[BUFSIZETIME];
                        char p_sPersonalBest[BUFSIZELOCL];
                        char p_sLocalized[BUFSIZELOCL];
                        mom_UTIL->FormatTime(seconds, p_sPersonalBestTime);
                        LOCALIZE_TOKEN(p_wcPersonalBest, "MOM_PersonalBestTime", p_sPersonalBest);
                        Q_snprintf(p_sLocalized, BUFSIZELOCL, "%s: %s", p_sPersonalBest, p_sPersonalBestTime);
                        m_lPlayerPersonaBest->SetText(p_sLocalized);
                    }
                    /*char globalRank[BUFSIZELOCL];
                    char grLocalized[BUFSIZELOCL];
                    LOCALIZE_TOKEN(p_wcGlobalRank, "MOM_GlobalRank", globalRank);

                    Q_snprintf(globalRank, 50, "%s: %i/%i", grLocalized, playdata->GetInt("globalRank", -1),
                    playdata->GetInt("globalCount", -1));
                    m_lPlayerGlobalRank->SetText(globalRank);*/
                    m_fLastHeaderUpdate = gpGlobals->curtime;
                }
            }
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
bool CClientTimesDisplay::GetPlayerTimes(KeyValues *kv)
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

    // Fill online times (global)
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
        AddHeader(m_lMapSummary);

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
        m_pPlayerStats->SetVisible(true); // And seen again!
    }

    // Leaderboards
    // @Gocnak: If we use event-driven updates and caching, it shouldn't overload the API.
    // The idea is to cache things to show, which will be called every FillScoreBoard call,
    // but it will only call the methods to update the data if booleans are set to.
    // For example, if a local time was saved, the event "timer_timesaved" or something is passed here,
    // and on the GetPlayerTimes passthrough, we'd update the local times, which then gets stored in
    // the Panel object until next update

    GetPlayerTimes(m_kvPlayerData);

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
                if (m_umMapNames.Key(mIter) == m_vOnlineTimes.Element(entry).steamid)
                {
                    const char *personaname = m_umMapNames.Element(mIter);
                    if (Q_strcmp(personaname, "") != 0)
                    {
                        m_vOnlineTimes.Element(entry).m_kv->SetString("personaname", personaname);
                    }
                    break;
                }
            }

            int itemID = FindItemIDForOnlineTime(m_vOnlineTimes.Element(entry).id);
            if (itemID == -1)
            {
                m_pOnlineLeaderboards->AddItem(m_iSectionId, m_vOnlineTimes.Element(entry).m_kv);
            }
            else
            {
                m_pOnlineLeaderboards->ModifyItem(itemID, m_iSectionId, m_vOnlineTimes.Element(entry).m_kv);
            }
        }
        if (m_pOnlineLeaderboards && m_lLoadingOnlineTimes)
        {
            m_pOnlineLeaderboards->SetVisible(true);
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
            Q_snprintf(recordingName, MAX_PATH, "%i-%f", selectedRun->GetInt("date_t"),
                       selectedRun->GetFloat("time_f"));

            CReplayContextMenu *pContextMenu = GetLeaderboardReplayContextMenu(pPanel->GetParent());
            pContextMenu->AddMenuItem("StartMap", "#MOM_Leaderboards_WatchReplay",
                                      new KeyValues("ContextWatchReplay", "runName", recordingName), this);
            pContextMenu->ShowMenu();
        }
        else if (pPanel->GetParent() == m_pOnlineLeaderboards && m_pOnlineLeaderboards->IsItemIDValid(itemID))
        {
            CReplayContextMenu *pContextMenu = GetLeaderboardReplayContextMenu(pPanel->GetParent());
            KeyValues *pKv = new KeyValues("ContextVisitProfile");
            pKv->SetUint64("profile", m_pOnlineLeaderboards->GetItemData(itemID)->GetUint64("steamid"));
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