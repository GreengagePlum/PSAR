#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "message.hpp"

int main(void)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));
    ss.ss_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &((struct sockaddr_in *)&ss)->sin_addr.s_addr);
    // ((struct sockaddr_in *)&ss)->sin_port = htons(10000);
    ((struct sockaddr_in *)&ss)->sin_port = htons(53);
    if (bind(s, (struct sockaddr *)&ss, sizeof(struct sockaddr_in)) == -1)
    {
        std::cerr << "bind failed" << std::endl;
        perror("Error reason: ");
        return -1;
    }

    char buf[512];
    dns::Message m;
    for (int i = 0;; i++)
    {
        struct sockaddr_storage ss2;
        memset(&ss2, 0, sizeof(ss2));
        socklen_t slen = sizeof(ss2);
        ssize_t ret = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&ss2, &slen);
        if (ret == -1)
        {
            std::cerr << "recvfrom error" << std::endl;
            break;
        }
        // if (ret == 0) {
        //   std::cerr << "recvfrom error" << std::endl;
        //   break;
        // }
        if ((unsigned long)ret >= sizeof(buf))
        {
            std::cerr << "big packet skipped" << std::endl;
            continue;
        }
        m.decode(buf, ret);
        std::cout << "[" << i << "]" << m.asString();
        std::cout << m.getQueries()[0]->getName() << std::endl;
        m.setQr(1);
        m.setRCode(3);
        unsigned size;
        m.encode(buf, sizeof(buf), size);
        sendto(s, buf, size, 0, (struct sockaddr *)&ss2, slen);
    }

    close(s);
    return 0;
}
