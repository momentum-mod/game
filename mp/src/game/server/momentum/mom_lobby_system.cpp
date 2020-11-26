#include "cbase.h"

#include "mom_lobby_system.h"

#include "filesystem.h"
#include "fmtstr.h"
#include "ghost_client.h"
#include "mom_online_ghost.h"
#include "mom_system_gamemode.h"
#include "mom_system_saveloc.h"
#include "mom_player_shared.h"
#include "mom_modulecomms.h"
#include "mom_timer.h"
#include "mom_system_steam_richpresence.h"
#include "util/mom_util.h"
#include "steam/isteamnetworkingmessages.h"

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

CON_COMMAND(mom_lobby_teleport, "Teleport to a given lobby member's SteamID (exact) or name (partial) on your map.\n")
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
static MAKE_TOGGLE_CONVAR(mom_lobby_debug, "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Toggles printing debug info about the lobby. 0 = OFF, 1 = ON\n");

void CMomentumLobbySystem::HandleNewP2PRequest(SteamNetworkingMessagesSessionRequest_t *pParam)
{
    const auto hUserID = pParam->m_identityRemote.GetSteamID();

    if (!IsInLobby(hUserID))
        return;

    if (MomUtil::IsSteamUserBlocked(hUserID.ConvertToUint64()) && !m_vecBlocked.HasElement(hUserID))
    {
        m_vecBlocked.AddToTail(hUserID);
    }

    if (m_vecBlocked.HasElement(hUserID))
    {
        const char *pName = SteamFriends()->GetFriendPersonaName(hUserID);
        DevLog("Not allowing %s to talk with us, we've marked them as blocked!\n", pName);
        return;
    }

    SteamNetworkingMessages()->AcceptSessionWithUser(pParam->m_identityRemote);
}

void CMomentumLobbySystem::HandleP2PConnectionFail(SteamNetworkingMessagesSessionFailed_t *pParam)
{
    if (mom_lobby_debug.GetBool())
    {
        const auto iEndReason = pParam->m_info.m_eEndReason;
        if (iEndReason >= k_ESteamNetConnectionEnd_AppException_Min && iEndReason <= k_ESteamNetConnectionEnd_AppException_Max)
        {
            Warning("P2P Connection failed due to an application problem! Error code: %i\n", iEndReason);
        }
        else if (iEndReason >= k_ESteamNetConnectionEnd_Local_Min && iEndReason <= k_ESteamNetConnectionEnd_Local_Max)
        {
            Warning("P2P Connection failed due to a local problem! Error code: %i\n", iEndReason);
        }
        else if (iEndReason >= k_ESteamNetConnectionEnd_Remote_Min && iEndReason <= k_ESteamNetConnectionEnd_Remote_Max)
        {
            Warning("P2P Connection failed due to a remote problem! Error code: %i\n", iEndReason);
        }
        else if (iEndReason >= k_ESteamNetConnectionEnd_Misc_Min && iEndReason <= k_ESteamNetConnectionEnd_Misc_Max)
        {
            Warning("P2P Connection failed due to a miscellaneous problem! Error code: %i\n", iEndReason);
        }
    }

    SteamNetworkingMessages()->CloseSessionWithUser(pParam->m_info.m_identityRemote);
}

void CMomentumLobbySystem::ResetOtherAppearanceData()
{
    if (!LobbyValid())
        return;

    auto index = m_mapLobbyGhosts.FirstInorder();
    while (index != m_mapLobbyGhosts.InvalidIndex())
    {
        const auto pEntity = m_mapLobbyGhosts[index];
        if (pEntity)
            pEntity->SetAppearanceData(*pEntity->GetAppearanceData(), true);

        index = m_mapLobbyGhosts.NextInorder(index);
    }
}

bool CMomentumLobbySystem::SendSavelocReqPacket(CSteamID& target, SavelocReqPacket* p)
{
    return LobbyValid() && SendPacket(p, target, k_nSteamNetworkingSend_Reliable);
}

