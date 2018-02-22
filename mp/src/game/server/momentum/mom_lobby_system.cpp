#include "cbase.h"
#include "mom_lobby_system.h"
#include "base64.h"
#include "mom_steam_helper.h"
#include "ghost_client.h"
#include "mom_online_ghost.h"

#include "tier0/memdbgon.h"

CSteamID CMomentumLobbySystem::m_sLobbyID = k_steamIDNil;
float CMomentumLobbySystem::m_flNextUpdateTime = -1.0f;

CON_COMMAND(mom_lobby_create, "Starts hosting a lobby\n")
{
    g_pMomentumLobbySystem->StartLobby();
}

CON_COMMAND(mom_lobby_leave, "Leave your current lobby\n")
{
    g_pMomentumLobbySystem->LeaveLobby();
}

// Used when joining when the game isn't loaded
CON_COMMAND(connect_lobby, "Connect to a given SteamID's lobby\n")
{
    g_pMomentumLobbySystem->JoinLobbyFromString(args.Arg(1));
}

CON_COMMAND(mom_lobby_invite, "Invite friends to your lobby\n")
{
    if (g_pMomentumLobbySystem->LobbyValid())
        steamapicontext->SteamFriends()->ActivateGameOverlayInviteDialog(g_pMomentumLobbySystem->GetLobbyId());
}

