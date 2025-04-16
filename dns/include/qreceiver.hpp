#pragma once

#include <string.h>
#include <sys/socket.h>

#include "config.hpp"
#include "mqueue.hpp"

namespace microdns
{

const size_t MAX_DNS_LEN = 512;
const int MAX_STUB_PENDING = 20;

class QueryReceiver
{
    int socketUDP;
    int socketTCP;
    struct sockaddr_storage ss;

    const std::string *ip;
    unsigned short port;

    char buf[MAX_DNS_LEN];

    MessageQueue &mq;
    const ConfigLoader &cl;

    bool isGood;

  public:
    QueryReceiver(const ConfigLoader &cl, MessageQueue &mq);

    ~QueryReceiver();

    bool good();

    bool fail();

    void start();

    void stop();
};

} // namespace microdns
