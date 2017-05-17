#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <time.h>
#include <stdio.h>
#include <queue>
#include <condition_variable>

#include "mom_shareddefs.h"
#include "mom_ghostdefs.h"

//single-file UDP library
#include "zed_net.h"

#define DEFAULT_MAP "triggertests"
#define SECONDS_TO_TIMEOUT 10
#define NEW_MAP_CMD "MOMENTUM_QUEUE_NEWMAP"

template <class T>
class SafeQueue;
struct playerData;
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
    static SafeQueue<char*> m_sqEventQueue;
    static std::mutex m_vecPlayers_mutex;
    static std::mutex m_bShouldExit_mutex;
private:
    static zed_net_socket_t m_Socket;
    static int m_iTickRate;
    static char m_szMapName[64];
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
    {}

    // Add an element to the queue.
    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
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
        std::lock_guard<std::mutex> lock(m_mtx);
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
    ghostNetFrame_t currentFrame;
    ghostAppearance_t currentLooks;
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