static MAKE_CONVAR(mom_lobby_max_players, "10", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the maximum number of players allowed in lobbies you create.\n", 2, 250);
static MAKE_CONVAR(mom_lobby_type, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the type of the lobby. 0 = Invite only, 1 = Friends Only, 2 = Public\n", 0, 2);

// So basically, if a user wants to connect to us, we're considered the host. 
void CMomentumLobbySystem::HandleNewP2PRequest(P2PSessionRequest_t* info)
{
    const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(info->m_steamIDRemote);

    // MOM_TODO: Make a (temp) block list that only refreshes on game restart?
    if (m_vecBlocked.Find(info->m_steamIDRemote) != -1)
    {
        DevLog("Not allowing %s to talk with us, we've marked them as blocked!\n", pName);
        return;
    }

    // MOM_TODO: Take into account that this user could potentially not be in our lobby (security)

    // Needs to be done to open the connection with them
    steamapicontext->SteamNetworking()->AcceptP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumLobbySystem::HandleP2PConnectionFail(P2PSessionConnectFail_t* info)
{
    const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(info->m_steamIDRemote);
    if (info->m_eP2PSessionError == k_EP2PSessionErrorTimeout)
        DevLog("Dropping connection with %s due to timing out! (They probably left/disconnected)\n", pName);
    else
        Warning("Steam P2P failed with user %s because of the error: %i\n", pName, info->m_eP2PSessionError);
    
    steamapicontext->SteamNetworking()->CloseP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumLobbySystem::SendChatMessage(char* pMessage)
{
    if (LobbyValid())
    {
        int len = Q_strlen(pMessage) + 1;
        bool result = steamapicontext->SteamMatchmaking()->SendLobbyChatMsg(m_sLobbyID, pMessage, len);
        if (result)
            DevLog("Sent chat message! Message: %s\n", pMessage);
        else
            DevLog("Did not send lobby message!\n");
    }
    else
    {
        DevLog("Could not send message because you are not connected!\n");
    }
}

void CMomentumLobbySystem::ResetOtherAppearanceData()
{
    if (LobbyValid())
    {
        uint16 index = m_mapLobbyGhosts.FirstInorder();
        while (index != m_mapLobbyGhosts.InvalidIndex())
        {
            CMomentumOnlineGhostEntity *pEntity = m_mapLobbyGhosts[index];
            if (pEntity)
                pEntity->SetGhostAppearance(pEntity->GetGhostAppearance(), true);

            index = m_mapLobbyGhosts.NextInorder(index);
        }
    }
}

// Called when trying to join somebody else's lobby. We need to actually call JoinLobby here.
void CMomentumLobbySystem::HandleLobbyJoin(GameLobbyJoinRequested_t* pJoin)
{
    if (LobbyValid())
    {
        LeaveLobby();
        Log("Leaving your current lobby to join the new one...\n");
    }
    
    if (m_cLobbyJoined.IsActive())
    {
        Warning("Not joining the lobby due to you already joining one!\n");
    }
    else
    {
        SteamAPICall_t call = steamapicontext->SteamMatchmaking()->JoinLobby(pJoin->m_steamIDLobby);
        m_cLobbyJoined.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyJoined);
    }
}

CMomentumLobbySystem::CMomentumLobbySystem() : m_bHostingLobby(false)
{
    SetDefLessFunc(m_mapLobbyGhosts);
}

CMomentumLobbySystem::~CMomentumLobbySystem()
{
    
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
        DevLog("Result is okay! We got a lobby!\n");
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

    DevLog("(LOBBY ENTER CALL RESULT): Got the callresult with result %i\n", pEntered->m_EChatRoomEnterResponse);
    // Note: There's both a callback and a call result from this. 
    // The callback handles actually entering, this is just for assurance that the call result completed.
}

void CMomentumLobbySystem::StartLobby()
{
    if (!(m_cLobbyCreated.IsActive() || LobbyValid()))
    {
        SteamAPICall_t call = steamapicontext->SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>(mom_lobby_type.GetInt()), mom_lobby_max_players.GetInt());
        m_cLobbyCreated.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyCreated);
        DevLog("The lobby call successfully happened!\n");
    }
    else
        DevLog("The lobby could not be created because you already made one or are in one!\n");
}

void CMomentumLobbySystem::LeaveLobby()
{
    if (LobbyValid())
    {
        // Actually leave the lobby
        steamapicontext->SteamMatchmaking()->LeaveLobby(m_sLobbyID);
        // Clear the ghosts stored in our lobby system
        g_pMomentumGhostClient->ClearCurrentGhosts(true);
        // Clear out any rich presence 
        steamapicontext->SteamFriends()->ClearRichPresence();

        // Notify literally everything that can listen that we left
        FIRE_GAME_WIDE_EVENT("lobby_leave");
        
        // Lastly, set the lobby ID to nil
        m_sLobbyID = k_steamIDNil;

        DevLog("Left the lobby!\n");
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

    DevLog("(LOBBY ENTER CALLBACK) Lobby entered! Lobby ID: %lld\n", pEnter->m_ulSteamIDLobby);

    if (!m_sLobbyID.IsValid())
    {
        m_sLobbyID = CSteamID(pEnter->m_ulSteamIDLobby);
    }

    FIRE_GAME_WIDE_EVENT("lobby_join");

    // Set our own data
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, gpGlobals->mapname.ToCStr());
    // Note: Our appearance is also set on spawn, so no worries if we're null here.
    CMomentumPlayer *pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
    if (pPlayer)
    {
        DevLog("Sending our appearance.\n");
        SetAppearanceInMemberData(pPlayer->m_playerAppearanceProps);
    }
    SetGameInfoStatus();
    // Get everybody else's data
    CheckToAdd(nullptr);
}

// We got a message yaay
void CMomentumLobbySystem::HandleLobbyChatMsg(LobbyChatMsg_t* pParam)
{
    // MOM_TODO: Keep this for if we ever end up using binary messages 

    char *message = new char[4096];
    int written = steamapicontext->SteamMatchmaking()->GetLobbyChatEntry(CSteamID(pParam->m_ulSteamIDLobby), pParam->m_iChatID, nullptr, message, 4096, nullptr);
    DevLog("SERVER: Got a chat message! Wrote %i byte(s) into buffer.\n", written);
    Msg("SERVER: Chat message: %s\n", message);
    delete[] message;
}
void CMomentumLobbySystem::SetAppearanceInMemberData(ghostAppearance_t app)
{
    if (LobbyValid())
    {
        char base64Appearance[1024];
        base64_encode(&app, sizeof app, base64Appearance, 1024);
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_APPEARANCE, base64Appearance);
    }
}
LobbyGhostAppearance_t CMomentumLobbySystem::GetAppearanceFromMemberData(const CSteamID &member)
{
    LobbyGhostAppearance_t toReturn;
    const char *pAppearance = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, LOBBY_DATA_APPEARANCE);
    Q_strncpy(toReturn.base64, pAppearance, sizeof(toReturn.base64));
    if (!FStrEq(pAppearance, ""))
    {
        ghostAppearance_t newAppearance;
        base64_decode(pAppearance, &newAppearance, sizeof(ghostAppearance_t));
        toReturn.appearance = newAppearance;
    }
    return toReturn;
}

