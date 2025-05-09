#pragma once

#include <sys/socket.h>

#include <thread>

#include "common.hpp"
#include "config.hpp"
#include "mqueue.hpp"

namespace microdns
{

class QueryForwarder
{
    int socketUDP = -1;
    const unsigned mqueueWaitTimeout = 200;     // in milliseconds, maximum blocking time waiting on queryQueue
    const unsigned startingResponseTimeout = 1; // in seconds, starting value for exponentially increasing timeout
    const unsigned retriesPerNameserver = 3;

    std::vector<struct sockaddr_storage> nameservers;

    char sendbuf[MAX_DNS_LEN];
    char recvbuf[MAX_DNS_LEN];

    MessageQueue &queryQueue;
    MessageQueue &responseQueue;
    const ConfigLoader &cl;

    std::atomic_bool isGood;

    std::thread processor;
    std::atomic_bool isStarted;
    std::atomic_bool shouldStop;
    std::atomic_bool threadFinishedInit;

    bool closeSockets();
    void processLoop();
    bool handleOutgoingUDP();

  public:
    QueryForwarder(const ConfigLoader &cl, MessageQueue &queryQueue, MessageQueue &responseQueue);

    ~QueryForwarder();

    bool good() const;

    bool fail() const;

    bool started() const;

    bool start();

    bool stop();

    const ConfigLoader &getConfig() const;
};

} // namespace microdns
