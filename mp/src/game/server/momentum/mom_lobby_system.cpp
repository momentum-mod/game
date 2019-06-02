#include "cbase.h"

#include "mom_lobby_system.h"

#include "filesystem.h"
#include "ghost_client.h"
#include "mom_online_ghost.h"
#include "mom_system_saveloc.h"
#include "mom_player_shared.h"
#include "mom_modulecomms.h"
#include "mom_timer.h"
#include "fmtstr.h"

#include "tier0/valve_minmax_off.h"
// This is wrapped by minmax_off due to Valve making a macro for min and max...
#include <cryptopp/base64.h>
// Now we can unwrap
#include "tier0/valve_minmax_on.h"

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
    if (args.ArgC() < 2)
    {
        Log("Usage: connect_lobby <lobby_id>");
        return;
    }
    g_pMomentumLobbySystem->TryJoinLobbyFromString(args.Arg(1));
}

CON_COMMAND(mom_lobby_invite, "Invite friends to your lobby\n")
{
    if (g_pMomentumLobbySystem->LobbyValid())
        SteamFriends()->ActivateGameOverlayInviteDialog(g_pMomentumLobbySystem->GetLobbyId());
}

CON_COMMAND(mom_lobby_teleport, "Teleport to a given lobby member's SteamID on your map.")
{
    if (args.ArgC() >= 2)
        g_pMomentumLobbySystem->TeleportToLobbyMember(args.Arg(1));
}

static void LobbyMaxPlayersChanged(IConVar *pVar, const char *pVal, float oldVal)
{
    g_pMomentumLobbySystem->OnLobbyMaxPlayersChanged(ConVarRef(pVar).GetInt());
}

static void LobbyTypeChanged(IConVar *pVar, const char *pVal, float oldVal)
{
    g_pMomentumLobbySystem->OnLobbyTypeChanged(ConVarRef(pVar).GetInt());
}

