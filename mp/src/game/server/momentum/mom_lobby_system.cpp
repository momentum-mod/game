#include "cbase.h"
#include "mom_lobby_system.h"

#include "tier0/memdbgon.h"


CSteamID CMomentumLobbySystem::m_sLobbyID = k_steamIDNil;
CMomentumLobbySystem* CMomentumLobbySystem::m_pInstance = nullptr;

CON_COMMAND(mom_host_lobby, "Starts hosting a lobby\n")
{
    g_pMomentumLobbySystem->StartLobby();
}

CON_COMMAND(mom_leave_lobby, "Leave your current lobby\n")
{
    g_pMomentumLobbySystem->LeaveLobby();
}

CON_COMMAND(connect_lobby, "Connect to a given SteamID's lobby\n")
{
    g_pMomentumLobbySystem->JoinLobbyFromString(args.Arg(1));
}
// So basically, if a user wants to connect to us, we're considered the host. 
void CMomentumLobbySystem::HandleNewP2PRequest(P2PSessionRequest_t* info)
{
    // MOM_TODO: Store their CSteamID somewhere
    // Are we not connected to another person?
    if (!m_sHostID.IsValid())
    {
        // Then we're the host
        m_sHostID = steamapicontext->SteamUser()->GetSteamID();
    }
    else
    {
        // Somebody else is the host, forward that to them
    }
    // Needs to be done to open the connection with them
    steamapicontext->SteamNetworking()->AcceptP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumLobbySystem::HandleP2PConnectionFail(P2PSessionConnectFail_t* info)
{
    Warning("Couldn't do Steam P2P because of the error: %i\n", info->m_eP2PSessionError);

    // MOM_TODO: Make a block list that only refreshes on game restart? Helps bad connections from continuously looping
    steamapicontext->SteamNetworking()->CloseP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumLobbySystem::SendChatMessage(char* pMessage)
{
    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        int len = Q_strlen(pMessage) + 1;
        bool result = steamapicontext->SteamMatchmaking()->SendLobbyChatMsg(m_sLobbyID, pMessage, len);
        if (result)
            DevLog("Sent chat message! Message: %s\n", pMessage);
        else
            DevLog("Did not send lobby message!\n");
    }
    else // MOM_TODO: Check if connected to a server and send the chat packet
    {
        DevLog("Could not send message because you are not connected!\n");
    }
}

void CMomentumLobbySystem::GetLobbyMemberSteamData(CSteamID pMember)
{
    if (steamapicontext->SteamFriends()->RequestUserInformation(pMember, false))
    {
        // It's calling stuff about them, we gotta wait a bit
    }
    else
    {
        // We have the data about this person, call stuff immediately
        const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(pMember);
        DevLog("We were able to get their name immediately: %\n", pName);
    }
}

// Called when joining a friend from their Join Game option in steam
void CMomentumLobbySystem::HandleFriendJoin(GameRichPresenceJoinRequested_t* pJoin)
{
    // MOM_TODO: Have a global convar that auto blocks requests (busy vs online) 
    // m_sHostID = pJoin->m_steamIDFriend;
    //steamapicontext->SteamNetworking()->SendP2PPacket(pJoin->m_steamIDFriend, );


}

// Called when trying to join somebody else's lobby. We need to actually call JoinLobby here.
void CMomentumLobbySystem::HandleLobbyJoin(GameLobbyJoinRequested_t* pJoin)
{
    // Get the lobby owner
    //CSteamID owner = steamapicontext->SteamMatchmaking()->GetLobbyOwner(pJoin->m_steamIDLobby);

    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        Warning("You are already in a lobby! Do \"mom_leave_lobby\" to leave the lobby!\n");
    }
    else
    {
        SteamAPICall_t call = steamapicontext->SteamMatchmaking()->JoinLobby(pJoin->m_steamIDLobby);
        m_cLobbyJoined.Set(call, m_pInstance, &CMomentumLobbySystem::CallResult_LobbyJoined);
    }
}

// Called when we created the lobby
void CMomentumLobbySystem::CallResult_LobbyCreated(LobbyCreated_t* pCreated, bool ioFailure)
{
    if (ioFailure || !pCreated)
    {
        Warning("Could not create lobby due to IO error!\n");
        return;
    }

    DevLog("Lobby created call result! We got a result %i with a steam lobby: %u\n", pCreated->m_eResult, pCreated->m_ulSteamIDLobby);
    if (pCreated->m_eResult == k_EResultOK)
    {
        DevLog("Result is okay! We got a lobby bois!\n");
        m_sLobbyID = CSteamID(pCreated->m_ulSteamIDLobby);
        m_bHostingLobby = true;

        // Note: We set our info in the lobby join method
    }
}

void CMomentumLobbySystem::CallResult_LobbyJoined(LobbyEnter_t* pEntered, bool IOFailure)
{
    if (!pEntered || IOFailure)
    {
        Warning("Could not join lobby due to IO error!\n");
        return;
    }

    if (pEntered->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
    {
        DevWarning("Failed to enter chat room! Error code: %i\n", pEntered->m_EChatRoomEnterResponse);
    }
    else
    {
        Log("Successfully joined the lobby! %lld\n", pEntered->m_ulSteamIDLobby);
    }
}

void CMomentumLobbySystem::StartLobby()
{
    if (!(m_cLobbyCreated.IsActive() || m_sLobbyID.IsValid() || m_sLobbyID.IsLobby()))
    {
        SteamAPICall_t call = steamapicontext->SteamMatchmaking()->CreateLobby(k_ELobbyTypeFriendsOnly, 10);
        m_cLobbyCreated.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyCreated);
        DevLog("The lobby call successfully happened!\n");
    }
    else
        DevLog("The lobby could not be created because you already made one or are in one!\n");
}

