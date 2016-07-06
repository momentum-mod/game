#include "pch_mapselection.h"

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
        PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 2));
        PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 1));
        PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 1));
    }
    if (m_pConnectButton)
    {
        m_pConnectButton->SetCommand(new KeyValues("Connect"));
    }
    if (m_pCloseButton)
    {
        m_pCloseButton->SetCommand(new KeyValues("Close"));
    }

    // refresh immediately
    //RequestInfo();

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
#if 0 // TBD delete this func
    if (m_SteamIDFriend && m_SteamIDFriend == pPersonaStateChange->m_ulSteamID)
    {
        // friend may have changed servers
        uint64 nGameID;
        uint32 unGameIP;
        uint16 usGamePort;
        uint16 usQueryPort;

        if (SteamFriends()->GetFriendGamePlayed(m_SteamIDFriend, &nGameID, &unGameIP, &usGamePort, &usQueryPort))
        {
            if (pPersonaStateChange->m_nGameIDPrevious != nGameID
                || pPersonaStateChange->m_unGameServerIPPrevious != unGameIP
                || pPersonaStateChange->m_usGameServerPortPrevious != usGamePort
                || pPersonaStateChange->m_usGameServerQueryPortPrevious != usQueryPort)
            {
                ChangeGame(unGameIP, usQueryPort, usGamePort);
            }
        }
    }
#endif
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
    
    m_pPlayerList->SetEmptyListText("No one wants to run this map");
    
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
        return -1;
    if (p1time < p2time)
        return 1;

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
        Warning("CDialogMapInfo::Could not use steamapi/steamapi->SteamHTTP() due to nullptr!\n");
    }
}

void CDialogMapInfo::GetMapInfoCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    // If something fails or the server tells us it could not find the map...
    if (bIOFailure || pCallback->m_eStatusCode == k_EHTTPStatusCode204NoContent)
        return;
    uint32 size;

    steamapicontext->SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        Warning("CDialogMapInfo::GetMapInfo: 0 body size!\n");
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
            JsonValue oval = val.toNode()->value;
            if (oval.getTag() == JSON_ARRAY)
            {
                for (auto i : oval)
                {
                    if (i->value.getTag() == JSON_OBJECT)
                    {
                        for (auto j : i->value)
                        {
                            if (!Q_strcmp(j->key, "difficulty"))
                            {
                                char buffer[32];
                                Q_snprintf(buffer, sizeof(buffer), "Tier %g", j->value.toNumber());
                                SetControlString("DifficultyText", buffer);
                            }
                            else if (!Q_strcmp(j->key, "layout"))
                            {
                                char buffer[32];
                                int layout = j->value.toNumber();
                                if (layout > 0)
                                {
                                    char locl[BUFSIZELOCL];
                                    LOCALIZE_TOKEN(staged, "MOM_AmountStages", locl);
                                    Q_snprintf(buffer, sizeof(buffer), locl, layout);
                                }
                                else
                                {
                                    LOCALIZE_TOKEN(linear, "MOM_Linear", buffer);
                                }
                                SetControlString("LayoutText", buffer);
                            }
                            else if (!Q_strcmp(j->key, "submitter"))
                            {
                                char buffer[32];
                                Q_snprintf(buffer, sizeof(buffer), "%llu", (uint64)j->value.toNumber());
                                SetControlString("AuthorText", buffer);
                            }
                            else if (!Q_strcmp(j->key, "gamemode"))
                            {
                                char* buffer;
                                switch ((int)j->value.toNumber())
                                {
                                case MOMGM_SURF:
                                    buffer = "Surf";
                                    break;
                                case MOMGM_BHOP:
                                    buffer = "Bunnyhop";
                                    break;
                                default:
                                    buffer = "Unknown";
                                    break;
                                }
                                SetControlString("GamemodeText", buffer);
                            }
                            else if (!Q_strcmp(j->key, "result") && !Q_strcmp(j->value.toString(), "false"))
                            {
                                char locl[BUFSIZELOCL];
                                LOCALIZE_TOKEN(staged, "MOM_API_Unavailable", locl);
                                SetControlString("DifficultyText", locl);
                                SetControlString("GamemodeText", locl);
                                SetControlString("AuthorText", locl);
                                SetControlString("LayoutText", locl);
                                break;
                            }
                            else
                            {
                                DevLog("Uncaught key %s with value %s\n", j->key, j->value.getTag() == JSON_STRING ? j->value.toString() : "It's a number");
                            }
                        }
                    }
                }
            }
        }
    }
}