void CMomentumLobbySystem::TeleportToLobbyMember(const char *pIDStr)
{
    CHECK_STEAM_API(SteamMatchmaking());

    if (!LobbyValid())
        return;

    const auto pPlayer = CMomentumPlayer::GetLocalPlayer();
    if (!pPlayer)
        return;

    if (g_pMomentumTimer->IsRunning())
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "You can only teleport to targets when your timer is not running!");
        return;
    }

    auto *pOtherEntity = GetLobbyMemberEntity(Q_atoui64(pIDStr));

    if (!pOtherEntity)
        pOtherEntity = GetLobbyMemberEntity(pIDStr);

    if (!pOtherEntity)
    {
        ClientPrint(pPlayer, HUD_PRINTTALK, "Failed to find valid teleport target!");
        return;
    }

    if (pPlayer->GetObserverMode() == OBS_MODE_NONE)
    {
        PositionPacket p;
        if (pOtherEntity->GetCurrentPositionPacketData(&p))
        {
            pPlayer->ManualTeleport(&p.Position, &p.EyeAngle, nullptr);
        }
    }
    else
    {
        pPlayer->TrySpectate(pIDStr);
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

    if (mom_lobby_debug.GetBool())
        Log("Lobby created call result! We got a result %i with a steam lobby: %lld\n", pCreated->m_eResult, pCreated->m_ulSteamIDLobby);

    if (pCreated->m_eResult == k_EResultOK)
    {
        if (mom_lobby_debug.GetBool())
            Log("Result is okay! We got a lobby!\n");

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

    if (mom_lobby_debug.GetBool())
        Log("(LOBBY ENTER CALL RESULT): Got the callresult with result %i\n", pEntered->m_EChatRoomEnterResponse);

    // Note: There's both a callback and a call result from this. 
    // The callback handles actually entering, this is just for assurance that the call result completed.
}

void CMomentumLobbySystem::StartLobby()
{
    if (LobbyValid())
    {
        Warning("The lobby could not be created because you are already in one!\n");
        return;
    }

    if (m_cLobbyCreated.IsActive())
    {
        Warning("You are already creating a lobby!\n");
        return;
    }

    CHECK_STEAM_API(SteamMatchmaking());
    SteamAPICall_t call = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>(mom_lobby_type.GetInt()), mom_lobby_max_players.GetInt());
    m_cLobbyCreated.Set(call, this, &CMomentumLobbySystem::CallResult_LobbyCreated);

    if (mom_lobby_debug.GetBool())
        Log("The lobby call successfully happened!\n");
}

void CMomentumLobbySystem::LeaveLobby() const
{
    if (!LobbyValid())
    {
        Log("Could not leave lobby, are you in one?\n");
        return;
    }

    // Actually leave the lobby
    SteamMatchmaking()->LeaveLobby(m_sLobbyID);

    // Clear the ghosts stored in our lobby system
    g_pMomentumGhostClient->ClearCurrentGhosts(true);

    // Notify literally everything that can listen that we left
    FIRE_GAME_WIDE_EVENT("lobby_leave");
    
    m_sLobbyID = k_steamIDNil;

    g_pSteamRichPresence->Update();

    if (mom_lobby_debug.GetBool())
        Log("Left the lobby!\n");
}

// Called when we enter a lobby
void CMomentumLobbySystem::HandleLobbyEnter(LobbyEnter_t* pEnter)
{
    if (pEnter->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
    {
        Warning("Failed to enter chat room! Error code: %i\n", pEnter->m_EChatRoomEnterResponse);
        return;
    }

    if (mom_lobby_debug.GetBool())
        Log("Lobby entered! Lobby ID: %lld\n", pEnter->m_ulSteamIDLobby);

    if (!m_sLobbyID.IsValid())
    {
        m_sLobbyID = CSteamID(pEnter->m_ulSteamIDLobby);
    }

    FIRE_GAME_WIDE_EVENT("lobby_join");

    UpdateCurrentLobbyMap(gpGlobals->mapname.ToCStr());

    g_pSteamRichPresence->Update();

    CreateLobbyGhostEntities();
}

void CMomentumLobbySystem::SetAppearanceInMemberData(const AppearanceData_t &app)
{
    CHECK_STEAM_API(SteamMatchmaking());

    if (!LobbyValid())
        return;

    KeyValuesAD pAppearanceKV("app");
    app.ToKV(pAppearanceKV);

    CUtlBuffer buf;
    buf.SetBufferType(true, false);

    pAppearanceKV->RecursiveSaveToFile(buf, 0);

    SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_APPEARANCE, buf.String());
}