CMomentumOnlineGhostEntity* CMomentumLobbySystem::GetLobbyMemberEntity(const uint64 &id)
{
    uint16 findIndx = m_mapLobbyGhosts.Find(id);
    if (findIndx != m_mapLobbyGhosts.InvalidIndex())
        return m_mapLobbyGhosts[findIndx];
    
    return nullptr;
}

void CMomentumLobbySystem::ClearCurrentGhosts(bool bRemoveEnts)
{
    // We have to remove every entity manually if we left this lobby
    if (m_mapLobbyGhosts.Count() > 0)
    {
        if (bRemoveEnts)
        {
            unsigned short currIndx = m_mapLobbyGhosts.FirstInorder();
            while (currIndx != m_mapLobbyGhosts.InvalidIndex())
            {
                CMomentumOnlineGhostEntity *pEnt = m_mapLobbyGhosts[currIndx];
                if (pEnt)
                    pEnt->Remove();

                currIndx = m_mapLobbyGhosts.NextInorder(currIndx);
            }
        }

        m_mapLobbyGhosts.RemoveAll(); // No need to purge, the game handles the entities' memory
    }
}

void CMomentumLobbySystem::SendPacket(MomentumPacket_t *packet, CSteamID *pTarget, EP2PSend sendType /* = k_EP2PSendUnreliable*/)
{
    // Write the packet out to binary
    CUtlBuffer buf(0, 1200);
    buf.SetBigEndian(false);
    packet->Write(buf);

    if (pTarget)
    {
        if (steamapicontext->SteamNetworking()->SendP2PPacket(*pTarget, buf.Base(), buf.TellPut(), sendType))
        {
            // DevLog("Sent the packet!\n");
        }
    }
    else if (m_mapLobbyGhosts.Count() > 0) // It's everybody
    {
        uint16 index = m_mapLobbyGhosts.FirstInorder();
        while (index != m_mapLobbyGhosts.InvalidIndex())
        {
            CSteamID ghost = m_mapLobbyGhosts[index]->GetGhostSteamID();

            if (steamapicontext->SteamNetworking()->SendP2PPacket(ghost, buf.Base(), buf.TellPut(), sendType))
            {
                // DevLog("Sent the packet!\n");
            }

            index = m_mapLobbyGhosts.NextInorder(index);
        }
    }
}

void CMomentumLobbySystem::WriteMessage(LOBBY_MSG_TYPE type, uint64 pID_int)
{
    if (CMomentumGhostClient::m_pPlayer)
    {
        CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "LobbyUpdateMsg");
        WRITE_BYTE(type);
        WRITE_BYTES(&pID_int, sizeof(uint64));
        MessageEnd();
    }
}