static MAKE_CONVAR_C(mom_lobby_max_players, "16", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the maximum number of players allowed in lobbies you create.\n", 2, 250, LobbyMaxPlayersChanged);
static MAKE_CONVAR_C(mom_lobby_type, "1", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the type of the lobby. 0 = Invite only, 1 = Friends Only, 2 = Public\n", 0, 2, LobbyTypeChanged);

// So basically, if a user wants to connect to us, we're considered the host. 
void CMomentumLobbySystem::HandleNewP2PRequest(P2PSessionRequest_t* info)
{
    const char *pName = SteamFriends()->GetFriendPersonaName(info->m_steamIDRemote);

    // MOM_TODO: Make a (temp) block list that only refreshes on game restart?
    if (m_vecBlocked.Find(info->m_steamIDRemote) != -1)
    {
        DevLog("Not allowing %s to talk with us, we've marked them as blocked!\n", pName);
        return;
    }

    // MOM_TODO: Take into account that this user could potentially not be in our lobby (security)

    // Needs to be done to open the connection with them
    SteamNetworking()->AcceptP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumLobbySystem::HandleP2PConnectionFail(P2PSessionConnectFail_t* info)
{
    const char *pName = SteamFriends()->GetFriendPersonaName(info->m_steamIDRemote);
    if (info->m_eP2PSessionError == k_EP2PSessionErrorTimeout)
        DevLog("Dropping connection with %s due to timing out! (They probably left/disconnected)\n", pName);
    else
        Warning("Steam P2P failed with user %s because of the error: %i\n", pName, info->m_eP2PSessionError);
    
    SteamNetworking()->CloseP2PSessionWithUser(info->m_steamIDRemote);
}

void CMomentumLobbySystem::SendChatMessage(char* pMessage)
{
    if (LobbyValid())
    {
        CHECK_STEAM_API(SteamMatchmaking());
        int len = Q_strlen(pMessage) + 1;
        bool result = SteamMatchmaking()->SendLobbyChatMsg(m_sLobbyID, pMessage, len);
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
                pEntity->SetLobbyGhostAppearance(pEntity->GetLobbyGhostAppearance(), true);

            index = m_mapLobbyGhosts.NextInorder(index);
        }
    }
}

bool CMomentumLobbySystem::SendSavelocReqPacket(CSteamID& target, SavelocReqPacket_t* p)
{
    return LobbyValid() && SendPacket(p, &target, k_EP2PSendReliable);
}

void CMomentumLobbySystem::TeleportToLobbyMember(const char *pIDStr)
{
    // Check a few things first
    CHECK_STEAM_API(SteamMatchmaking());

    const auto lobbyMemID = CSteamID(Q_atoui64(pIDStr));

    // Are they valid, and even in the lobby?
    if (lobbyMemID.IsValid() && LobbyValid())
    {
        const auto pEnt = GetLobbyMemberEntity(lobbyMemID);
        if (pEnt)
        {
            // Ok cool, but...
            // Are we spectating or in a run?
            const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
            if (pPlayer && pPlayer->GetObserverMode() == OBS_MODE_NONE && !g_pMomentumTimer->IsRunning())
            {
                // Teleport em
                PositionPacket_t p;
                if (pEnt->GetCurrentPositionPacketData(&p))
                {
                    pPlayer->Teleport(&p.Position, &p.EyeAngle, nullptr);
                }
            }
            else
            {
                Warning("Cannot teleport to player while spectating or in a run!\n");
            }
        }
    }
}

// Called when trying to join somebody else's lobby. We need to actually call JoinLobby here.
void CMomentumLobbySystem::HandleLobbyJoin(GameLobbyJoinRequested_t* pJoin)
{
    TryJoinLobby(pJoin->m_steamIDLobby);
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

        SteamMatchmaking()->SetLobbyData(m_sLobbyID, LOBBY_DATA_TYPE, CFmtStrN<10>("%i", mom_lobby_type.GetInt()));
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
        CHECK_STEAM_API(SteamMatchmaking());
        SteamAPICall_t call = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>(mom_lobby_type.GetInt()), mom_lobby_max_players.GetInt());
        m_cLobbyCreated.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyCreated);
        DevLog("The lobby call successfully happened!\n");
    }
    else
        Warning("The lobby could not be created because you already made one or are in one!\n");
}

void CMomentumLobbySystem::LeaveLobby()
{
    if (LobbyValid())
    {
        // Actually leave the lobby
        SteamMatchmaking()->LeaveLobby(m_sLobbyID);
        // Clear the ghosts stored in our lobby system
        g_pMomentumGhostClient->ClearCurrentGhosts(true);
        // Clear out any rich presence 
        SteamFriends()->ClearRichPresence();

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
        Warning("Failed to enter chat room! Error code: %i\n", pEnter->m_EChatRoomEnterResponse);
    }
    else
    {
        Log("Lobby entered! Lobby ID: %lld\n", pEnter->m_ulSteamIDLobby);

        if (!m_sLobbyID.IsValid())
        {
            m_sLobbyID = CSteamID(pEnter->m_ulSteamIDLobby);
        }

        FIRE_GAME_WIDE_EVENT("lobby_join");

        // Set our own data
        SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, gpGlobals->mapname.ToCStr());
        // Note: Our appearance is also set on spawn, so no worries if we're null here.
        CMomentumPlayer *pPlayer = CMomentumPlayer::GetLocalPlayer();
        if (pPlayer)
        {
            SetAppearanceInMemberData(pPlayer->m_playerAppearanceProps);
        }
        SetGameInfoStatus();
        // Get everybody else's data
        CheckToAdd(nullptr);
    }
}

