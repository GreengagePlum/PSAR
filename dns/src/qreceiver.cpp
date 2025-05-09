#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <utility>

#include "dns.hpp"
#include "message.hpp"
#include "qreceiver.hpp"

namespace microdns
{

bool QueryReceiver::closeSockets()
{
    int ret = 0;
    if (socketUDP != -1 && (ret |= close(socketUDP)) == -1)
        perror("QueryReceiver UDP socket close failed");
    if (socketTCP != -1 && (ret |= close(socketTCP)) == -1)
        perror("QueryReceiver TCP socket close failed");
    socketUDP = socketTCP = -1;
    return ret == 0;
}

/**
 * This constructor does a few necessary things:
 *
 * - Fetch parameters from config
 * - Configure domain matching regex by escaping domain string into a regex itself first
 * - Create and bind downstream sockets
 *
 */
QueryReceiver::QueryReceiver(const ConfigLoader &cl, MessageQueue &forwardQueue, MessageQueue &managedQueue,
                             MessageQueue &responseQueue)
    : ip(&cl.getParam("LOCAL_DNS_IP")), port(std::stoul(cl.getParam("LOCAL_DNS_PORT"))),
      domain(&cl.getParam("BASE_DOMAIN")),
      domainMatcher("\\." + std::regex_replace(*domain, specialChars, R"(\$&)") + "$"), buf(),
      forwardQueue(forwardQueue), managedQueue(managedQueue), responseQueue(responseQueue), cl(cl), isGood(false),
      isStarted(false), shouldStop(false)
{
    struct sockaddr_in *sin;

    // Create sockets
    if ((socketUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("QueryReceiver UDP socket creation failed");
        return;
    }
    if ((socketTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("QueryReceiver TCP socket creation failed");
        goto sock_fail;
    }

    // Just to ease casting later
    sin = (struct sockaddr_in *)&ss;

    // Initialize addr struct (shared between the two sockets)
    memset(&ss, 0, sizeof(ss));
    ss.ss_family = AF_INET;
    if (inet_pton(AF_INET, ip->c_str(), &sin->sin_addr) == 0)
    {
        std::cerr << "Invalid IP address" << std::endl;
        goto sock_fail;
    }
    sin->sin_port = htons(port);

    // Configure UDP
    if (bind(socketUDP, (struct sockaddr *)&ss, sizeof(struct sockaddr_in)) == -1)
    {
        perror("QueryReceiver UDP bind failed");
        goto sock_fail;
    }

    // Configure TCP
    if (bind(socketTCP, (struct sockaddr *)&ss, sizeof(struct sockaddr_in)) == -1)
    {
        perror("QueryReceiver TCP bind failed");
        goto sock_fail;
    }
    if (listen(socketTCP, MAX_STUB_PENDING) == -1)
    {
        perror("QueryReceiver TCP listen failed");
        goto sock_fail;
    }

    queryMap.reserve(2 << (sizeof(queryIDPool) * 8));
    isGood = true;
    return;

sock_fail:
    closeSockets();
}

bool QueryReceiver::good() const
{
    return isGood;
}

bool QueryReceiver::fail() const
{
    return !isGood;
}

bool QueryReceiver::started() const
{
    return isStarted;
}

bool QueryReceiver::start()
{
    if (isStarted)
        return false;
    if (!isGood)
        return false;

    shouldStop = false;
    threadFinishedInit = false;
    processor = std::thread(&QueryReceiver::processLoop, this);
    // TODO: Maybe reinforce the wait condition since an error in thread creation might cause this context to block
    // forever. Assuming this doesn't happen for now...
    threadFinishedInit.wait(false);
    return isStarted;
}

void QueryReceiver::processLoop()
{
    constexpr nfds_t POLLSIZE = 5;
    struct pollfd pfd[POLLSIZE];
    pfd[0].fd = socketUDP;
    pfd[0].events = POLLIN | POLLOUT;
    pfd[1].fd = socketTCP;
    pfd[1].events = POLLIN;
    pfd[2].fd = forwardQueue.getWritefd();
    pfd[2].events = POLLOUT;
    pfd[3].fd = managedQueue.getWritefd();
    pfd[3].events = POLLOUT;
    pfd[4].fd = responseQueue.getReadfd();
    pfd[4].events = POLLIN;

    int timeout = 0; // Makes `poll` return immediately
    bool alreadyNotified = false;

    int pollret;
    // 0 means timeout reached without any events (only the first time here)
    while (!shouldStop.load(std::memory_order_relaxed) && (pollret = poll(pfd, POLLSIZE, timeout)) >= 0)
    {
        // Notify original thread on our well-being
        if (!alreadyNotified)
        {
            alreadyNotified = true;
            // timeout = -1; // Makes `poll` block indefinitely if no events on file descriptors
            timeout = pollTimeout; // Makes `poll` block for a maximum of 200 milliseconds
            isStarted = true;
            isGood = true;
            threadFinishedInit = true;
            threadFinishedInit.notify_one();
        }

        // UDP, incoming
        if (pfd[0].revents & POLLIN && pfd[2].revents & POLLOUT && pfd[3].revents & POLLOUT)
        {
            if (!handleIncomingUDP())
            {
                std::cerr << "QueryReceiver failed incoming message handling from UDP socket" << std::endl;
                goto error;
            }
        }

        // UDP, outgoing
        if (pfd[4].revents & POLLIN && pfd[0].revents & POLLOUT)
        {
            if (!handleOutgoingUDP())
            {
                std::cerr << "QueryReceiver failed outgoing message handling from UDP socket" << std::endl;
                goto error;
            }
        }

        // UDP, error
        if (pfd[0].revents & POLLERR)
        {
            std::cerr << "QueryReceiver poll returned error for UDP socket" << std::endl;
            goto error;
        }

        // TODO: TCP, incoming, ignoring for now
        if (pfd[1].revents & POLLIN)
        {
            // accept() call should be here...
            pfd[1].fd = -1; // Ignore fd
            std::cerr << "QueryReceiver received from TCP socket, ignoring..." << std::endl;
            continue;
        }
    }

    // -1 means eithor error or signal interrupt
    if (pollret == -1)
        perror("QueryReceiver poll failed or interrupted");
    else // means exiting gracefully due to `shouldStop` flag from `stop()` request
        goto usual;

error:
    closeSockets();
    isGood = false;

usual:
    isStarted = false;
    // Notify original thread if not already
    if (!alreadyNotified)
    {
        threadFinishedInit = true;
        threadFinishedInit.notify_one();
    }
}

bool QueryReceiver::handleIncomingUDP()
{
    struct sockaddr_storage ssret;
    memset(&ssret, 0, sizeof(ssret));
    socklen_t slenret = sizeof(ssret);

    ssize_t ret = recvfrom(socketUDP, buf, sizeof(buf), MSG_TRUNC, (struct sockaddr *)&ssret, &slenret);
    if (ret == -1)
    {
        std::cerr << "QueryReceiver recvfrom error" << std::endl;
        return false;
    }
    if ((unsigned long)ret > sizeof(buf))
    {
        std::cerr << "QueryReceiver big UDP packet skipped" << std::endl;
        return true;
    }

    auto m = std::make_shared<dns::Message>();
    try
    {
        m->decode(buf, ret);
    }
    catch (const std::exception &e)
    {
        std::cerr << "" << e.what() << std::endl;
        return true;
    }

    const auto &queries = m->getQueries();
    if (queries.empty())
        return true;

    // Insert into a map with a local unique ID to keep track of the query response to be delivered back later
    auto retAddr = std::make_shared<QueryReceiver::ReturnAddressDetails>();
    memcpy(&retAddr->ss, &ssret, sizeof(ssret));
    memcpy(&retAddr->slen, &slenret, sizeof(slenret));
    queryMap.insert(std::make_pair(++queryIDPool, retAddr));
    auto queryEntry = std::make_pair(queryIDPool, m);

    /* TODO: Maybe iterate over the queries in a more sophisticated way (possibility of multiple query entries
     * with ours mixed up in there)
     *
     * For now, assuming only one query record per query and that our domain is present in the first
     * query record of a query
     * */

    /* Check multiple things about the query records inside the query:
     *
     * if class = IN (for Internet) and type = 1 (A record for IPv4)
     * if query domain concerns us (mathes with the domain from the config)
     *
     * otherwise put query in the forward queue (not our concern)
     */
    if (queries[0]->getClass() == dns::eQClass::QCLASS_IN && queries[0]->getType() == 1 &&
        std::regex_search(queries[0]->getName(), domainMatcher))
    {
        std::cerr << "Pushing to managedQueue: " << m->asString() << std::endl;
        managedQueue.push(queryEntry);
    }
    else
    {
        std::cerr << "Pushing to forwardQueue: " << m->asString() << std::endl;
        forwardQueue.push(queryEntry);
    }
    return true;
}

bool QueryReceiver::handleOutgoingUDP()
{
    auto queryEntry = responseQueue.pop();
    auto retAddr = queryMap.at(queryEntry.first);
    if (queryMap.erase(queryEntry.first) != 1)
    {
        std::cerr << "QueryReceiver queryMap erase error" << std::endl;
        return false;
    }

    dns::uint encodedSize;
    try
    {
        queryEntry.second->encode(buf, sizeof(buf), encodedSize);
    }
    catch (const std::exception &e)
    {
        std::cerr << "" << e.what() << std::endl;
        return true;
    }

    if (sendto(socketUDP, buf, encodedSize, 0, (struct sockaddr *)&retAddr->ss, retAddr->slen) != encodedSize)
    {
        perror("QueryReceiver UDP sendto error");
        return false;
    }

    return true;
}

bool QueryReceiver::stop()
{
    if (!isStarted)
        return false;
    shouldStop.store(true, std::memory_order_relaxed);
    processor.join();
    isStarted = false;
    return true;
}

QueryReceiver::~QueryReceiver()
{
    closeSockets();
}

const ConfigLoader &QueryReceiver::getConfig() const
{
    return cl;
}

} // namespace microdns
