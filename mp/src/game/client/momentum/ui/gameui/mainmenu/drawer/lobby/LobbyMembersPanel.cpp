#include "cbase.h"

#include "LobbyMembersPanel.h"
#include "SavelocRequestFrame.h"
#include "leaderboards/ClientTimesDisplay.h"

#include <vgui_controls/ImageList.h>
#include "vgui_controls/ListPanel.h"
#include "controls/FileImage.h"
#include "controls/ContextMenu.h"

#include "mom_shareddefs.h"
#include "vgui_avatarimage.h"

#include "mom_modulecomms.h"
#include "mom_ghostdefs.h"

#include "fmtstr.h"
#include "ilocalize.h"
#include "vgui/ILocalize.h"

#include "steam/steam_api.h"

#include "tier0/memdbgon.h"

using namespace vgui;

enum LobbyMembersPanelColumn_t
{
    COLUMN_AVATAR_IMG,
    COLUMN_NAME,
    COLUMN_MAP,
    COLUMN_STATE
};

static CSteamID s_LobbyID = k_steamIDNil;

LobbyMembersPanel::LobbyMembersPanel(Panel *pParent) : BaseClass(pParent, "LobbyMembers")
{
    SetProportional(true);
    SetKeyBoardInputEnabled(true);
    SetMouseInputEnabled(true);

    // set the scheme before any child control is created
    SetScheme("ClientScheme");

    AddActionSignalTarget(this);

    m_pImageListLobby = new ImageList(true);
    SetDefLessFunc(m_mapLobbyIDToImageListIndx);

    m_pSavelocReqFrame = new SavelocReqFrame;
    
    m_pMemberList = new ListPanel(this, "MemberList");
    m_pMemberList->SetRowHeightOnFontChange(false);
    m_pMemberList->SetRowHeight(GetScaledVal(20));
    m_pMemberList->SetMultiselectEnabled(false);
    m_pMemberList->SetAutoTallHeaderToFont(true);

    LoadControlSettings("resource/ui/mainmenu/LobbyMembersPanel.res");

    m_pMemberList->SetKeyBoardInputEnabled(true);
    m_pContextMenu = new ContextMenu(m_pMemberList);
    m_pContextMenu->SetAutoDelete(false);
    m_pContextMenu->AddActionSignalTarget(this);
    m_pContextMenu->SetVisible(false);

    InitLobbyPanelSections();
    m_pMemberList->ResetScrollBar();

    m_pLobbyOwnerImage = new FileImage("materials/vgui/icon/lobby_panel/lobby_owner.png");
}

LobbyMembersPanel::~LobbyMembersPanel()
{
    delete m_pLobbyOwnerImage;

    if (m_pContextMenu)
        m_pContextMenu->DeletePanel();

    if (m_pSavelocReqFrame)
        m_pSavelocReqFrame->DeletePanel();
}

void LobbyMembersPanel::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
    {
        // Add this user to the panel
        AddLobbyMember(CSteamID(pParam->m_ulSteamIDUserChanged));
        m_pMemberList->InvalidateLayout(true);
    }
    else if (pParam->m_rgfChatMemberStateChange & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
    {
        // Get em outta here
        const auto itemID = FindItemIDForLobbyMember(pParam->m_ulSteamIDUserChanged);
        if (itemID > -1)
        {
            m_pMemberList->RemoveItem(itemID);
            m_pMemberList->SortList();
        }
    }

    UpdateLobbyMemberCount();
}

void LobbyMembersPanel::AddLobbyMember(const CSteamID& steamID)
{
    if (FindItemIDForLobbyMember(steamID) > -1)
        return;

    KeyValuesAD pNewUser("LobbyMember");
    const auto steamIdInt = steamID.ConvertToUint64();
    pNewUser->SetUint64("steamid", steamIdInt);
    pNewUser->SetInt("avatar", TryAddAvatar(steamIdInt));
    pNewUser->SetString("personaname", SteamFriends()->GetFriendPersonaName(steamID));

    m_pMemberList->AddItem(pNewUser, 0, false, true);
}