// We got a message yaay
void CMomentumLobbySystem::HandleLobbyChatMsg(LobbyChatMsg_t* pParam)
{
    // MOM_TODO: Keep this for if we ever end up using binary messages 

    char *message = new char[4096];
    int written = SteamMatchmaking()->GetLobbyChatEntry(CSteamID(pParam->m_ulSteamIDLobby), pParam->m_iChatID, nullptr, message, 4096, nullptr);
    DevLog("SERVER: Got a chat message! Wrote %i byte(s) into buffer.\n", written);
    Msg("SERVER: Chat message: %s\n", message);
    delete[] message;
}
void CMomentumLobbySystem::SetAppearanceInMemberData(GhostAppearance_t app)
{
    if (LobbyValid())
    {
        CHECK_STEAM_API(SteamMatchmaking());
        std::string base64Appearance;

        CryptoPP::StringSource ss(reinterpret_cast<unsigned char *>(&app), 
                                  sizeof(GhostAppearance_t), 
                                  true,
                                  new CryptoPP::Base64Encoder(new CryptoPP::StringSink(base64Appearance))
        );

        SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_APPEARANCE, base64Appearance.c_str());
    }
}
bool CMomentumLobbySystem::GetAppearanceFromMemberData(const CSteamID &member, LobbyGhostAppearance_t &out)
{
    bool toReturn = false;
    const char *pAppearance = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, LOBBY_DATA_APPEARANCE);
    if (!FStrEq(pAppearance, ""))
    {
        Q_strncpy(out.base64, pAppearance, sizeof(out.base64));

        std::string encoded(pAppearance);

        CryptoPP::Base64Decoder decoder;
        decoder.Put((byte*)encoded.data(), encoded.size());
        decoder.MessageEnd();

        GhostAppearance_t newAppearance;

        const auto size = decoder.MaxRetrievable();
        if (size && size == sizeof(GhostAppearance_t))
        {
            decoder.Get((byte*)&newAppearance, sizeof(GhostAppearance_t));
            out.appearance = newAppearance;
            toReturn = true;
        }
    }
    return toReturn;
}

CMomentumOnlineGhostEntity* CMomentumLobbySystem::GetLobbyMemberEntity(const uint64 &id)
{
    const auto findIndx = m_mapLobbyGhosts.Find(id);
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

bool CMomentumLobbySystem::SendPacket(MomentumPacket_t *packet, CSteamID *pTarget, EP2PSend sendType /* = k_EP2PSendUnreliable*/)
{
    CHECK_STEAM_API_B(SteamNetworking());

    if (!pTarget && m_mapLobbyGhosts.Count() == 0)
        return false;

    // Write the packet out to binary
    CUtlBuffer buf;
    buf.SetBigEndian(false);
    packet->Write(buf);

    if (pTarget)
    {
        if (SteamNetworking()->SendP2PPacket(*pTarget, buf.Base(), buf.TellPut(), sendType))
        {
            return true;
        }
    }
    else if (m_mapLobbyGhosts.Count() > 0) // It's everybody
    {
        auto index = m_mapLobbyGhosts.FirstInorder();
        while (index != m_mapLobbyGhosts.InvalidIndex())
        {
            const auto ghostID = m_mapLobbyGhosts[index]->GetGhostSteamID();

            if (!SteamNetworking()->SendP2PPacket(ghostID, buf.Base(), buf.TellPut(), sendType))
            {
                DevWarning("Failed to send the packet to %s!\n", SteamFriends()->GetFriendPersonaName(ghostID));
            }

            index = m_mapLobbyGhosts.NextInorder(index);
        }
        return true;
    }

    return false;
}

void CMomentumLobbySystem::WriteLobbyMessage(LobbyMessageType_t type, uint64 pID_int)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        CSingleUserRecipientFilter user(pPlayer);
        user.MakeReliable();
        UserMessageBegin(user, "LobbyUpdateMsg");
        WRITE_BYTE(type);
        WRITE_BYTES(&pID_int, sizeof(uint64));
        MessageEnd();
    }
}

