#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <chrono>
#include "tcp_commander.hpp"

using namespace std::chrono_literals;


TcpCommander::TcpCommander(uint16_t port)
{
    m_port = port;
    bzero((char*)&m_serv_addr, sizeof(m_serv_addr));
    m_serv_addr.sin_family = AF_INET;
    m_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serv_addr.sin_port = htons(port);

    m_socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(m_socket_desc < 0)
    {
        std::cerr << "Can't open socket\n";
        throw 0;
    }

    if(bind(m_socket_desc, (struct sockaddr*) &m_serv_addr, sizeof(m_serv_addr)) < 0)
    {
        std::cerr << "Error binding socket to local address\n";
        close(m_socket_desc);
        throw 1;
    }
}

void TcpCommander::Process()
{
    bool continue_work = true;
    while(continue_work)
    {
        std::cout << "Waiting for a client to connect...\n";

        listen(m_socket_desc, 5);

        sockaddr_in new_sock_addr;
        socklen_t new_sock_addr_size = sizeof(new_sock_addr);
        m_client_sock_desc = accept(m_socket_desc, (sockaddr *)&new_sock_addr, &new_sock_addr_size);

        if(m_client_sock_desc < 0)
        {
            std::cerr << "Error accepting request from client!\n";
            close(m_socket_desc);
            throw 2;
        }

        std::cout << "Connected with client!\n";
        int bytes_read;
        while(1)
        {
            send(m_client_sock_desc, ">", 1, 0);

            std::cout << "Awaiting client response...\n";
            char msg[1500];
            memset(&msg, 0, sizeof(msg));
            bytes_read += recv(m_client_sock_desc, (char*)&msg, sizeof(msg), 0);

            if(strcmp(msg, "exit\n") == 0)
            {
                std::cout << "Client has quit the session\n";
                break;
            }

            std::cout << "Client: " << msg << "\n";

            if(strchr(msg, '\n') != NULL)
            {
                if(!ParseAndPushCommand(msg))
                {
                    const char* responce = "Message signature is wrong!\n";
                    send(m_client_sock_desc, responce, strlen(responce), 0);
                }
                else
                {
                    std::cout << "Command added sucessfuly.\n";
                }
            }
        }
        close(m_client_sock_desc);
        m_client_sock_desc = -1;
        std::cout << "Connection closed...\n";
    }

    close(m_socket_desc);
}

CameraProvider::IStreamCommander::Command TcpCommander::GetNextCommand()
{
    Command cmd;
    m_qmutex.lock();
    if(m_CommandQueue.size() != 0)
    {
        cmd = m_CommandQueue.front();
        m_CommandQueue.pop();
    }
    else
    {
        cmd.CommandType = Command::Type::None;
    }
    m_qmutex.unlock();

    return cmd;
}

void TcpCommander::SendResponce(uint64_t responce)
{
    if(m_client_sock_desc <= 0)
        return;

    std::string str_resp = std::to_string(responce);
    send(m_client_sock_desc, str_resp.c_str(), str_resp.size(), 0);
}

bool TcpCommander::ParseAndPushCommand(char * cmd)
{
    int cmd_num, param = 0;
    if(int nargs = sscanf(cmd, "%d, %d", &cmd_num, &param); nargs != 2)
    {
        return false;
    }
    if(cmd_num >= static_cast<int>(Command::Type::NofCommands))
    {
        return false;
    }

    Command command;
    command.CommandType = static_cast<Command::Type>(cmd_num);
    command.Parameter = static_cast<uint64_t>(param);

    m_qmutex.lock();
    m_CommandQueue.push(command);
    m_qmutex.unlock();

    return true;
}