void LobbyMembersPanel::UpdateLobbyMemberData(const CSteamID& memberID)
{
    CHECK_STEAM_API(SteamMatchmaking());

    if (!s_LobbyID.IsValid())
        return;

    const auto itemID = FindItemIDForLobbyMember(memberID);
    if (itemID < 0)
        return;

    const auto pData = m_pMemberList->GetItem(itemID);
    if (!pData)
        return;

    const auto bOwner = memberID == SteamMatchmaking()->GetLobbyOwner(s_LobbyID);
    pData->SetBool("isOwner", bOwner);

    const char *pMap = SteamMatchmaking()->GetLobbyMemberData(s_LobbyID, memberID, LOBBY_DATA_MAP);
    if (!pMap)
        pMap = "";
    const auto bMainMenu = Q_strlen(pMap) == 0;
    auto bBackground = false;
    if (!bMainMenu)
    {
        bBackground = !Q_strnicmp(pMap, "bg_", 3);
    }

    pData->SetString("map", bMainMenu ? "Main Menu" : pMap);

    const auto pSpec = SteamMatchmaking()->GetLobbyMemberData(s_LobbyID, memberID, LOBBY_DATA_IS_SPEC);
    if (pSpec && pSpec[0])
    {
        pData->SetString("state", "#MOM_Lobby_Member_Spectating");
    }
    else if (!(bMainMenu || bBackground))
    {
        pData->SetString("state", "#MOM_ReplayStatusPlaying");
    }
    else
    {
        pData->SetString("state", "");
    }

    m_pMemberList->ApplyItemChanges(itemID);
}

void LobbyMembersPanel::UpdateLobbyMemberName(const CSteamID &memberID)
{
    const auto pData = m_pMemberList->GetItem(FindItemIDForLobbyMember(memberID));
    if (!pData)
        return;

    pData->SetString("personaname", SteamFriends()->GetFriendPersonaName(memberID));
}

int LobbyMembersPanel::StaticLobbyMemberSortFunc(ListPanel* list, const ListPanelItem &item1, const ListPanelItem &item2)
{
    KeyValues *it1 = item1.kv;
    KeyValues *it2 = item2.kv;
    const char *pMapName = g_pGameRules->MapName();

    if (!pMapName)
        pMapName = "";

    Assert(it1 && it2);

    if (it1->GetBool("isOwner"))
        return -1;
    if (it2->GetBool("isOwner"))
        return 1;

    const auto map1 = it1->GetString("map");
    const auto map2 = it2->GetString("map");
    const auto name1 = it1->GetString("personaname");
    const auto name2 = it2->GetString("personaname");

    bool is1OnMap = FStrEq(pMapName, map1);
    bool is2OnMap = FStrEq(pMapName, map2);

    if (is1OnMap)
    {
        if (is2OnMap)
        {
            // If both are on the same map, go by name. We're rooting for it1 to be in front here, so
            // if strcmp returns negative, it1 is before it2. Hopefully they aren't the same string!
            return Q_strcmp(name1, name2);
        }
        // else it1 is on our map, they go first
        return -1;
    }
    //else 
    if (is2OnMap)
        return 1; // it2 goes first, since they're on our map

    // If they're both not on our map, check some more things...
    // If they're on the same map, go by name
    if (FStrEq(map1, map2))
        return Q_strcmp(name1, name2);

    // Otherwise go by the map names
    return Q_strcmp(map1, map2);
}

int LobbyMembersPanel::FindItemIDForLobbyMember(const uint64 steamID)
{
    for (int row = 0; row <= m_pMemberList->GetItemCount(); row++)
    {
        const auto itemID = m_pMemberList->GetItemIDFromRow(row);
        if (itemID > -1)
        {
            const auto pKv = m_pMemberList->GetItem(itemID);
            if (pKv && pKv->GetUint64("steamid") == steamID)
            {
                return itemID;
            }
        }
    }
    return -1;
}

int LobbyMembersPanel::FindItemIDForLobbyMember(const CSteamID& id)
{
    return FindItemIDForLobbyMember(id.ConvertToUint64());
}

void LobbyMembersPanel::OnReloadControls()
{
    BaseClass::OnReloadControls();
    
    if (s_LobbyID.IsValid())
        LobbyEnterSuccess();
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
    engine->ClientCmd_Unrestricted(CFmtStr("gameui_activate;map %s", map));
}

void LobbyMembersPanel::OnContextReqSavelocs(uint64 target)
{
    KeyValues *pReq = new KeyValues("req_savelocs");
    // Stage 1 is request the count, make the other player make a copy of their savelocs for us
    pReq->SetInt("stage", SAVELOC_REQ_STAGE_COUNT_REQ);
    pReq->SetUint64("target", target);
    g_pModuleComms->FireEvent(pReq);

    m_pSavelocReqFrame->Activate(target);
}

void LobbyMembersPanel::OnContextMakeOwner(uint64 target)
{
    if (target && s_LobbyID.IsValid() && s_LobbyID.IsLobby())
    {
        if (SteamMatchmaking()->SetLobbyOwner(s_LobbyID, CSteamID(target)))
        {
            Msg("Relinquished lobby ownership to %s!\n", SteamFriends()->GetFriendPersonaName(CSteamID(target)));
        }
        else
        {
            Warning("Failed to set lobby owner! Are you the lobby owner anymore?\n");
        }
    }
}