void CMomentumLobbySystem::WriteSpecMessage(SpectateMessageType_t type, uint64 playerID, uint64 ghostID)
{
    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (pPlayer)
    {
        CSingleUserRecipientFilter user(pPlayer);
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
            if (memberChanged == SteamUser()->GetSteamID())
                return;
            
            // Check their appearance for any changes
            CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(memberChanged);
            if (pEntity)
            {
                LobbyGhostAppearance_t appear;
                if (GetAppearanceFromMemberData(memberChanged, appear))
                    pEntity->SetLobbyGhostAppearance(appear);
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

        WriteLobbyMessage(LOBBY_UPDATE_MEMBER_JOIN, pParam->m_ulSteamIDUserChanged);
    }
    if (state & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
    {
        DevLog("User left/disconnected!\n");

        // Check if they're a saveloc requester
        g_pMOMSavelocSystem->RequesterLeft(changedPerson.ConvertToUint64());

        uint16 findMember = m_mapLobbyGhosts.Find(changedPerson.ConvertToUint64());
        if (findMember != m_mapLobbyGhosts.InvalidIndex())
        {
            // Remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = m_mapLobbyGhosts[findMember];
            if (pEntity)
                pEntity->Remove();

            m_mapLobbyGhosts.RemoveAt(findMember);
        }

        WriteLobbyMessage(LOBBY_UPDATE_MEMBER_LEAVE, pParam->m_ulSteamIDUserChanged);
    }
}

void CMomentumLobbySystem::HandlePersonaCallback(PersonaStateChange_t* pParam)
{
    //DevLog("HandlePersonaCallback: %u with changeflags: %i\n", pParam->m_ulSteamID, pParam->m_nChangeFlags);
    CSteamID person = CSteamID(pParam->m_ulSteamID);
    if (pParam->m_nChangeFlags & k_EPersonaChangeName && LobbyValid())
    {
        // Quick and ugly check to see if they're in our lobby
        const char *pCheck = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, person, LOBBY_DATA_MAP);
        if (pCheck)
        {
            // It's not null so they're here, but are they in our map?
            CMomentumOnlineGhostEntity *pGhost = GetLobbyMemberEntity(pParam->m_ulSteamID);
            if (pGhost)
            {
                // Yes they are
                const char *pName = SteamFriends()->GetFriendPersonaName(person);
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
        CHECK_STEAM_API(SteamMatchmaking());
        DevLog("Setting the map to %s!\n", pMapName ? pMapName : "INVALID (main menu/loading)");
        SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, pMapName);
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
    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());

    CSteamID localID = SteamUser()->GetSteamID();

    if (pID)
    {
        CHECK_STEAM_API(SteamFriends());
        const char *pName = SteamFriends()->GetFriendPersonaName(*pID);

        // Check if this person was block communication'd
        EFriendRelationship relationship = SteamFriends()->GetFriendRelationship(*pID);
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
        const char *pOtherMap = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, *pID, LOBBY_DATA_MAP);

        if (pMapName && pMapName[0] && pOtherMap && FStrEq(pMapName, pOtherMap)) //We're on the same map
        {
            // Don't add them again if they reloaded this map for some reason, or if we're in background/credits
            if (!validIndx && gpGlobals->eLoadType != MapLoad_Background && !FStrEq(pMapName, "credits"))
            {
                CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                newPlayer->SetGhostSteamID(*pID);
                newPlayer->SetGhostName(pName);
                newPlayer->Spawn();
                LobbyGhostAppearance_t appear;
                if (GetAppearanceFromMemberData(*pID, appear))
                    newPlayer->SetLobbyGhostAppearance(appear, true); // Appearance after spawn!

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
                WriteLobbyMessage(LOBBY_UPDATE_MEMBER_JOIN_MAP, pID_int);
            }
        }
        else if (validIndx)
        {
            // They changed map remove their entity from the CUtlMap
            CMomentumOnlineGhostEntity *pEntity = m_mapLobbyGhosts[findIndx];
            if (pEntity)
            {
                pEntity->UpdatePlayerSpectate();
                pEntity->Remove();
            }
            
            m_mapLobbyGhosts.RemoveAt(findIndx);

            // Remove them if they're a requester
            g_pMOMSavelocSystem->RequesterLeft(pID_int);

            // "_____ just left your map."
            WriteLobbyMessage(LOBBY_UPDATE_MEMBER_LEAVE_MAP, pID_int);
        }
    }
    else
    {
        int numMembers = SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
        for (int i = 0; i < numMembers; i++)
        {
            CSteamID member = SteamMatchmaking()->GetLobbyMemberByIndex(m_sLobbyID, i);
            if (member == localID) // If it's us, don't care
                continue;

            CheckToAdd(&member);
        }
    }
}

bool CMomentumLobbySystem::TryJoinLobby(const CSteamID &lobbyID)
{
    CHECK_STEAM_API_B(SteamMatchmaking());

    if (m_sLobbyID == lobbyID)
    {
        Warning("Already in this lobby!");
        return false;
    }
    if (m_cLobbyJoined.IsActive())
    {
        Warning("You are already trying to join a lobby!\n");
        return false;
    }
    if (m_sLobbyID.IsValid() && m_sLobbyID.IsLobby())
    {
        Warning("You are already in a lobby! Do \"mom_lobby_leave\" to exit it!\n");
        return false;
    }

    SteamAPICall_t call = SteamMatchmaking()->JoinLobby(lobbyID);
    m_cLobbyJoined.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyJoined);

    return true;
}