bool CMomentumLobbySystem::GetAppearanceFromMemberData(const CSteamID &member, AppearanceData_t &out)
{
    CHECK_STEAM_API_B(SteamMatchmaking());

    const char *pAppearance = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, member, LOBBY_DATA_APPEARANCE);
    if (pAppearance && !FStrEq(pAppearance, ""))
    {
        KeyValuesAD pAppearanceKV("app");
        if (pAppearanceKV->LoadFromBuffer(nullptr, pAppearance))
        {
            out.FromKV(pAppearanceKV);
            return true;
        }
    }

    return false;
}

CMomentumOnlineGhostEntity* CMomentumLobbySystem::GetLobbyMemberEntity(const uint64 &id)
{
    const auto findIndx = m_mapLobbyGhosts.Find(id);
    if (findIndx != m_mapLobbyGhosts.InvalidIndex())
        return m_mapLobbyGhosts[findIndx];
    
    return nullptr;
}

CMomentumOnlineGhostEntity* CMomentumLobbySystem::GetLobbyMemberEntity(const char *pNamePartial)
{
    FOR_EACH_MAP_FAST(m_mapLobbyGhosts, i)
    {
        const auto pMemberName = SteamFriends()->GetFriendPersonaName(CSteamID(m_mapLobbyGhosts.Key(i)));

        if (Q_stristr(pMemberName, pNamePartial))
            return m_mapLobbyGhosts[i];
    }

    return nullptr;
}

void CMomentumLobbySystem::ClearCurrentGhosts(bool bLeavingLobby)
{
    if (m_mapLobbyGhosts.Count() == 0)
        return;

    // If leaving the lobby while in the map, we need to remove the ghosts ourselves
    if (bLeavingLobby)
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

    m_mapLobbyGhosts.RemoveAll();
}

bool CMomentumLobbySystem::SendPacket(MomentumPacket *packet, const CSteamID &target, int sendType /*= k_nSteamNetworkingSend_Unreliable*/) const
{
    if (m_mapLobbyGhosts.Count() == 0)
        return false;

    CHECK_STEAM_API_B(SteamNetworkingMessages());

    CUtlBuffer buf;
    packet->Write(buf);

    SteamNetworkingIdentity identity;
    identity.SetSteamID(target);
    const auto eResult = SteamNetworkingMessages()->SendMessageToUser(identity, buf.Base(), buf.TellPut(), sendType, 0);

    return eResult == k_EResultOK;
}

bool CMomentumLobbySystem::SendPacketToEveryone(MomentumPacket *pPacket, int sendType /*= k_nSteamNetworkingSend_Unreliable*/)
{
    if (m_mapLobbyGhosts.Count() == 0)
        return false;

    CHECK_STEAM_API_B(SteamNetworkingMessages());

    CUtlBuffer buf;
    pPacket->Write(buf);

    auto index = m_mapLobbyGhosts.FirstInorder();
    while (index != m_mapLobbyGhosts.InvalidIndex())
    {
        const auto ghostID = m_mapLobbyGhosts.Key(index);

        SteamNetworkingIdentity identity;
        identity.SetSteamID64(ghostID);

        const auto eResult = SteamNetworkingMessages()->SendMessageToUser(identity, buf.Base(), buf.TellPut(), sendType, 0);

        if (eResult != k_EResultOK)
        {
            DevWarning("Failed to send the packet to %s!\n", SteamFriends()->GetFriendPersonaName(ghostID));
        }

        index = m_mapLobbyGhosts.NextInorder(index);
    }
    return true;
}

