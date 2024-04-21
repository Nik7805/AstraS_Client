#pragma once

#include <queue>
#include "camera_provider.hpp"


class DebugCommander: public CameraProvider::IStreamCommander
{
    public:
    ~DebugCommander();
    virtual Command GetNextCommand() override;
    virtual void SendResponce(uint64_t responce) override;
    void AddCommand(CameraProvider::IStreamCommander::Command::Type cmd, uint64_t param);

    private:
    std::queue<CameraProvider::IStreamCommander::Command> m_CommandQueue;
};