bool CMomentumLobbySystem::TryJoinLobbyFromString(const char* pString)
{
    if (!pString)
        return false;

    DevLog("Trying to join the lobby from the string %s!\n", pString);
    uint64 steamID = Q_atoui64(pString);
    if (steamID > 0)
    {
        CSteamID toJoin;
        toJoin.FullSet(steamID, k_EUniversePublic, k_EAccountTypeChat);
        DevLog("Got the ID! %lld\n", toJoin.ConvertToUint64());

        return TryJoinLobby(toJoin);
    }

    Warning("Could not join lobby due to malformed ID '%s'!\n", pString);
    return false;
}

void CMomentumLobbySystem::SendAndReceiveP2PPackets()
{
    if (m_mapLobbyGhosts.Count() > 0)
    {
        // Read data
        uint32 size;
        while (SteamNetworking()->IsP2PPacketAvailable(&size))
        {
            // Read the packet's data
            uint8 *bytes = new uint8[size];
            uint32 bytesRead;
            CSteamID fromWho;
            SteamNetworking()->ReadP2PPacket(bytes, size, &bytesRead, &fromWho);
            
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
                    WriteSpecMessage(update.spec_type, fromWhoID, specTargetID);
                }
                break;

            case PT_SAVELOC_REQ:
                {
                    SavelocReqPacket_t saveloc(buf);

                    // Done/fail states:
                    // 1. They hit "cancel" (most common)
                    // 2. They leave the map (same as 1, just accidental maybe)
                    // 3. They leave the lobby/server (manually, due to power outage, etc)
                    // 4. We leave the map
                    // 5. We leave the lobby/server
                    // 6. They get the savelocs they need

                    // Of the above, 1 and 6 are the ones that are manually sent.
                    // 2<->5 can be automatically detected with lobby/server hooks

                    // Fail requirements:
                    // Requester: set "requesting" to false, close the request UI
                    // Requestee: remove requester from requesters vector

                    switch (saveloc.stage)
                    {
                    case 0:
                    default:
                        DevWarning("Invalid stage for the saveloc request packet!\n");
                        break;
                    case 1:
                        {
                            DevLog(2, "Received a stage 1 saveloc request packet!\n");
                            // Somebody wants our savelocs, let the saveloc system handle this
                            g_pMOMSavelocSystem->AddSavelocRequester(fromWho.ConvertToUint64());

                            // Send them our saveloc count
                            SavelocReqPacket_t response;
                            response.stage = 2;
                            response.saveloc_count = g_pMOMSavelocSystem->GetSavelocCount();

                            SendPacket(&response, &fromWho, k_EP2PSendReliable);
                        }
                        break;
                    case 2:
                        {
                            DevLog(2, "Received a stage 2 saveloc request packet!\n");
                            // We got the number of savelocs, pass this to the client
                            KeyValues *pKV = new KeyValues("req_savelocs");
                            pKV->SetInt("stage", 2);
                            pKV->SetInt("count", saveloc.saveloc_count);
                            g_pModuleComms->FireEvent(pKV);
                        }
                        break;
                    case 3:
                        {
                            DevLog(2, "Received a stage 3 saveloc request packet!\n");
                            // Somebody sent us the number of the savelocs they want, saveloc system pls help
                            SavelocReqPacket_t response;
                            response.stage = 4;

                            if (g_pMOMSavelocSystem->FillSavelocReq(true, &saveloc, &response))
                                SendPacket(&response, &fromWho, k_EP2PSendReliable);
                        }
                        break;
                    case 4:
                        {
                            DevLog(2, "Received a stage 4 saveloc request packet!\n");
                            // We got their savelocs, add it to the player's list of savelocs
                            if (g_pMOMSavelocSystem->FillSavelocReq(false, &saveloc, nullptr))
                            {
                                // Send them a packet that we're all good
                                SavelocReqPacket_t response;
                                response.stage = -1;
                                if (SendPacket(&response, &fromWho, k_EP2PSendReliable))
                                {
                                    // Send ourselves an event saying we're all good
                                    KeyValues *pKv = new KeyValues("req_savelocs");
                                    pKv->SetInt("stage", -1);
                                    g_pModuleComms->FireEvent(pKv);
                                }
                            }
                        }
                        break;
                    case -1: // The other player is all done/cancelled
                        {
                            // Remove the requester
                            DevLog(2, "Received a stage -1 saveloc request packet!\n");
                            g_pMOMSavelocSystem->RequesterLeft(fromWho.ConvertToUint64());
                        }
                        break;
                    }
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
            if (g_pMomentumGhostClient->CreateNewNetFrame(frame) && SendPacket(&frame))
            {
                m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());
            }
        }
    }
}
void CMomentumLobbySystem::SetIsSpectating(bool bSpec)
{
    CHECK_STEAM_API(SteamMatchmaking());
    if (LobbyValid())
        SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_IS_SPEC, bSpec ? "1" : nullptr);
}

