#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <stdarg.h>
#include <algorithm>

#include "ghostServer.h"
volatile int CMOMGhostServer::numPlayers;
std::vector<playerData* > CMOMGhostServer::m_vecPlayers;
std::mutex CMOMGhostServer::m_vecPlayers_mutex;
std::mutex CMOMGhostServer::m_bShouldExit_mutex;
//used for arbitrary events synchronized with all threads that are handled whenever the thread is free (e.g changing maps)
SafeQueue<char*> CMOMGhostServer::m_sqEventQueue; 

zed_net_socket_t CMOMGhostServer::m_Socket;
bool CMOMGhostServer::m_bShouldExit = false;
int CMOMGhostServer::m_iTickRate;
char CMOMGhostServer::m_szMapName[96];
const std::chrono::seconds CMOMGhostServer::m_secondsToTimeout(SECONDS_TO_TIMEOUT);

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char** argv)
{
#ifdef _WIN32
    //Disable mouse highlighting which, for some reason, pauses ALL THREADS while something is highlighted...
    HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwOldInputMode;
    GetConsoleMode(input_handle, &dwOldInputMode);
    SetConsoleMode(input_handle, dwOldInputMode & ~ENABLE_QUICK_EDIT_MODE);

#endif
    char map[32];
    uint16_t port = 0;
    int status = -1;

    for (int i = 0; i < argc; ++i)
    {
#ifdef _WIN32
        if (strcmp(argv[i], "-nowindow") == 0)
        {
            ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false); //Don't create a new window when we launch with -nowindow command
        }
#endif
        if (strcmp(argv[i], "-map") == 0)
        {
            _snprintf(map, sizeof(map), "%s", argv[i + 1]);
        }
        if (strcmp(argv[i], "-port") == 0)
        {
            port = uint16_t(atoi(argv[i + 1]));
        }
    }
    if (port != 0 && map != nullptr)
    {
        status = CMOMGhostServer::runGhostServer(port, map);
    }
    else
    {
        status = CMOMGhostServer::runGhostServer(DEFAULT_PORT, DEFAULT_MAP);
    }
    if (status != 0)
        return 0;

    std::thread t(CMOMGhostServer::acceptNewConnections); //create a new thread that listens for incoming client connections
    t.detach(); //continuously run thread

    while (!CMOMGhostServer::m_bShouldExit)
    {
        CMOMGhostServer::handleConsoleInput();
    }
    zed_net_shutdown();
#ifdef _WIN32
    SetConsoleMode(input_handle, dwOldInputMode); //ignores mouse input on the input buffer
#endif

    return 0;
}