void CMomentumLobbySystem::WriteLobbyMessage(LobbyMessageType_t type, uint64 pID_int)
{
    const auto pEvent = gameeventmanager->CreateEvent("lobby_update_msg");

    if (pEvent)
    {
        pEvent->SetInt("type", type);
        pEvent->SetString("id", CFmtStr("%llu", pID_int).Get());

        gameeventmanager->FireEventClientSide(pEvent);
    }
}

void CMomentumLobbySystem::WriteSpecMessage(SpectateMessageType_t type, uint64 playerID, uint64 targetID)
{
    const auto pEvent = gameeventmanager->CreateEvent("lobby_spec_update_msg");

    if (pEvent)
    {
        pEvent->SetInt("type", type);

        pEvent->SetString("id", CFmtStr("%llu", playerID));
        pEvent->SetString("target", CFmtStr("%llu", targetID));

        gameeventmanager->FireEventClientSide(pEvent);
    }
}

bool CMomentumLobbySystem::IsInSameMapAs(const CSteamID &other)
{
    const char *pMapName = gpGlobals->mapname.ToCStr();
    const char *pOtherMap = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, other, LOBBY_DATA_MAP);

    return pMapName && pMapName[0] && pOtherMap && gpGlobals->eLoadType != MapLoad_Background && FStrEq(pMapName, pOtherMap);
}

bool CMomentumLobbySystem::IsInLobby(const CSteamID &other)
{
    CHECK_STEAM_API_B(SteamMatchmaking());

    if (!LobbyValid())
        return false;

    const auto numMembers = SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
    for (int i = 0; i < numMembers; i++)
    {
        if (other == SteamMatchmaking()->GetLobbyMemberByIndex(m_sLobbyID, i))
            return true;
    }

    return false;
}

void CMomentumLobbySystem::UpdateCurrentLobbyMap(const char *pMapName)
{
    if (!LobbyValid())
        return;

    CHECK_STEAM_API(SteamMatchmaking());
    CHECK_STEAM_API(SteamUser());

    SteamMatchmaking()->SetLobbyMemberData(m_sLobbyID, LOBBY_DATA_MAP, pMapName);

    const auto hOwner = SteamMatchmaking()->GetLobbyOwner(m_sLobbyID);
    if (hOwner == SteamUser()->GetSteamID())
    {
        SteamMatchmaking()->SetLobbyData(m_sLobbyID, LOBBY_DATA_OWNER_MAP, pMapName);
    }
}

void CMomentumLobbySystem::UpdateLobbyOwner()
{
    if (!LobbyValid())
        return;

    CHECK_STEAM_API(SteamMatchmaking());
    CHECK_STEAM_API(SteamUser());

    const auto hOwnerData = Q_atoui64(SteamMatchmaking()->GetLobbyData(m_sLobbyID, LOBBY_DATA_OWNER));
    const auto hOwner = SteamMatchmaking()->GetLobbyOwner(m_sLobbyID);
    if (hOwner == SteamUser()->GetSteamID())
    {
        // We became the owner, update the owner and map if need be
        if (hOwnerData != hOwner.ConvertToUint64())
        {
            SteamMatchmaking()->SetLobbyData(m_sLobbyID, LOBBY_DATA_OWNER, CFmtStr("%llu", hOwner.ConvertToUint64()).Get());
            UpdateCurrentLobbyMap(gpGlobals->mapname.ToCStr());
        }
    }
}

void CMomentumLobbySystem::UpdateLobbyEntityFromMemberData(CMomentumOnlineGhostEntity *pEntity)
{
    const auto steamID = CSteamID(pEntity->GetSteamID());

    AppearanceData_t appear;
    if (GetAppearanceFromMemberData(steamID, appear))
        pEntity->SetAppearanceData(appear, false);

    const auto state = pEntity->UpdateSpectateState(GetIsSpectatingFromMemberData(steamID), GetSpectatingTargetFromMemberData(steamID));
    if (state != SPEC_UPDATE_INVALID)
    {
        WriteSpecMessage(state, pEntity->GetSteamID(), pEntity->GetSpecTarget());
    }
}