//Return true if the lobby member is currently spectating.
bool CMomentumLobbySystem::GetIsSpectatingFromMemberData(const CSteamID &who)
{
    CHECK_STEAM_API_B(SteamMatchmaking());
    const char* specChar = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, who, LOBBY_DATA_IS_SPEC);
    return specChar[0] ? true : false;
}

bool CMomentumLobbySystem::SendDecalPacket(DecalPacket_t *packet)
{
    return LobbyValid() && SendPacket(packet);
}

void CMomentumLobbySystem::SetSpectatorTarget(const CSteamID &ghostTarget, bool bStartedSpectating, bool bLeft)
{
    CHECK_STEAM_API(SteamMatchmaking());

    SpectateMessageType_t type;
    if (bStartedSpectating)
    {
        type = SPEC_UPDATE_JOIN;
    }
    else if (!ghostTarget.IsValid() && ghostTarget.ConvertToUint64() != 1)
    {
        type = bLeft ? SPEC_UPDATE_LEAVE : SPEC_UPDATE_STOP;
    }
    else
        type = SPEC_UPDATE_CHANGETARGET;

    // MOM_TODO: Keep me for updating the client
    if (type == SPEC_UPDATE_STOP || type == SPEC_UPDATE_LEAVE)
    {
        SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_SPEC_TARGET, nullptr);
    }
    else
    {
        char steamID[64];
        Q_snprintf(steamID, 64, "%llu", ghostTarget.ConvertToUint64());
        SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_SPEC_TARGET, steamID);
    }
    
    SendSpectatorUpdatePacket(ghostTarget, type);
}
//Sends the spectator info update packet to all current ghosts
void CMomentumLobbySystem::SendSpectatorUpdatePacket(const CSteamID &ghostTarget, SpectateMessageType_t type)
{
    SpecUpdatePacket_t newUpdate(ghostTarget.ConvertToUint64(), type);
    if (SendPacket(&newUpdate, nullptr, k_EP2PSendReliable))
    {
        uint64 playerID = SteamUser()->GetSteamID().ConvertToUint64();
        uint64 ghostID = ghostTarget.ConvertToUint64();
        WriteSpecMessage(type, playerID, ghostID);
    }
}

