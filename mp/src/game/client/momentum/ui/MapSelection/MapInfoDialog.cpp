#include "pch_mapselection.h"
#include <util/jsontokv.h>

using namespace vgui;
//extern class IAppInformation *g_pAppInformation; // may be NULL

static const long RETRY_TIME = 10000;		// refresh server every 10 seconds
static const long CHALLENGE_ENTRIES = 1024;

extern "C"
{
    DLL_EXPORT bool JoiningSecureServerCall()
    {
        return true;
    }
}

//-----------------------------------------------------------------------------
// Purpose: Comparison function used in query redblack tree
//-----------------------------------------------------------------------------
/*bool QueryLessFunc(const struct challenge_s &item1, const struct challenge_s &item2)
{
return false;
// compare port then ip
if (item1.addr.GetPort() < item2.addr.GetPort())
return true;
else if (item1.addr.GetPort() > item2.addr.GetPort())
return false;

//int ip1 = item1.addr.GetIP();
//int ip2 = item2.addr.GetIP();
//  return ip1 < ip2;
}*/


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogMapInfo::CDialogMapInfo(vgui::Panel *parent, const char *mapname) :
Frame(parent, "DialogMapInfo")
#ifndef NO_STEAM
, m_CallbackPersonaStateChange(this, &CDialogMapInfo::OnPersonaStateChange)
#endif
{
    SetBounds(0, 0, 512, 512);
    SetMinimumSize(416, 340);
    SetDeleteSelfOnClose(true);

    m_bConnecting = false;
    m_bPlayerListUpdatePending = false;

    LoadControlSettings("resource/ui/DialogMapInfo.res");

    m_pConnectButton = FindControl<Button>("Connect", true);
    m_pCloseButton = FindControl<Button>("Close", true);
    m_pPlayerList = FindControl<ListPanel>("PlayerList", true);;

    if (!m_pConnectButton || !m_pCloseButton || !m_pPlayerList)
    {
        Assert("Nullptr pointers on CDialogMapInfo");
    }
    // set the defaults for sorting
    // hack, need to make this more explicit functions in ListPanel
    if (m_pPlayerList)
    {
        m_pPlayerList->AddColumnHeader(0, "PlayerName", "#MOM_Name", 166);
        m_pPlayerList->AddColumnHeader(1, "Score", "#MOM_Rank", 54);
        m_pPlayerList->AddColumnHeader(2, "Time", "#MOM_Time", 64);

        m_pPlayerList->SetSortFunc(2, &PlayerTimeColumnSortFunc);

        PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 2));
        //PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 1));
        //PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 1));
    }
    if (m_pConnectButton)
    {
        m_pConnectButton->SetCommand(new KeyValues("Connect"));
    }
    if (m_pCloseButton)
    {
        m_pCloseButton->SetCommand(new KeyValues("Close"));
    }

    // let us be ticked every frame
    ivgui()->AddTickSignal(this->GetVPanel());

    MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogMapInfo::~CDialogMapInfo()
{
    //MOM_TODO: Cancel any queries sent out
    /*#ifndef NO_STEAM
        if (!SteamMatchmakingServers())
        return;

        if (m_hPingQuery != HSERVERQUERY_INVALID)
        SteamMatchmakingServers()->CancelServerQuery(m_hPingQuery);
        if (m_hPlayersQuery != HSERVERQUERY_INVALID)
        SteamMatchmakingServers()->CancelServerQuery(m_hPlayersQuery);
        #endif*/
}

//-----------------------------------------------------------------------------
// Purpose: Activates the dialog
//-----------------------------------------------------------------------------
void CDialogMapInfo::Run(const char *titleName)
{

    SetTitle("#ServerBrowser_GameInfoWithNameTitle", true);

    SetDialogVariable("game", titleName);

    
    //MOM_TODO: LoadLocalInfo(); //Loads the local information (local PBs, local replays)
    // We ask for some info of this map to the web. IF it is a valid map, we'll fill the info
    SetControlString("MapText", titleName);
    // get the info for the map
    RequestInfo(titleName);
    Activate();
}

