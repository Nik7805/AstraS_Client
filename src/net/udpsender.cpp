#include <thread>
#include <chrono>
#include "udpsender.hpp"

using namespace std::chrono_literals;

UDPSender::UDPSender(const std::string ip, uint16_t port)
{
    m_FD = socket(AF_INET, SOCK_DGRAM, 0);

    if(m_FD < 0)
        throw Exceptions::OPEN_SOCK_ERR;
    
    bzero(&m_ServAddr, sizeof(m_ServAddr));

    m_ServAddr.sin_family = AF_INET;
    m_ServAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    m_ServAddr.sin_port = htons(port);
}

UDPSender::~UDPSender()
{
    close(m_FD);
}

int UDPSender::Send(std::string &msg) const
{
    return this->Send((void*)msg.c_str(), msg.length());
}

int UDPSender::Send(void *data, size_t bytes) const
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

        std::this_thread::sleep_for(1us);
    }

    return (result < 0) ? result : sendedBytes + result;
}