void LobbyMembersPanel::OnContextTeleport(uint64 target)
{
    if (target && s_LobbyID.IsValid() && s_LobbyID.IsLobby())
    {
        engine->ExecuteClientCmd(CFmtStr("mom_lobby_teleport %lld\n", target));
    }
}

void LobbyMembersPanel::LobbyEnterSuccess()
{
    // Add everyone now
    PopulateLobbyPanel();
    m_pMemberList->SortList();

    UpdateLobbyMemberCount();

    InvalidateLayout(true);
}

void LobbyMembersPanel::OnContextVisitProfile(uint64 profile)
{
    if (profile != 0 && SteamFriends())
    {
        SteamFriends()->ActivateGameOverlayToUser("steamid", CSteamID(profile));
    }
}

void LobbyMembersPanel::OnSpectateLobbyMember(uint64 target)
{
    engine->ClientCmd_Unrestricted(CFmtStr("mom_spectate %llu\n", target));
}

void LobbyMembersPanel::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    if (pParam->m_ulSteamIDLobby != s_LobbyID.ConvertToUint64())
        return;

    if (pParam->m_ulSteamIDMember == pParam->m_ulSteamIDLobby)
    {
        // Update owner(s) if they changed
        PopulateLobbyPanel();

        UpdateLobbyMemberCount();
    }
    else
    {
        // A member in the lobby changed
        UpdateLobbyMemberData(CSteamID(pParam->m_ulSteamIDMember));
    }

    m_pMemberList->SortList();
}

void LobbyMembersPanel::OnLobbyEnter(LobbyEnter_t* pParam)
{
    s_LobbyID = CSteamID(pParam->m_ulSteamIDLobby);

    LobbyEnterSuccess();
}

void LobbyMembersPanel::OnLobbyLeave()
{
    // Clear out the index map and the image list when you leave the lobby
    m_pMemberList->RemoveAll();

    // And like a phoenix, rise from the ashes
    InitImageList();

    s_LobbyID.Clear();
}

void LobbyMembersPanel::PopulateLobbyPanel()
{
    if (!s_LobbyID.IsValid())
        return;

    const int numLobbyMembers = SteamMatchmaking()->GetNumLobbyMembers(s_LobbyID);

    for (int i = 0; i < numLobbyMembers; i++)
    {
        const CSteamID inLobby = SteamMatchmaking()->GetLobbyMemberByIndex(s_LobbyID, i);

        // Get their initial data (name, avatar, steamID)
        AddLobbyMember(inLobby);
        // Get their status data (map, spectating, etc)
        UpdateLobbyMemberData(inLobby);
    }
}

void LobbyMembersPanel::InitImageList()
{
    m_mapLobbyIDToImageListIndx.RemoveAll();
    if (m_pImageListLobby)
    {
        delete m_pImageListLobby;
        m_pImageListLobby = nullptr;
    }

    m_pImageListLobby = new ImageList(true);

    m_pMemberList->SetImageList(m_pImageListLobby, false);
}

void LobbyMembersPanel::InitLobbyPanelSections()
{
    InitImageList();

    m_pMemberList->AddColumnHeader(COLUMN_AVATAR_IMG, "avatar", "", GetScaledVal(30), 
                                   ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE_SIZETOFIT | ListPanel::COLUMN_IMAGE_SIZE_MAINTAIN_ASPECT_RATIO | ListPanel::COLUMN_DISABLED);
    m_pMemberList->AddColumnHeader(COLUMN_NAME, "personaname", "#MOM_Name", GetScaledVal(90), GetScaledVal(30), GetScaledVal(100), 0);
    m_pMemberList->AddColumnHeader(COLUMN_MAP, "map", "#MOM_Map", GetScaledVal(120), 0);
    m_pMemberList->AddColumnHeader(COLUMN_STATE, "state", "#MOM_Lobby_Member_State", GetScaledVal(30));

    m_pMemberList->SetSortFunc(COLUMN_NAME, StaticLobbyMemberSortFunc);
    m_pMemberList->SetSortColumn(COLUMN_NAME);

    m_pMemberList->SetColumnTextAlignment(COLUMN_AVATAR_IMG, Label::a_center);
}

