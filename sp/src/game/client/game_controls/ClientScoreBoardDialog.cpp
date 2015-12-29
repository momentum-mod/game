//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>
#include "IGameUIFuncs.h" // for key bindings
#include "inputsystem/iinputsystem.h"
#include "clientscoreboarddialog.h"
#include <voice_status.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include <game/client/iviewport.h>
#include <igameresources.h>

#include "vgui_avatarimage.h"
#include "filesystem.h"
#include <time.h>

extern IFileSystem *filesystem;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

bool AvatarIndexLessFunc(const int &lhs, const int &rhs)
{
    return lhs < rhs;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientScoreBoardDialog::CClientScoreBoardDialog(IViewPort *pViewPort) : EditablePanel(NULL, PANEL_SCOREBOARD)
{
    m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");
    m_nCloseKey = BUTTON_CODE_INVALID;

    m_pViewPort = pViewPort;

    // initialize dialog
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);

    // set the scheme before any child control is created
    SetScheme("ClientScheme");

    m_pPlayerList = new SectionedListPanel(this, "PlayerList");
    m_pPlayerList->SetVerticalScrollbar(false);

    LoadControlSettings("Resource/UI/timesdisplay.res");

    m_pHeader = FindControl<Panel>("Header", true);
    m_lMapSummary = FindControl<Label>("MapSummary", true);
    m_pPlayerStats = FindControl<Panel>("PlayerStats", true);
    m_pPlayerAvatar = FindControl<ImagePanel>("PlayerAvatar", true);
    m_lPlayerName = FindControl<Label>("PlayerName", true);
    m_lPlayerMapRank = FindControl<Label>("PlayerMapRank", true);
    m_lPlayerGlobalRank = FindControl<Label>("PlayerGlobalRank", true);
    m_pLeaderboards = FindControl<Panel>("Leaderboards", true);
    m_pOnlineLeaderboards = FindControl<SectionedListPanel>("OnlineNearbyLeaderboard", true);
    m_pLocalBests = FindControl<SectionedListPanel>("LocalPersonalBest", true);

    if (!m_pHeader || !m_lMapSummary || !m_pPlayerStats || !m_pPlayerAvatar || !m_lPlayerName ||
        !m_lPlayerMapRank || !m_lPlayerGlobalRank || !m_pLeaderboards || !m_pOnlineLeaderboards || !m_pLocalBests)
    {
        Assert("Null pointer(s) on scoreboards");
    }
    SetSize(scheme()->GetProportionalScaledValue(640), scheme()->GetProportionalScaledValue(480));
    m_pHeader->SetParent(this);
    m_pPlayerStats->SetParent(this);
    m_pLeaderboards->SetParent(this);
    m_lMapSummary->SetParent(m_pHeader);
    m_pPlayerAvatar->SetParent(m_pPlayerStats);
    m_lPlayerName->SetParent(m_pPlayerStats);
    m_lPlayerMapRank->SetParent(m_pPlayerStats);
    m_lPlayerGlobalRank->SetParent(m_pPlayerStats);
    m_pOnlineLeaderboards->SetParent(m_pLeaderboards);
    m_pLocalBests->SetParent(m_pLeaderboards);

    m_pOnlineLeaderboards->SetVerticalScrollbar(false);
    m_pLocalBests->SetVerticalScrollbar(false);

    m_iDesiredHeight = GetTall();
    m_pPlayerList->SetVisible(false); // hide this until we load the images in applyschemesettings

    // update scoreboard instantly if on of these events occure
    ListenForGameEvent("runtime_saved");
    ListenForGameEvent("runtime_posted");

    m_pImageList = NULL;
    m_mapAvatarsToImageList.SetLessFunc(DefLessFunc(CSteamID));
    m_mapAvatarsToImageList.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientScoreBoardDialog::~CClientScoreBoardDialog()
{
    if (NULL != m_pImageList)
    {
        delete m_pImageList;
        m_pImageList = NULL;
    }
    // MOM_TODO: Ensure a good destructor
}

//-----------------------------------------------------------------------------
// Call every frame
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::OnThink()
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
            gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, false);
            GetClientVoiceMgr()->StopSquelchMode();
        }
    }
}

