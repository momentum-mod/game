#include "cbase.h"

#include "mom_api_requests.h"
#include "mom_shareddefs.h"
#include "util/mom_util.h"
#include "mom_map_cache.h"

#include "MapInfoDialog.h"

#include "fmtstr.h"
#include "vgui/IVGui.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/CvarToggleCheckButton.h"

#include "tier0/memdbgon.h"


using namespace vgui;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogMapInfo::CDialogMapInfo(Panel *parent, MapData *pMapData) : Frame(parent, "DialogMapInfo"), m_pMapData(pMapData)
{
    // SetProportional(true);
    SetBounds(0, 0, 512, 512);
    SetMinimumSize(416, 340);
    SetDeleteSelfOnClose(true);
    
    m_bConnecting = false;
    m_bPlayerListUpdatePending = false;

    LoadControlSettings("resource/ui/MapSelector/DialogMapInfo.res");

    m_pConnectButton = FindControl<Button>("Connect", true);
    m_pCloseButton = FindControl<Button>("Close", true);
    m_pPlayerList = FindControl<ListPanel>("PlayerList", true);

    if (!m_pConnectButton || !m_pCloseButton || !m_pPlayerList)
    {
        AssertMsg(0, "Nullptr pointers on CDialogMapInfo");
    }
    // set the defaults for sorting
    // hack, need to make this more explicit functions in ListPanel
    if (m_pPlayerList)
    {
        m_pPlayerList->AddColumnHeader(0, "rank", "#MOM_Rank", 54);
        m_pPlayerList->AddColumnHeader(1, "name", "#MOM_Name", 166);
        m_pPlayerList->AddColumnHeader(2, "time", "#MOM_Time", 64);
        m_pPlayerList->AddColumnHeader(3, "date", "#MOM_Achieved", 100);

        m_pPlayerList->SetSortFunc(2, &PlayerTimeColumnSortFunc);

        m_pPlayerList->SetSortColumn(2);

        m_pPlayerList->SetEmptyListText("#MOM_API_NoTimesReturned");
    }
    if (m_pConnectButton)
    {
        m_pConnectButton->SetCommand(new KeyValues("Connect"));
    }
    if (m_pCloseButton)
    {
        m_pCloseButton->SetCommand(new KeyValues("Close"));
    }

    MoveToCenterOfScreen();

    SetTitle("#MOM_MapSelector_InfoDialog", true);

    ListenForGameEvent("map_data_update");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogMapInfo::~CDialogMapInfo()
{
}

//-----------------------------------------------------------------------------
// Purpose: Activates the dialog
//-----------------------------------------------------------------------------
void CDialogMapInfo::Run()
{
    // get the info for the map
    RequestInfo();
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
    
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Forces the game info dialog to try and connect
//-----------------------------------------------------------------------------
void CDialogMapInfo::Connect()
{
    OnConnect();
}

void CDialogMapInfo::FireGameEvent(IGameEvent* event)
{
    if (event->GetBool("info") || event->GetBool("main"))
        FillMapInfo();
}

//-----------------------------------------------------------------------------
// Purpose: Connects the user to this game
//-----------------------------------------------------------------------------
void CDialogMapInfo::OnConnect()
{
    // flag that we are attempting connection
    m_bConnecting = true;

    ApplyConnectCommand();
    Close();
}

//-----------------------------------------------------------------------------
// Purpose: Requests the right info from the server
//-----------------------------------------------------------------------------
void CDialogMapInfo::RequestInfo()
{
    GetMapInfo();
    GetTop10MapTimes();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the map, or starts downloading a map if not downloading
//-----------------------------------------------------------------------------
void CDialogMapInfo::ApplyConnectCommand()
{
    g_pMapCache->PlayMap(m_pMapData->m_uID);
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
// Purpose: player list received
//-----------------------------------------------------------------------------
void CDialogMapInfo::ClearPlayerList()
{
    m_pPlayerList->DeleteAllItems();
    Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: Sorting function for time column
//-----------------------------------------------------------------------------
int CDialogMapInfo::PlayerTimeColumnSortFunc(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
    float p1time = p1.kv->GetFloat("time_f");
    float p2time = p2.kv->GetFloat("time_f");

    if (p1time > p2time)
        return 1;
    if (p1time < p2time)
        return -1;

    return 0;
}

void CDialogMapInfo::GetMapInfo()
{
    // Update it, map cache can return false here meaning it doesn't need to
    g_pMapCache->UpdateMapInfo(m_pMapData->m_uID);

    FillMapInfo();
}

void CDialogMapInfo::FillMapInfo()
{
    // Name
    SetControlString("MapText", m_pMapData->m_szMapName);

    //Difficulty
    SetControlString("DifficultyText", CFmtStr("Tier %i", m_pMapData->m_Info.m_iDifficulty));

    //Layout
    SetControlString("LayoutText", m_pMapData->m_Info.m_bIsLinear ?
                     g_pVGuiLocalize->Find("#MOM_Linear") :
                     g_pVGuiLocalize->Find("#MOM_Staged"));

    //Zones
    SetControlString("NumZones", CFmtStr("%i", m_pMapData->m_Info.m_iNumZones));

    //Author
    CUtlString authors;
    m_pMapData->GetCreditString(&authors, CREDIT_AUTHOR);
    SetControlString("AuthorText", authors.Get());


    //Game mode
    SetControlString("GamemodeText", g_szGameModes[m_pMapData->m_eType]);
}

void CDialogMapInfo::GetTop10MapTimes()
{
    g_pAPIRequests->GetTop10MapTimes(m_pMapData->m_uID, UtlMakeDelegate(this, &CDialogMapInfo::Get10MapTimesCallback));
}

void CDialogMapInfo::Get10MapTimesCallback(KeyValues *pKvResponse)
{
    KeyValues *pData = pKvResponse->FindKey("data");
    KeyValues *pErr = pKvResponse->FindKey("error");
    if (pData)
    {
        KeyValues *pRuns = pData->FindKey("runs");

        if (pRuns && pData->GetInt("count") > 0)
        {
            ClearPlayerList();

            // Iterate through each loaded run
            FOR_EACH_SUBKEY(pRuns, pRun)
            {
                KeyValuesAD kvEntry("Entry");

                float fTime = pRun->GetFloat("time");
                kvEntry->SetFloat("time_f", fTime);
                char buf[BUFSIZETIME];
                g_pMomentumUtil->FormatTime(fTime, buf);
                kvEntry->SetString("time", buf);

                // Date
                char timeAgoStr[64];
                if (g_pMomentumUtil->GetTimeAgoString(pRun->GetString("dateAchieved"), timeAgoStr, sizeof(timeAgoStr)))
                    kvEntry->SetString("date", timeAgoStr);
                else
                    kvEntry->SetString("date", pRun->GetString("dateAchieved"));

                KeyValues *kvUserObj = pRun->FindKey("user");
                if (kvUserObj)
                {
                    /*uint64 steamID = Q_atoui64(kvUserObj->GetString("id"));
                    kvEntry->SetUint64("steamid", steamID);

                    int permissions = kvUserObj->GetInt("permissions");*/

                    // MOM_TODO: check if alias banned
                    kvEntry->SetString("name", kvUserObj->GetString("alias"));
                }

                // Rank
                KeyValues *kvRankObj = pRun->FindKey("rank");
                if (kvRankObj)
                {
                    kvEntry->SetInt("rank", kvRankObj->GetInt("rank"));
                }

                m_pPlayerList->AddItem(kvEntry, 0, false, true);
            }

            //Update the control with this new info
            InvalidateLayout();
            m_pPlayerList->InvalidateLayout();
            Repaint();
        }
    }
    else if (pErr)
    {
        // MOM_TODO error handle
    }
}