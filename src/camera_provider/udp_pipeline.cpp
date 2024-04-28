#include <thread>
#include <chrono>
#include "udp_pipeline.hpp"

using namespace std::chrono_literals;

UDPipeline::UDPipeline(const std::string ip, uint16_t port)
{
    m_FD = socket(AF_INET, SOCK_DGRAM, 0);

    if(m_FD < 0)
        throw 0;
    
    bzero(&m_ServAddr, sizeof(m_ServAddr));

    m_ServAddr.sin_family = AF_INET;
    m_ServAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    m_ServAddr.sin_port = htons(port);
}

UDPipeline::~UDPipeline()
{
    close(m_FD);
}

void UDPipeline::SendFrame(OBFormat format, uint16_t width, uint16_t height, void *data)
{
    VideoFrameHeader_t header;
    header.width = width;
    header.height = height;

    switch (format)
    {
    case OB_FORMAT_GRAY:
    case OB_FORMAT_Y16:
    case OB_FORMAT_UYVY:
        header.depth = 2;
        break;
    case OB_FORMAT_BGR:
    case OB_FORMAT_RGB:
        header.depth = 3;
        break;
    case OB_FORMAT_Y8:
        header.depth = 1;
        break;
    
    default:
        std::cerr << "Unsupported frame format!\n";
        return;
    }

    size_t iBytes = static_cast<size_t>(header.depth) *
                    static_cast<size_t>(header.height) *
                    static_cast<size_t>(header.width);
    
    SendRaw(static_cast<void*>(&header), sizeof(VideoFrameHeader_t));
    SendRaw(data, iBytes);
}

int UDPipeline::SendRaw(void *data, size_t bytes) const
{
    const size_t MAX_MSG_SIZE = 1500;
    ssize_t result;
    int sendedBytes = 0;
    while(1)
    {
        if(bytes > MAX_MSG_SIZE)
        {
            result = sendto(m_FD,
                            data, 
                            MAX_MSG_SIZE, 
                            MSG_WAITALL,
                            (sockaddr*)(&m_ServAddr),
                            sizeof(m_ServAddr));
            
            data = (void*)(((uint8_t*)data) + result);
            bytes -= result;
        }
        else
        {
            result = sendto(m_FD,
                            data, 
                            bytes, 
                            MSG_WAITALL,
                            (sockaddr*)(&m_ServAddr),
                            sizeof(m_ServAddr));

            break;
        }

        if(result < 0)
            return result;
        else
            sendedBytes += result;

        std::this_thread::yield();
        // std::this_thread::sleep_for(1us);
    }

    return (result < 0) ? result : sendedBytes + result;
}
