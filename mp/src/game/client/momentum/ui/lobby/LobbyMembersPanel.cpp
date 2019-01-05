#include "cbase.h"

#include "LobbyMembersPanel.h"
#include "SavelocRequestFrame.h"
#include "leaderboards/ClientTimesDisplay.h"
#include "leaderboards/LeaderboardsContextMenu.h"

#include "vgui/ISurface.h"
#include <vgui_controls/ImageList.h>
#include "vgui_controls/SectionedListPanel.h"
#include <vgui_controls/Button.h>

#include "mom_shareddefs.h"
#include "vgui_avatarimage.h"

#include "mom_modulecomms.h"

#include "tier0/memdbgon.h"

using namespace vgui;

// Flip the 0 to 1 to test the panel with a local name
#define TEST_LOBBY_CREATE 0

LobbyMembersPanel::LobbyMembersPanel(IViewPort *pParent) : BaseClass(nullptr, PANEL_LOBBY_MEMBERS)
{
    m_iSectionId = 0;

    // initialize dialog
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    // Create a "popup" so we can get the mouse to detach
    surface()->CreatePopup(GetVPanel(), false, false, false, false, false);

    // set the scheme before any child control is created
    SetScheme("ClientScheme");

    AddActionSignalTarget(this);

    m_pImageListLobby = new ImageList(true);
    SetDefLessFunc(m_mapLobbyIDToImageListIndx);

    m_pSavelocReqFrame = new SavelocReqFrame();

    m_pMemberList = new SectionedListPanel(this, "MemberList");
    m_pLobbyToggle = new Button(this, "LobbyToggle", "#GameUI2_HostLobby", this, "HostLobby");
    m_pInviteFriends = new Button(this, "InviteFriends", "#GameUI2_InviteLobby", this, "InviteFriends");

    LoadControlSettings("resource/ui/LobbyMembersPanel.res");

    m_pMemberList->SetKeyBoardInputEnabled(true);
    m_pContextMenu = new CLeaderboardsContextMenu(m_pMemberList);
    m_pContextMenu->SetAutoDelete(false);
    m_pContextMenu->AddActionSignalTarget(this);
    m_pContextMenu->SetVisible(false);

    InitLobbyPanelSections();

    ListenForGameEvent("lobby_leave");
}

LobbyMembersPanel::~LobbyMembersPanel()
{
    if (m_pContextMenu)
    {
        m_pContextMenu->DeletePanel();
    }
}

void LobbyMembersPanel::FireGameEvent(IGameEvent* event)
{
    // Clear out the index map and the image list when you leave the lobby
    m_pMemberList->DeleteAllItems();
    m_mapLobbyIDToImageListIndx.RemoveAll();
    if (m_pImageListLobby)
    {
        delete m_pImageListLobby;
        m_pImageListLobby = nullptr;
    }

    // And like a phoenix, rise from the ashes
    m_pImageListLobby = new ImageList(true);
    m_pMemberList->SetImageList(m_pImageListLobby, false);

    m_pLobbyToggle->SetText("#GameUI2_HostLobby");
    m_pLobbyToggle->SetCommand("HostLobby");
    m_pInviteFriends->SetVisible(false);
    InvalidateLayout(true);
}


void LobbyMembersPanel::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
    {
        // Add this user to the panel
        AddLobbyMember(CSteamID(pParam->m_ulSteamIDUserChanged));
    }
    else if (pParam->m_rgfChatMemberStateChange & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
    {
        // Get em outta here
        m_pMemberList->RemoveItem(FindItemIDForLobbyMember(pParam->m_ulSteamIDUserChanged));
    }
}