void CMomentumLobbySystem::OnLobbyMemberDataChanged(const CSteamID &memberChanged)
{
    if (memberChanged == SteamUser()->GetSteamID())
        return;

    const bool bSameMap = IsInSameMapAs(memberChanged);

    const auto pEntity = GetLobbyMemberEntity(memberChanged);
    if (pEntity)
    {
        if (bSameMap)
        {
            UpdateLobbyEntityFromMemberData(pEntity);
        }
        else
        {
            OnLobbyMemberLeave(memberChanged);

            // "_____ just left your map."
            WriteLobbyMessage(LOBBY_UPDATE_MEMBER_LEAVE_MAP, memberChanged.ConvertToUint64());
        }
    }
    else
    {
        CreateLobbyGhostEntity(memberChanged);
    }
}

void CMomentumLobbySystem::OnLobbyMemberLeave(const CSteamID &member)
{
    const auto lobbyMemberID = member.ConvertToUint64();

    // Remove them if they're a requester
    g_pSavelocSystem->RequesterLeft(lobbyMemberID);

    const auto findIndex = m_mapLobbyGhosts.Find(lobbyMemberID);

    if (!m_mapLobbyGhosts.IsValidIndex(findIndex))
        return;

    const auto pEntity = m_mapLobbyGhosts[findIndex];
    if (pEntity)
    {
        pEntity->UpdatePlayerSpectate();
        pEntity->Remove();
    }

    m_mapLobbyGhosts.RemoveAt(findIndex);
}

void CMomentumLobbySystem::HandleLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
    if (!pParam->m_bSuccess)
        return;

    const auto lobbyId = CSteamID(pParam->m_ulSteamIDLobby);
    const auto memberChanged = CSteamID(pParam->m_ulSteamIDMember);

    if (lobbyId != m_sLobbyID)
        return;

    if (lobbyId.ConvertToUint64() == memberChanged.ConvertToUint64())
    {
        // The lobby itself changed:
        // We could have a new owner
        // Or new member limit
        // Or new lobby type

        // Are we the new owner?
        UpdateLobbyOwner();

        g_pSteamRichPresence->Update();
    }
    else
    {
        OnLobbyMemberDataChanged(memberChanged);
    }
}

// Somebody left/joined, or the owner of the lobby was changed
void CMomentumLobbySystem::HandleLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
    uint32 state = pParam->m_rgfChatMemberStateChange;
    CSteamID changedPerson = CSteamID(pParam->m_ulSteamIDUserChanged);
    if (state & k_EChatMemberStateChangeEntered)
    {
        if (mom_lobby_debug.GetBool())
            Log("A user just joined us!\n");

        // Note: The lobby data update method handles adding the ghost

        WriteLobbyMessage(LOBBY_UPDATE_MEMBER_JOIN, pParam->m_ulSteamIDUserChanged);

        g_pSteamRichPresence->Update();
    }
    if (state & (k_EChatMemberStateChangeLeft | k_EChatMemberStateChangeDisconnected))
    {
        if (mom_lobby_debug.GetBool())
            Log("User left/disconnected!\n");

        OnLobbyMemberLeave(changedPerson);

        WriteLobbyMessage(LOBBY_UPDATE_MEMBER_LEAVE, pParam->m_ulSteamIDUserChanged);

        g_pSteamRichPresence->Update();
    }
}

