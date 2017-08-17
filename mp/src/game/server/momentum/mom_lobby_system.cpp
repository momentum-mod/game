#include "cbase.h"
#include "mom_lobby_system.h"
#include "base64.h"

#include "tier0/memdbgon.h"

CSteamID CMomentumLobbySystem::m_sLobbyID = k_steamIDNil;
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
    const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(info->m_steamIDRemote);
    Warning("Couldn't do Steam P2P with user %s because of the error: %i\n", pName, info->m_eP2PSessionError);

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

// Called when trying to join somebody else's lobby. We need to actually call JoinLobby here.
void CMomentumLobbySystem::HandleLobbyJoin(GameLobbyJoinRequested_t* pJoin)
{
    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        Warning("You are already in a lobby! Do \"mom_leave_lobby\" to leave the lobby!\n");
    }
    else
    {
        SteamAPICall_t call = steamapicontext->SteamMatchmaking()->JoinLobby(pJoin->m_steamIDLobby);
        m_cLobbyJoined.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyJoined);
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

// This is called when we explicitly call JoinLobby. Creating a lobby does not call this.
void CMomentumLobbySystem::CallResult_LobbyJoined(LobbyEnter_t* pEntered, bool IOFailure)
{
    if (!pEntered || IOFailure)
    {
        Warning("Could not join lobby due to IO error!\n");
        return;
    }

    HandleLobbyEnter(pEntered);
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
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, gpGlobals->mapname.ToCStr());

    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (pPlayer)
    {
        DevLog("Sending our appearance.\n");
        SetAppearanceInMemberData(pPlayer->m_playerAppearanceProps);
    }

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
void CMomentumLobbySystem::SetAppearanceInMemberData(ghostAppearance_t app)
{
    char base64Appearance[1024];
    base64_encode(&app, sizeof app, base64Appearance, 1024);
    //DevLog("Base64 encoded appearance: %s\n", base64Appearance);
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_APPEARANCE, base64Appearance);
}
LobbyGhostAppearance_t CMomentumLobbySystem::GetAppearanceFromMemberData(CSteamID member)
{
    LobbyGhostAppearance_t toReturn;
    const char *pAppearance = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, LOBBY_DATA_APPEARANCE);
    Q_strncpy(toReturn.base64, pAppearance, sizeof(toReturn.base64));
    //DevLog("Got appearance Base64: %s\n", pAppearance);
    ghostAppearance_t newAppearance;
    base64_decode(pAppearance, &newAppearance, sizeof(ghostAppearance_t));
    toReturn.appearance = newAppearance;
    return toReturn;
}

CMomentumOnlineGhostEntity* CMomentumLobbySystem::GetLobbyMemberEntity(uint64_t id)
{
    uint16_t findIndx = CMomentumGhostClient::m_mapOnlineGhosts.Find(id);
    if (findIndx != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
        return CMomentumGhostClient::m_mapOnlineGhosts[findIndx];
    
    return nullptr;
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
            
            // Check their appearance for any changes
            CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(memberChanged);
            if (pEntity)
            {
                pEntity->SetGhostAppearance(GetAppearanceFromMemberData(memberChanged));

                if (!GetIsSpectatingFromMemberData(memberChanged)) //they are not spectating, so we need to make sure they are visible.
                {
                    pEntity->UnHideGhost();
                }
            }

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

        CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "LobbyUpdateMsg");
        WRITE_BYTE(LOBBY_UPDATE_MEMBER_JOIN);
        WRITE_BYTES(&pParam->m_ulSteamIDUserChanged, sizeof(uint64));
        MessageEnd();
    }
    if (state & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
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

        CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "LobbyUpdateMsg");
        WRITE_BYTE(LOBBY_UPDATE_MEMBER_LEAVE);
        WRITE_BYTES(&pParam->m_ulSteamIDUserChanged, sizeof(uint64));
        MessageEnd();
    }
}

