#include "cbase.h"

#include "LobbyMembersPanel.h"
#include <vgui_controls/ImageList.h>
#include "mom_shareddefs.h"
#include "vgui_avatarimage.h"


#include "tier0/memdbgon.h"

using namespace vgui;

LobbyMembersPanel::LobbyMembersPanel(Panel *pParent) : BaseClass(pParent, "LobbyMembers")
{
    m_iSectionId = 0;

    m_pImageListLobby = new ImageList(true);
    SetDefLessFunc(m_mapLobbyIDToImageListIndx);

    InitLobbyPanelSections();

    ListenForGameEvent("lobby_leave");
}

LobbyMembersPanel::~LobbyMembersPanel()
{
}

void LobbyMembersPanel::FireGameEvent(IGameEvent* event)
{
    // Clear out the index map and the image list when you leave the lobby
    DeleteAllItems();
    RemoveAllSections();
    m_mapLobbyIDToImageListIndx.RemoveAll();
    if (m_pImageListLobby)
    {
        delete m_pImageListLobby;
        m_pImageListLobby = nullptr;
    }

    // And like a phoenix, rise from the ashes
    m_pImageListLobby = new ImageList(true);
    InitLobbyPanelSections();
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
        RemoveItem(FindItemIDForLobbyMember(pParam->m_ulSteamIDUserChanged));
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

    AddItem(m_iSectionId, pNewUser);
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
        KeyValues *pData = GetItemData(itemID)->MakeCopy();
        if (pData)
        {
            const char *pMap = SteamMatchmaking()->GetLobbyMemberData(m_idLobby, memberID, LOBBY_DATA_MAP);
            pData->SetString("map", pMap);
            // MOM_TODO: Spectating? Typing? 

            ModifyItem(itemID, m_iSectionId, pData);
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
    for (int i = 0; i <= GetHighestItemID(); i++)
    {
        if (IsItemIDValid(i))
        {
            KeyValues *kv = GetItemData(i);
            if (kv && (kv->GetUint64("steamid") == steamID))
            {
                return i;
            }
        }
    }
    return -1;
}


void LobbyMembersPanel::OnLobbyCreated(LobbyCreated_t* pParam)
{
    // Flip the 0 to 1 to test the panel with a local name
#if 1
    KeyValues *pNewUser = new KeyValues("LobbyMember");

    uint64 steamID = SteamUser()->GetSteamID().ConvertToUint64();

    pNewUser->SetUint64("steamid", steamID);
    pNewUser->SetInt("avatar", TryAddAvatar(steamID, &m_mapLobbyIDToImageListIndx, m_pImageListLobby));
    pNewUser->SetString("personaname", SteamFriends()->GetPersonaName());
    pNewUser->SetString("map", "triggertests");

    AddItem(m_iSectionId, pNewUser);
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
    // Loop through the lobby and add people
    m_idLobby = CSteamID(pParam->m_ulSteamIDLobby);

    // Add everyone now
    PopulateLobbyPanel();
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
    AddSection(m_iSectionId, "", StaticLobbyMemberSortFunc);
    SetSectionAlwaysVisible(m_iSectionId);
    SetImageList(m_pImageListLobby, false);
    AddColumnToSection(m_iSectionId, "avatar", "", COLUMN_IMAGE | COLUMN_CENTER, 45);
    AddColumnToSection(m_iSectionId, "personaname", "#MOM_Name", 0, 160);
    AddColumnToSection(m_iSectionId, "map", "#MOM_MapSelector_Map", 0, 160);

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