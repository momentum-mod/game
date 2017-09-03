#pragma once
#include <vector>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <queue>
#include <condition_variable>
#include <mutex>

#include "mom_shareddefs.h"
#include "mom_ghostdefs.h"

#ifndef V_snprintf 
#define V_snprintf _snprintf 
#endif

#include "steam\steam_gameserver.h"

#ifdef _WIN32
#include <Windows.h>
#define thread_local __declspec(thread)
#else
#define _snprintf snprintf
#define thread_local __thread
#endif

#define GHOST_SERVER_VERSION "0.1"
#define DEFAULT_MAP "triggertests"

#define SECONDS_TO_TIMEOUT 10

CSteamAPIContext *steamapicontext;

template <class T>class SafeQueue;
struct playerData;

class CMOMGhostServer
{
    // EVERYTHING is static because there is no server object, there is only one server.
public:
    static void handlePlayer(playerData *newPlayer);
    static void handleConsoleInput();
    static bool runGhostServer(uint16_t externalPort, uint16_t steamPort, uint16_t masterServerPort, bool shouldAuthenticate, const char *mapname);
    static void newConnection();
    static void acceptNewConnections();
    static void disconnectPlayer(playerData *player);
    static void sendNewAppearances(playerData *player);
    static void conMsg(const char* msg, ...);

    static volatile int numPlayers;
    static bool m_bShouldExit;
    static std::vector<playerData*>m_vecPlayers;
    static SafeQueue<char*> m_sqEventQueue;
    static std::mutex m_vecPlayers_mutex;
    static std::mutex m_bShouldExit_mutex;
private:
    static int m_iTickRate;
    static char m_szMapName[96];
    static const std::chrono::seconds m_secondsToTimeout;
};
// A threadsafe-queue.
template <class T>
class SafeQueue
{
public:
    SafeQueue()
        : m_queue()
        , m_mtx()
        , m_condVar()
    {}

    ~SafeQueue()
    {
        while (!m_queue.empty()) m_queue.pop();
    }

    // Add an element to the queue.
    void enqueue(T t)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_queue.push(t);
        m_condVar.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_queue.empty())
        {
            // release lock as long as the wait and reaquire it afterwards.
            m_condVar.wait(lock);
        }
        T val = m_queue.front();
        m_queue.pop();
        return val;
    }
    int numInQueue()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        return m_queue.size();
    }
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mtx;
    std::condition_variable m_condVar;
};

struct playerData
{
    int clientIndex;
    PositionPacket_t currentFrame;
    ghostAppearance_t currentLooks;
    uint64_t SteamID64;
    playerData(PositionPacket_t frame, ghostAppearance_t looks, int idx, uint64_t steamID)
        : clientIndex(idx), currentFrame(frame), currentLooks(looks), SteamID64(steamID)
    {
    }
};
inline std::string currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return std::move(std::string(buf));
}