//-----------------------------------------------------------------------------
// Purpose: updates the dialog if it's watching a friend who changes servers
//-----------------------------------------------------------------------------
#ifndef NO_STEAM
void CDialogMapInfo::OnPersonaStateChange(PersonaStateChange_t *pPersonaStateChange)
{

    if (pPersonaStateChange->m_nChangeFlags & k_EPersonaChangeNameFirstSet || 
        pPersonaStateChange->m_nChangeFlags & k_EPersonaChangeName)
    {
        SetControlString("AuthorText", steamapicontext->SteamFriends()->GetFriendPersonaName(CSteamID(pPersonaStateChange->m_ulSteamID)));
    }
}
#endif // NO_STEAM

//-----------------------------------------------------------------------------
// Purpose: lays out the data
//-----------------------------------------------------------------------------
void CDialogMapInfo::PerformLayout()
{
    BaseClass::PerformLayout();
    /*if (!Q_strlen(m_Server.m_szGameDescription) && m_SteamIDFriend && g_pAppInformation)
    {
    // no game description set yet
    // get the game from the friends info if we can
    uint64 nGameID = 0;
    #ifndef NO_STEAM
    //SteamFriends()->GetFriendGamePlayed(m_SteamIDFriend, &nGameID, NULL, NULL, NULL);
    #endif
    if (nGameID)
    {
    //SetControlString("GameText", g_pAppInformation->GetAppName(g_pAppInformation->GetIAppForAppID(nGameID)));
    }
    else
    {
    SetControlString("GameText", "#ServerBrowser_NotInGame");
    }
    }*/


    // Conditions that allow for a map to be played: none, always playable
    m_pConnectButton->SetEnabled(true);
    
    m_pPlayerList->SetEmptyListText("Nobody has run this map!");
    
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Forces the game info dialog to try and connect
//-----------------------------------------------------------------------------
void CDialogMapInfo::Connect()
{
    OnConnect();
}

//-----------------------------------------------------------------------------
// Purpose: Connects the user to this game
//-----------------------------------------------------------------------------
void CDialogMapInfo::OnConnect()
{
    // flag that we are attempting connection
    m_bConnecting = true;

    InvalidateLayout();

    ApplyConnectCommand(GetControlString("MapText"));
    Close();
}

//-----------------------------------------------------------------------------
// Purpose: Requests the right info from the server
//-----------------------------------------------------------------------------
void CDialogMapInfo::RequestInfo(const char* mapName)
{
    GetMapInfo(mapName);
    Get10MapTimes(mapName);
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame, handles resending network messages
//-----------------------------------------------------------------------------
void CDialogMapInfo::OnTick()
{
    BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: Constructs a command to send a running game to connect to a server,
// based on the server type
//
// TODO it would be nice to push this logic into the IRunGameEngine interface; that
// way we could ask the engine itself to construct arguments in ways that fit.
// Might be worth the effort as we start to add more engines.
//-----------------------------------------------------------------------------
void CDialogMapInfo::ApplyConnectCommand(const char *mapName)
{
    char command[256];
    // send engine command to change servers
    Q_snprintf(command, Q_ARRAYSIZE(command), "progress_enable;map %s\n", mapName);
    engine->ExecuteClientCmd(command);
}

//-----------------------------------------------------------------------------
// Purpose: Connects to the server
//-----------------------------------------------------------------------------
void CDialogMapInfo::ConnectToServer()
{
    m_bConnecting = false;
    // close this dialog
    PostMessage(this, new KeyValues("Close"));
}

//-----------------------------------------------------------------------------
// Purpose: called when the current refresh list is complete
//-----------------------------------------------------------------------------
void CDialogMapInfo::RefreshComplete(EMatchMakingServerResponse response)
{
}

//-----------------------------------------------------------------------------
// Purpose: player list received
//-----------------------------------------------------------------------------
void CDialogMapInfo::ClearPlayerList()
{
    m_pPlayerList->DeleteAllItems();
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: on individual player added
//-----------------------------------------------------------------------------
void CDialogMapInfo::AddPlayerToList(const char *playerName, int score, float timePlayedSeconds)
{
    if (m_bPlayerListUpdatePending)
    {
        m_bPlayerListUpdatePending = false;
        m_pPlayerList->RemoveAll();
    }

    KeyValues *player = new KeyValues("player");
    player->SetString("PlayerName", playerName);
    player->SetInt("Score", score);
    player->SetInt("TimeSec", (int)timePlayedSeconds);

    // construct a time string
    int seconds = (int)timePlayedSeconds;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    seconds %= 60;
    minutes %= 60;
    char buf[64];
    buf[0] = 0;
    if (hours)
    {
        Q_snprintf(buf, sizeof(buf), "%dh %dm %ds", hours, minutes, seconds);
    }
    else if (minutes)
    {
        Q_snprintf(buf, sizeof(buf), "%dm %ds", minutes, seconds);
    }
    else
    {
        Q_snprintf(buf, sizeof(buf), "%ds", seconds);
    }
    player->SetString("Time", buf);

    m_pPlayerList->AddItem(player, 0, false, true);
    player->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Sorting function for time column
//-----------------------------------------------------------------------------
int CDialogMapInfo::PlayerTimeColumnSortFunc(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
    int p1time = p1.kv->GetInt("TimeSec");
    int p2time = p2.kv->GetInt("TimeSec");

    if (p1time > p2time)
        return 1;
    if (p1time < p2time)
        return -1;

    return 0;
}

void CDialogMapInfo::GetMapInfo(const char* mapname)
{
    if (steamapicontext && steamapicontext->SteamHTTP())
    {
        char szURL[512];
        Q_snprintf(szURL, 512, "%s/getmapinfo/%s", MOM_APIDOMAIN, mapname);
        HTTPRequestHandle handle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, szURL);
        SteamAPICall_t apiHandle;

        if (steamapicontext->SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            m_bPlayerListUpdatePending = true;
            cbGetMapInfoCallback.Set(apiHandle, this, &CDialogMapInfo::GetMapInfoCallback);
        }
        else
        {
            Warning("Failed to send HTTP Request to get map info!\n");
            steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("CDialogMapInfo::GetMapInfo() - Could not use steamapi/steamapi->SteamHTTP() due to nullptr!\n");
    }
}

void CDialogMapInfo::GetMapInfoCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    // If something fails or the server tells us it could not find the map...
    if (bIOFailure)
    {
        Warning("%s::%s - bIOFailure is true!\n", __FILE__, __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode409Conflict)
    {
        char locl[BUFSIZELOCL];
        LOCALIZE_TOKEN(staged, "MOM_API_Unavailable", locl);
        SetControlString("DifficultyText", locl);
        SetControlString("GamemodeText", locl);
        SetControlString("AuthorText", locl);
        SetControlString("LayoutText", locl);
        Warning("%s::%s - Map not found on server!\n", __FILE__, __FUNCTION__);
        return;
    }

    uint32 size;
    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        Warning("%s::%s - 0 body size!\n", __FILE__, __FUNCTION__);
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

            //Difficulty
            char buffer[32];
            Q_snprintf(buffer, sizeof(buffer), "Tier %g", pResponse->GetFloat("difficulty"));
            SetControlString("DifficultyText", buffer);

            V_memset(buffer, 0, sizeof(buffer));

            //Layout
            bool linear = pResponse->GetBool("linear");
            
            if (linear)
            {
                LOCALIZE_TOKEN(linear, "MOM_Linear", buffer);
            }
            else
            {
                LOCALIZE_TOKEN(staged, "MOM_MapSelector_InfoDialog_Staged", buffer);
            }
            SetControlString("LayoutText", buffer);

            V_memset(buffer, 0, sizeof(buffer));

            //Zones
            int zones = static_cast<int>(pResponse->GetFloat("zones"));

            char locl[BUFSIZELOCL];
            LOCALIZE_TOKEN(staged, "MOM_AmountZones", locl);
            Q_snprintf(buffer, sizeof(buffer), locl, zones);

            SetControlString("NumZones", buffer);

            //Author
            uint64 steamID = Q_atoui64(pResponse->GetString("submitter"));
            CSteamID submitter(steamID);

            if (steamapicontext->SteamFriends()->RequestUserInformation(submitter, true))
            {
                SetControlString("AuthorText", "Requesting map author...");
            }
            else
            {
                SetControlString("AuthorText", steamapicontext->SteamFriends()->GetFriendPersonaName(submitter));
            }

            //Game mode
            //MOM_TODO: Potentially have this part of the site?
            int gameMode = static_cast<int>(pResponse->GetFloat("gamemode"));
            const char *gameType;
            switch (gameMode)
            {
            case MOMGM_SURF:
                gameType = "Surf";
                break;
            case MOMGM_BHOP:
                gameType = "Bunnyhop";
                break;
            default:
                gameType = "Unknown";
                break;
            }
            SetControlString("GamemodeText", gameType);
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }

    //Cleanup
    alloc.deallocate();
    steamapicontext->SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    delete[] pData;
    pData = nullptr;
}
void CDialogMapInfo::Get10MapTimes(const char* mapname)
{
    if (steamapicontext && steamapicontext->SteamHTTP())
    {
        char szURL[512];
        Q_snprintf(szURL, 512, "%s/getscores/%s", MOM_APIDOMAIN, mapname);
        HTTPRequestHandle handle = steamapicontext->SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, szURL);
        SteamAPICall_t apiHandle;
        if (steamapicontext->SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            cbGet10MapTimesCallback.Set(apiHandle, this, &CDialogMapInfo::Get10MapTimesCallback);
        }
        else
        {
            Warning("Failed to send HTTP Request to get map scores!\n");
            steamapicontext->SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("CDialogMapInfo::Get10MapTimes() - Could not use steamapi/steamapi->SteamHTTP() due to nullptr!\n");
    }

}
void CDialogMapInfo::Get10MapTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    Warning("Callback received.\n");
    if (bIOFailure || pCallback->m_eStatusCode == k_EHTTPStatusCode204NoContent || pCallback->m_eStatusCode == k_EHTTPStatusCode404NotFound)
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
                // If there is something there (not sure how it would be there at this point) remove it
                ClearPlayerList();
                for (auto i : oval)
                {
                    if (i->value.getTag() == JSON_OBJECT)
                    {
                        KeyValues *kv = new KeyValues("entry");
                        bool bFalseResult = false;
                        for (auto j : i->value)
                        {
                            if (!Q_strcmp(j->key, "time"))
                            {
                                kv->SetFloat("time", j->value.toNumber());
                            }
                            else if (!Q_strcmp(j->key, "steamid"))
                            {
                                uint64 steamid = j->value.toNumber();

                                kv->SetUint64("steamid", steamid);
                                // MOM_TODO: Localize this string
                                char locl[BUFSIZELOCL];
                                LOCALIZE_TOKEN(apiresponse, "MOM_API_WaitingForResponse", locl);
                                kv->SetString("personaname", locl);
                                
                            }
                            else if (!Q_strcmp(j->key, "rank"))
                            {
                                // MOM_TODO: Implement
                            }
                            else if (!Q_strcmp(j->key, "rate"))
                            {
                                kv->SetInt("rate", j->value.toNumber());
                            }
                            else if (!Q_strcmp(j->key, "date"))
                            {
                                // MOM_TODO: Implement.
                            }
                            else if (!Q_strcmp(j->key, "id"))
                            {
                                kv->SetInt("id", j->value.toNumber());
                            }
                            else if (!Q_strcmp(j->key, "result") && !Q_strcmp(j->value.toString(), "false"))
                            {
                                char locl[BUFSIZELOCL];
                                LOCALIZE_TOKEN(staged, "MOM_API_Unavailable", locl);
                                SetControlString("PlayerList", locl);
                                bFalseResult = true;
                                break;
                            }
                            else
                            {
                                if (j->value.getTag() == JSON_STRING)
                                    DevLog("Uncaught key %s with string value %s\n", j->key, j->value.toString());
                                else if (j->value.getTag() == JSON_STRING)
                                    DevLog("Uncaught key %s with numerical value %i\n", j->key, (int)j->value.toNumber());
                                else
                                    DevLog("Uncaught key %s with object value\n");
                            }
                        }
                        if (!bFalseResult)
                        {
                            AddPlayerToList(kv->GetString("personaname"), kv->GetInt("id"), kv->GetFloat("time"));
                            InvalidateLayout();
                            m_pPlayerList->InvalidateLayout();
                            Repaint();
                        }
                        kv->deleteThis();
                    }
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
