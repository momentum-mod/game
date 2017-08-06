#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "mom_online_ghost.h"
#include "icommandline.h"

#include "tier0/memdbgon.h"

ConVar mm_updaterate("mom_ghost_online_updaterate", "25",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of updates per second to and from the ghost server.\n", true, 1.0f, true, 1000.0f);

ConVar mm_timeOutDuration("mom_ghost_online_timeout_duration", "10",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Seconds to wait when timimg out from a ghost server.\n", true, 5.0f, true, 30.0f);

//we have to wait a few ticks to let the interpolation catch up with our ghosts!
ConVar mm_lerpRatio("mom_ghost_online_lerp_ratio", "2",
    FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE,
    "Number of ticks to wait before updating ghosts, to allow client to interpolate.\n", true, 0.0f, true, 10.0f);


CMomentumPlayer* CMomentumGhostClient::m_pPlayer = nullptr;
CUtlMap<uint64, CMomentumOnlineGhostEntity*> CMomentumGhostClient::m_mapOnlineGhosts;
CMomentumGhostClient *CMomentumGhostClient::m_pInstance = nullptr;

bool isLobbyValid = g_pMomentumLobbySystem->m_sLobbyID.IsValid() && g_pMomentumLobbySystem->m_sLobbyID.IsLobby();

void CMomentumGhostClient::PostInit()
{
    //Log("================= COMMAND LINE: %s\n", CommandLine()->GetCmdLine());
    const char *pLobbyID = CommandLine()->ParmValue("+connect_lobby", nullptr);
    g_pMomentumLobbySystem->JoinLobbyFromString(pLobbyID);
}

void CMomentumGhostClient::LevelInitPostEntity()
{
    // MOM_TODO: AdvertiseGame needs to use k_steamIDNonSteamGS and pass the IP (as hex) and port if it is inside a server 
    // steamapicontext->SteamUser()->AdvertiseGame(steamapicontext->SteamUser()->GetSteamID(), 0, 0); // Gives game info of current server, useful if actually on server
    // steamapicontext->SteamFriends()->SetRichPresence("connect", "blah"); // Allows them to click "Join game" from Steam

    if (isLobbyValid)
    {
        const char *pMapName = gpGlobals->mapname.ToCStr();
        DevLog("Setting the map to %s!\n", pMapName);
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(g_pMomentumLobbySystem->m_sLobbyID, "map", pMapName);

        // Now check if this map is the same as somebody else's in the lobby
        CheckToAdd(nullptr);
    }
}
void CMomentumGhostClient::LevelShutdownPreEntity()
{
    if (isLobbyValid)
    {
        DevLog("Setting map to null, since we're going to the menu.\n");
        steamapicontext->SteamMatchmaking()->SetLobbyMemberData(g_pMomentumLobbySystem->m_sLobbyID, "map", nullptr);

        if (m_mapOnlineGhosts.Count() > 0)
        {
            m_mapOnlineGhosts.RemoveAll(); // NOTE: The game handles clearing the entities! No need to delete here.
        }
    }
    
}
void CMomentumGhostClient::FrameUpdatePreEntityThink()
{
    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());
}

void CMomentumGhostClient::Shutdown()
{
    g_pMomentumLobbySystem->LeaveLobby(); // Leave the lobby if we're still in it
}

void CMomentumGhostClient::ClearCurrentGhosts()
{
    // We have to remove every entity manually if we left this lobby
    if (m_mapOnlineGhosts.Count() > 0)
    {
        unsigned short currIndx = m_mapOnlineGhosts.FirstInorder();
        while (currIndx != m_mapOnlineGhosts.InvalidIndex())
        {
            CMomentumOnlineGhostEntity *pEnt = m_mapOnlineGhosts[currIndx];
            if (pEnt)
                pEnt->Remove();

            currIndx = m_mapOnlineGhosts.NextInorder(currIndx);
        }

        m_mapOnlineGhosts.RemoveAll(); // No need to purge, the game handles the entities' memory
    }
}
ghostNetFrame_t CMomentumGhostClient::CreateNewNetFrame(CMomentumPlayer *pPlayer)
{
    return ghostNetFrame_t(pPlayer->EyeAngles(),
        pPlayer->GetAbsOrigin(),
        pPlayer->GetAbsVelocity(),
        pPlayer->GetViewOffset().z,
        pPlayer->m_nButtons);
}
void CMomentumGhostClient::CheckToAdd(CSteamID *pID)
{
    CSteamID localID = steamapicontext->SteamUser()->GetSteamID();

    if (pID)
    {
        const char *pOtherMap = steamapicontext->SteamMatchmaking()->GetLobbyMemberData(g_pMomentumLobbySystem->m_sLobbyID, *pID, "map");
        unsigned short findIndx = m_mapOnlineGhosts.Find(pID->ConvertToUint64());
        // Just joined this map, we haven't created them 
        if (FStrEq(gpGlobals->mapname.ToCStr(), pOtherMap))
        {
            // Don't add them again if they reloaded this map for some reason
            if (findIndx == m_mapOnlineGhosts.InvalidIndex())
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

                if (g_pMomentumLobbySystem->m_flNextUpdateTime < 0)
                    g_pMomentumLobbySystem->m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat()); // MOM_TODO: Probably move this out of the for loop and use a boolean check instead
            }
        }
        else if (findIndx != m_mapOnlineGhosts.InvalidIndex())
        {
            // They changed map, remove their entity from the CUtlMap

        }
    }
    else
    {
        int numMembers = steamapicontext->SteamMatchmaking()->GetNumLobbyMembers(g_pMomentumLobbySystem->m_sLobbyID);
        for (int i = 0; i < numMembers; i++)
        {
            CSteamID member = steamapicontext->SteamMatchmaking()->GetLobbyMemberByIndex(g_pMomentumLobbySystem->m_sLobbyID, i);
            if (member == localID) // If it's us, don't care
                continue;

            CheckToAdd(&member);
        }
    }
}
unsigned CMomentumGhostClient::SendAndRecieveP2PPackets(void* args)
{
    bool IsConnected = g_pMomentumLobbySystem->m_sLobbyID.IsValid() && g_pMomentumLobbySystem->m_sLobbyID.IsLobby();
    while (IsConnected)
    {
        if (m_pPlayer)
        {
            if (m_mapOnlineGhosts.Count() > 0 && g_pMomentumLobbySystem->m_flNextUpdateTime > 0 && gpGlobals->curtime > g_pMomentumLobbySystem->m_flNextUpdateTime)
            {
                // Read data
                uint32 size;
                if (steamapicontext->SteamNetworking()->IsP2PPacketAvailable(&size))
                {
                    DevLog("Packet available! Size: %u bytes where sizeof frame is %i bytes\n", size, sizeof ghostNetFrame_t);
                    if (size % sizeof ghostNetFrame_t == 0)
                    {
                        ghostNetFrame_t frame;
                        uint32 bytesRead;
                        CSteamID fromWho;
                        if (steamapicontext->SteamNetworking()->ReadP2PPacket(&frame, sizeof frame, &bytesRead, &fromWho))
                        {
                            DevLog("Read the packet successfully! Read bytes: %u, from steamID %lld\n", bytesRead,
                                fromWho.ConvertToUint64());
                            uint16_t findIndex = m_mapOnlineGhosts.Find(fromWho.ConvertToUint64());
                            if (findIndex != m_mapOnlineGhosts.InvalidIndex())
                            {
                                m_mapOnlineGhosts[findIndex]->SetCurrentNetFrame(frame);
                            }
                        }
                    }
                }

                // Send data
                // MOM_TODO: Change this to be server-client with the lobby owner being the "server" for everyone. 
                // Only one packet should be sent here if you are not the lobby owner. Otherwise, send everybody's data to everybody.
                // Right now it's just pure P2P.

                // CSteamID owner = steamapicontext->SteamMatchmaking()->GetLobbyOwner(m_sLobbyID);

                uint16_t firstIndex = m_mapOnlineGhosts.FirstInorder();
                while (firstIndex != m_mapOnlineGhosts.InvalidIndex())
                {
                    CSteamID ghost = m_mapOnlineGhosts[firstIndex]->GetGhostSteamID();
                    ghostNetFrame_t frame = CreateNewNetFrame(m_pPlayer);
                    if (steamapicontext->SteamNetworking()->SendP2PPacket(ghost, &frame, sizeof frame, k_EP2PSendUnreliable))
                    {
                        DevLog("Sent the packet!\n");
                    }
                }

                g_pMomentumLobbySystem->m_flNextUpdateTime = gpGlobals->curtime + (1.0f / mm_updaterate.GetFloat());
            }
        }
    }
    ConDColorMsg(Color(255,0,255,255), "Exiting thread!\n");
    return 0;
}
static CMomentumGhostClient s_MOMGhostClient("CMomentumGhostClient");
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;