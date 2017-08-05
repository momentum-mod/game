#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "mom_online_ghost.h"
#include "icommandline.h"

#include "tier0/memdbgon.h"

static ConVar mm_address("mom_ghost_address", "127.0.0.1:9000", FCVAR_HIDDEN | FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE);

ConVar mm_updaterate("mom_ghost_online_updaterate", "20",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of updates per second to and from the ghost server.\n", true, 1.0f, true, 1000.0f);

ConVar mm_timeOutDuration("mom_ghost_online_timeout_duration", "10",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Seconds to wait when timimg out from a ghost server.\n", true, 5.0f, true, 30.0f);

//we have to wait a few ticks to let the interpolation catch up with our ghosts!
ConVar mm_lerpRatio("mom_ghost_online_lerp_ratio", "2",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of ticks to wait before updating ghosts, to allow client to interpolate.\n", true, 0.0f, true, 10.0f);

zed_net_socket_t CMomentumGhostClient::m_socket;
zed_net_address_t CMomentumGhostClient::m_address;

bool CMomentumGhostClient::m_ghostClientConnected = false;
bool CMomentumGhostClient::m_bRanThread = false;

CMomentumPlayer* CMomentumGhostClient::m_pPlayer;
uint64 CMomentumGhostClient::m_SteamID;
CUtlVector<CMomentumOnlineGhostEntity*> CMomentumGhostClient::ghostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxGhostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxpPlayer;
ghostAppearance_t CMomentumGhostClient::oldAppearance;

ThreadHandle_t netIOThread;
CMessageQueue<int> SentPacketQueue;

void CMomentumGhostClient::PostInit()
{
    Log("================= COMMAND LINE: %s\n", CommandLine()->GetCmdLine());
}

void CMomentumGhostClient::LevelInitPostEntity()
{
    // MOM_TODO: AdvertiseGame needs to use k_steamIDNonSteamGS and pass the IP (as hex) and port if it is inside a server 
    // steamapicontext->SteamUser()->AdvertiseGame(steamapicontext->SteamUser()->GetSteamID(), 0, 0); // Gives game info of current server, useful if actually on server
    // steamapicontext->SteamFriends()->SetRichPresence("connect", "blah"); // Allows them to click "Join game" from Steam

    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        const char *pMapName = gpGlobals->mapname.ToCStr();
        DevLog("Setting the map to %s!\n", pMapName);
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, "map", pMapName);

        // Now check if this map is the same as somebody else's in the lobby
        CheckToAdd(nullptr);
    }
}
void CMomentumGhostClient::LevelShutdownPreEntity()
{
    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        DevLog("Setting map to null, since we're going to the menu.\n");
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, "map", nullptr);

        if (m_mapOnlineGhosts.Count() > 0)
        {
            m_mapOnlineGhosts.RemoveAll(); // NOTE: The game handles clearing the entities! No need to delete here.
        }
    }
    
}
void CMomentumGhostClient::FrameUpdatePreEntityThink()
{
    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());

    if (m_pPlayer)
    {
        if (m_mapOnlineGhosts.Count() > 0 && m_flNextUpdateTime > 0 && gpGlobals->curtime > m_flNextUpdateTime)
        {
            // Read data
            uint32 size;
            if (steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&size))
            {
                DevLog("Packet available! Size: %u bytes\n", size);
                ghostNetFrame_t frame;
                uint32 bytesRead;
                CSteamID fromWho;
                if (steamapicontext->SteamNetworking()->ReadP2PPacket(&frame, sizeof frame, &bytesRead, &fromWho))
                {
                    DevLog("Read the packet successfully! Read bytes: %u, from steamID %lld\n", bytesRead, fromWho.ConvertToUint64());
                    unsigned short findIndex = m_mapOnlineGhosts.Find(fromWho.ConvertToUint64());
                    if (findIndex != m_mapOnlineGhosts.InvalidIndex())
                    {
                        m_mapOnlineGhosts[findIndex]->SetCurrentNetFrame(frame);
                    }
                }
            }

            // Send data
            unsigned short firstIndex = m_mapOnlineGhosts.FirstInorder();
            while (firstIndex != m_mapOnlineGhosts.InvalidIndex())
            {
                CSteamID ghost = m_mapOnlineGhosts[firstIndex]->GetGhostSteamID();
                ghostNetFrame_t frame = CreateNewNetFrame(m_pPlayer);
                if (steamapicontext->SteamNetworking()->SendP2PPacket(ghost, &frame, sizeof frame, k_EP2PSendUnreliable))
                {
                    DevLog("Sent the packet!\n");
                }
            }

            m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());
        }
    }
}