void CMomentumLobbySystem::HandlePersonaCallback(PersonaStateChange_t* pParam)
{
    if (!LobbyValid())
        return;

    const auto person = CSteamID(pParam->m_ulSteamID);
    const char *pName = SteamFriends()->GetFriendPersonaName(person);

    if (pParam->m_nChangeFlags & k_EPersonaChangeName)
    {
        if (IsInLobby(person))
        {
            const auto pGhost = GetLobbyMemberEntity(pParam->m_ulSteamID);
            if (pGhost)
            {
                pGhost->SetGhostName(pName);
            }
        }
    }

    if (pParam->m_nChangeFlags & k_EPersonaChangeRelationshipChanged)
    {
        if (MomUtil::IsSteamUserBlocked(person.ConvertToUint64()))
        {
            Warning("%s was just blocked!\n", pName);

            if (!m_vecBlocked.HasElement(person))
            {
                m_vecBlocked.AddToTail(person);
            }

            const auto findIndex = m_mapLobbyGhosts.Find(person.ConvertToUint64());

            if (m_mapLobbyGhosts.IsValidIndex(findIndex))
            {
                // This player was just blocked by us and their ghost currently exists, remove it
                const auto pEntity = m_mapLobbyGhosts[findIndex];

                if (pEntity)
                {
                    pEntity->Remove();
                }

                m_mapLobbyGhosts.RemoveAt(findIndex);
            }
        }
        else if (m_vecBlocked.HasElement(person))
        {
            Warning("%s was just unblocked!\n", pName);
            m_vecBlocked.FindAndRemove(person);

            // This player was just unblocked, attempt to spawn their ghost
            CreateLobbyGhostEntity(person);
        }
    }
}

void CMomentumLobbySystem::LevelChange(const char* pMapName)
{
    UpdateCurrentLobbyMap(pMapName);

    m_flNextUpdateTime = -1.0f;

    const bool bValidMap = pMapName && !FStrEq(pMapName, "");
    if (bValidMap)
    {
        CreateLobbyGhostEntities();
    }
    else
    {
        g_pMomentumGhostClient->ClearCurrentGhosts(false);
    }
}

void CMomentumLobbySystem::CreateLobbyGhostEntity(const CSteamID &lobbyMember)
{
    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());
    CHECK_STEAM_API(SteamFriends());

    const char *pName = SteamFriends()->GetFriendPersonaName(lobbyMember);

    if (m_vecBlocked.HasElement(lobbyMember))
    {
        Warning("Not allowing %s to talk with us, we have them ignored!\n", pName);
        return;
    }

    const auto lobbyMemberID = lobbyMember.ConvertToUint64();

    const auto findIndex = m_mapLobbyGhosts.Find(lobbyMemberID);
    const bool bValidGhost = m_mapLobbyGhosts.IsValidIndex(findIndex);

    if (!bValidGhost && IsInSameMapAs(lobbyMember))
    {
        const auto pNewPlayer = dynamic_cast<CMomentumOnlineGhostEntity *>(CreateEntityByName("mom_online_ghost"));
        pNewPlayer->SetSteamID(lobbyMemberID);
        pNewPlayer->SetGhostName(pName);
        pNewPlayer->Spawn();

        UpdateLobbyEntityFromMemberData(pNewPlayer);

        m_mapLobbyGhosts.Insert(lobbyMemberID, pNewPlayer);

        if (m_flNextUpdateTime < 0)
            m_flNextUpdateTime = gpGlobals->curtime + (1.0f / MOM_ONLINE_GHOST_UPDATERATE);

        // "_____ just joined your map."
        WriteLobbyMessage(LOBBY_UPDATE_MEMBER_JOIN_MAP, lobbyMemberID);
    }
}

void CMomentumLobbySystem::CreateLobbyGhostEntities()
{
    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());

    const auto localID = SteamUser()->GetSteamID();

    const auto numMembers = SteamMatchmaking()->GetNumLobbyMembers(m_sLobbyID);
    for (int i = 0; i < numMembers; i++)
    {
        const auto member = SteamMatchmaking()->GetLobbyMemberByIndex(m_sLobbyID, i);

        if (member == localID)
            continue;

        CreateLobbyGhostEntity(member);
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

    if (mom_lobby_debug.GetBool())
        Log("Trying to join the lobby from the string %s!\n", pString);

    uint64 steamID = Q_atoui64(pString);
    if (steamID > 0)
    {
        CSteamID toJoin;
        toJoin.FullSet(steamID, k_EUniversePublic, k_EAccountTypeChat);

        if (mom_lobby_debug.GetBool())
            Log("Got the ID! %lld\n", toJoin.ConvertToUint64());

        return TryJoinLobby(toJoin);
    }

    Warning("Could not join lobby due to malformed ID '%s'!\n", pString);
    return false;
}

