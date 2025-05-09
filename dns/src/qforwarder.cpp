#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

#include "dns.hpp"
#include "message.hpp"
#include "qforwarder.hpp"

namespace microdns
{

bool QueryForwarder::closeSockets()
{
    int ret = 0;
    if (socketUDP != -1 && (ret |= close(socketUDP)) == -1)
        perror("QueryForwarder UDP socket close failed");
    socketUDP = -1;
    return ret == 0;
}

/**
 * This constructor does a few necessary things:
 *
 * - Fetch parameters from config
 * - Create and bind upstream socket
 *
 */
QueryForwarder::QueryForwarder(const ConfigLoader &cl, MessageQueue &queryQueue, MessageQueue &responseQueue)
    : sendbuf(), recvbuf(), queryQueue(queryQueue), responseQueue(responseQueue), cl(cl), isGood(false),
      isStarted(false), shouldStop(false)
{
    // Create socket
    if ((socketUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("QueryForwarder UDP socket creation failed");
        return;
    }

    for (const auto &ip : cl.getNameservers())
    {
        struct sockaddr_storage ss;
        memset(&ss, 0, sizeof(ss));

        // Just to ease casting later
        struct sockaddr_in *sin = (struct sockaddr_in *)&ss;

        // Initialize addr struct
        ss.ss_family = AF_INET;
        // sin->sin_family = AF_INET;
        sin->sin_addr = ip;
        sin->sin_port = htons(53);

        nameservers.emplace_back(std::move(ss));
    }

    isGood = true;
}

bool QueryForwarder::good() const
{
    return isGood;
}

bool QueryForwarder::fail() const
{
    return !isGood;
}

bool QueryForwarder::started() const
{
    return isStarted;
}

bool QueryForwarder::start()
{
    if (isStarted)
        return false;
    if (!isGood)
        return false;

    shouldStop = false;
    threadFinishedInit = false;
    processor = std::thread(&QueryForwarder::processLoop, this);
    // TODO: Maybe reinforce the wait condition since an error in thread creation might cause this context to block
    // forever. Assuming this doesn't happen for now...
    threadFinishedInit.wait(false);
    return isStarted;
}

void QueryForwarder::processLoop()
{
    isStarted = true;
    isGood = true;
    threadFinishedInit = true;
    threadFinishedInit.notify_one();

    while (!shouldStop.load(std::memory_order_relaxed))
    {
        if (!handleOutgoingUDP())
        {
            std::cerr << "QueryForwarder failed outgoing message handling from UDP socket" << std::endl;
            goto error;
        }
    }
    goto usual;

error:
    closeSockets();
    isGood = false;

usual:
    isStarted = false;
}

bool QueryForwarder::handleOutgoingUDP()
{
    auto queryEntry = queryQueue.try_pop(std::chrono::milliseconds(mqueueWaitTimeout));
    if (!queryEntry)
        return true;

    dns::uint encodedSize;
    try
    {
        queryEntry->second->encode(sendbuf, sizeof(sendbuf), encodedSize);
    }
    catch (const std::exception &e)
    {
        std::cerr << "" << e.what() << std::endl;
        queryEntry->second->setQr(1);    // Set query to reply
        queryEntry->second->setRCode(2); // Set response code to `SERVFAIL`
        responseQueue.push(*queryEntry);
        return true;
    }

    fd_set readfds, exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&exceptfds);
    FD_SET(socketUDP, &readfds);
    FD_SET(socketUDP, &exceptfds);
    for (const auto &addr : nameservers)
    {
        unsigned originalTimeout = startingResponseTimeout; // in seconds
        struct timeval timeout = {originalTimeout, 0};
        for (unsigned long i = 0; i < retriesPerNameserver; i++)
        {
            if (sendto(socketUDP, sendbuf, encodedSize, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) !=
                encodedSize)
            {
                perror("QueryForwarder UDP sendto error");
                return false;
            }

            int retSelect;
            while ((retSelect = select(socketUDP + 1, &readfds, NULL, &exceptfds, &timeout)) > 0 &&
                   (timeout.tv_sec > 0 || timeout.tv_usec > 0))
            {
                // TODO: Should maybe set `O_NONBLOCK` for this socket since it is said even after `select()` reads
                // could block
                ssize_t ret = recvfrom(socketUDP, recvbuf, sizeof(recvbuf), MSG_TRUNC, NULL, NULL);
                if (ret == -1)
                {
                    std::cerr << "QueryForwarder recvfrom error" << std::endl;
                    return false;
                }
                if ((unsigned long)ret > sizeof(recvbuf))
                {
                    std::cerr << "QueryForwarder big UDP packet skipped" << std::endl;
                    continue;
                }
                auto m = std::make_shared<dns::Message>();
                try
                {
                    m->decode(recvbuf, ret);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "" << e.what() << std::endl;
                    continue;
                }
                if (m->getId() == queryEntry->second->getId())
                {
                    responseQueue.push(std::make_pair(queryEntry->first, m));
                    return true;
                }
            }
            if (retSelect == -1)
            {
                perror("QueryForwarder UDP select error");
                return false;
            }
            timeout = {originalTimeout <<= 1, 0};
            if (shouldStop.load(std::memory_order_relaxed))
                return true;
        }
    }

    queryEntry->second->setQr(1);    // Set query to reply
    queryEntry->second->setRCode(3); // Set response code to `NXDOMAIN`
    responseQueue.push(*queryEntry);
    return true;
}

bool QueryForwarder::stop()
{
    if (!isStarted)
        return false;
    shouldStop.store(true, std::memory_order_relaxed);
    processor.join();
    isStarted = false;
    return true;
}

QueryForwarder::~QueryForwarder()
{
    closeSockets();
}

const ConfigLoader &QueryForwarder::getConfig() const
{
    return cl;
}

} // namespace microdns
