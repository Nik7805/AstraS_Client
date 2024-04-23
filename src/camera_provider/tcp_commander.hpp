#pragma once

#include <queue>
#include <stdint.h>
#include <mutex>
#include "camera_provider.hpp"

class TcpCommander: public CameraProvider::IStreamCommander
{
public:
    TcpCommander(uint16_t port);
    void Process();
    virtual Command GetNextCommand() override;
    virtual void SendResponce(uint64_t responce) override;

private:
    int m_port = 0;
    int m_socket_desc = 0;
    int m_client_sock_desc = 0;
    sockaddr_in m_serv_addr;
    std::mutex m_qmutex;
    std::queue<CameraProvider::IStreamCommander::Command> m_CommandQueue;

    bool ParseAndPushCommand(char* cmd);
};