void LobbyMembersPanel::UpdateLobbyMemberCount() const
{
    if (!s_LobbyID.IsValid())
        return;

    CHECK_STEAM_API(SteamMatchmaking());

    const auto current = SteamMatchmaking()->GetNumLobbyMembers(s_LobbyID);
    const auto max = SteamMatchmaking()->GetLobbyMemberLimit(s_LobbyID);

    KeyValuesAD statusKV("statusKV");
    statusKV->SetString("status", CFmtStr("(%i / %i)", current, max));

    m_pMemberList->SetColumnHeaderText(COLUMN_NAME, CConstructLocalizedString(g_pVGuiLocalize->FindSafe("#MOM_Drawer_Lobby_Members"), static_cast<KeyValues*>(statusKV)));
}

int LobbyMembersPanel::TryAddAvatar(const uint64& steamID)
{
    // Update their avatar
    if (m_pImageListLobby)
    {
        // See if we already have that avatar in our list
        const unsigned short mapIndex = m_mapLobbyIDToImageListIndx.Find(steamID);
        int iImageIndex;
        if (!m_mapLobbyIDToImageListIndx.IsValidIndex(mapIndex))
        {
            CAvatarImage *pImage = new CAvatarImage();
            // 64 is enough up to full HD resolutions.
            pImage->SetAvatarSteamID(CSteamID(steamID), k_EAvatarSize64x64);

            pImage->SetDrawFriend(false);
            pImage->SetAvatarSize(32, 32);
            iImageIndex = m_pImageListLobby->AddImage(pImage);
            m_mapLobbyIDToImageListIndx.Insert(steamID, iImageIndex);
        }
        else
        {
            iImageIndex = m_mapLobbyIDToImageListIndx.Element(mapIndex);
        }
        return iImageIndex;
    }
    return -1;
}

void LobbyMembersPanel::OnItemContextMenu(int itemID)
{
    CHECK_STEAM_API(SteamUser());

    const auto pKVData = m_pMemberList->GetItem(itemID);
    if (!pKVData)
        return;

    const auto steamID = pKVData->GetUint64("steamid");
    const auto locID = SteamUser()->GetSteamID();

    // Don't show it for ourselves
    if (steamID == locID.ConvertToUint64())
        return;

    const auto ownerID = SteamMatchmaking()->GetLobbyOwner(s_LobbyID);

    const char *pMap = pKVData->GetString("map");
    const char *pOurMap = g_pGameRules->MapName();
    const auto bMainMenu = FStrEq(pMap, "Main Menu");
    auto bBackground = false;
    if (!bMainMenu)
    {
        bBackground = !Q_strnicmp(pMap, "bg_", 3);
    }

    const auto pSpec = SteamMatchmaking()->GetLobbyMemberData(s_LobbyID, CSteamID(steamID), LOBBY_DATA_IS_SPEC);
    const auto bSpectating = pSpec && pSpec[0] != '\0';

    KeyValues *pKv;

    m_pContextMenu->OnKillFocus();
    m_pContextMenu->DeleteAllItems();

    if (pOurMap && FStrEq(pMap, pOurMap))
    {
        if (!bSpectating)
        {
            pKv = new KeyValues("ContextSpectate");
            pKv->SetUint64("target", steamID);
            m_pContextMenu->AddMenuItem("SpectateLobbyMember", "#MOM_Lobby_Spectate", pKv, this);

            pKv = new KeyValues("ContextTeleport");
            pKv->SetUint64("target", steamID);
            m_pContextMenu->AddMenuItem("TeleportToLobbyMember", "#MOM_Lobby_TeleportTo", pKv, this);
        }

        pKv = new KeyValues("ContextReqSavelocs");
        pKv->SetUint64("target", steamID);
        m_pContextMenu->AddMenuItem("ReqSavelocs", "#MOM_Saveloc_Frame", pKv, this);
    }
    else if (!(bMainMenu || bBackground))
    {
        pKv = new KeyValues("ContextGoToMap");
        pKv->SetString("map", pMap);
        m_pContextMenu->AddMenuItem("GoToMap", "#MOM_Lobby_GoToMap", pKv, this);
    }

    if (ownerID == locID)
    {
        m_pContextMenu->AddSeparator();
        pKv = new KeyValues("ContextMakeOwner");
        pKv->SetUint64("target", steamID);
        m_pContextMenu->AddMenuItem("MakeLobbyOwner", "#MOM_Lobby_MakeOwner", pKv, this);
    }

    // MOM_TODO: More options here, such as:
    // hiding decals (maybe toggle paint, bullets separately?)
    // etc
    m_pContextMenu->AddSeparator();
    // Visit profile
    pKv = new KeyValues("ContextVisitProfile");
    pKv->SetUint64("profile", steamID);
    m_pContextMenu->AddMenuItem("VisitProfile", "#MOM_Leaderboards_SteamProfile", pKv, this);

    m_pContextMenu->ShowMenu();
}