void CMomentumLobbySystem::LeaveLobby()
{
    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        steamapicontext->SteamMatchmaking()->LeaveLobby(m_sLobbyID);
        DevLog("Left the lobby!\n");
        m_sLobbyID = k_steamIDNil;
        g_pMomentumGhostClient->ClearCurrentGhosts();
    }
    else
        DevLog("Could not leave lobby, are you in one?\n");
}

// Called when we enter a lobby
void CMomentumLobbySystem::HandleLobbyEnter(LobbyEnter_t* pEnter)
{
    if (pEnter->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
    {
        DevWarning("Failed to enter chat room! Error code: %i\n", pEnter->m_EChatRoomEnterResponse);
        return;
    }

    DevLog("Lobby entered! Lobby ID: %lld\n", pEnter->m_ulSteamIDLobby);

    if (!m_sLobbyID.IsValid())
    {
        m_sLobbyID = CSteamID(pEnter->m_ulSteamIDLobby);
    }

    // Set our own data
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, "map", gpGlobals->mapname.ToCStr());
    // MOM_TODO: Set our ghost appearance data

    // create a new thread to begin network IO
    struct MyThreadParams_t{}; //empty class so we can force the threaded function to work xd
    MyThreadParams_t vars; //bogus params containing NOTHING hahAHAHAhaHHa
    ConDColorMsg(Color(255, 0, 255, 255), "Running thread!\n");
    ThreadHandle_t netIOThread = CreateSimpleThread(CMomentumGhostClient::SendAndRecieveP2PPackets, &vars);
    ThreadDetach(netIOThread);

    // Get everybody else's data
    g_pMomentumGhostClient->CheckToAdd(nullptr);
}

// We got a message yaay
void CMomentumLobbySystem::HandleLobbyChatMsg(LobbyChatMsg_t* pParam)
{
    char *message = new char[4096];
    int written = steamapicontext->SteamMatchmaking()->GetLobbyChatEntry(CSteamID(pParam->m_ulSteamIDLobby), pParam->m_iChatID, nullptr, message, 4096, nullptr);
    DevLog("SERVER: Got a chat message! Wrote %i byte(s) into buffer.\n", written);
    Msg("SERVER: Chat message: %s\n", message);
    delete[] message;
}

void CMomentumLobbySystem::HandleLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    CSteamID lobbyId = CSteamID(pParam->m_ulSteamIDLobby);
    CSteamID memberChanged = CSteamID(pParam->m_ulSteamIDMember);
    if (pParam->m_bSuccess)
    {
        if (lobbyId.ConvertToUint64() == memberChanged.ConvertToUint64())
        {
            // The lobby itself changed
            // We could have a new owner
            // Or new member limit
            // Or new lobby type
        }
        else
        {
            // Don't care if it's us that changed
            if (memberChanged == steamapicontext->SteamUser()->GetSteamID())
                return;

            // An individual member changed
            g_pMomentumGhostClient->CheckToAdd(&memberChanged);
        }
    }
}

// Somebody left/joined, or the owner of the lobby was changed
void CMomentumLobbySystem::HandleLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    uint32 state = pParam->m_rgfChatMemberStateChange;
    CSteamID changedPerson = CSteamID(pParam->m_ulSteamIDUserChanged);
    if (state & k_EChatMemberStateChangeEntered)
    {
        // Somebody joined us! Huzzah!
        // GetLobbyMemberSteamData(changedPerson); MOM_TODO: Does this happen asynchronously?

        DevLog("A user just joined us!\n");
    }
    if (state & k_EChatMemberStateChangeLeft || state & k_EChatMemberStateChangeDisconnected)
    {
        DevLog("User left/disconnected!\n");
    }
}

void CMomentumLobbySystem::HandlePersonaCallback(PersonaStateChange_t* pParam)
{
    //DevLog("HandlePersonaCallback: %u with changeflags: %i\n", pParam->m_ulSteamID, pParam->m_nChangeFlags);
    CSteamID person = CSteamID(pParam->m_ulSteamID);
    if (pParam->m_nChangeFlags & k_EPersonaChangeName)
    {
        DevLog("Got the name of %lld: %s\n", pParam->m_ulSteamID, steamapicontext->SteamFriends()->GetFriendPersonaName(person));
    }
}

void CMomentumLobbySystem::JoinLobbyFromString(const char* pString)
{
    if (pString)
    {
        if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
        {
            Warning("You are already in a lobby! Do \"mom_leave_lobby\" to exit it!\n");
        }
        else if (m_cLobbyJoined.IsActive())
        {
            Warning("You are already trying to join a lobby!\n");
        }
        else
        {
            DevLog("Trying to join the lobby from the string %s!\n", pString);
            uint64 steamID = Q_atoui64(pString);
            if (steamID > 0)
            {
                CSteamID toJoin;
                toJoin.FullSet(steamID, k_EUniversePublic, k_EAccountTypeChat);
                DevLog("Got the ID! %lld\n", toJoin.ConvertToUint64());

                SteamAPICall_t call = steamapicontext->SteamMatchmaking()->JoinLobby(toJoin);
                m_cLobbyJoined.Set(call, m_pInstance, &CMomentumLobbySystem::CallResult_LobbyJoined);
            }
            else
            {
                Warning("Could not join lobby due to malformed ID!\n");
            }
        }
    }
}
static CMomentumLobbySystem s_MOMLobbySystem("CMomentumLobbySystem");
CMomentumLobbySystem *g_pMomentumLobbySystem = &s_MOMLobbySystem;