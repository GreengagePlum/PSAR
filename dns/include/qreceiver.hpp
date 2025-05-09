#pragma once

#include <sys/socket.h>

#include <memory>
#include <regex>
#include <thread>
#include <unordered_map>

#include "common.hpp"
#include "config.hpp"
#include "mqueue.hpp"

namespace microdns
{

class QueryReceiver
{
    int socketUDP = -1;
    int socketTCP = -1;
    struct sockaddr_storage ss;
    const int pollTimeout = 200; // in milliseconds, how frequently should the processor thread check for stop condition

    const std::string *const ip;
    const unsigned short port;
    const std::string *const domain;

    // matches any characters that need to be escaped in RegEx, (actually unnecessary except for '-')
    const std::regex specialChars{R"([-[\]{}()*+?.,\^$|#\s])"};
    const std::regex domainMatcher;

    char buf[MAX_DNS_LEN];

    MessageQueue &forwardQueue;
    MessageQueue &managedQueue;
    MessageQueue &responseQueue;
    const ConfigLoader &cl;

    std::atomic_bool isGood;

    std::thread processor;
    std::atomic_bool isStarted;
    std::atomic_bool shouldStop;
    std::atomic_bool threadFinishedInit;

    QueryID queryIDPool; // Local IDs to avoid DNS transaction ID collisions
    typedef struct
    {
        struct sockaddr_storage ss;
        socklen_t slen;
    } ReturnAddressDetails;
    std::unordered_map<QueryID, std::shared_ptr<ReturnAddressDetails>> queryMap;

    bool closeSockets();
    void processLoop();
    bool handleIncomingUDP();
    bool handleOutgoingUDP();

  public:
    QueryReceiver(const ConfigLoader &cl, MessageQueue &forwardQueue, MessageQueue &managedQueue,
                  MessageQueue &responseQueue);

    ~QueryReceiver();

    bool good() const;

    bool fail() const;

    bool started() const;

    bool start();

    bool stop();

    const ConfigLoader &getConfig() const;
};

} // namespace microdns
