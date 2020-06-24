#include "cbase.h"

#include "LobbyMembersPanel.h"
#include "SavelocRequestFrame.h"
#include "leaderboards/ClientTimesDisplay.h"
#include "leaderboards/LeaderboardsContextMenu.h"

#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include <vgui_controls/ImageList.h>
#include "vgui_controls/ListPanel.h"
#include <vgui_controls/Button.h>
#include "controls/FileImage.h"
#include "vgui_controls/Tooltip.h"

#include "MessageboxPanel.h"

#include "mom_shareddefs.h"
#include "vgui_avatarimage.h"

#include "mom_modulecomms.h"
#include "mom_ghostdefs.h"

#include "fmtstr.h"
#include "ilocalize.h"

#include "steam/steam_api.h"

#include "tier0/memdbgon.h"

extern bool g_bRollingCredits;

using namespace vgui;

static CSteamID s_LobbyID = k_steamIDNil;

LobbyMembersPanel::LobbyMembersPanel(IViewPort *pParent) : BaseClass(nullptr, PANEL_LOBBY_MEMBERS)
{
    m_pViewport = pParent;
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

    m_pLobbyMemberCount = new Label(this, "LobbyMemberCount", "");
    m_pLobbyType = new ImagePanel(this, "LobbyType");
    m_pLobbyType->InstallMouseHandler(this);
    m_pLobbyType->SetVisible(false);
    m_pLobbyType->SetShouldScaleImage(true);
    
    m_pMemberList = new ListPanel(this, "MemberList");
    m_pMemberList->SetRowHeightOnFontChange(false);
    m_pMemberList->SetRowHeight(GetScaledVal(20));
    m_pMemberList->SetMultiselectEnabled(false);
    m_pMemberList->SetAutoTallHeaderToFont(true);
    m_pLobbyToggle = new Button(this, "LobbyToggle", "#GameUI2_HostLobby", this, "HostLobby");
    m_pInviteFriends = new Button(this, "InviteFriends", "#GameUI2_InviteLobby", this, "InviteFriends");

    LoadControlSettings("resource/ui/LobbyMembersPanel.res");

    m_pMemberList->SetKeyBoardInputEnabled(true);
    m_pContextMenu = new CLeaderboardsContextMenu(m_pMemberList);
    m_pContextMenu->SetAutoDelete(false);
    m_pContextMenu->AddActionSignalTarget(this);
    m_pContextMenu->SetVisible(false);

    InitLobbyPanelSections();
    m_pMemberList->ResetScrollBar();

    m_pLobbyTypePublic = new FileImage("materials/vgui/icon/lobby_panel/lobby_public.png");
    m_pLobbyTypeFriends = new FileImage("materials/vgui/icon/lobby_panel/lobby_friends.png");
    m_pLobbyTypePrivate = new FileImage("materials/vgui/icon/lobby_panel/lobby_private.png");

    ListenForGameEvent("lobby_leave");
}

// NOTE: This gets called when the user changes resolution!
LobbyMembersPanel::~LobbyMembersPanel()
{
    if (m_pContextMenu)
        m_pContextMenu->DeletePanel();

    if (m_pSavelocReqFrame)
        m_pSavelocReqFrame->DeletePanel();

    delete m_pLobbyTypePublic;
    delete m_pLobbyTypeFriends;
    delete m_pLobbyTypePrivate;
}

void LobbyMembersPanel::FireGameEvent(IGameEvent* event)
{
    // Clear out the index map and the image list when you leave the lobby
    m_pMemberList->RemoveAll();

    // And like a phoenix, rise from the ashes
    InitImageList();
    m_pMemberList->SetImageList(m_pImageListLobby, false);

    m_pLobbyToggle->SetText("#GameUI2_HostLobby");
    m_pLobbyToggle->SetCommand("HostLobby");
    m_pInviteFriends->SetVisible(false);
    m_pLobbyMemberCount->SetVisible(false);
    m_pLobbyType->SetVisible(false);

    s_LobbyID.Clear();
}

