#include "cbase.h"
#include "mom_lobby_system.h"

#include "tier0/memdbgon.h"


CSteamID CMomentumLobbySystem::m_sLobbyID = k_steamIDNil;
CMomentumLobbySystem* CMomentumLobbySystem::m_pInstance = nullptr;
float CMomentumLobbySystem::m_flNextUpdateTime = -1.0f;

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

CON_COMMAND(mom_invite_lobby, "Invite friends to your lobby\n")
{
    steamapicontext->SteamFriends()->ActivateGameOverlayInviteDialog(g_pMomentumLobbySystem->GetLobbyId());
}

// So basically, if a user wants to connect to us, we're considered the host. 
void CMomentumLobbySystem::HandleNewP2PRequest(P2PSessionRequest_t* info)
{
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

    DevLog("Lobby created call result! We got a result %i with a steam lobby: %lld\n", pCreated->m_eResult, pCreated->m_ulSteamIDLobby);
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
        g_pMomentumGhostClient->ClearCurrentGhosts(true);
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
    /*struct MyThreadParams_t{}; //empty class so we can force the threaded function to work xd
    MyThreadParams_t vars; //bogus params containing NOTHING hahAHAHAhaHHa
    ConDColorMsg(Color(255, 0, 255, 255), "Running thread!\n");
    ThreadHandle_t netIOThread = CreateSimpleThread(SendAndRecieveP2PPackets, &vars);
    ThreadDetach(netIOThread);*/

    // Get everybody else's data
    CheckToAdd(nullptr);
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
            CheckToAdd(&memberChanged);
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
        // Note: The lobby data update method handles adding
    }
    if (state & k_EChatMemberStateChangeLeft || state & k_EChatMemberStateChangeDisconnected)
    {
        DevLog("User left/disconnected!\n");

        uint16 findMember = CMomentumGhostClient::m_mapOnlineGhosts.Find(changedPerson.ConvertToUint64());
        if (findMember != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
        {
            // Remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = CMomentumGhostClient::m_mapOnlineGhosts[findMember];
            if (pEntity)
                pEntity->Remove();

            CMomentumGhostClient::m_mapOnlineGhosts.RemoveAt(findMember);
        }
    }
}

void CMomentumLobbySystem::HandlePersonaCallback(PersonaStateChange_t* pParam)
{
    //DevLog("HandlePersonaCallback: %u with changeflags: %i\n", pParam->m_ulSteamID, pParam->m_nChangeFlags);
    CSteamID person = CSteamID(pParam->m_ulSteamID);
    if (pParam->m_nChangeFlags & k_EPersonaChangeName)
    {
        DevLog("Got the name of %lld: %s\n", person, steamapicontext->SteamFriends()->GetFriendPersonaName(person));
        for (int i = 0; i < steamapicontext->SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID); i++)
        {
            CSteamID member = steamapicontext->SteamMatchmaking()->GetLobbyMemberByIndex(m_sLobbyID, i);
            unsigned short findIndx = CMomentumGhostClient::m_mapOnlineGhosts.Find(member.ConvertToUint64()); //god damnit valve

            if (member == person) // set their name IFF they are a member of our lobby
            {
                CMomentumGhostClient::m_mapOnlineGhosts[findIndx]->SetGhostName(steamapicontext->SteamFriends()->GetFriendPersonaName(person));
            }
        }
    }
}

CSteamID CMomentumLobbySystem::GetLobbyId()
{
    return m_sLobbyID;
}

void CMomentumLobbySystem::LevelChange(const char* pMapName)
{
    if (LobbyValid())
    {
        DevLog("Setting the map to %s!\n", pMapName);
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, "map", pMapName);

        // Now check if this map is the same as somebody else's in the lobby
        if (pMapName)
            CheckToAdd(nullptr);
        else
        {
            g_pMomentumGhostClient->ClearCurrentGhosts(false);
        }
    }
}

