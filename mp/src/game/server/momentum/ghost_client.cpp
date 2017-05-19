#include "cbase.h"
#include "ghost_client.h"
#include "util/mom_util.h"

#include "mom_online_ghost.h"

#include "tier0/memdbgon.h"


zed_net_socket_t CMomentumGhostClient::m_socket;
zed_net_address_t CMomentumGhostClient::m_address;

char CMomentumGhostClient::data[256];
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
        //vars->pPlayer = m_pPlayer;
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
        //
    }
}
bool CMomentumGhostClient::initGhostClient()
{
    m_socket = zed_net_socket_t();
    m_address = zed_net_address_t();
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
    int data = MOM_SIGNOFF;
    bool returnResult;
    if (zed_net_tcp_socket_send(&m_socket, &data, sizeof(data)) < 0)
    {
        Warning("Could not send signoff packet! Error: %s\n", zed_net_get_error());
        returnResult = false;
    }
    int bytes_read = zed_net_tcp_socket_receive(&m_socket, &data, sizeof(data));

    if (bytes_read && data == MOM_SIGNOFF)
    {
        ConColorMsg(Color(255, 255, 0, 255), "Sent signoff packet, exiting ghost client...\n");
        returnResult = true;
    }
    else
    {
        Warning("Did not recieve ACK from server. Server will have to wait for us to time out...\n");
        returnResult = false;
    }

    zed_net_socket_close(&m_socket);
    zed_net_shutdown();
    m_mtxGhostPlayers.Lock();
    //delete all the memory allocated to players
    for (auto i : ghostPlayers)
    {
        i->Remove();
    }
    ghostPlayers.Purge();
    m_mtxGhostPlayers.Unlock();
    return returnResult;
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

    int data = MOM_SIGNON;
    zed_net_tcp_socket_send(&m_socket, &data, sizeof(data));
    ConColorMsg(Color(255, 255, 0, 255), "Sending signon packet...\n");
    char mapName[64];
    int bytes_read = zed_net_tcp_socket_receive(&m_socket, &mapName, sizeof(mapName));

    if (bytes_read) //Success!
    {
        if (Q_strcmp(mapName, gpGlobals->mapname.ToCStr()) == 0)
        {
            ConColorMsg(Color(255, 255, 0, 255), "Recieved ACK from server, we are on the same map: %s\n", mapName);
            m_bRanThread = false; //reset so we can run the connection thread again
            return true;
        }
        DevWarning("Recieved ACK from server, but could not syncronize maps! \
                               Server map: %s. Client map: %s. Disconnecting...\n", mapName, gpGlobals->mapname.ToCStr());
        return false;
    }
    DevWarning("Server did not ACK, we are not connected!\n");
    return false;

}
//Threaded function
unsigned CMomentumGhostClient::sendAndRecieveData(void *params)
{
    static bool firstNewFrame = true;
    //Don't need a mutex for m_ghostClientConnected because if it ever changes from when the thread starts, the thread should exit anyways
    while (m_ghostClientConnected)
    {
        int tickSleep = 1000 * (float(1) / mm_updaterate.GetFloat());
        ThreadSleep(tickSleep); //sleep   

        // ************************************
        // ------------------------------------
        // DATA SENT TO SERVER
        // ------------------------------------
        // ************************************

        //Send an identifier to the server that we're about to send a new frame. 
        //When we get an ACK from the server, we send the frame.
        int newFrameIdentifier = MOM_C_SENDING_NEWFRAME; //Client sending new data to server 
        zed_net_tcp_socket_send(&m_socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        int data;
        int tick1 = gpGlobals->tickcount;
        int bytes_read = zed_net_tcp_socket_receive(&m_socket, &data, sizeof(data));
        while (bytes_read != 4) // 
        {
            int tick2 = gpGlobals->tickcount;
            float deltaT = float(tick2 - tick1) * gpGlobals->interval_per_tick;
            // Can't use ThreadSleep here for some reason since GabeN called me up and said "OH MY GOD WOULD YOU JUST FUCK OFF" when I tried to
            // ... um, I mean, ... It messes up gpGlobals->tickcount
            // So this code checks if deltaT is an integer, effectively only printing every 1 second.
            
            //MOM_TODO: Fix this (it prints... a lot..)
            if (deltaT == round(deltaT))
            {
                if (!m_ghostClientConnected) break; //prevents hang on hard quit
                Warning("Lost connection! Waiting to time out... %f\n", deltaT);
            }
            
            if (deltaT > mm_timeOutDuration.GetInt())
            {
                Warning("Lost connection to ghost server.\n");
                zed_net_socket_close(&m_socket);
                zed_net_shutdown();
                m_mtxGhostPlayers.Lock();
                for (auto i : ghostPlayers)      // delet this
                {
                    i->Remove();
                }
                m_mtxGhostPlayers.Unlock();
                m_ghostClientConnected = false;
                firstNewFrame = true; //set to true again since the thread exited, so we disconnected
                return 1;
            }
        }

        // ---------------
        // Handle sending new net-frame data to server
        // ----------------
        m_mtxpPlayer.Lock();
        if (bytes_read && data == MOM_C_RECIEVING_NEWFRAME && m_pPlayer) //SYN-ACK , Server acknowledges new frame is coming
        {
            oldAppearance = m_pPlayer->m_playerAppearanceProps;
            ghostNetFrame_t newFrame(m_pPlayer->EyeAngles(),
                m_pPlayer->GetAbsOrigin(),
                m_pPlayer->GetViewOffset(),
                m_pPlayer->m_nButtons,
                m_SteamID,
                m_pPlayer->GetPlayerName());

            zed_net_tcp_socket_send(&m_socket, &newFrame, sizeof(newFrame)); 

            //Send the appearance to the server too, so when new players connect we can see their customization!
            if (firstNewFrame)
            {
                zed_net_tcp_socket_send(&m_socket, &oldAppearance, sizeof(oldAppearance));
                firstNewFrame = false;
            }
        }
        m_mtxpPlayer.Unlock();

        //------------------
        // Handle sending appearance data to server, IFF it has changed for the local player.
        // ----------------
        m_mtxpPlayer.Lock();
        if (!(m_pPlayer->m_playerAppearanceProps == oldAppearance)) //appearance changed! we need to send it to the server.
        {
            newFrameIdentifier = MOM_C_SENDING_NEWPROPS; //Client is sending new data to the server. 
            zed_net_tcp_socket_send(&m_socket, &newFrameIdentifier, sizeof(newFrameIdentifier));

            int bytes_read = zed_net_tcp_socket_receive(&m_socket, &data, sizeof(data));

            if (bytes_read && data == MOM_C_RECIEVING_NEWPROPS)
            {
                zed_net_tcp_socket_send(&m_socket, &m_pPlayer->m_playerAppearanceProps, sizeof(ghostAppearance_t));
            }
        }
        m_mtxpPlayer.Unlock();


        // ************************************
        // ------------------------------------
        // DATA RECIEVED FROM SERVER
        // ------------------------------------
        // ************************************
        newFrameIdentifier = MOM_S_RECIEVING_NEWFRAME; //Client ready to recieve data from server 
        zed_net_tcp_socket_send(&m_socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN

        int recvData;
        bytes_read = zed_net_tcp_socket_receive(&m_socket, &recvData, sizeof(recvData));

        // ----------------
        // Handle recieving new net-frames from the server
        // ----------------
        if (bytes_read && recvData == MOM_S_SENDING_NEWFRAME) //SYN-ACK
        {
            //The server then sends number of players to the client
            int playerNum = 0;
            bytes_read = zed_net_tcp_socket_receive(&m_socket, &playerNum, sizeof(playerNum));

            //now that we have the number of players, we know how many packets to recieve. we loop through and recieve the data 
            m_mtxGhostPlayers.Lock();
            for (int i = 0; i < playerNum; i++) 
            {
                ghostNetFrame_t newFrame;
                zed_net_tcp_socket_receive(&m_socket, &newFrame, sizeof(ghostNetFrame_t));

                bool didFindPlayer = false;
                for (auto i : ghostPlayers) //Look through all players currently connected
                {
                    if (i->GetCurrentNetFrame().SteamID64 == newFrame.SteamID64) //If the player is already connected to server
                    {
                        didFindPlayer = true;
                        i->SetCurrentNetFrame(newFrame); //update their current frame
                        break;
                    }
                }
                if (!didFindPlayer) //they weren't in the vector of players already
                {
                    // we need to recieve their looks as well.
                    ghostAppearance_t newLooks;
                    zed_net_tcp_socket_receive(&m_socket, &newLooks, sizeof(ghostAppearance_t));

                    CMomentumOnlineGhostEntity *newPlayer = static_cast<CMomentumOnlineGhostEntity*>(CreateEntityByName("mom_online_ghost"));
                    newPlayer->SetCurrentNetFrame(newFrame);
                    newPlayer->Spawn();
                    newPlayer->SetGhostAppearance(newLooks);
                    ghostPlayers.AddToTail(newPlayer);
                    DevMsg("Added new player: %s\n There are now %i connected players.", newFrame.PlayerName, ghostPlayers.Size());
                }
                didFindPlayer = false;
            }
            if (ghostPlayers.Size() > playerNum) //Someone disconnected, so the server told us about it.
            {
                //remove all the players that don't exist in the server anymore
                for (int i = 0; i < (ghostPlayers.Size() - playerNum); i++)
                {
                    if (ghostPlayers[i] != nullptr) ghostPlayers[i]->Remove();
                    ghostPlayers.Remove(i); 
                }
            }
            m_mtxGhostPlayers.Unlock();
        }

        //------------------
        // Recieve new appearance data if it happens to change.
        // ----------------
        m_mtxGhostPlayers.Lock();
        if (bytes_read && recvData == MOM_S_SENDING_NEWPROPS) //ghost server is sending new appearances
        {
            uint64 steamIDOfNewAppearance;
            zed_net_tcp_socket_receive(&m_socket, &steamIDOfNewAppearance, sizeof(uint64));

            newFrameIdentifier = MOM_S_RECIEVING_NEWPROPS; //Client ready to recieve data from server 
            zed_net_tcp_socket_send(&m_socket, &newFrameIdentifier, sizeof(newFrameIdentifier)); //SYN
            recvData = 0;
            if (bytes_read && recvData == MOM_S_SENDING_NEWPROPS)
            {
                ghostAppearance_t newAppearnece;
                zed_net_tcp_socket_receive(&m_socket, &newAppearnece, sizeof(ghostAppearance_t));

                for (auto i : ghostPlayers) //Look through all players currently connected
                {
                    if (i->GetCurrentNetFrame().SteamID64 == steamIDOfNewAppearance) //found them!
                    {
                        i->SetGhostAppearance(newAppearnece); //update their appearance properties
                        break;
                    }
                }
            }
        }
        m_mtxGhostPlayers.Unlock();

        //------------------
        // Handle recieving new map data from the server
        // ----------------
        if (bytes_read && recvData == MOM_S_SENDING_NEWMAP)
        {
            char newMapName[64];
            zed_net_tcp_socket_receive(&m_socket, newMapName, sizeof(newMapName));
            if (Q_strcmp(gpGlobals->mapname.ToCStr(), newMapName) != 0) //The new map is different from the one we are currently playing on
            {
                engine->ClientCommand(m_pPlayer->edict(), "map %s", newMapName);
                firstNewFrame = true; //set to true again since the thread exited, so we disconnected
                return 0; //exit the thread
            }
        }

    }
    firstNewFrame = true; //set to true again since the thread exited, so we disconnected
    return 0;
}
static CMomentumGhostClient s_MOMGhostClient;
CMomentumGhostClient *g_pMomentumGhostClient = &s_MOMGhostClient;