void CMomentumGhostClient::Shutdown()
{
    LeaveLobby(); // Leave the lobby if we're still in it
}

CON_COMMAND(mom_host_lobby, "Starts hosting a lobby\n")
{
    DevMsg("Called command!\n");
    g_pMomentumGhostClient->StartLobby();
}

CON_COMMAND(mom_leave_lobby, "Leave your current lobby\n")
{
    g_pMomentumGhostClient->LeaveLobby();
}

// So basically, if a user wants to connect to us, we're considered the host. 
void CMomentumGhostClient::HandleNewP2PRequest(P2PSessionRequest_t* info)
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

void CMomentumGhostClient::HandleP2PConnectionFail(P2PSessionConnectFail_t* info)
{
    Warning("Couldn't do Steam P2P because of the error: %i\n", info->m_eP2PSessionError);

    // MOM_TODO: Make a block list that only refreshes on game restart? Helps bad connections from continuously looping
    steamapicontext->SteamNetworking()->CloseP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumGhostClient::SendChatMessage(char* pMessage)
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

void CMomentumGhostClient::GetLobbyMemberSteamData(CSteamID pMember)
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
void CMomentumGhostClient::HandleFriendJoin(GameRichPresenceJoinRequested_t* pJoin)
{
    // MOM_TODO: Have a global convar that auto blocks requests (busy vs online) 
    // m_sHostID = pJoin->m_steamIDFriend;
    //steamapicontext->SteamNetworking()->SendP2PPacket(pJoin->m_steamIDFriend, );
    
   
}

// Called when trying to join somebody else's lobby. We need to actually call JoinLobby here.
void CMomentumGhostClient::HandleLobbyJoin(GameLobbyJoinRequested_t* pJoin)
{
    // Get the lobby owner
    CSteamID owner = steamapicontext->SteamMatchmaking()->GetLobbyOwner(pJoin->m_steamIDLobby);

    steamapicontext->SteamMatchmaking()->JoinLobby(pJoin->m_steamIDLobby);
}

// Called when we created the lobby
void CMomentumGhostClient::HandleLobbyCreated(LobbyCreated_t* pCreated, bool ioFailure)
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

        // Set some info
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, "map", gpGlobals->mapname.ToCStr());
        // MOM_TODO: Set appearance of our ghost here
    }
}

void CMomentumGhostClient::StartLobby()
{
    if (!(m_cLobbyCreated.IsActive() || m_sLobbyID.IsValid() || m_sLobbyID.IsLobby()))
    {
        SteamAPICall_t call = steamapicontext->SteamMatchmaking()->CreateLobby(k_ELobbyTypeFriendsOnly, 10);
        m_cLobbyCreated.Set(call, this, &CMomentumGhostClient::HandleLobbyCreated);
        DevLog("The lobby call successfully happened!\n");
    }
    else
        DevLog("The lobby could not be created because you already made one or are in one!\n");
}

void CMomentumGhostClient::LeaveLobby()
{
    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        steamapicontext->SteamMatchmaking()->LeaveLobby(m_sLobbyID);
        DevLog("Left the lobby!\n");
        m_sLobbyID = k_steamIDNil;
    }
    else
        DevLog("Could not leave lobby, are you in one?\n");
}

