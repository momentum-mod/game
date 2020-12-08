#include "cbase.h"

#include "DrawerPanel_Lobby.h"

#include "LobbyInfoPanel.h"
#include "LobbyMembersPanel.h"
#include "LobbySearchPanel.h"

#include "MessageboxPanel.h"
#include "chat/ChatPanel.h"

#include "vgui_controls/ListPanel.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Tooltip.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DrawerPanel_Lobby::DrawerPanel_Lobby(Panel* pParent) : BaseClass(pParent, "DrawerPanel_Lobby")
{
    SetProportional(true);

    m_bInLobby = false;

    m_pPublicLobbies = nullptr;
    m_pLobbyMembers = nullptr;
    m_pLobbyChat = nullptr;
    m_pLobbyInfo = nullptr;
}

void DrawerPanel_Lobby::OnLobbyEnter(LobbyEnter_t *pParam)
{
    /*
    if (pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess)
    {
        m_bInLobby = true;
        m_pLobbyInfo->OnLobbyEnter(pParam);
        m_pLobbyMembers->OnLobbyEnter(pParam);
        m_pPublicLobbiesSearch->OnLobbyEnter(pParam);
        m_pFriendsLobbiesSearch->OnLobbyEnter(pParam);

        // s_LobbyID = CSteamID(pParam->m_ulSteamIDLobby);

        OnLobbyStateChange();
    }
    else
    {
        // NOTE: The unlocalized ones are ones we aren't sure if they're ever used
        static const char *const szJoinFails[] = {
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
     */
}

void DrawerPanel_Lobby::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    //m_pLobbyMembers->OnLobbyChatUpdate(pParam);
}

void DrawerPanel_Lobby::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    /*
    m_pLobbyInfo->OnLobbyDataUpdate(pParam);
    m_pLobbyMembers->OnLobbyDataUpdate(pParam);
    m_pPublicLobbiesSearch->OnLobbyDataUpdate(pParam);
    m_pFriendsLobbiesSearch->OnLobbyDataUpdate(pParam);
    */
}

void DrawerPanel_Lobby::OnLobbyLeave()
{
    /*
    m_bInLobby = false;

    m_pLobbyInfo->OnLobbyLeave();
    m_pLobbyMembers->OnLobbyLeave();
    m_pPublicLobbiesSearch->OnLobbyLeave();
    m_pFriendsLobbiesSearch->OnLobbyLeave();

    OnLobbyStateChange();
     */
}

void DrawerPanel_Lobby::OnResetData()
{
    /*m_pLobbyInfo = new LobbyInfoPanel(this);
    m_pLobbyMembers = new LobbyMembersPanel(this);
    m_pLobbyChat = new ChatContainer(this);
    m_pLobbyChat->SetAutomaticMessageMode(MESSAGE_MODE_MENU);
    m_pPublicLobbies = new PropertySheet(this, "PublicLobbiesSheet");

    m_pPublicLobbiesSearch = new LobbySearchPanel(this, false);
    m_pFriendsLobbiesSearch = new LobbySearchPanel(this, true);

    m_pPublicLobbies->AddPage(m_pPublicLobbiesSearch, "#MOM_Drawer_Lobby_Searching_Public");
    m_pPublicLobbies->AddPage(m_pFriendsLobbiesSearch, "#MOM_Drawer_Lobby_Searching_Friends");

    LoadControlSettings("resource/ui/mainmenu/DrawerPanel_Lobby.res");*/
    GetTooltip()->SetText("Lobbies are unavailable in this build!");

    OnLobbyStateChange();
}

void DrawerPanel_Lobby::OnLobbyStateChange()
{
    /*
    if (m_bInLobby)
    {
        m_pLobbyMembers->SetVisible(true);
        m_pLobbyChat->SetVisible(true);

        m_pPublicLobbies->SetVisible(false);

        m_pPublicLobbiesSearch->CancelSearch();
        m_pFriendsLobbiesSearch->CancelSearch();
    }
    else
    {
        m_pPublicLobbies->SetVisible(true);

        m_pFriendsLobbiesSearch->SearchForLobbies();
        m_pPublicLobbiesSearch->SearchForLobbies();

        m_pLobbyMembers->SetVisible(false);
        m_pLobbyChat->SetVisible(false);
    }
     */
}

void DrawerPanel_Lobby::OnReloadControls()
{
    const auto bVisible = IsVisible();

    BaseClass::OnReloadControls();

    //if (m_bInLobby && bVisible)
    //    m_pLobbyChat->SetVisible(true);
}

void DrawerPanel_Lobby::OnPersonaStateChange(PersonaStateChange_t *pParams)
{
    /*
    if (pParams->m_nChangeFlags & k_EPersonaChangeName)
    {
        m_pLobbyInfo->UpdateLobbyName();
        m_pLobbyMembers->UpdateLobbyMemberName(pParams->m_ulSteamID);
    }
     */
}