void LobbyMembersPanel::AddLobbyMember(const CSteamID& steamID)
{
    if (FindItemIDForLobbyMember(steamID) > -1)
        return;

    KeyValues *pNewUser = new KeyValues("LobbyMember");
    uint64 steamIdInt = steamID.ConvertToUint64();
    pNewUser->SetUint64("steamid", steamIdInt);
    pNewUser->SetInt("avatar", TryAddAvatar(steamIdInt, &m_mapLobbyIDToImageListIndx, m_pImageListLobby));
    pNewUser->SetString("personaname", SteamFriends()->GetFriendPersonaName(steamID));

    m_pMemberList->AddItem(m_iSectionId, pNewUser);
    pNewUser->deleteThis(); // Copied over in AddItem
}

void LobbyMembersPanel::UpdateLobbyMemberData(const CSteamID& memberID)
{
    if (!m_idLobby.IsValid())
        return;

    int itemID = FindItemIDForLobbyMember(memberID);
    if (itemID > -1)
    {
        // Old one gets deleted in the ModifyItem code, don't worry
        KeyValues *pData = m_pMemberList->GetItemData(itemID)->MakeCopy();
        if (pData)
        {
            const char *pMap = SteamMatchmaking()->GetLobbyMemberData(m_idLobby, memberID, LOBBY_DATA_MAP);
            pData->SetString("map", pMap);
            // MOM_TODO: Spectating? Typing? 

            m_pMemberList->ModifyItem(itemID, m_iSectionId, pData);
            pData->deleteThis();
        }
    }
}

bool LobbyMembersPanel::StaticLobbyMemberSortFunc(vgui::SectionedListPanel* list, int itemID1, int itemID2)
{
    KeyValues *it1 = list->GetItemData(itemID1);
    KeyValues *it2 = list->GetItemData(itemID2);
    const char *pMapName = g_pGameRules->MapName();

    if (!pMapName)
        return false;

    Assert(it1 && it2);

    bool is1OnMap = FStrEq(pMapName, it1->GetString("map"));
    bool is2OnMap = FStrEq(pMapName, it2->GetString("map"));

    if (is1OnMap)
    {
        if (is2OnMap)
        {
            // If both are on the same map, go by name. We're rooting for it1 to be in front here, so
            // if strcmp returns negative, it1 is before it2. Hopefully they aren't the same string!
            return Q_strcmp(it1->GetString("personaname"), it2->GetString("personaname")) < 0;
        }
        // else it1 is on our map, they go first
        return true;
    }
    //else 
    if (is2OnMap)
        return false; // it2 goes first, since they're on our map

    // If all else fails just do item ID comparison idk
    return itemID1 < itemID2;
}

int LobbyMembersPanel::FindItemIDForLobbyMember(uint64 steamID)
{
    for (int i = 0; i <= m_pMemberList->GetHighestItemID(); i++)
    {
        if (m_pMemberList->IsItemIDValid(i))
        {
            KeyValues *kv = m_pMemberList->GetItemData(i);
            if (kv && (kv->GetUint64("steamid") == steamID))
            {
                return i;
            }
        }
    }
    return -1;
}

const char* LobbyMembersPanel::GetName()
{
    return PANEL_LOBBY_MEMBERS;
}

void LobbyMembersPanel::Reset()
{
    m_pMemberList->ClearSelection();
}

void LobbyMembersPanel::ShowPanel(bool bShow)
{
    // Catch the case where we call ShowPanel before ApplySchemeSettings, eg when
    // going from windowed <-> fullscreen
    if (!m_pImageListLobby && bShow)
    {
        InvalidateLayout(true, true);
    }

    if (m_pContextMenu && m_pContextMenu->IsVisible())
    {
        // Close the menu
        m_pContextMenu->OnKillFocus();
    }

    if (BaseClass::IsVisible() == bShow)
        return;

    if (bShow)
    {
        Reset();
        SetVisible(true);
        // SetEnabled(true);
        MoveToFront();
    }
    else
    {
        SetVisible(false);
        SetMouseInputEnabled(false);
    }
}

