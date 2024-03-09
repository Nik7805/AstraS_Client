#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>

class UDPSender
{
    public:

    enum class Exceptions
    {
        OPEN_SOCK_ERR,
    };

    UDPSender(const std::string ip, uint16_t port);
    ~UDPSender();
    int Send(std::string& msg) const;
    int Send(void* data, size_t bytes) const;

    private:
    int m_FD;
    sockaddr_in m_ServAddr;
};