void LobbyMembersPanel::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    if (pParam->m_rgfChatMemberStateChange & k_EChatMemberStateChangeEntered)
    {
        // Add this user to the panel
        AddLobbyMember(CSteamID(pParam->m_ulSteamIDUserChanged));
        UpdateLobbyMemberCount();
        m_pMemberList->InvalidateLayout(true);
    }
    else if (pParam->m_rgfChatMemberStateChange & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
    {
        // Get em outta here
        const auto itemID = FindItemIDForLobbyMember(pParam->m_ulSteamIDUserChanged);
        if (itemID > -1)
        {
            m_pMemberList->RemoveItem(itemID);
            UpdateLobbyMemberCount();
            m_pMemberList->SortList();
        }
    }
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
    if (!s_LobbyID.IsValid())
        return;

    const auto itemID = FindItemIDForLobbyMember(memberID);
    if (itemID > -1)
    {
        KeyValues *pData = m_pMemberList->GetItem(itemID);
        if (pData)
        {
            const auto owner = SteamMatchmaking()->GetLobbyOwner(s_LobbyID);
            pData->SetBool("isOwner", memberID == owner);

            const char *pMap = SteamMatchmaking()->GetLobbyMemberData(s_LobbyID, memberID, LOBBY_DATA_MAP);
            if (!pMap)
                pMap = "";
            const auto bMainMenu = Q_strlen(pMap) == 0;
            auto bCredits = false;
            auto bBackground = false;
            if (!bMainMenu)
            {
                bBackground = !Q_strnicmp(pMap, "bg_", 3);
                if (!bBackground)
                    bCredits = FStrEq(pMap, "credits");
            }

            pData->SetString("map", bMainMenu ? "Main Menu" : pMap);

            const auto pSpec = SteamMatchmaking()->GetLobbyMemberData(s_LobbyID, memberID, LOBBY_DATA_IS_SPEC);
            if (pSpec && pSpec[0])
                pData->SetString("state", "#MOM_Lobby_Member_Spectating");
            else if (!(bMainMenu || bCredits || bBackground))
                pData->SetString("state", "#MOM_ReplayStatusPlaying");
            else
                pData->SetString("state", "");

            m_pMemberList->ApplyItemChanges(itemID);
        }
    }
}

int LobbyMembersPanel::StaticLobbyMemberSortFunc(ListPanel* list, const ListPanelItem &item1, const ListPanelItem &item2)
{
    KeyValues *it1 = item1.kv;
    KeyValues *it2 = item2.kv;
    const char *pMapName = g_pGameRules->MapName();

    if (!pMapName)
        return 0;

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

    // If they're both not on the our map, check some more things...
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

void LobbyMembersPanel::OnMousePressed(MouseCode code)
{
    if (code == MOUSE_LEFT && input()->GetMouseOver() == m_pLobbyType->GetVPanel())
    {
        SetLobbyType(); // Cycle and update the lobby type
    }
}

const char* LobbyMembersPanel::GetName()
{
    return PANEL_LOBBY_MEMBERS;
}

void LobbyMembersPanel::Reset()
{
    m_pMemberList->ClearSelectedItems();
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
        if (g_bRollingCredits)
            return;

        Reset();
        SetVisible(true);
        // SetEnabled(true);
        MoveToFront();

        const auto pSpecUI = m_pViewport->FindPanelByName(PANEL_SPECGUI);
        if (pSpecUI && pSpecUI->IsVisible() && ipanel()->IsMouseInputEnabled(pSpecUI->GetVPanel()))
            SetMouseInputEnabled(true);
    }
    else
    {
        SetVisible(false);
        SetMouseInputEnabled(false);
    }
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
    ShowPanel(false);
    engine->ClientCmd_Unrestricted(CFmtStr("gameui_activate;map %s", map));
}

void LobbyMembersPanel::OnContextReqSavelocs(uint64 target)
{
    ShowPanel(false);
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
    m_pLobbyToggle->SetText("#GameUI2_LeaveLobby");
    m_pLobbyToggle->SetCommand("LeaveLobby");
    m_pInviteFriends->SetVisible(true);
    m_pLobbyMemberCount->SetVisible(true);

    // Add everyone now
    PopulateLobbyPanel();
    UpdateLobbyMemberCount();
    m_pMemberList->SortList();
    SetLobbyTypeImage();

    InvalidateLayout(true);
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
    ShowPanel(false);
    engine->ClientCmd_Unrestricted(CFmtStr("mom_spectate %llu\n", target));
}

