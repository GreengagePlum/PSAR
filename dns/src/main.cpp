#include <signal.h>

#include <iostream>

#include "config.hpp"
#include "mqueue.hpp"
#include "qforwarder.hpp"
#include "qreceiver.hpp"

int main(void)
{
    microdns::ConfigLoader cl("./microDNS.conf");
    microdns::MessageQueue managed, forward, response;
    microdns::QueryReceiver qr(cl, forward, managed, response);
    microdns::QueryForwarder qf(cl, forward, response);
    if (qr.fail() || qf.fail())
    {
        std::cerr << "Main QueryReceiver or QueryForwarder init failed" << std::endl;
        return -1;
    }

    // Block signals in all threads
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    qr.start();
    if (qr.fail())
    {
        std::cerr << "Main QueryReceiver start failed" << std::endl;
        return -1;
    }

    qf.start();
    if (qf.fail())
    {
        std::cerr << "Main QueryForwarder start failed" << std::endl;
        return -1;
    }

    // Wait for signal
    int sig;
    sigwait(&sigset, &sig);
    std::cout << "Main signal received. Initiating shutdown..." << std::endl;

    // qr.stop();
    // qf.stop();
    if ((int)!qr.stop() | !qf.stop())
        return -1;

    return 0;
}