void CMomentumLobbySystem::OnLobbyMaxPlayersChanged(int newMax)
{
    // We can only change while in a lobby if we're the lobby owner
    if (LobbyValid())
    {
        CHECK_STEAM_API(SteamUser());
        const auto pLocID = SteamUser()->GetSteamID().ConvertToUint64();
        if (SteamMatchmaking()->GetLobbyOwner(m_sLobbyID).ConvertToUint64() == pLocID)
        {
            // Change the lobby type to this type
            newMax = clamp<int>(newMax, 2, 250);
            if (SteamMatchmaking()->SetLobbyMemberLimit(m_sLobbyID, newMax))
                Log("Successfully changed the maximum player count to %i!\n", newMax);
        }
        else
            Warning("Cannot change the lobby max players; you are not the lobby owner!\n");
    }
    // else the lobby isn't valid, but it'll apply to our next one!
}

void CMomentumLobbySystem::OnLobbyTypeChanged(int newType)
{
    // We can only change while in a lobby if we're the lobby owner
    if (LobbyValid())
    {
        CHECK_STEAM_API(SteamUser());
        const auto pLocID = SteamUser()->GetSteamID().ConvertToUint64();
        if (SteamMatchmaking()->GetLobbyOwner(m_sLobbyID).ConvertToUint64() == pLocID)
        {
            // Change the lobby type to this type
            newType = clamp<int>(newType, k_ELobbyTypePrivate, k_ELobbyTypePublic);
            if (SteamMatchmaking()->SetLobbyType(m_sLobbyID, (ELobbyType)newType))
            {
                if (SteamMatchmaking()->SetLobbyData(m_sLobbyID, LOBBY_DATA_TYPE, CFmtStrN<10>("%i", newType).Get()))
                   Log("Successfully changed the lobby type to %i!\n", newType);
            }
        }
        else
            Warning("Cannot change the lobby type; you are not the lobby owner!\n");
    }
    // else the lobby isn't valid, but it'll apply to our next one!
}


void CMomentumLobbySystem::SetGameInfoStatus()
{
    CHECK_STEAM_API(SteamFriends());
    ConVarRef gm("mom_gamemode");
    const char *gameMode;
    switch (gm.GetInt())
    {
    case GAMEMODE_SURF:
        gameMode = "Surfing";
        break;
    case GAMEMODE_BHOP:
        gameMode = "Bhopping";
        break;
    case GAMEMODE_KZ:
        gameMode = "Climbing";
        break;
    case GAMEMODE_UNKNOWN:
    default:
        gameMode = "Playing";
        break;
    }
    char gameInfoStr[64];// , connectStr[64];
    int numPlayers = SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
    V_snprintf(gameInfoStr, sizeof(gameInfoStr), numPlayers <= 1 ? "%s on %s" : "%s on %s with %i other player%s",
               gameMode, STRING(gpGlobals->mapname), numPlayers - 1, numPlayers > 2 ? "s" : "");
    //V_snprintf(connectStr, 64, "+connect_lobby %llu +map %s", m_sLobbyID, gpGlobals->mapname);

    //SteamFriends()->SetRichPresence("connect", connectStr);
    SteamFriends()->SetRichPresence("status", gameInfoStr);
}
static CMomentumLobbySystem s_MOMLobbySystem;
CMomentumLobbySystem *g_pMomentumLobbySystem = &s_MOMLobbySystem;
