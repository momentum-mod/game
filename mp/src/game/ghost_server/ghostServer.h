#include <vector>
#include <thread>
#include <mutex>
#include <time.h>
#include <stdio.h>

#include "mom_shareddefs.h"

//single-file UDP library
#include "zed_net.h"

#define DEFAULT_MAP "triggertests"
#define SECONDS_TO_TIMEOUT 10
struct playerData
{
    int clientIndex;
    ghostNetFrame currentFrame;
    zed_net_socket_t remote_socket;
    zed_net_address_t remote_address;
    playerData(zed_net_socket_t socket, zed_net_address_t addr, int idx)
        : remote_socket(socket), remote_address(addr), clientIndex(idx)
    {
    }
};
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}
class CMOMGhostServer
{
    // EVERYTHING is static because there is no server object, there is only one server.
public:
    static void handlePlayer(playerData *newPlayer);
    static void handleConsoleInput();
    static int runGhostServer(const unsigned short port, const char* mapname);
    static const void newConnection(zed_net_socket_t socket, zed_net_address_t address);
    static void acceptNewConnections();
    static void disconnectPlayer(playerData *player);
    static void conMsg(const char* msg, ...);

    static volatile int numPlayers;
    static bool m_bShouldExit;
    static std::vector<playerData*>m_vecPlayers;
    static std::mutex m_vecPlayers_mutex;
    static std::mutex m_bShouldExit_mutex;

private:
    static zed_net_socket_t m_Socket;
    static int m_iTickRate;
    static char m_szMapName[64];
};