// Called when we enter a lobby
// NOTE: I'm not actually sure if this gets called after HandleLobbyCreated, given the user created the lobby
// Hopefully it is, that way we can know for sure they created the lobby
// Even then we could just set some boolean....
void CMomentumGhostClient::HandleLobbyEnter(LobbyEnter_t* pEnter)
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
    CSteamID localID = steamapicontext->SteamUser()->GetSteamID();
    // Get everybody in the lobby's data
    int numMembers = steamapicontext->SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
    for (int i = 0; i < numMembers; i++)
    {
        CSteamID member = steamapicontext->SteamMatchmaking()->GetLobbyMemberByIndex(m_sLobbyID, i);
        if (member == localID) // If it's us, don't care
            continue;
        //GetLobbyMemberSteamData(member); // Get their name and avatar MOM_TODO: Does this just happen asynchronously?
        const char *pMapName = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "map"); // Or whatever
        DevLog("User %lld is on map %s\n", member.ConvertToUint64(), pMapName);
    }
}

// We got a message yaay
void CMomentumGhostClient::HandleLobbyChatMsg(LobbyChatMsg_t* pParam)
{
    char *message = new char[4096];
    int written = steamapicontext->SteamMatchmaking()->GetLobbyChatEntry(CSteamID(pParam->m_ulSteamIDLobby), pParam->m_iChatID, nullptr, message, 4096, nullptr);
    DevLog("SERVER: Got a chat message! Wrote %i byte(s) into buffer.\n", written);
    Msg("SERVER: Chat message: %s\n", message);
    delete[] message;
}

void CMomentumGhostClient::HandleLobbyDataUpdate(LobbyDataUpdate_t* pParam)
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
            CheckToAdd(memberChanged); // See the todo on the method
        }
    }
}

// Somebody left/joined, or the owner of the lobby was changed
void CMomentumGhostClient::HandleLobbyChatUpdate(LobbyChatUpdate_t* pParam)
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

void CMomentumGhostClient::HandlePersonaCallback(PersonaStateChange_t* pParam)
{
    //DevLog("HandlePersonaCallback: %u with changeflags: %i\n", pParam->m_ulSteamID, pParam->m_nChangeFlags);
    CSteamID person = CSteamID(pParam->m_ulSteamID);
    if (pParam->m_nChangeFlags & k_EPersonaChangeName)
    {
        DevLog("Got the name of %lld: %s\n", pParam->m_ulSteamID, steamapicontext->SteamFriends()->GetFriendPersonaName(person));
    }
}