void CMomentumLobbySystem::SendAndReceiveP2PPackets()
{
    if (m_mapLobbyGhosts.Count() == 0)
        return;

    SendP2PPackets();

    ReceiveP2PPackets();
}

void CMomentumLobbySystem::ReceiveP2PPackets()
{
    CHECK_STEAM_API(SteamNetworkingMessages());

#define MAX_MESSAGES_PER_READ 64

    SteamNetworkingMessage_t *messages[MAX_MESSAGES_PER_READ];
    int read = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES_PER_READ);
    while (read > 0)
    {
        for (int i = 0; i < read; i++)
        {
            auto pMessage = messages[i];

            CSteamID fromWho = pMessage->m_identityPeer.GetSteamID();

            if (m_vecBlocked.HasElement(fromWho))
            {
                pMessage->Release();
                continue;
            }

            CUtlBuffer buf(pMessage->m_pData, pMessage->m_cbSize, CUtlBuffer::READ_ONLY);
            buf.SetBigEndian(false);

            const auto type = buf.GetUnsignedChar();
            switch (type)
            {
            case PACKET_TYPE_POSITION:
            {
                PositionPacket frame(buf);
                CMomentumOnlineGhostEntity *pEntity = GetLobbyMemberEntity(fromWho);
                if (pEntity)
                    pEntity->AddPositionFrame(frame);
            }
            break;
            case PACKET_TYPE_DECAL:
            {
                DecalPacket decals(buf);
                if (decals.decal_type == DECAL_INVALID)
                    break;

                const auto pEntity = GetLobbyMemberEntity(fromWho);
                if (pEntity)
                {
                    pEntity->AddDecalFrame(decals);
                }
            }
            break;
            case PACKET_TYPE_SAVELOC_REQ:
            {
                SavelocReqPacket saveloc(buf);

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

                if (mom_lobby_debug.GetBool())
                    Log("Received a stage %i saveloc request packet!\n", saveloc.stage);

                switch (saveloc.stage)
                {
                case SAVELOC_REQ_STAGE_COUNT_REQ:
                {
                    if (!g_pSavelocSystem->AddSavelocRequester(fromWho.ConvertToUint64()))
                        break;

                    SavelocReqPacket response;
                    response.stage = SAVELOC_REQ_STAGE_COUNT_ACK;
                    response.saveloc_count = g_pSavelocSystem->GetSavelocCount();

                    SendPacket(&response, fromWho, k_nSteamNetworkingSend_Reliable);
                }
                break;
                case SAVELOC_REQ_STAGE_COUNT_ACK:
                {
                    KeyValues *pKV = new KeyValues("req_savelocs");
                    pKV->SetInt("stage", SAVELOC_REQ_STAGE_COUNT_ACK);
                    pKV->SetInt("count", saveloc.saveloc_count);
                    g_pModuleComms->FireEvent(pKV);
                }
                break;
                case SAVELOC_REQ_STAGE_SAVELOC_REQ:
                {
                    SavelocReqPacket response;
                    response.stage = SAVELOC_REQ_STAGE_SAVELOC_ACK;

                    if (g_pSavelocSystem->WriteRequestedSavelocs(&saveloc, &response, fromWho.ConvertToUint64()))
                        SendPacket(&response, fromWho, k_nSteamNetworkingSend_Reliable);
                }
                break;
                case SAVELOC_REQ_STAGE_SAVELOC_ACK:
                {
                    if (g_pSavelocSystem->ReadReceivedSavelocs(&saveloc, fromWho.ConvertToUint64()))
                    {
                        SavelocReqPacket response;
                        response.stage = SAVELOC_REQ_STAGE_DONE;
                        if (SendPacket(&response, fromWho, k_nSteamNetworkingSend_Reliable))
                        {
                            KeyValues *pKv = new KeyValues("req_savelocs");
                            pKv->SetInt("stage", SAVELOC_REQ_STAGE_DONE);
                            g_pModuleComms->FireEvent(pKv);
                        }
                    }
                }
                break;
                case SAVELOC_REQ_STAGE_DONE:
                {
                    g_pSavelocSystem->RequesterLeft(fromWho.ConvertToUint64());
                }
                break;
                case SAVELOC_REQ_STAGE_INVALID:
                default:
                    DevWarning(2, "Invalid stage for the saveloc request packet!\n");
                    break;
                }
            }
            break;
            default:
                break;
            }

            pMessage->Release();
        }

        read = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, messages, MAX_MESSAGES_PER_READ);
    }
}

