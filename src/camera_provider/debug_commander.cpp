#include "debug_commander.hpp"

DebugCommander::~DebugCommander()
{
}

CameraProvider::IStreamCommander::Command DebugCommander::GetNextCommand()
{
    Command cmd;
    if(m_CommandQueue.size() != 0)
    {
        cmd = m_CommandQueue.front();
        m_CommandQueue.pop();
    }
    else
    {
        cmd.CommandType = Command::Type::None;
    }

    return cmd;
}

void DebugCommander::SendResponce(uint64_t responce)
{
    std::cout << "Responce: " << responce << "\n";
}

void DebugCommander::AddCommand(CameraProvider::IStreamCommander::Command::Type cmd, uint64_t param)
{
    Command command;
    command.CommandType = cmd;
    command.Parameter = param;
    m_CommandQueue.push(command);
}