void CMomentumLobbySystem::WriteMessage(SPECTATE_MSG_TYPE type, uint64 playerID, uint64 ghostID)
{
    if (CMomentumGhostClient::m_pPlayer)
    {
        CSingleUserRecipientFilter user(CMomentumGhostClient::m_pPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "SpecUpdateMsg");
        WRITE_BYTE(type);
        WRITE_BYTES(&playerID, sizeof(uint64));
        WRITE_BYTES(&ghostID, sizeof(uint64));
        MessageEnd();
    }
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
        DevLog("A user just joined us!\n");
        // Note: The lobby data update method handles adding

        WriteMessage(LOBBY_UPDATE_MEMBER_JOIN, pParam->m_ulSteamIDUserChanged);
    }
    if (state & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
    {
        DevLog("User left/disconnected!\n");

        uint16 findMember = m_mapLobbyGhosts.Find(changedPerson.ConvertToUint64());
        if (findMember != m_mapLobbyGhosts.InvalidIndex())
        {
            // Remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = m_mapLobbyGhosts[findMember];
            if (pEntity)
                pEntity->Remove();

            m_mapLobbyGhosts.RemoveAt(findMember);
        }

        WriteMessage(LOBBY_UPDATE_MEMBER_LEAVE, pParam->m_ulSteamIDUserChanged);
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
        DevLog("Setting the map to %s!\n", pMapName ? pMapName : "INVALID (main menu/loading)");
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, pMapName);
        SetGameInfoStatus();
        m_flNextUpdateTime = -1.0f;

        // Now check if this map is the same as somebody else's in the lobby
        if (pMapName && !FStrEq(pMapName, ""))
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
        const char *pName = steamapicontext->SteamFriends()->GetFriendPersonaName(*pID);

        // Check if this person was block communication'd
        EFriendRelationship relationship = steamapicontext->SteamFriends()->GetFriendRelationship(*pID);
        if (relationship == k_EFriendRelationshipIgnored || relationship == k_EFriendRelationshipIgnoredFriend)
        {
            DevLog("Not allowing %s to talk with us, we have them ignored!\n", pName);
            m_vecBlocked.AddToTail(*pID);
            return;
        }

        uint64 pID_int = pID->ConvertToUint64();

        unsigned short findIndx = m_mapLobbyGhosts.Find(pID_int);
        bool validIndx = findIndx != m_mapLobbyGhosts.InvalidIndex();
        
        const char *pMapName = gpGlobals->mapname.ToCStr();
        const char *pOtherMap = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, *pID, LOBBY_DATA_MAP);

        if (pMapName && pMapName[0] && pOtherMap && FStrEq(pMapName, pOtherMap)) //We're on the same map
        {
            // Don't add them again if they reloaded this map for some reason
            if (!validIndx)
            {
                CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                newPlayer->SetGhostSteamID(*pID);
                newPlayer->SetGhostName(pName);
                newPlayer->Spawn();
                newPlayer->SetGhostAppearance(GetAppearanceFromMemberData(*pID), true); // Appearance after spawn!

                bool isSpectating = GetIsSpectatingFromMemberData(*pID);

                // Spawn but hide them 
                if (isSpectating)
                {
                    newPlayer->m_bSpectating = true;
                    newPlayer->HideGhost();
                }

                m_mapLobbyGhosts.Insert(pID_int, newPlayer);

                if (m_flNextUpdateTime < 0)
                    m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());

                // "_____ just joined your map."
                WriteMessage(LOBBY_UPDATE_MEMBER_JOIN_MAP, pID_int);
            }
        }
        else if (validIndx)
        {
            // They changed map remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = m_mapLobbyGhosts[findIndx];
            if (pEntity)
                pEntity->Remove();
            
            m_mapLobbyGhosts.RemoveAt(findIndx);

            // "_____ just left your map."
            WriteMessage(LOBBY_UPDATE_MEMBER_LEAVE_MAP, pID_int);
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
            Warning("You are already in a lobby! Do \"mom_lobby_leave\" to exit it!\n");
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
    if (m_mapLobbyGhosts.Count() > 0)
    {
        // Read data
        uint32 size;
        while (steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&size))
        {
            // Read the packet's data
            uint8 *bytes = new uint8[size];
            uint32 bytesRead;
            CSteamID fromWho;
            steamapicontext->SteamNetworking()->ReadP2PPacket(bytes, size, &bytesRead, &fromWho);
            
            // Throw the data into a manageable reader
            CUtlBuffer buf(bytes, size, CUtlBuffer::READ_ONLY);
            buf.SetBigEndian(false);
            
            // Determine what type it is
            uint8 type = buf.GetUnsignedChar();
            switch (type)
            {
            case PT_POS_DATA: // Position update frame
                {
                    PositionPacket_t frame(buf);
                    CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(fromWho);
                    if (pEntity)
                        pEntity->AddPositionFrame(frame);
                }
                break;
            case PT_DECAL_DATA:
                {
                    DecalPacket_t decals(buf);
                    CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(fromWho);
                    if (pEntity)
                    {
                        pEntity->AddDecalFrame(decals);
                    }
                }
                break;
            case PT_SPEC_UPDATE:
                {
                    SpecUpdatePacket_t update(buf);
                    uint64 fromWhoID = fromWho.ConvertToUint64(), specTargetID = update.specTarget;

                    CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(fromWho);
                    if (pEntity)
                    {
                        pEntity->m_bSpectating = update.specTarget != 0;
                        update.specTarget != 0 ? pEntity->HideGhost() : pEntity->UnHideGhost();
                    }

                    // Write it out to the Hud Chat
                    WriteMessage(update.spec_type, fromWhoID, specTargetID);
                }
                break;
            default:
                break;
            }

            // Clear the buffer and free the memory
            buf.Purge();
            delete[] bytes;
        }

        // Send position data
        if (m_flNextUpdateTime > 0 && gpGlobals->curtime > m_flNextUpdateTime)
        {
            PositionPacket_t frame;
            if (g_pMomentumGhostClient->CreateNewNetFrame(frame))
            {
                SendPacket(&frame);
                m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());
            }
        }
    }
}
void CMomentumLobbySystem::SetIsSpectating(bool bSpec)
{
    steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_IS_SPEC, bSpec ? "1" : nullptr);
}

