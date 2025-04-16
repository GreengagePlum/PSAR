#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <exception>
#include <iostream>
#include <string>

#include "message.hpp"
#include "qreceiver.hpp"

namespace microdns
{

QueryReceiver::QueryReceiver(const ConfigLoader &cl, MessageQueue &mq) : buf(), mq(mq), cl(cl)
{
    isGood = false;

    // Fetch config parameters
    try
    {
        ip = &cl.getParam("LOCAL_DNS_IP");
        port = std::stoul(cl.getParam("LOCAL_DNS_PORT"));
    }
    catch (const std::exception &e)
    {
        // TODO: Log better
        std::cerr << e.what() << std::endl;
        return;
    }

    // Create sockets,
    if ((socketUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("QueryReceiver UDP socket creation failed");
        return;
    }
    if ((socketTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("QueryReceiver TCP socket creation failed");
        if (close(socketUDP) == -1)
            perror("QueryReceiver UDP socket close failed");
        return;
    }

    // Just to ease casting later
    struct sockaddr_in *sin = (struct sockaddr_in *)&ss;

    // Initialize addr struct (shared between two sockets)
    memset(&ss, 0, sizeof(ss));
    ss.ss_family = AF_INET;
    if (inet_pton(AF_INET, ip->c_str(), &sin->sin_addr.s_addr) == 0)
    {
        std::cerr << "Invalid IP address" << std::endl;
        return;
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

    isGood = true;
    return;

sock_fail:
    if (close(socketUDP) == -1)
        perror("QueryReceiver UDP socket close failed");
    if (close(socketTCP) == -1)
        perror("QueryReceiver TCP socket close failed");
}

bool QueryReceiver::good()
{
    return isGood;
}

bool QueryReceiver::fail()
{
    return !isGood;
}

void QueryReceiver::start()
{
    struct pollfd pfd[2];
    pfd[0].fd = socketUDP;
    pfd[0].events = POLLIN;
    pfd[1].fd = socketTCP;
    pfd[1].events = POLLIN;

    int p;
    dns::Message m;
    int i = 0;
    while ((p = poll(pfd, 2, -1)) > 0)
    {
        // UDP
        if (pfd[0].revents & POLLIN)
        {
            struct sockaddr_storage ss2;
            memset(&ss2, 0, sizeof(ss2));
            socklen_t slen = sizeof(ss2);
            ssize_t ret = recvfrom(socketUDP, buf, sizeof(buf), 0, (struct sockaddr *)&ss2, &slen);
            if (ret == -1)
            {
                std::cerr << "recvfrom error" << std::endl;
                break;
            }
            if ((unsigned long)ret >= sizeof(buf))
            {
                std::cerr << "big packet skipped" << std::endl;
                continue;
            }
            m.decode(buf, ret);
            std::cout << "[" << i++ << "]" << m.asString();
            m.setQr(1);
            m.setRCode(3);
            unsigned size;
            m.encode(buf, sizeof(buf), size);
            sendto(socketUDP, buf, size, 0, (struct sockaddr *)&ss2, slen);
        }

        // TCP
        if (pfd[1].revents & POLLIN)
        {
            // accept() call should be here...
            pfd[1].fd = -1; // Ignore fd
            std::cerr << "QueryReceiver received from TCP socket, ignoring..." << std::endl;
            continue;
        }
    }

    // 0 means timeout reached without anything
    // -1 means eithor error or signal
    if (p == -1)
    {
        perror("QueryReceiver poll failed");
        if (close(socketUDP) == -1)
            perror("QueryReceiver UDP socket close failed");
        if (close(socketTCP) == -1)
            perror("QueryReceiver TCP socket close failed");
        isGood = false;
    }
}

QueryReceiver::~QueryReceiver()
{
    if (close(socketUDP) == -1)
        perror("QueryReceiver UDP socket close failed");
    if (close(socketTCP) == -1)
        perror("QueryReceiver TCP socket close failed");
}

} // namespace microdns
