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
    char map[96];
    _snprintf(map, sizeof(map), "%s", DEFAULT_MAP);
    uint16_t port = DEFAULT_PORT, steamPort = DEFAULT_STEAM_PORT, masterServerPort = DEFAULT_MASTER_SERVER_PORT;
    bool shouldAuthenticate = true;
    bool serverStatus = false;

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
        if (strcmp(argv[i], "-steamport") == 0)
        {
            steamPort = uint16_t(atoi(argv[i + 1]));
        }
        if (strcmp(argv[i], "-masterport") == 0)
        {
            masterServerPort = uint16_t(atoi(argv[i + 1]));
        }
        if (strcmp(argv[i], "-insecure") == 0)
        {
            shouldAuthenticate = false;
        }
    }

    serverStatus = CMOMGhostServer::runGhostServer(port, steamPort, masterServerPort, shouldAuthenticate, map);

    if (!serverStatus)
    {
        CMOMGhostServer::conMsg("ERROR! Server not started\n");
        return 0;
    }

    std::thread t(CMOMGhostServer::acceptNewConnections); //create a new thread that listens for incoming client connections
    t.detach(); //continuously run thread

    while (!CMOMGhostServer::m_bShouldExit)
    {
        CMOMGhostServer::handleConsoleInput();
    }
    SteamAPI_Shutdown();
#ifdef _WIN32
    SetConsoleMode(input_handle, dwOldInputMode); //ignores mouse input on the input buffer
#endif

    return 0;
}
void CMOMGhostServer::acceptNewConnections()
{

}
bool CMOMGhostServer::runGhostServer(uint16_t port, uint16_t steamPort, uint16_t masterServerPort, bool shouldAuthenticate, const char* mapName)
{
    _snprintf(m_szMapName, sizeof(m_szMapName), "%s", mapName);

    SteamGameServer_InitSafe(INADDR_ANY, port, steamPort, masterServerPort, 
        shouldAuthenticate ? eServerModeAuthenticationAndSecure : eServerModeNoAuthentication, GHOST_SERVER_VERSION);

    if (!steamapicontext->Init())
    {
        conMsg("ERROR: COULD NOT INIT STEAM API!\n");
        return false;
    }
    conMsg("Running ghost server on %s on port %d!\n", mapName, port);
    return true;

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
        //m_sqEventQueue.enqueue(NEW_MAP_CMD);
    }
    if (strcmp(command, "currentmap") == 0)
    {
        conMsg("Current map is: %s\n", m_szMapName);
    }
}
/*
void CMOMGhostServer::handlePlayer(playerData *newPlayer)
{

    static thread_local bool FirstNewFrame = true;
    if (FirstNewFrame)
    {
        //send a full signon of ALL the players currently in the server.
        m_vecPlayers_mutex.lock();
        int signOnSize = sizeof(ghostSignOnPacket_t);
        char *buffer = new char[signOnSize * numPlayers];
        for (int i = 0; i < numPlayers; i++)
        {
            ghostSignOnPacket_t newPlayerSignOn;
            newPlayerSignOn.newApps = m_vecPlayers[i]->currentLooks;
            newPlayerSignOn.newFrame = m_vecPlayers[i]->currentFrame;
            newPlayerSignOn.SteamID = m_vecPlayers[i]->SteamID64;
            memcpy(buffer + (signOnSize * i), &newPlayerSignOn, signOnSize);
        }
        zed_net_tcp_socket_send(&newPlayer->remote_socket, buffer, numPlayers * signOnSize, PT_SIGNON);
        m_vecPlayers_mutex.unlock();
        FirstNewFrame = false;
    }
    m_sqEventQueue.enqueue(NEW_FRAMES_CMD);
    //process the event queue first.
    if (m_sqEventQueue.numInQueue() != 0) //queue is not empty
    {
        char* newEvent = m_sqEventQueue.dequeue();
        if (strcmp(newEvent, NEW_FRAMES_CMD) == 0)
        {
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
        }
        if (strcmp(newEvent, NEW_MAP_CMD) == 0)
        {
            ghostNewMapEvent_t newMapEvent;
            strncpy(newMapEvent.MapName, m_szMapName, sizeof(newMapEvent.MapName));
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &newMapEvent, sizeof(newMapEvent), PT_NEWMAP);
        }
        if (strcmp(newEvent, NEW_APPEARENCES_CMD) == 0)
        {
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
            if (newPlayer->SteamID64 != newPlayerSignOn.SteamID) //don't send ourselves the signon packet since we got one when we first connected.
            {
                printf("Sending new signon packet for %s to %s\n", newPlayerSignOn.newFrame.PlayerName, newPlayer->currentFrame.PlayerName);
                zed_net_tcp_socket_send(&newPlayer->remote_socket, &newPlayerSignOn, sizeof(ghostSignOnPacket_t), PT_SIGNON);
            }

            m_vecPlayers_mutex.unlock();
        }
    }

    int packet_type;
    char buffer[uint16_t(~0)]; //largest packet size we can expect, 65536
    int bytes_read = zed_net_tcp_socket_receive(&newPlayer->remote_socket, buffer, uint16_t(~0), &packet_type);

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
            ghostSignOffPacket_t *newSignOff = reinterpret_cast<ghostSignOffPacket_t*>(buffer);
            conMsg("%s disconnected: %s\n", newPlayer->currentFrame.PlayerName, newSignOff->Message);
            ghostAckPacket_t newAck;
            newAck.AckSuccess = true;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &newAck, sizeof(newAck), PT_ACK);
            FirstNewFrame = true;
            disconnectPlayer(newPlayer);
        }
    }
    const auto t1 = Clock::now();
    while (bytes_read <= 0) //Oops, we didn't get anything from the client!
    {
        const auto t2 = Clock::now();
        const auto deltaT = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
        conMsg("%s lost connection! Waiting to time out... %ll\n", newPlayer->currentFrame.PlayerName, deltaT.count());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (deltaT > m_secondsToTimeout)
        {
            conMsg("%s timed out...\n", newPlayer->currentFrame.PlayerName);
            disconnectPlayer(newPlayer);
            FirstNewFrame = true;
            break;
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
*/
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