bool CMomentumGhostClient::initGhostClient()
{
    m_socket = zed_net_socket_t();
    m_address = zed_net_address_t();
    CMessageQueue<int>(SentPacketQueue); //reset queue (it doesnt actually work i dont think lol)
    if (zed_net_init() == 0)
    {
        int success = zed_net_tcp_socket_open(&m_socket, 0, 0, 0);
        return success == 0 ? true : false;
    }
    Warning("Could not init network! Error: %s\n", zed_net_get_error());
    return false;
}
// Returns true if we successfully exited
bool CMomentumGhostClient::exitGhostClient()
{
    bool returnValue;
    ghostSignOffPacket_t newSignOff;
    Q_strncpy(newSignOff.Message, "Disconnected", sizeof(newSignOff.Message));
    if (zed_net_tcp_socket_send(&m_socket, &newSignOff, sizeof(newSignOff), PT_SIGNOFF) < 0)
    {
        Warning("Could not send signoff packet! Error: %s\n", zed_net_get_error());
        returnValue = false;
    }
    else
    {
        ConDColorMsg(Color(255, 255, 0, 255), "Sent signoff packet, exiting ghost client...\n");
        char data[256];
        int packet_type;
        int bytes_read = zed_net_tcp_socket_receive(&m_socket, data, 256, &packet_type);
        if (bytes_read && packet_type == PT_ACK)
        {
            returnValue = true;
            ConDColorMsg(Color(255, 255, 0, 255), "Got ACK from server. We are disconnected\n");
        }
        else
        {
            DevWarning("Did not receive signoff ACK: Error: %s\n", zed_net_get_error());
            returnValue = false;
        }
    }

    zed_net_socket_close(&m_socket);
    zed_net_shutdown();
    m_mtxGhostPlayers.Lock();
    ghostPlayers.Purge();
    m_mtxGhostPlayers.Unlock();
    m_bRanThread = false;
    return returnValue;
}
bool CMomentumGhostClient::connectToGhostServer(const char* host, unsigned short port)
{
    if (zed_net_get_address(&m_address, host, port) != 0)
    {
        Warning("Error: %s\n", zed_net_get_error());

        zed_net_socket_close(&m_socket);
        zed_net_shutdown();
        return false;
    }

    if (zed_net_tcp_connect(&m_socket, m_address))
    {
        Warning("Failed to connect to %s:%d\n", host, port);
        return false;
    }
    ConColorMsg(Color(255, 255, 0, 255), "Connected to %s:%d\n", host, port);
    return true;
}
ghostNetFrame_t CMomentumGhostClient::CreateNewNetFrame(CMomentumPlayer *pPlayer)
{
    return ghostNetFrame_t(pPlayer->EyeAngles(),
                           pPlayer->GetAbsOrigin(),
                           pPlayer->GetAbsVelocity(),
                           pPlayer->GetViewOffset().z,
                           pPlayer->m_nButtons);
}
bool CMomentumGhostClient::SendSignonMessage()
{
    ghostSignOnPacket_t newSignOn;
    if (m_pPlayer)
    {
        newSignOn.newFrame = CreateNewNetFrame(m_pPlayer);
        newSignOn.newApps = CreateAppearance(m_pPlayer);
        newSignOn.SteamID = m_SteamID;
        Q_strncpy(newSignOn.MapName, gpGlobals->mapname.ToCStr(), sizeof(newSignOn.MapName));
    }
    if (zed_net_tcp_socket_send(&m_socket, &newSignOn, sizeof(newSignOn), PT_SIGNON) < 0)
    {
        Warning("Error: %s\n", zed_net_get_error());
        return false;
    }
    else
    {
        ConColorMsg(Color(255, 255, 0, 255), "Sending signon packet...\n");
        char buffer[256];
        int packet_type;
        int bytes_read = zed_net_tcp_socket_receive(&m_socket, buffer, 256, &packet_type);
        if (bytes_read && packet_type == PT_ACK) //server ACKed us! :)
        {
            ghostAckPacket_t *newAck = reinterpret_cast<ghostAckPacket_t*>(buffer);
            if (newAck->AckSuccess)
            {
                ConColorMsg(Color(255, 255, 0, 255), "Server ACKed. We are on the same map.\n");
                return true;
            }
            else
            {
                ConColorMsg(Color(255, 255, 0, 255), "Server ACKed, But we're on the wrong map.\n");
            }
        }
    }
    return false;
}
bool CMomentumGhostClient::SendAppearanceData(ghostAppearance_t apps)
{
    oldAppearance = m_pPlayer->m_playerAppearanceProps;
    if (zed_net_tcp_socket_send(&m_socket, &m_pPlayer->m_playerAppearanceProps, sizeof(m_pPlayer->m_playerAppearanceProps), PT_APPEARANCE) < 0)
        return false;
    return true;
}
bool CMomentumGhostClient::SendNetFrame(ghostNetFrame_t frame)
{
    if (zed_net_tcp_socket_send(&m_socket, &frame, sizeof(ghostNetFrame_t), PT_NET_FRAME) < 0)
        return false;
    return true;
}