//Return true if the lobby member is currently spectating.
bool CMomentumLobbySystem::GetIsSpectatingFromMemberData(const CSteamID &who)
{
    const char* specChar = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, who, LOBBY_DATA_IS_SPEC);
    return specChar[0] ? true : false;
}

void CMomentumLobbySystem::SendDecalPacket(DecalPacket_t *packet)
{
    if (LobbyValid())
        SendPacket(packet);
}

void CMomentumLobbySystem::SetSpectatorTarget(const CSteamID &ghostTarget, bool bStartedSpectating)
{
    SPECTATE_MSG_TYPE type;
    if (bStartedSpectating)
    {
        type = SPEC_UPDATE_JOIN;
        SetIsSpectating(true);
    }
    else if (!ghostTarget.IsValid())
    {
        type = SPEC_UPDATE_LEAVE;
        SetIsSpectating(false);
    }
    else
        type = SPEC_UPDATE_CHANGETARGET;

    // MOM_TODO: Keep me for updating the client
    if (type != SPEC_UPDATE_LEAVE)
    {
        char steamID[64];
        Q_snprintf(steamID, 64, "%llu", ghostTarget.ConvertToUint64());
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_SPEC_TARGET, steamID);
    }
    else
    {
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_SPEC_TARGET, nullptr);
    }
    
    SendSpectatorUpdatePacket(ghostTarget, type);
}
//Sends the spectator info update packet to all current ghosts
void CMomentumLobbySystem::SendSpectatorUpdatePacket(const CSteamID &ghostTarget, SPECTATE_MSG_TYPE type)
{
    SpecUpdatePacket_t newUpdate(ghostTarget.ConvertToUint64(), type);
    SendPacket(&newUpdate, nullptr, k_EP2PSendReliable);

    uint64 playerID = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
    uint64 ghostID = ghostTarget.ConvertToUint64();
    WriteMessage(type, playerID, ghostID);
}

void CMomentumLobbySystem::SetGameInfoStatus()
{
    ConVarRef gm("mom_gamemode");
    const char *gameMode;
    switch (gm.GetInt())
    {
    case MOMGM_SURF:
        gameMode = "Surfing";
        break;
    case MOMGM_BHOP:
        gameMode = "Bhopping";
        break;
    case MOMGM_SCROLL:
        gameMode = "Scrolling";
        break;
    case MOMGM_UNKNOWN:
    default:
        gameMode = "Playing";
        break;
    }
    char gameInfoStr[64];// , connectStr[64];
    int numPlayers = steamapicontext->SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
    V_snprintf(gameInfoStr, 64, numPlayers < 1 ? "%s on %s" : "%s on %s with %i other player%s", gameMode, gpGlobals->mapname, numPlayers - 1, numPlayers > 2 ? "s" : "");
    //V_snprintf(connectStr, 64, "+connect_lobby %llu +map %s", m_sLobbyID, gpGlobals->mapname);

    //steamapicontext->SteamFriends()->SetRichPresence("connect", connectStr);
    steamapicontext->SteamFriends()->SetRichPresence("status", gameInfoStr);
}
static CMomentumLobbySystem s_MOMLobbySystem;
CMomentumLobbySystem *g_pMomentumLobbySystem = &s_MOMLobbySystem;