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
#include "camera_provider.hpp"

class UDPipeline: public CameraProvider::IStreamPipeline
{
    public:
    UDPipeline(const std::string ip, uint16_t port);
    ~UDPipeline();
    virtual void SendFrame(OBFormat format, uint16_t width, uint16_t height, void* data) override;

    private:
    #pragma pack(push, 1)
    struct VideoFrameHeader_t
    {
        uint64_t sync = 0xAA00CC55EE77FF99;
        uint64_t frameID;
        uint16_t width;
        uint16_t height;
        uint16_t depth;
    };
    #pragma pack(pop)

    int m_FD;
    sockaddr_in m_ServAddr;

    int SendRaw(void* data, size_t bytes) const;
};