void CMomentumLobbySystem::CheckToAdd(CSteamID *pID)
{
    CSteamID localID = steamapicontext->SteamUser()->GetSteamID();

    if (pID)
    {
        const char *pOtherMap = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, *pID, "map");
        unsigned short findIndx = CMomentumGhostClient::m_mapOnlineGhosts.Find(pID->ConvertToUint64());
        // Just joined this map, we haven't created them 
        if (FStrEq(gpGlobals->mapname.ToCStr(), pOtherMap))
        {
            // Don't add them again if they reloaded this map for some reason
            if (findIndx == CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
            {
                CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                newPlayer->SetGhostSteamID(*pID);
                newPlayer->Spawn();
                newPlayer->SetGhostName(steamapicontext->SteamFriends()->GetFriendPersonaName(*pID));
                // MOM_TODO: Also get their appearance data, which for now can just be colors and trail stuff?
                // const char *pData = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "trail_length"); // Or whatever
                // const char *pData2 = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "trail_color"); // Or whatever
                // const char *pData3 = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "ghost_color"); // Or whatever

                //newPlayer->SetCurrentNetFrame(newSignOn->newFrame);
                //newPlayer->SetGhostAppearance(newSignOn->newApps);
                //newPlayer->SetGhostSteamID(newSignOn->SteamID);

                CMomentumGhostClient::m_mapOnlineGhosts.Insert(pID->ConvertToUint64(), newPlayer);

                if (m_flNextUpdateTime < 0)
                    m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat()); // MOM_TODO: Probably move this out of the for loop and use a boolean check instead
            }
        }
        else if (findIndx != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
        {
            // They changed map, remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = CMomentumGhostClient::m_mapOnlineGhosts[findIndx];
            if (pEntity)
                pEntity->Remove();
            
            CMomentumGhostClient::m_mapOnlineGhosts.RemoveAt(findIndx);
        }
    }
    else
    {
        int numMembers = steamapicontext->SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
        for (int i = 0; i < numMembers; i++)
        {
            CSteamID member = steamapicontext->SteamMatchmaking()->GetLobbyMemberByIndex(m_sLobbyID, i);
            if (member == localID) // If it's us, don't care
                continue;

            CheckToAdd(&member);
        }
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

void CMomentumLobbySystem::SendAndRecieveP2PPackets()
{
    if (CMomentumGhostClient::m_mapOnlineGhosts.Count() > 0)
    {
        // Read data
        uint32 size;
        if (steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&size))
        {
            //DevLog("Packet available! Size: %u bytes where sizeof frame is %i bytes\n", size, sizeof ghostNetFrame_t);
            if (size % sizeof ghostNetFrame_t == 0)
            {
                ghostNetFrame_t frame;
                uint32 bytesRead;
                CSteamID fromWho;
                if (steamapicontext->SteamNetworking()->ReadP2PPacket(&frame, sizeof frame, &bytesRead, &fromWho))
                {
                    //DevLog("Read the packet successfully! Read bytes: %u, from steamID %lld\n", bytesRead, fromWho.ConvertToUint64());
                    uint16_t findIndex = CMomentumGhostClient::m_mapOnlineGhosts.Find(fromWho.ConvertToUint64());
                    if (findIndex != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
                    {
                        CMomentumGhostClient::m_mapOnlineGhosts[findIndex]->SetCurrentNetFrame(frame);
                    }
                }
            }
        }

        // Send data
        // MOM_TODO: Change this to be server-client with the lobby owner being the "server" for everyone.
        // Only one packet should be sent here if you are not the lobby owner. Otherwise, send everybody's data to
        // everybody. Right now it's just pure P2P.

        // CSteamID owner = steamapicontext->SteamMatchmaking()->GetLobbyOwner(m_sLobbyID);

        if (m_flNextUpdateTime > 0 && gpGlobals->curtime > m_flNextUpdateTime)
        {
            ghostNetFrame_t frame;
            if (g_pMomentumGhostClient->CreateNewNetFrame(frame))
            {
                uint16_t index = CMomentumGhostClient::m_mapOnlineGhosts.FirstInorder();
                while (index != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
                {
                    CSteamID ghost = CMomentumGhostClient::m_mapOnlineGhosts[index]->GetGhostSteamID();
               
                    if (steamapicontext->SteamNetworking()->SendP2PPacket(ghost, &frame, sizeof frame, k_EP2PSendUnreliable))
                    {
                        // DevLog("Sent the packet!\n");
                    }

                    index = CMomentumGhostClient::m_mapOnlineGhosts.NextInorder(index);
                }

                m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());
            }
        }
    }
}

static CMomentumLobbySystem s_MOMLobbySystem("CMomentumLobbySystem");
CMomentumLobbySystem *g_pMomentumLobbySystem = &s_MOMLobbySystem;