void CMomentumLobbySystem::SendP2PPackets()
{
    if (m_flNextUpdateTime > 0.0f && gpGlobals->curtime > m_flNextUpdateTime)
    {
        PositionPacket frame;
        if (g_pMomentumGhostClient->CreateNewNetFrame(frame) && SendPacketToEveryone(&frame))
        {
            m_flNextUpdateTime = gpGlobals->curtime + (1.0f / MOM_ONLINE_GHOST_UPDATERATE);
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
    return (specChar && specChar[0]) ? true : false;
}

uint64 CMomentumLobbySystem::GetSpectatingTargetFromMemberData(const CSteamID &person)
{
    CHECK_STEAM_API_I(SteamMatchmaking());

    uint64 toReturn = 0;

    const auto specTarget = SteamMatchmaking()->GetLobbyMemberData(m_sLobbyID, person, LOBBY_DATA_SPEC_TARGET);
    if (specTarget && specTarget[0])
        toReturn = Q_atoui64(specTarget);

    return toReturn;
}

bool CMomentumLobbySystem::SendDecalPacket(DecalPacket *packet)
{
    return LobbyValid() && SendPacketToEveryone(packet);
}

void CMomentumLobbySystem::SetSpectatorTarget(const CSteamID &ghostTarget, bool bStartedSpectating, bool bLeft)
{
    CHECK_STEAM_API(SteamMatchmaking());
    CHECK_STEAM_API(SteamUser());

    if (!LobbyValid())
        return;

    SpectateMessageType_t type;
    if (bStartedSpectating)
    {
        type = SPEC_UPDATE_STARTED;
    }
    else if (!ghostTarget.IsValid() && ghostTarget.ConvertToUint64() != 1)
    {
        type = bLeft ? SPEC_UPDATE_LEAVE : SPEC_UPDATE_STOP;
    }
    else
        type = SPEC_UPDATE_CHANGETARGET;

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

    uint64 playerID = SteamUser()->GetSteamID().ConvertToUint64();
    uint64 ghostID = ghostTarget.ConvertToUint64();
    WriteSpecMessage(type, playerID, ghostID);
}

void CMomentumLobbySystem::OnLobbyMaxPlayersChanged(int newMax)
{
    // If the lobby isn't valid, it'll apply to our next one!
    if (!LobbyValid())
        return;

    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());

    const auto pLocID = SteamUser()->GetSteamID().ConvertToUint64();
    if (SteamMatchmaking()->GetLobbyOwner(m_sLobbyID).ConvertToUint64() == pLocID)
    {
        // Change the lobby type to this type
        newMax = clamp<int>(newMax, 2, 250);
        if (SteamMatchmaking()->SetLobbyMemberLimit(m_sLobbyID, newMax))
            Log("Successfully changed the maximum player count to %i!\n", newMax);
    }
    else
    {
        Warning("Cannot change the lobby max players; you are not the lobby owner!\n");
    }
}

void CMomentumLobbySystem::OnLobbyTypeChanged(int newType)
{
    // If the lobby isn't valid, it'll apply to our next one!
    if (!LobbyValid())
        return;

    CHECK_STEAM_API(SteamUser());
    CHECK_STEAM_API(SteamMatchmaking());

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
    {
        Warning("Cannot change the lobby type; you are not the lobby owner!\n");
    }
}

static CMomentumLobbySystem s_MOMLobbySystem;
CMomentumLobbySystem *g_pMomentumLobbySystem = &s_MOMLobbySystem;