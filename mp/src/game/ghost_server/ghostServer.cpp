#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <stdarg.h>

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
char CMOMGhostServer::m_szMapName[64];

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

    for (int i = 0; i < argc; i++)
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
const void CMOMGhostServer::newConnection(zed_net_socket_t socket, zed_net_address_t address)
{ 
    const char* host;
    int data;
    host = zed_net_host_to_str(address.host);
    conMsg("Accepted connection from %s:%d\n", host, address.port);

    host = zed_net_host_to_str(address.host);
    int bytes_read = zed_net_tcp_socket_receive(&socket, &data, sizeof(data));
    if (bytes_read)
    {
        //printf("Received %d bytes from %s:%d:\n", bytes_read, host, address.port);
        if (data == MOM_SIGNON) //Player signs on for the first time
        {
            //printf("Data matches MOM_SIGNON pattern!\n");

            //send ACK to client that they are connected. This is the current mapname.
            zed_net_tcp_socket_send(&socket, &m_szMapName, sizeof(m_szMapName));

            //Describes a newly connected player with client idx equal to the current number of players
            playerData *newPlayer = new playerData(socket, address, numPlayers); 

            m_vecPlayers_mutex.lock();

            m_vecPlayers.push_back(newPlayer);
            numPlayers = m_vecPlayers.size();

            m_vecPlayers_mutex.unlock();

            //listen(sock->handle, SOMAXCONN) != 0
            while (newPlayer->remote_socket.ready == 0) //socket is open
            {
                handlePlayer(newPlayer);
            }
            delete newPlayer;
        }
    }
    //printf("Thread terminating\n");
    //End of thread
}
void CMOMGhostServer::acceptNewConnections()
{
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    remote_socket.held_bytes = 0;
    remote_socket.prefix_size = 2;

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
    int data, bytes_read = 0;
    auto t1 = Clock::now(); 
    bytes_read = zed_net_tcp_socket_receive(&newPlayer->remote_socket, &data, sizeof(data));
    while (bytes_read == 0) //Oops, we didn't get anything from the client!
    {
        auto t2 = Clock::now();
        auto deltaT = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
        conMsg("Lost connection! Waiting to time out... %ll\n", deltaT.count());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (deltaT > std::chrono::seconds(SECONDS_TO_TIMEOUT))
        {
            conMsg("%s timed out...\n", newPlayer->currentFrame.PlayerName);
            disconnectPlayer(newPlayer);
            break;
        }
    }
    if (bytes_read && newPlayer)
    {
        static thread_local bool firstNewFrame = true;
        static thread_local bool firstNewSentFrame = true;
        if (data == MOM_C_SENDING_NEWPROPS)
        {
            data = MOM_C_SENDING_NEWPROPS;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data)); //SYN-ACK
            // Wait for client to get our acknowledgement, and recieve appearance update from client
            zed_net_tcp_socket_receive(&newPlayer->remote_socket, &newPlayer->currentLooks, sizeof(ghostAppearance_t)); //ACK
            m_sqEventQueue.enqueue(NEW_APPEARENCES_CMD); //queue up the new appearances to be sent 
        }
        if (data == MOM_C_SENDING_NEWFRAME)
        {
            data = MOM_C_RECIEVING_NEWFRAME;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data)); //SYN-ACK
            // Wait for client to get our acknowledgement, and recieve frame update from client
            zed_net_tcp_socket_receive(&newPlayer->remote_socket, &newPlayer->currentFrame, sizeof(ghostNetFrame_t)); //ACK
            if (firstNewFrame)
            {
                conMsg("%s connected!\n", newPlayer->currentFrame.PlayerName);
                zed_net_tcp_socket_receive(&newPlayer->remote_socket, &newPlayer->currentLooks, sizeof(ghostAppearance_t)); 
                firstNewFrame = false;
            }
        }
        if (data == MOM_S_RECIEVING_NEWFRAME) //SYN
        {
            // Send out a ACK to the client that we're going to be sending them an update
            data = MOM_S_SENDING_NEWFRAME;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data)); //ACK
            int playerNum = numPlayers;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &playerNum, sizeof(playerNum));

            for (int i = 0; i < playerNum; i++)
            {
                m_vecPlayers_mutex.lock();
                zed_net_tcp_socket_send(&newPlayer->remote_socket, &m_vecPlayers[i]->currentFrame, sizeof(ghostNetFrame_t)); 
                if (firstNewSentFrame)
                {
                    zed_net_tcp_socket_send(&newPlayer->remote_socket, &m_vecPlayers[i]->currentLooks, sizeof(ghostAppearance_t));
                    firstNewSentFrame = false;
                }
                m_vecPlayers_mutex.unlock();
            }
            
        }
        if (data == MOM_SIGNOFF)
        {
            conMsg("%s disconnected...\n", newPlayer->currentFrame.PlayerName);
            disconnectPlayer(newPlayer);
            firstNewFrame = true;
            firstNewSentFrame = true;
            //printf("Remote socket status: %i\n", newPlayer->remote_socket.ready);
        }
    }
    if (m_sqEventQueue.numInQueue() != 0) //queue is not empty
    {
        char* newEvent = m_sqEventQueue.dequeue();
        if (strcmp(newEvent, NEW_MAP_CMD) == 0)
        {
            int data = MOM_S_SENDING_NEWMAP;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data));
            zed_net_tcp_socket_send(&newPlayer->remote_socket, m_szMapName, sizeof(m_szMapName));
        }
        if (strcmp(newEvent, NEW_APPEARENCES_CMD) == 0)
        {
            int data = MOM_S_SENDING_NEWPROPS;
            zed_net_tcp_socket_send(&newPlayer->remote_socket, &data, sizeof(data));
            bytes_read = zed_net_tcp_socket_receive(&newPlayer->remote_socket, &data, sizeof(data));
            if (bytes_read && data == MOM_S_RECIEVING_NEWPROPS)
            {
                zed_net_tcp_socket_send(&newPlayer->remote_socket, &newPlayer->currentFrame.SteamID64, sizeof(newPlayer->currentFrame.SteamID64));
                zed_net_tcp_socket_send(&newPlayer->remote_socket, &newPlayer->currentLooks, sizeof(ghostAppearance_t));
            }
        }
    }
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
void CMOMGhostServer::disconnectPlayer(playerData *player)
{
    int data = MOM_SIGNOFF;
    zed_net_tcp_socket_send(&player->remote_socket, &data, sizeof(data)); //send back an ACK that they are disconnecting.
    zed_net_socket_close(&player->remote_socket);
    m_vecPlayers_mutex.lock();

    m_vecPlayers.erase(m_vecPlayers.begin() + player->clientIndex);
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