//-----------------------------------------------------------------------------
// Called by vgui panels that activate the client scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::OnPollHideCode(int code)
{
    m_nCloseKey = (ButtonCode_t) code;
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::Reset()
{
    Reset(false);
}

void CClientScoreBoardDialog::Reset(bool pFullReset)
{
    if (pFullReset)
    {
        FillScoreBoard(true);
        Update(true);
    }
    // clear
    m_pPlayerList->DeleteAllItems();
    m_pPlayerList->RemoveAllSections();

    m_iSectionId = 0;
    m_fNextUpdateTime = 0;
    // add all the sections
    InitScoreboardSections();
}

//-----------------------------------------------------------------------------
// Purpose: adds all the team sections to the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::InitScoreboardSections()
{
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::ApplySchemeSettings(IScheme *pScheme)
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
void CClientScoreBoardDialog::PostApplySchemeSettings(vgui::IScheme *pScheme)
{
    // resize the images to our resolution
    for (int i = 0; i < m_pImageList->GetImageCount(); i++)
    {
        int wide, tall;
        m_pImageList->GetImage(i)->GetSize(wide, tall);
        m_pImageList->GetImage(i)->SetSize(scheme()->GetProportionalScaledValueEx(GetScheme(), wide),
            scheme()->GetProportionalScaledValueEx(GetScheme(), tall));
    }

    m_pPlayerList->SetImageList(m_pImageList, false);
    m_pPlayerList->SetVisible(true);

    m_lMapSummary->SetVisible(true);


    m_pLocalBests->AddSection(m_iSectionId, "", StaticLocalTimeSortFunc);
    m_pLocalBests->SetSectionAlwaysVisible(m_iSectionId);
    m_pLocalBests->AddColumnToSection(m_iSectionId, "time", "#MOM_Time", 0, NAME_WIDTH);
    m_pLocalBests->AddColumnToSection(m_iSectionId, "date", "#MOM_Date", 0, NAME_WIDTH);

    //MOM_TODO: online needs rank, name, time, date achieved?

    //MOM_TODO: friends follows online format

    // light up scoreboard a bit
    SetBgColor(Color(0, 0, 0, 0));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::ShowPanel(bool bShow)
{
    // Catch the case where we call ShowPanel before ApplySchemeSettings, eg when
    // going from windowed <-> fullscreen
    if (m_pImageList == NULL)
    {
        InvalidateLayout(true, true);
    }

    if (!bShow)
    {
        m_nCloseKey = BUTTON_CODE_INVALID;
    }

    if (BaseClass::IsVisible() == bShow)
        return;

    if (bShow)
    {
        Reset(true);
        // Updating twice...
        Update(false);
        SetVisible(true);
        MoveToFront();
    }
    else
    {
        BaseClass::SetVisible(false);
        SetMouseInputEnabled(false);
        SetKeyBoardInputEnabled(false);
    }
}

void CClientScoreBoardDialog::FireGameEvent(IGameEvent *event)
{
    const char * type = event->GetName();

    if (Q_strcmp(type, "runtime_saved") == 0)
    {
        //this updates the local times file, needing a reload of it
        bLocalTimesNeedUpdate = true;
    }
    else if (Q_strcmp(type, "runtime_posted") == 0)
    {
        //MOM_TODO: this updates your rank (friends/online panel)
    }

    if (IsVisible())
        Update(true);

}

bool CClientScoreBoardDialog::NeedsUpdate(void)
{
    return (m_fNextUpdateTime < gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::Update(void)
{
    Update(false);
}

void CClientScoreBoardDialog::Update(bool pFullUpdate)
{
    m_pPlayerList->DeleteAllItems();
    if (pFullUpdate)
        FillScoreBoard();

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
    // we don't need to update this too often. (Player is not finish a run every second, so...)
    // MOM_TODO: Discuss update interval
    m_fNextUpdateTime = gpGlobals->curtime + 3.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Sort all the teams
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdateTeamInfo()
{
    // TODO: work out a sorting algorithm for team display for TF2
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdatePlayerInfo(KeyValues *kv)
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

    // MOM_TODO: Get real data from the API
    playerData->SetInt("globalRank", -1);
    playerData->SetInt("globalCount", -1);
    playerData->SetInt("mapRank", -1);
    playerData->SetInt("mapCount", -1);

    kv->AddSubKey(playerData);
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::AddHeader()
{
    char gameMode[4];

    if (!Q_strnicmp(g_pGameRules->MapName(), "surf_", Q_strlen("surf_")))
        Q_strcpy(gameMode, "SURF");

    else if (!Q_strnicmp(g_pGameRules->MapName(), "bhop_", Q_strlen("bhop_")))
        Q_strcpy(gameMode, "BHOP");
    else
        Q_strcpy(gameMode, "????");
    //MOM_TODO: There'll probably be official gametypes later on in mod development
    //We'll have some sort of shared class or global setting that we will use to determine
    //what game mode we're on (mom_UTIL.GameMode() or something)

    char mapSummary[64];
    Q_snprintf(mapSummary, 64, "%s || %s", gameMode, g_pGameRules->MapName());
    // add the top header
    if (m_lMapSummary)
        m_lMapSummary->SetText(mapSummary);

}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::AddSection(int teamType, int teamNumber)
{
}


bool CClientScoreBoardDialog::StaticLocalTimeSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
{
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);

    float t1 = it1->GetFloat("time_f");
    float t2 = it2->GetFloat("time_f");
    //Ascending order
    if (t1 < t2)
        return true;//this time is faster, place it up higher
    else if (t1 > t2)
        return false;

    return itemID1 < itemID2;
}


//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
{
    //MOM_TODO: change this to sort by times?
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    Assert(it1 && it2);

    // first compare frags
    int v1 = it1->GetInt("frags");
    int v2 = it2->GetInt("frags");
    if (v1 > v2)
        return true;
    else if (v1 < v2)
        return false;

    // next compare deaths
    v1 = it1->GetInt("deaths");
    v2 = it2->GetInt("deaths");
    if (v1 > v2)
        return false;
    else if (v1 < v2)
        return true;

    // the same, so compare itemID's (as a sentinel value to get deterministic sorts)
    return itemID1 < itemID2;
}

void CClientScoreBoardDialog::LoadLocalTimes(KeyValues *kv)
{
    KeyValues *pLoaded = new KeyValues("local");
    char fileName[MAX_PATH];
    Q_strcpy(fileName, "maps/");
    Q_strcat(fileName, g_pGameRules->MapName(), MAX_PATH);
    Q_strncat(fileName, ".tim", MAX_PATH);
    DevLog("Loading from file %s...\n", fileName);
    if (pLoaded->LoadFromFile(filesystem, fileName, "MOD"))
    {
        for (KeyValues* kvLocalTime = pLoaded->GetFirstSubKey(); kvLocalTime; kvLocalTime = kvLocalTime->GetNextKey())
        {
            int ticks = Q_atoi(kvLocalTime->GetName());
            float rate = kvLocalTime->GetFloat("rate");
            time_t date = (time_t) kvLocalTime->GetInt("date");
            float secondsF = ((float) ticks) * rate;
            //MOM_TODO: consider adding a "100 tick" column?

            KeyValues *kvLocalTimeFormatted = new KeyValues("localtime");
            kvLocalTimeFormatted->SetFloat("time_f", secondsF);//Used for static compare
            kvLocalTimeFormatted->SetInt("date_t", date);//Used for finding
            char timeString[15];
            
            int hours = secondsF / (60.0f * 60.0f);
            int minutes = fmod(secondsF / 60.0f, 60.0f);
            int seconds = fmod(secondsF, 60.0f);
            int millis = fmod(secondsF, 1.0f) * 1000.0f;
            
            if (hours > 0)
                Q_snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);
            else if (minutes > 0)
                Q_snprintf(timeString, sizeof(timeString), "%02d:%02d.%03d", minutes, seconds, millis);
            else
            {
                // This localized token is "s.", but only because if the string is too big, the text will be chopped
                // MOM_TODO: Change the seconds localization when the bug with the scoreboard is sorted out
                char mrLocalized[50];
                wchar_t *uSecondsUnicode = g_pVGuiLocalize->Find("#MOM_seconds");
                g_pVGuiLocalize->ConvertUnicodeToANSI(uSecondsUnicode ? uSecondsUnicode : L"#MOM_seconds", mrLocalized, 50);
                Q_snprintf(timeString, sizeof(timeString), "%02d.%03d %s", seconds, millis, mrLocalized);
            }

            kvLocalTimeFormatted->SetString("time", timeString);

            char dateString[64];
            tm * local;
            local = localtime(&date);
            if (local)
            {
                // Why Y/m/d and not d/m/Y?
                strftime(dateString, sizeof(dateString), "%Y/%m/%d %H:%M:%S", local);
                kvLocalTimeFormatted->SetString("date", dateString);
            }
            else
            {
                kvLocalTimeFormatted->SetInt("date", date);
            }

            kv->AddSubKey(kvLocalTimeFormatted);
        }
        bLocalTimesLoaded = true;
        bLocalTimesNeedUpdate = false;
    }
    pLoaded->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the leaderboard lists
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::GetPlayerTimes(KeyValues *kv)
{
    //We're going to need to update all three of these, even if only two are shown,
    //because the user can change the panels at any time

    //We could also make it update based on when players change their leaderboard
    //to show something else, to reduce 
    if (!kv)
        return false;
    // MOM_TODO: QUERY THE API AND FILL THE LEADERBOARD LISTS
    KeyValues *pLeaderboards = new KeyValues("leaderboards");

    KeyValues *pLocal = new KeyValues("local");
    // Fill local times:
    if (!bLocalTimesLoaded || bLocalTimesNeedUpdate)
        LoadLocalTimes(pLocal);

    pLeaderboards->AddSubKey(pLocal);

    KeyValues *pOnline = new KeyValues("online");
    // Fill online times (global)

    pLeaderboards->AddSubKey(pOnline);

    KeyValues *pFriends = new KeyValues("friends");
    // Fill online times (friends)


    pLeaderboards->AddSubKey(pFriends);

    kv->AddSubKey(pLeaderboards);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdatePlayerAvatar(int playerIndex, KeyValues *kv)
{
    // Update their avatar
    if (kv && ShowAvatars() && steamapicontext->SteamFriends() && steamapicontext->SteamUtils())
    {
        player_info_t pi;
        if (engine->GetPlayerInfo(playerIndex, &pi))
        {
            if (pi.friendsID)
            {
                CSteamID steamIDForPlayer(pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual);

                // See if we already have that avatar in our list
                int iMapIndex = m_mapAvatarsToImageList.Find(steamIDForPlayer);
                int iImageIndex;
                if (iMapIndex == m_mapAvatarsToImageList.InvalidIndex())
                {
                    CAvatarImage *pImage = new CAvatarImage();
                    // 64 is enough until full HD resolutions.
                    pImage->SetAvatarSteamID(steamIDForPlayer,k_EAvatarSize64x64);
                    pImage->SetAvatarSize(64, 64);	// Deliberately non scaling
                    iImageIndex = m_pImageList->AddImage(pImage);

                    m_mapAvatarsToImageList.Insert(steamIDForPlayer, iImageIndex);
                }
                else
                {
                    iImageIndex = m_mapAvatarsToImageList[iMapIndex];
                }

                kv->SetInt("avatar", iImageIndex);

                CAvatarImage *pAvIm = (CAvatarImage *) m_pImageList->GetImage(iImageIndex);
                pAvIm->UpdateFriendStatus();
                // Set friend draw to false, so the offset is not set
                pAvIm->SetDrawFriend(false);
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::FillScoreBoard()
{
    // update player info
    FillScoreBoard(false);
}

void CClientScoreBoardDialog::FillScoreBoard(bool pFullUpdate)
{
    KeyValues *m_kvPlayerData = new KeyValues("playdata");
    UpdatePlayerInfo(m_kvPlayerData);
    if (pFullUpdate)
    {
        AddHeader();
    }

    // Player Stats panel:
    if (m_pPlayerStats && m_pPlayerAvatar && m_lPlayerName && m_lPlayerGlobalRank && m_lPlayerMapRank && m_kvPlayerData
        && !m_kvPlayerData->IsEmpty())
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
                {
                    m_pPlayerAvatar->SetImage("default_steam");
                }
                else {
                    m_pPlayerAvatar->SetImage(m_pImageList->GetImage(pAvatarIndex));    
                }
                m_pPlayerAvatar->GetImage()->SetSize(scheme()->GetProportionalScaledValue(32), scheme()->GetProportionalScaledValue(32));
            }

            char mapRank[50];
            char mrLocalized[50];
            wchar_t *uMapRankUnicode = g_pVGuiLocalize->Find("#MOM_MapRank");
            g_pVGuiLocalize->ConvertUnicodeToANSI(uMapRankUnicode ? uMapRankUnicode : L"#MOM_MapRank", mrLocalized, 50);

            Q_snprintf(mapRank, 50, "%s: %i/%i", mrLocalized, playdata->GetInt("mapRank", -1), playdata->GetInt("mapCount", -1));
            m_lPlayerMapRank->SetText(mapRank);
            
            char globalRank[50];
            char grLocalized[50];
            wchar_t *wGlobalLocal = g_pVGuiLocalize->Find("#MOM_GlobalRank");
            g_pVGuiLocalize->ConvertUnicodeToANSI(wGlobalLocal ? wGlobalLocal : L"#MOM_GlobalRank", grLocalized, 50);

            Q_snprintf(globalRank, 50, "%s: %i/%i", grLocalized, playdata->GetInt("globalRank", -1),
                playdata->GetInt("globalCount", -1));
            m_lPlayerGlobalRank->SetText(globalRank);
        }
        m_pPlayerStats->SetVisible(true); // And seen again!
    }

    // Leaderboards
    // MOM_TODO: Discuss if this should always update (Api overload?)

    // @Gocnak: If we use event-driven updates and caching, it shouldn't overload the API.
    // The idea is to cache things to show, which will be called every FillScoreBoard call,
    // but it will only call the methods to update the data if booleans are set to.
    // For example, if a local time was saved, the event "timer_timesaved" or something is passed here,
    // and on the GetPlayerTimes passthrough, we'd update the local times, which then gets stored in
    // the Panel object until next update

    GetPlayerTimes(m_kvPlayerData);

    if (m_pLeaderboards && m_pOnlineLeaderboards && m_pLocalBests && m_kvPlayerData && !m_kvPlayerData->IsEmpty())
    {
        m_pLeaderboards->SetVisible(false);

        //MOM_TODO: switch (currentFilter)
        //case (ONLINE):
        //etc

        //Just doing local times for reference now
        KeyValues *kvLeaderboards = m_kvPlayerData->FindKey("leaderboards");
        KeyValues *kvLocalTimes = kvLeaderboards->FindKey("local");
        if (kvLocalTimes && !kvLocalTimes->IsEmpty())
        {
            for (KeyValues *kvLocalTime = kvLocalTimes->GetFirstSubKey(); kvLocalTime;
                kvLocalTime = kvLocalTime->GetNextKey())
            {
                int itemID = FindItemIDForLocalTime(kvLocalTime);
                if (itemID == -1)
                    m_pLocalBests->AddItem(m_iSectionId, kvLocalTime);
                else
                    m_pLocalBests->ModifyItem(itemID, m_iSectionId, kvLocalTime);
            }
        }

        m_pLeaderboards->SetVisible(true);
    }
    m_kvPlayerData->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: searches for the player in the scoreboard
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::FindItemIDForPlayerIndex(int playerIndex)
{
    //MOM_TODO: make this return an ItemID for another person's time (friend/global)
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

int CClientScoreBoardDialog::FindItemIDForLocalTime(KeyValues *kvRef)
{
    for (int i = 0; i <= m_pLocalBests->GetHighestItemID(); i++)
    {
        if (m_pLocalBests->IsItemIDValid(i))
        {
            KeyValues *kv = m_pLocalBests->GetItemData(i);
            if (kv && (kv->GetInt("date_t") == kvRef->GetInt("date_t")))
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
void CClientScoreBoardDialog::MoveLabelToFront(const char *textEntryName)
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
void CClientScoreBoardDialog::MoveToCenterOfScreen()
{
    int wx, wy, ww, wt;
    surface()->GetWorkspaceBounds(wx, wy, ww, wt);
    SetPos((ww - GetWide()) / 2, (wt - GetTall()) / 2);
}