void LobbyMembersPanel::OnCommand(const char* command)
{
    if (FStrEq("HostLobby", command))
    {
        // Host a lobby
        engine->ClientCmd_Unrestricted("mom_lobby_create\n");
    }
    else if (FStrEq("LeaveLobby", command))
    {
        engine->ClientCmd_Unrestricted("mom_lobby_leave\n");
    }
    else if (FStrEq("InviteFriends", command))
    {
        engine->ClientCmd_Unrestricted("mom_lobby_invite\n");
    }

    BaseClass::OnCommand(command);
}

void LobbyMembersPanel::OnContextGoToMap(const char* map)
{
    // MOM_TODO: We're going to need to feed this into a map downloader first, if they don't have the map!
    char command[128];
    Q_snprintf(command, 128, "map %s\n", map);
    ShowPanel(false);
    engine->ClientCmd_Unrestricted(command);
}

void LobbyMembersPanel::OnContextReqSavelocs(uint64 target)
{
    ShowPanel(false);
    KeyValues *pReq = new KeyValues("req_savelocs");
    // Stage 1 is request the count, make the other player make a copy of their savelocs for us
    pReq->SetInt("stage", 1);
    pReq->SetUint64("target", target);
    g_pModuleComms->FireEvent(pReq);

    m_pSavelocReqFrame->Activate(target);
}

void LobbyMembersPanel::OnContextVisitProfile(uint64 profile)
{
    if (profile != 0 && SteamFriends())
    {
        SteamFriends()->ActivateGameOverlayToUser("steamid", CSteamID(profile));
        ShowPanel(false);
    }
}

void LobbyMembersPanel::OnSpectateLobbyMember(uint64 target)
{
    char command[128];
    Q_snprintf(command, 128, "mom_spectate %llu\n", target);
    ShowPanel(false);
    engine->ClientCmd_Unrestricted(command);
}

void LobbyMembersPanel::OnLobbyCreated(LobbyCreated_t* pParam)
{
#if TEST_LOBBY_CREATE
    KeyValues *pNewUser = new KeyValues("LobbyMember");

    uint64 steamID = SteamUser()->GetSteamID().ConvertToUint64();

    pNewUser->SetUint64("steamid", steamID);
    pNewUser->SetInt("avatar", TryAddAvatar(steamID, &m_mapLobbyIDToImageListIndx, m_pImageListLobby));
    pNewUser->SetString("personaname", SteamFriends()->GetPersonaName());
    pNewUser->SetString("map", "triggertests");

    m_pMemberList->AddItem(m_iSectionId, pNewUser);
    pNewUser->deleteThis();
#endif
}

void LobbyMembersPanel::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    if (pParam->m_ulSteamIDMember == pParam->m_ulSteamIDLobby)
    {
        // The lobby itself changed
        // MOM_TODO: Have some sort of data about the lobby in this panel?
    }
    else
    {
        // A member in the lobby changed
        CSteamID local = SteamUser()->GetSteamID();
        if (local.ConvertToUint64() != pParam->m_ulSteamIDMember)
            UpdateLobbyMemberData(CSteamID(pParam->m_ulSteamIDMember));
    }
}


void LobbyMembersPanel::OnLobbyEnter(LobbyEnter_t* pParam)
{
    m_idLobby = CSteamID(pParam->m_ulSteamIDLobby);

    m_pLobbyToggle->SetText("#GameUI2_LeaveLobby");
    m_pLobbyToggle->SetCommand("LeaveLobby");
    m_pInviteFriends->SetVisible(true);

    // Add everyone now
    PopulateLobbyPanel();

    InvalidateLayout(true);
}


