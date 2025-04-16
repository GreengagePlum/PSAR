#include <iostream>

#include "config.hpp"
#include "mqueue.hpp"
#include "qreceiver.hpp"

int main(void)
{
    microdns::ConfigLoader cl("./microDNS.conf");
    microdns::MessageQueue mq;
    microdns::QueryReceiver qr(cl, mq);
    if (qr.fail())
    {
        std::cerr << "QueryReceiver init failed in main" << std::endl;
        return -1;
    }
    qr.start();

    return 0;
}