int CMOMGhostServer::runGhostServer(const unsigned short port, const char* mapName)
{
    zed_net_init();

    zed_net_tcp_socket_open(&m_Socket, port, 0, 1);
    conMsg("Running ghost server on %s on port %d!\n", mapName, port);
    _snprintf(m_szMapName, sizeof(m_szMapName), "%s", mapName);
    return 0;

}
void CMOMGhostServer::newConnection(zed_net_socket_t socket, zed_net_address_t address)
{ 
    const char* host = zed_net_host_to_str(address.host);
    conMsg("Accepted connection from %s:%d\n", host, address.port);
    char buffer[512];
    int packetType;
    int bytes_read = zed_net_tcp_socket_receive(&socket, buffer, 512, &packetType);
    //printf("bytes_read: %i, packet_type : %i\n", bytes_read, packetType);
    if (bytes_read && packetType == PT_SIGNON)
    {
        ghostSignOnPacket_t *newPacket = reinterpret_cast<ghostSignOnPacket_t*>(buffer);
        if (strcmp(newPacket->MapName, m_szMapName) == 0)
        {
            ghostAckPacket_t newAck;
            newAck.AckSuccess = true;
            zed_net_tcp_socket_send(&socket, &newAck, sizeof(newAck), PT_ACK);

            playerData *newPlayer = new playerData(socket, address, newPacket->newFrame, newPacket->newApps, numPlayers + 1, newPacket->SteamID); //numPlayers is current, add 1 for new idx

            m_sqEventQueue.enqueue(NEW_PLAYER_CMD); //queue up the new player packet to be sent.

            m_vecPlayers_mutex.lock();

            m_vecPlayers.push_back(newPlayer);
            numPlayers = m_vecPlayers.size();

            m_vecPlayers_mutex.unlock();
            conMsg("Added new player: %s. There are now %i connected players.\n", newPacket->newFrame.PlayerName, numPlayers);

            while (newPlayer->remote_socket.ready == 0) //socket is open
            {
                handlePlayer(newPlayer);
            }
            delete newPlayer;

        }
        else
        {
            conMsg("Player %s tired to connect, but on the wrong map. %s\n", newPacket->newFrame.PlayerName, newPacket->MapName);
            ghostAckPacket_t newAck;
            newAck.AckSuccess = false;
            zed_net_tcp_socket_send(&socket, &newAck, sizeof(newAck), PT_ACK);
        }
    }
}
void CMOMGhostServer::acceptNewConnections()
{
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    remote_socket.held_bytes = 0;
    remote_socket.prefix_size = 2;
    remote_socket.magic_num_size = 4;
    while (!m_bShouldExit)
    {
        zed_net_tcp_accept(&m_Socket, &remote_socket, &remote_address);

        std::thread t(newConnection, remote_socket, remote_address); //create a new thread to deal with the connection
        t.detach(); //each connection is dealt with in a seperate thread
    }
}
void CMOMGhostServer::handleConsoleInput()
{
    char buffer[256], command[256], argument[256];
    fgets(buffer, 256, stdin);
    buffer[strlen(buffer) - 1] = '\0';

    if (strlen(buffer) > 0)
    {
        _snprintf(argument-1, sizeof(argument), "%s", strpbrk(buffer, " "));
        _snprintf(command, sizeof(command), "%s", strtok(buffer, " "));
    }
    if (strcmp(command, "say") == 0)
    {
        conMsg("You tried to say: \"%s\" to the server\n", argument);
        m_sqEventQueue.enqueue(argument);
    }
    if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0)
    {
        m_bShouldExit_mutex.lock();
        m_bShouldExit = true;
        m_bShouldExit_mutex.unlock();
    }
    if (strcmp(command, "help") == 0)
    {
        conMsg("flags: -port <port>, -map <mapname> \n");
        conMsg("Commands: numplayers, currentmap, say, map, exit\n");
    }
    if (strcmp(command, "numplayers") == 0)
    {
        conMsg("Number of connected players: %i\n", numPlayers);
    }
    if (strcmp(command, "map") == 0)
    {
        _snprintf(m_szMapName, sizeof(m_szMapName), argument); 
        conMsg("Changing map to %s...\n", argument);
        m_sqEventQueue.enqueue(NEW_MAP_CMD);
    }
    if (strcmp(command, "currentmap") == 0)
    {
        conMsg("Current map is: %s\n", m_szMapName);
    }
}
void CMOMGhostServer::handlePlayer(playerData *newPlayer)
{   
    //process the event queue first.
    if (m_sqEventQueue.numInQueue() != 0) //queue is not empty
    {
        char* newEvent = m_sqEventQueue.dequeue();
        if (strcmp(newEvent, NEW_MAP_CMD) == 0)
        {
            ghostNewMapEvent_t newMapEvent;
            strncpy(newMapEvent.MapName, m_szMapName, sizeof(newMapEvent.MapName));
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &newMapEvent, sizeof(newMapEvent), PT_NEWMAP);
        }
        if (strcmp(newEvent, NEW_APPEARENCES_CMD) == 0)
        {
            //now send this player all of the net frames.
            int GhostAppsSize = sizeof(ghostAppearance_t);
            char *appearanceBuffer = new char[GhostAppsSize * numPlayers];
            m_vecPlayers_mutex.lock();
            for (int i = 0; i < numPlayers; i++)
            {
                memcpy(appearanceBuffer + (GhostAppsSize * i), &m_vecPlayers[i]->currentLooks, GhostAppsSize);
            }
            m_vecPlayers_mutex.unlock();

            zed_net_tcp_socket_send(&newPlayer->remote_socket, appearanceBuffer, GhostAppsSize * numPlayers, PT_APPEARANCE);

            delete[] appearanceBuffer;
        }
        if (strcmp(newEvent, NEW_PLAYER_CMD) == 0)
        {
            ghostSignOnPacket_t newPlayerSignOn;
            m_vecPlayers_mutex.lock();

            //idx of numPlayers is the newest connected player.
            newPlayerSignOn.newApps = m_vecPlayers[numPlayers-1]->currentLooks;
            newPlayerSignOn.newFrame = m_vecPlayers[numPlayers-1]->currentFrame;
            newPlayerSignOn.SteamID = m_vecPlayers[numPlayers - 1]->SteamID64;

            zed_net_tcp_socket_send(&newPlayer->remote_socket, &newPlayerSignOn, sizeof(ghostSignOnPacket_t), PT_SIGNON);
            m_vecPlayers_mutex.unlock();
        }
    }
    //now send this player all of the net frames.
    int GhostFrameSize = sizeof(ghostNetFrame_t);
    char *netFrameBuffer = new char[GhostFrameSize * numPlayers];
    m_vecPlayers_mutex.lock();

    for (int i = 0; i < numPlayers; i++)
    {
        memcpy(netFrameBuffer + (GhostFrameSize * i), &m_vecPlayers[i]->currentFrame, GhostFrameSize);
    }
    zed_net_tcp_socket_send(&newPlayer->remote_socket, netFrameBuffer, GhostFrameSize * numPlayers, PT_NET_FRAME);
    m_vecPlayers_mutex.unlock();

    delete[] netFrameBuffer;

    int packet_type;
   
    char buffer[uint16_t(~0)]; //largest packet size we can expect, 65536
    int bytes_read = zed_net_tcp_socket_receive(&newPlayer->remote_socket, buffer, uint16_t(~0), &packet_type);
    const auto t1 = Clock::now();
    while (bytes_read == 0) //Oops, we didn't get anything from the client!
    {
        const auto t2 = Clock::now();
        const auto deltaT = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
        conMsg("Lost connection! Waiting to time out... %ll\n", deltaT.count());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (deltaT > m_secondsToTimeout)
        {
            conMsg("%s timed out...\n", newPlayer->currentFrame.PlayerName);
            disconnectPlayer(newPlayer);
            break;
        }
    }
    if (bytes_read)
    {
        if (packet_type == PT_APPEARANCE)
        {
            ghostAppearance_t *newApps = reinterpret_cast<ghostAppearance_t*>(buffer);
            newPlayer->currentLooks = *newApps;
            conMsg("Player %s sent new appearances.\n", newPlayer->currentFrame.PlayerName);
            m_sqEventQueue.enqueue(NEW_APPEARENCES_CMD); //queue up the new appearances to be sent 
        }
        if (packet_type == PT_NET_FRAME)
        {
            ghostNetFrame_t *newFrame = reinterpret_cast<ghostNetFrame_t*>(buffer);
            newPlayer->currentFrame = *newFrame;
        }
        if (packet_type == PT_SIGNOFF)
        {
            conMsg("%s disconnected...\n", newPlayer->currentFrame.PlayerName);
            disconnectPlayer(newPlayer);
        }
    }

}
void CMOMGhostServer::disconnectPlayer(playerData *player)
{
    zed_net_socket_close(&player->remote_socket);
    m_vecPlayers_mutex.lock();

    m_vecPlayers.erase(std::remove_if(m_vecPlayers.begin(), m_vecPlayers.end(), 
        [player](const playerData* i){ return i->SteamID64 == player->SteamID64;}),
        m_vecPlayers.end());

    numPlayers = m_vecPlayers.size();

    m_vecPlayers_mutex.unlock();
    conMsg("There are now %i connected players.\n", numPlayers);
}
//A replacement for printf that prints the time as well as the message. 
void CMOMGhostServer::conMsg(const char* msg, ...)
{
    va_list list;
    va_start(list, msg);
    char time[64], msgBuffer[256];
    _snprintf(time, sizeof(time), "%s", currentDateTime().c_str());
    _snprintf(msgBuffer, sizeof(msgBuffer), "%s - %s", time, msg);
    vprintf(msgBuffer, list);
}