void LobbyMembersPanel::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    if (pParam->m_ulSteamIDMember == pParam->m_ulSteamIDLobby)
    {
        // The lobby itself changed
        UpdateLobbyMemberCount();
        // Update owner(s) if they changed
        PopulateLobbyPanel();
        // Check if the type changed
        SetLobbyTypeImage();
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
    if (pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
    {
        s_LobbyID = CSteamID(pParam->m_ulSteamIDLobby);

        LobbyEnterSuccess();
    }
    else
    {
         // NOTE: The unlocalized ones are ones we aren't sure if they're ever used
        static const char * const szJoinFails[] = {
            "#MOM_Lobby_JoinFail_DoesntExist", // k_EChatRoomEnterResponseDoesntExist ( = 2 )
            "#MOM_Lobby_JoinFail_NotAllowed", // k_EChatRoomEnterResponseNotAllowed
            "#MOM_Lobby_JoinFail_Full", // k_EChatRoomEnterResponseFull
            "#MOM_Lobby_JoinFail_Error", // k_EChatRoomEnterResponseError
            "You are banned from this chat room.", // k_EChatRoomEnterResponseBanned
            "#MOM_Lobby_JoinFail_Limited", // k_EChatRoomEnterResponseLimited
            "The clan is locked or disabled!", // k_EChatRoomEnterResponseClanDisabled
            "Your account has a community ban!", // k_EChatRoomEnterResponseCommunityBan
            "Some member of the chat has blocked you from joining.", // k_EChatRoomEnterResponseMemberBlockedYou
            "You have blocked some member already in the chat.", // k_EChatRoomEnterResponseYouBlockedMember
            "Unused1",
            "Unused2",
            "Unused3",
            "Too many join attempts in a very short period of time. Try waiting a bit before trying again." // k_EChatRoomEnterResponseRatelimitExceeded
        };

        g_pMessageBox->CreateMessagebox("#MOM_Lobby_JoinFail", szJoinFails[pParam->m_EChatRoomEnterResponse - 2]);
    }
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

    // Add the default crown image to index 1
    m_pImageListLobby->AddImage(new FileImage("materials/vgui/icon/lobby_panel/lobby_owner.png"));
}

void LobbyMembersPanel::InitLobbyPanelSections()
{
    InitImageList();

    m_pMemberList->SetImageList(m_pImageListLobby, false);
    m_pMemberList->AddColumnHeader(0, "isOwner", "", GetScaledVal(30), 
                                   ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_DISABLED);
    m_pMemberList->AddColumnHeader(1, "avatar", "", GetScaledVal(30), 
                                   ListPanel::COLUMN_IMAGE | ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_DISABLED);
    m_pMemberList->AddColumnHeader(2, "personaname", "#MOM_Name", GetScaledVal(90), GetScaledVal(30), GetScaledVal(100), 0);
    m_pMemberList->AddColumnHeader(3, "map", "#MOM_MapSelector_Map", GetScaledVal(120), 0);
    m_pMemberList->AddColumnHeader(4, "state", "#MOM_Lobby_Member_State", GetScaledVal(30));

    m_pMemberList->SetSortFunc(2, StaticLobbyMemberSortFunc);
    m_pMemberList->SetSortColumn(2);

    m_pMemberList->SetColumnTextAlignment(0, Label::a_center);
    m_pMemberList->SetColumnTextAlignment(1, Label::a_center);
}

void LobbyMembersPanel::UpdateLobbyMemberCount() const
{
    if (s_LobbyID.IsValid())
    {
        const auto current = SteamMatchmaking()->GetNumLobbyMembers(s_LobbyID);
        const auto max = SteamMatchmaking()->GetLobbyMemberLimit(s_LobbyID);
        m_pLobbyMemberCount->SetText(CConstructLocalizedString(L" (%s1/%s2)", current, max));
    }
}

void LobbyMembersPanel::SetLobbyTypeImage() const
{
    const auto pTypeStr = SteamMatchmaking()->GetLobbyData(s_LobbyID, LOBBY_DATA_TYPE);
    if (pTypeStr && Q_strlen(pTypeStr) == 1)
    {
        const auto nType = clamp<int>(Q_atoi(pTypeStr), k_ELobbyTypePrivate, k_ELobbyTypePublic);

        IImage *pImages[3] = 
        {
            m_pLobbyTypePrivate,
            m_pLobbyTypeFriends,
            m_pLobbyTypePublic
        };
        const char *pTTs[3] = {
            "#MOM_Lobby_Type_Private",
            "#MOM_Lobby_Type_FriendsOnly",
            "#MOM_Lobby_Type_Public"
        };

        m_pLobbyType->SetVisible(true);
        m_pLobbyType->SetImage(pImages[nType]);
        m_pLobbyType->GetTooltip()->SetEnabled(true);
        m_pLobbyType->GetTooltip()->SetText(pTTs[nType]);
    }
}

void LobbyMembersPanel::SetLobbyType() const
{
    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());
    // But only if they're the lobby owner
    const auto locID = SteamUser()->GetSteamID();
    if (s_LobbyID.IsValid() && locID == SteamMatchmaking()->GetLobbyOwner(s_LobbyID))
    {
        const auto pTypeStr = SteamMatchmaking()->GetLobbyData(s_LobbyID, LOBBY_DATA_TYPE);
        if (pTypeStr && Q_strlen(pTypeStr) == 1)
        {
            const auto nType = clamp<int>(Q_atoi(pTypeStr), k_ELobbyTypePrivate, k_ELobbyTypePublic);
            const auto newType = (nType + 1) % (k_ELobbyTypePublic + 1);
            engine->ClientCmd_Unrestricted(CFmtStr("mom_lobby_type %i", newType));
        }
    }
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
    auto bCredits = false;
    auto bBackground = false;
    if (!bMainMenu)
    {
        bBackground = !Q_strnicmp(pMap, "bg_", 3);
        if (!bBackground)
            bCredits = FStrEq(pMap, "credits");
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
    else if (!(bMainMenu || bCredits || bBackground))
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
