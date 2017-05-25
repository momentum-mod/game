#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"
#include "mom_online_ghost.h"

#include "tier0/memdbgon.h"

zed_net_socket_t CMomentumGhostClient::m_socket;
zed_net_address_t CMomentumGhostClient::m_address;

bool CMomentumGhostClient::m_ghostClientConnected = false;
bool CMomentumGhostClient::m_bRanThread = false;
const char* CMomentumGhostClient::m_host = "127.0.0.1";
unsigned short CMomentumGhostClient::m_port = 9000;

CMomentumPlayer* CMomentumGhostClient::m_pPlayer;
uint64 CMomentumGhostClient::m_SteamID;
CUtlVector<CMomentumOnlineGhostEntity*> CMomentumGhostClient::ghostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxGhostPlayers;
CThreadMutex CMomentumGhostClient::m_mtxpPlayer;
ghostAppearance_t CMomentumGhostClient::oldAppearance;

ThreadHandle_t netIOThread;
CMessageQueue<int> SentPacketQueue;
void CMomentumGhostClient::LevelInitPostEntity()
{
    if (initGhostClient()) //init ghost client
    {
        m_ghostClientConnected = connectToGhostServer(m_host, m_port);
    }
    if (!m_ghostClientConnected)
    {
        exitGhostClient(); 
    }
}
void CMomentumGhostClient::LevelShutdownPreEntity()
{
    exitGhostClient(); //set ghost client connection to false when we disconnect
    m_ghostClientConnected = false;
    //ThreadJoin(netIOThread, 20);
}
void CMomentumGhostClient::FrameUpdatePostEntityThink()
{
    m_pPlayer = ToCMOMPlayer(UTIL_GetListenServerHost());

    // Run the thread that recieves and sends ghost data IFF we're connected to the server, AND it hasn't run before 
    if (!m_bRanThread && isGhostClientConnected() && m_pPlayer && steamapicontext)
    {
        MyThreadParams_t vars; //bogus params containing NOTHING hahAHAHAhaHHa
        oldAppearance = m_pPlayer->m_playerAppearanceProps;
        m_SteamID = steamapicontext->SteamUser()->GetSteamID().ConvertToUint64();
        // Create a new thread to handle network I/O
        ConColorMsg(Color(0, 100, 255, 255), "Running thread\n");
        netIOThread = CreateSimpleThread(CMomentumGhostClient::sendAndRecieveData, &vars);
        ThreadDetach(netIOThread);
        m_bRanThread = true;
    }

    // DO stuff with that data
    if (isGhostClientConnected() && m_pPlayer)
    {
        if (!(m_pPlayer->m_playerAppearanceProps == oldAppearance) && m_pPlayer)
        {
            SentPacketQueue.QueueMessage(PT_APPEARANCE);
            oldAppearance = m_pPlayer->m_playerAppearanceProps;
        }
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
        ConColorMsg(Color(255, 255, 0, 255), "Sent signoff packet, exiting ghost client...\n");
        returnValue = true;
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
        pPlayer->GetViewOffset(),
        pPlayer->m_nButtons,
        pPlayer->GetPlayerName());
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
                ghostPlayers[i]->SetCurrentNetFrame(*newFrame);
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
            ghostSignOnPacket_t *newSignOn = reinterpret_cast<ghostSignOnPacket_t*>(buffer);
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
                ghostPlayers[i]->SetGhostAppearance(*newApps);
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
static CMomentumGhostClient s_MOMGhostClient;
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;