void CMomentumGhostClient::CheckToAdd(CSteamID *pID)
{
    CSteamID localID = steamapicontext->SteamUser()->GetSteamID();

    if (pID)
    {
        const char *pOtherMap = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, *pID, "map");
        if (FStrEq(gpGlobals->mapname.ToCStr(), pOtherMap))
        {
            CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
            newPlayer->SetGhostSteamID(*pID);
            newPlayer->Spawn();

            // MOM_TODO: Also get their appearance data, which for now can just be colors and trail stuff?
            // const char *pData = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "trail_length"); // Or whatever
            // const char *pData2 = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "trail_color"); // Or whatever
            // const char *pData3 = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, "ghost_color"); // Or whatever

            //newPlayer->SetCurrentNetFrame(newSignOn->newFrame);
            //newPlayer->SetGhostAppearance(newSignOn->newApps);
            //newPlayer->SetGhostSteamID(newSignOn->SteamID);

            m_mapOnlineGhosts.Insert(pID->ConvertToUint64(), newPlayer);

            if (m_flNextUpdateTime < 0)
                m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat()); // MOM_TODO: Probably move this out of the for loop and use a boolean check instead
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

//Threaded function
unsigned CMomentumGhostClient::sendAndRecieveData(void *params)
{
    // @tuxxi: I wanted to use the MessageQueue system to send the signon packet too, but I couldn't for the life of me
    // get the Queue to completely reset when we disconnected/ reloaded a map. so, this is the solution :<
    static bool FirstNewFrame = true;
    if (FirstNewFrame)
    {
        m_ghostClientConnected = SendSignonMessage();
        FirstNewFrame = false;
    }

    while (m_ghostClientConnected)
    {
        // ------------------------------------
        // DATA SENT TO SERVER
        // ------------------------------------
        SentPacketQueue.QueueMessage(PT_NET_FRAME); //queue up a new net frame to be sent every cycle.

        int packetTypeToSend = -1;
        SentPacketQueue.WaitMessage(&packetTypeToSend);
        m_mtxpPlayer.Lock();
        switch (packetTypeToSend)
        {
        case PT_NET_FRAME:
            SendNetFrame(CreateNewNetFrame(m_pPlayer));
            break;
        case PT_APPEARANCE:
            SendAppearanceData(CreateAppearance(m_pPlayer));
            break;
        }
        m_mtxpPlayer.Unlock();

        // ------------------------------------
        // DATA RECIEVED FROM SERVER
        // ------------------------------------
        char buffer[uint16(~0)];  //maximum possible packet size
        int packet_type;
        int bytes_read = zed_net_tcp_socket_receive(&m_socket, buffer, uint16(~0), &packet_type); //SYN
        int tick1 = gpGlobals->tickcount;
        while (bytes_read <= 0) // 
        {
            if (!m_ghostClientConnected) break; //prevents hang on hard quit
            int tick2 = gpGlobals->tickcount;
            float deltaT = float(tick2 - tick1) * gpGlobals->interval_per_tick;
            // Can't use ThreadSleep here for some reason since GabeN called me up and said "OH MY GOD WOULD YOU JUST FUCK OFF" when I tried to
            // ... um, I mean, ... It messes up gpGlobals->tickcount
            // So this code checks if deltaT is an integer, effectively only printing every 1 second.

            //MOM_TODO: Fix this (it prints... a lot..)
            if (deltaT == round(deltaT))
            {
                //Warning("Lost connection! Waiting to time out... %f\n", deltaT);
            }

            if (deltaT > mm_timeOutDuration.GetInt())
            {
                Warning("Lost connection to ghost server.\n");
                for (int i = 0; i < (ghostPlayers.Size() ); i++)
                {
                    if (ghostPlayers[i] != nullptr) ghostPlayers[i]->Remove();
                    ghostPlayers.Remove(i);
                }

                zed_net_socket_close(&m_socket);
                zed_net_shutdown();
                m_mtxGhostPlayers.Lock();
                ghostPlayers.Purge();
                m_mtxGhostPlayers.Unlock();
                m_ghostClientConnected = false;
                FirstNewFrame = true;
                return 1;
            }
        }
        // ----------------
        // Handle recieving new net-frames from the server
        // ----------------
        if (bytes_read && packet_type == PT_NET_FRAME) //SYN-ACK
        {
            m_mtxGhostPlayers.Lock();
            int numGhosts = bytes_read / sizeof(ghostNetFrame_t);
            for (int i = 0; i < numGhosts; i++)
            {
                ghostNetFrame_t *newFrame = reinterpret_cast<ghostNetFrame_t*>(buffer + sizeof(ghostNetFrame_t) * i);
                if (i < ghostPlayers.Size()) ghostPlayers[i]->SetCurrentNetFrame(*newFrame);
            }
            if (ghostPlayers.Size() > numGhosts) //Someone disconnected, so the server told us about it.
            {
                //remove all the players that don't exist in the server anymore
                for (int i = 0; i < (ghostPlayers.Size() - numGhosts); i++)
                {
                    if (ghostPlayers[i] != nullptr) ghostPlayers[i]->Remove();
                    ghostPlayers.Remove(i); 
                }
            }
            m_mtxGhostPlayers.Unlock();
        }
        // -----------------------
        // Handle a new player signing on
        // -----------------------
        if (bytes_read && packet_type == PT_SIGNON) //a new player signed on.
        {
            ConDColorMsg(Color(255, 255, 0, 255), "Receiving new player signon from server!\n");
            int numGhosts = bytes_read / sizeof(ghostSignOnPacket_t);
            for (int i = 0; i < numGhosts; i++)
            {
                ghostSignOnPacket_t *newSignOn = reinterpret_cast<ghostSignOnPacket_t*>(buffer + sizeof(ghostSignOnPacket_t) * i);
                bool isLocalPlayer = m_SteamID == newSignOn->SteamID; //we don't want to add ourselves!
                if (!isLocalPlayer)
                {
                    CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                    newPlayer->Spawn();
                    newPlayer->SetCurrentNetFrame(newSignOn->newFrame);
                    newPlayer->SetGhostAppearance(newSignOn->newApps);
                    newPlayer->SetGhostSteamID(newSignOn->SteamID);
                    ghostPlayers.AddToTail(newPlayer);
                    DevMsg("Added new player: %s\nThere are now %i connected players.\n", newSignOn->newFrame.PlayerName, ghostPlayers.Size());
                }
                else
                {
                    CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                    newPlayer->SetCurrentNetFrame(newSignOn->newFrame);
                    ghostPlayers.AddToTail(newPlayer);

                    if (mm_ghostTesting.GetBool())
                    {
                        newPlayer->Spawn();
                        ConDColorMsg(Color(255, 255, 0, 255), "Added ghost of local player: %s\nThere are now %i connected players.\n",
                                     newSignOn->newFrame.PlayerName, ghostPlayers.Size());

                    }
                    else
                    {
                        ConDColorMsg(Color(255, 255, 0, 255), "Added local player %s, but did not spawn. Set mom_ghost_testing 1 to see local players.\n",
                                     newSignOn->newFrame.PlayerName);
                    }
                }
            }
        }
        
        //------------------
        // Recieve new appearance data if it happens to change.
        // ----------------
        m_mtxGhostPlayers.Lock();
        if (bytes_read && packet_type == PT_APPEARANCE) //ghost server is sending new appearances
        {
            ConDColorMsg(Color(255, 255, 0, 255), "Server is sending new appearances!\n");
            int numGhosts = bytes_read / sizeof(ghostAppearance_t);
            //client IDX __SHOULD__ be equal to server idx.
            for (int i = 0; i < numGhosts; i++)
            {
                ghostAppearance_t *newApps = reinterpret_cast<ghostAppearance_t*>(buffer + sizeof(ghostAppearance_t) * i);
                if (ghostPlayers[i]->HasSpawned() && i < ghostPlayers.Size())
                {
                    ConDColorMsg(Color(255, 255, 0, 255), "Set new appearance for %s!\n", ghostPlayers[i]->GetCurrentNetFrame().PlayerName);
                    ghostPlayers[i]->SetGhostAppearance(*newApps);
                }
            }
        }
        m_mtxGhostPlayers.Unlock();
        
        //------------------
        // Handle recieving new map data from the server
        // ----------------
        if (bytes_read && packet_type == PT_NEWMAP)
        {
            ghostNewMapEvent_t *newEvent = reinterpret_cast<ghostNewMapEvent_t*>(buffer);
            if (Q_strcmp(gpGlobals->mapname.ToCStr(), newEvent->MapName) != 0) //The new map is different from the one we are currently playing on
            {
                engine->ClientCommand(m_pPlayer->edict(), "map %s", newEvent->MapName);
                FirstNewFrame = true;
                return 0; //exit the thread
            }
        }

        int tickSleep = 1000 * (float(1) / mm_updaterate.GetFloat());
        ThreadSleep(tickSleep); //sleep   
    }
    FirstNewFrame = true;
    return 0;
}
static CMomentumGhostClient s_MOMGhostClient("CMomentumGhostClient");
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;