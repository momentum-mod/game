#include "pch_mapselection.h"

using namespace vgui;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogMapInfo::CDialogMapInfo(Panel *parent, const char *mapname) : Frame(parent, "DialogMapInfo")
{
    SetBounds(0, 0, 512, 512);
    SetMinimumSize(416, 340);
    SetDeleteSelfOnClose(true);

    m_bConnecting = false;
    m_bPlayerListUpdatePending = false;

    LoadControlSettings("resource/ui/DialogMapInfo.res");

    m_pConnectButton = FindControl<Button>("Connect", true);
    m_pCloseButton = FindControl<Button>("Close", true);
    m_pPlayerList = FindControl<ListPanel>("PlayerList", true);

    if (!m_pConnectButton || !m_pCloseButton || !m_pPlayerList)
    {
        Assert("Nullptr pointers on CDialogMapInfo");
    }
    // set the defaults for sorting
    // hack, need to make this more explicit functions in ListPanel
    if (m_pPlayerList)
    {
        m_pPlayerList->AddColumnHeader(0, "PlayerName", "#MOM_Name", 166);
        m_pPlayerList->AddColumnHeader(1, "Rank", "#MOM_Rank", 54);
        m_pPlayerList->AddColumnHeader(2, "Time", "#MOM_Time", 64);

        m_pPlayerList->SetSortFunc(2, &PlayerTimeColumnSortFunc);

        m_pPlayerList->SetSortColumn(2);
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
    ivgui()->AddTickSignal(GetVPanel());

    MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogMapInfo::~CDialogMapInfo()
{
    if (cbGetMapInfoCallback.IsActive())
        cbGetMapInfoCallback.Cancel();

    if (cbGet10MapTimesCallback.IsActive())
        cbGet10MapTimesCallback.Cancel();
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
// Purpose: lays out the data
//-----------------------------------------------------------------------------
void CDialogMapInfo::PerformLayout()
{
    BaseClass::PerformLayout();
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
    Q_snprintf(command, Q_ARRAYSIZE(command), "map %s\n", mapName);
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
void CDialogMapInfo::AddPlayerToList(KeyValues* pPlayerInfo)
{
    if (m_bPlayerListUpdatePending)
    {
        m_bPlayerListUpdatePending = false;
        m_pPlayerList->RemoveAll();
    }

    char buf[BUFSIZETIME];
    g_pMomentumUtil->FormatTime(pPlayerInfo->GetFloat("TimeSec"), buf);
    pPlayerInfo->SetString("Time", buf);

    m_pPlayerList->AddItem(pPlayerInfo, 0, false, true);
    pPlayerInfo->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Sorting function for time column
//-----------------------------------------------------------------------------
int CDialogMapInfo::PlayerTimeColumnSortFunc(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
    float p1time = p1.kv->GetFloat("TimeSec");
    float p2time = p2.kv->GetFloat("TimeSec");

    if (p1time > p2time)
        return 1;
    if (p1time < p2time)
        return -1;

    return 0;
}

void CDialogMapInfo::GetMapInfo(const char* mapname)
{
    if (SteamHTTP())
    {
        char szURL[BUFSIZ];
        Q_snprintf(szURL, BUFSIZ, "%s/getmapinfo/%s", MOM_APIDOMAIN, mapname);
        HTTPRequestHandle handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, szURL);
        SteamAPICall_t apiHandle;

        if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            m_bPlayerListUpdatePending = true;
            cbGetMapInfoCallback.Set(apiHandle, this, &CDialogMapInfo::GetMapInfoCallback);
        }
        else
        {
            Warning("%s - Failed to send HTTP Request to get map info!\n", __FUNCTION__);
            SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("%s - Could not use steamapi/steamapi->SteamHTTP() due to nullptr!\n", __FUNCTION__);
    }
}

void CDialogMapInfo::GetMapInfoCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
    // If something fails or the server tells us it could not find the map...
    if (bIOFailure)
    {
        Warning("%s - bIOFailure is true!\n", __FUNCTION__);
        return;
    }

    if (pCallback->m_eStatusCode == k_EHTTPStatusCode409Conflict || pCallback->m_eStatusCode == k_EHTTPStatusCode404NotFound)
    {
        char locl[BUFSIZELOCL];
        LOCALIZE_TOKEN(staged, "MOM_API_Unavailable", locl);
        SetControlString("DifficultyText", locl);
        SetControlString("GamemodeText", locl);
        SetControlString("AuthorText", locl);
        SetControlString("LayoutText", locl);
        Warning("%s - Map not found on server!\n", __FUNCTION__);
        return;
    }

    uint32 size;
    SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);
    if (size == 0)
    {
        Warning("%s - 0 body size!\n", __FUNCTION__);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

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

            SetControlString("AuthorText", pResponse->GetString("submitter"));


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
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    delete[] pData;
}
void CDialogMapInfo::Get10MapTimes(const char* mapname)
{
    if (SteamHTTP())
    {
        char szURL[BUFSIZ];
        Q_snprintf(szURL, BUFSIZ, "%s/getscores/1/%s/10", MOM_APIDOMAIN, mapname);
        HTTPRequestHandle handle = SteamHTTP()->CreateHTTPRequest(k_EHTTPMethodGET, szURL);
        SteamAPICall_t apiHandle;
        if (SteamHTTP()->SendHTTPRequest(handle, &apiHandle))
        {
            cbGet10MapTimesCallback.Set(apiHandle, this, &CDialogMapInfo::Get10MapTimesCallback);
        }
        else
        {
            Warning("%s - Failed to send HTTP Request to get map scores!\n", __FUNCTION__);
            SteamHTTP()->ReleaseHTTPRequest(handle); // GC
        }
    }
    else
    {
        Warning("%s - Could not use steamapi/steamapi->SteamHTTP() due to nullptr!\n", __FUNCTION__);
    }

}
void CDialogMapInfo::Get10MapTimesCallback(HTTPRequestCompleted_t *pCallback, bool bIOFailure)
{
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
        Warning("%s - No runs found for the map!\n", __FUNCTION__);
        return;
    }

    uint32 size;
    SteamHTTP()->GetHTTPResponseBodySize(pCallback->m_hRequest, &size);

    if (size == 0)
    {
        Warning("%s - 0 body size!\n", __FUNCTION__);
        return;
    }

    DevLog("Size of body: %u\n", size);
    uint8 *pData = new uint8[size];
    SteamHTTP()->GetHTTPResponseBodyData(pCallback->m_hRequest, pData, size);

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
            // If there is something there (not sure how it would be there at this point) remove it
            ClearPlayerList();

            KeyValues *pResponse = CJsonToKeyValues::ConvertJsonToKeyValues(val.toNode());
            KeyValues::AutoDelete ad(pResponse);

            KeyValues *pRuns = pResponse->FindKey("runs");

            if (pRuns && !pRuns->IsEmpty())
            {
                // Iterate through each loaded run
                FOR_EACH_SUBKEY(pRuns, pRun)
                {
                    KeyValues *kvEntry = new KeyValues("Entry");
                    //Time is handled by the converter
                    kvEntry->SetFloat("TimeSec", pRun->GetFloat("time"));

                    //Persona name for the time they accomplished the run
                    const char * pPerName = pRun->GetString("personaname_t");
                    kvEntry->SetString("PlayerName", !Q_strcmp(pPerName, "") ? "< blank >" : pRun->GetString("personaname_t"));

                    //Rank
                    kvEntry->SetInt("rank", static_cast<int>(pRun->GetFloat("rank")));

                    //This handles deleting the kvEntry
                    AddPlayerToList(kvEntry);
                }

                //Update the control with this new info
                InvalidateLayout();
                m_pPlayerList->InvalidateLayout();
                Repaint();
            }
        }
    }
    else
    {
        Warning("%s at %zd\n", jsonStrError(status), endPtr - pDataPtr);
    }
    // Last but not least, free resources
    alloc.deallocate();
    SteamHTTP()->ReleaseHTTPRequest(pCallback->m_hRequest);
    delete[] pData;
}