void LobbyMembersPanel::PopulateLobbyPanel()
{
    if (!m_idLobby.IsValid())
        return;

    const CSteamID local = SteamUser()->GetSteamID();
    const int numLobbyMembers = SteamMatchmaking()->GetNumLobbyMembers(m_idLobby);

    for (int i = 0; i < numLobbyMembers; i++)
    {
        const CSteamID inLobby = SteamMatchmaking()->GetLobbyMemberByIndex(m_idLobby, i);
        if (inLobby == local)
            continue;

        // Get their initial data (name, avatar, steamID)
        AddLobbyMember(inLobby);
        // Get their status data (map, spectating, etc)
        UpdateLobbyMemberData(inLobby);
    }
}

void LobbyMembersPanel::InitLobbyPanelSections()
{
    m_pMemberList->AddSection(m_iSectionId, "", StaticLobbyMemberSortFunc);
    m_pMemberList->SetSectionAlwaysVisible(m_iSectionId);
    m_pMemberList->SetImageList(m_pImageListLobby, false);
    m_pMemberList->AddColumnToSection(m_iSectionId, "avatar", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, 45);
    m_pMemberList->AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name", 0, 160);
    m_pMemberList->AddColumnToSection(m_iSectionId, "map", "#MOM_MapSelector_Map", 0, 160);

    // MOM_TODO: Have stuff like status and whatever else?
}

int LobbyMembersPanel::TryAddAvatar(const uint64& steamID, CUtlMap<uint64, int>* pIDtoIndxMap, vgui::ImageList* pImageList)
{
    // Update their avatar
    if (pIDtoIndxMap && pImageList)
    {
        // See if we already have that avatar in our list
        const unsigned short mapIndex = pIDtoIndxMap->Find(steamID);
        int iImageIndex;
        if (!pIDtoIndxMap->IsValidIndex(mapIndex))
        {
            CAvatarImage *pImage = new CAvatarImage();
            // 64 is enough up to full HD resolutions.
            pImage->SetAvatarSteamID(CSteamID(steamID), k_EAvatarSize64x64);

            pImage->SetDrawFriend(false);
            pImage->SetAvatarSize(32, 32);
            iImageIndex = pImageList->AddImage(pImage);
            pIDtoIndxMap->Insert(steamID, iImageIndex);
        }
        else
        {
            iImageIndex = pIDtoIndxMap->Element(mapIndex);
        }
        return iImageIndex;
    }
    return -1;
}

void LobbyMembersPanel::OnItemContextMenu(KeyValues *pData)
{
    int itemID = pData->GetInt("itemID", -1);
    KeyValues *pKVData = m_pMemberList->GetItemData(itemID);

    uint64 steamID = pKVData->GetUint64("steamid");
    const char *pMap = pKVData->GetString("map");
    const char *pOurMap = g_pGameRules->MapName();

    KeyValues *pKv;

    m_pContextMenu->DeleteAllItems();

    if (pOurMap && FStrEq(pMap, pOurMap))
    {
        pKv = new KeyValues("ContextSpectate");
        pKv->SetUint64("target", steamID);
        m_pContextMenu->AddMenuItem("SpectateLobbyMember", "#MOM_Leaderboards_Spectate", pKv, this);
    }
    else
    {
        pKv = new KeyValues("ContextGoToMap");
        pKv->SetString("map", pMap);
        m_pContextMenu->AddMenuItem("GoToMap", "#MOM_Leaderboards_GoToMap", pKv, this);
    }

    pKv = new KeyValues("ContextReqSavelocs");
    pKv->SetUint64("target", steamID);
    m_pContextMenu->AddMenuItem("ReqSavelocs", "#MOM_Saveloc_Frame", pKv, this);

    // MOM_TODO: More options here, such as:
    // kicking the player if we're the lobby leader
    // hiding decals (maybe toggle paint, bullets separately?)
    // etc
    m_pContextMenu->AddSeparator();
    // Visit profile
    pKv = new KeyValues("ContextVisitProfile");
    pKv->SetUint64("profile", steamID);
    m_pContextMenu->AddMenuItem("VisitProfile", "#MOM_Leaderboards_SteamProfile", pKv, this);

    m_pContextMenu->ShowMenu();
}