void CMomentumLobbySystem::HandlePersonaCallback(PersonaStateChange_t* pParam)
{
    //DevLog("HandlePersonaCallback: %u with changeflags: %i\n", pParam->m_ulSteamID, pParam->m_nChangeFlags);
    CSteamID person = CSteamID(pParam->m_ulSteamID);
    if (pParam->m_nChangeFlags & k_EPersonaChangeName && LobbyValid())
    {
        // Quick and ugly check to see if they're in our lobby
        const char *pCheck = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, person, LOBBY_DATA_MAP);
        if (pCheck)
        {
            // It's not null so they're here, but are they in our map?
            CMomentumOnlineGhostEntity *pGhost = GetLobbyMemberEntity(pParam->m_ulSteamID);
            if (pGhost)
            {
                // Yes they are
                const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(person);
                DevLog("Got the name of %lld: %s\n", pParam->m_ulSteamID, pName);
                pGhost->SetGhostName(pName);
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
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, pMapName);

        m_flNextUpdateTime = -1.0f;

        // Now check if this map is the same as somebody else's in the lobby
        if (pMapName)
            CheckToAdd(nullptr);
        else
            g_pMomentumGhostClient->ClearCurrentGhosts(false);
    }
}

void CMomentumLobbySystem::CheckToAdd(CSteamID *pID)
{
    CSteamID localID = steamapicontext->SteamUser()->GetSteamID();

    if (pID)
    {
        const char *pOtherMap = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, *pID, LOBBY_DATA_MAP);
        uint64 pID_int = pID->ConvertToUint64();
        unsigned short findIndx = CMomentumGhostClient::m_mapOnlineGhosts.Find(pID_int);
        // Just joined this map, we haven't created them 
        const char *pMapName = gpGlobals->mapname.ToCStr();
        bool isSpectating = GetIsSpectatingFromMemberData(*pID);
        if (pMapName && FStrEq(pMapName, pOtherMap)) //We're on the same map
        {
            // Don't add them again if they reloaded this map for some reason
            if (findIndx == CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex() && !isSpectating) //dont try to add them again if they are spectating
            {
                CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                newPlayer->SetGhostSteamID(*pID);
                newPlayer->Spawn();
                newPlayer->SetGhostName(steamapicontext->SteamFriends()->GetFriendPersonaName(*pID));
                newPlayer->SetGhostAppearance(GetAppearanceFromMemberData(*pID));

                CMomentumGhostClient::m_mapOnlineGhosts.Insert(pID_int, newPlayer);

                if (m_flNextUpdateTime < 0)
                    m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());

                // "_____ just joined your map."
                if (CMomentumGhostClient::m_pPlayer)
                {
                    CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
                    user.MakeReliable();
                    UserMessageBegin(user, "LobbyUpdateMsg");
                    WRITE_BYTE(LOBBY_UPDATE_MEMBER_JOIN_MAP);
                    WRITE_BYTES(&pID_int, sizeof(uint64));
                    MessageEnd();
                }
            }
            else if (isSpectating)
            {
                //they are spectating, hide their entity. If we remove it, we will stop sending them packets, which is a bad thing.
                
                CMomentumOnlineGhostEntity *pEntity = CMomentumGhostClient::m_mapOnlineGhosts[findIndx];               
                if (pEntity)
                    pEntity->HideGhost();
                
            }
        }
        else if (findIndx != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
        {
            // They changed map remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = CMomentumGhostClient::m_mapOnlineGhosts[findIndx];
            if (pEntity)
                pEntity->Remove();
            
            CMomentumGhostClient::m_mapOnlineGhosts.RemoveAt(findIndx);

            // "_____ just left your map."
            CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
            user.MakeReliable();
            UserMessageBegin(user, "LobbyUpdateMsg");
            WRITE_BYTE(LOBBY_UPDATE_MEMBER_LEAVE_MAP);
            WRITE_BYTES(&pID_int, sizeof(uint64));
            MessageEnd();
            
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
                m_cLobbyJoined.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyJoined);
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
        while (steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&size))
        {
            //DevLog("Packet available! Size: %u bytes where sizeof frame is %i bytes\n", size, sizeof ghostNetFrame_t);
            if (size == sizeof(ghostNetFrame_t))
            {
                ghostNetFrame_t frame;
                uint32 bytesRead;
                CSteamID fromWho;
                if (steamapicontext->SteamNetworking()->ReadP2PPacket(&frame, sizeof(frame), &bytesRead, &fromWho))
                {
                    //DevLog("Read the packet successfully! Read bytes: %u, from steamID %lld\n", bytesRead, fromWho.ConvertToUint64());
                    CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(fromWho);
                    if (pEntity)
                        pEntity->SetCurrentNetFrame(frame);
                }
            }
            else if (size == sizeof(ghostSpecUpdate_t))
            {
                ghostSpecUpdate_t update;
                uint32 bytesRead;
                CSteamID fromWho;
                if (steamapicontext->SteamNetworking()->ReadP2PPacket(&update, sizeof(update), &bytesRead, &fromWho))
                {
                    CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
                    user.MakeReliable();
                    UserMessageBegin(user, "SpecUpdateMsg");
                    WRITE_BYTE(update.type);
                    WRITE_BYTES(&fromWho, sizeof(uint64));
                    WRITE_BYTES(&update.specTarget, sizeof(uint64));
                    MessageEnd();
                }
            }
        }

        // Send data
        if (m_flNextUpdateTime > 0 && gpGlobals->curtime > m_flNextUpdateTime)
        {
            ghostNetFrame_t frame;
            if (g_pMomentumGhostClient->CreateNewNetFrame(frame))
            {
                uint16_t index = CMomentumGhostClient::m_mapOnlineGhosts.FirstInorder();
                while (index != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
                {
                    CSteamID ghost = CMomentumGhostClient::m_mapOnlineGhosts[index]->GetGhostSteamID();
               
                    if (steamapicontext->SteamNetworking()->SendP2PPacket(ghost, &frame, sizeof(frame), k_EP2PSendUnreliable))
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
void CMomentumLobbySystem::SetIsSpectating(bool bSpec)
{
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_IS_SPEC, bSpec ? "1" : "0");
}
//Return true if the lobby member is currently spectating.
bool CMomentumLobbySystem::GetIsSpectatingFromMemberData(CSteamID who)
{
    const char* specChar = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, who, LOBBY_DATA_IS_SPEC);
    return specChar[0] == '1';
}
void CMomentumLobbySystem::SetSpectatorTarget(CSteamID ghostTarget, SPECTATE_MSG_TYPE type)
{
    char base64SteamID[64];
    base64_encode(&ghostTarget, sizeof(ghostTarget), base64SteamID, 64);
    //DevLog("Base64 encoded appearance: %s\n", base64Appearance);
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_SPEC_TARGET, base64SteamID);
    SendSpectatorUpdatePacket(ghostTarget, type);
}
//Sends the spectator info update packet to all current ghosts
void CMomentumLobbySystem::SendSpectatorUpdatePacket(CSteamID ghostTarget, SPECTATE_MSG_TYPE type)
{
    ghostSpecUpdate_t newUpdate;
    newUpdate.specTarget = ghostTarget;
    newUpdate.type = type;
    uint16_t index = CMomentumGhostClient::m_mapOnlineGhosts.FirstInorder();
    while (index != CMomentumGhostClient::m_mapOnlineGhosts.InvalidIndex())
    {
        CSteamID ghost = CMomentumGhostClient::m_mapOnlineGhosts[index]->GetGhostSteamID();

        if (steamapicontext->SteamNetworking()->SendP2PPacket(ghost, &newUpdate, sizeof(newUpdate), k_EP2PSendReliable))
        {
            DevLog("Sent the spectate update packet!\n");
        }

        index = CMomentumGhostClient::m_mapOnlineGhosts.NextInorder(index);
    }
}
CSteamID CMomentumLobbySystem::GetSpectatorTargetFromMemberData(CSteamID who)
{
    const char *base64SteamID;
    CSteamID toReturn;
    base64SteamID = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, who, LOBBY_DATA_SPEC_TARGET);
    base64_decode(base64SteamID, &toReturn, sizeof(CSteamID));
    return toReturn;
}
static CMomentumLobbySystem s_MOMLobbySystem;
CMomentumLobbySystem *g_pMomentumLobbySystem = &s_MOMLobbySystem;