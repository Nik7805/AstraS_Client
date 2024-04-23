#include <iostream>

#include <errno.h>
#include <thread>
#include <chrono>
#include "libobsensor/ObSensor.hpp"
#include "libobsensor/hpp/Error.hpp"
#include "libobsensor/hpp/Pipeline.hpp"
#include "camera_provider.hpp"
#include "udp_pipeline.hpp"
#include "debug_commander.hpp"
#include "tcp_commander.hpp"

using namespace std::chrono_literals;

int main(int argc, char const *argv[])
{
    std::cout << "Camera app\r\n";
    auto pSC = new TcpCommander(6000);
    // pSC->AddCommand(CameraProvider::IStreamCommander::Command::Type::SetStreamFormat, (uint64_t)CameraProvider::StreamFormats::GRAY);
    // pSC->AddCommand(CameraProvider::IStreamCommander::Command::Type::StartStream, 1);
    CameraProvider::IStreamPipeline*  pSP = new UDPipeline("192.168.0.13", 60000);
    CameraProvider camera_provider(pSC, pSP);
    std::thread tcp_thread([pSC]{pSC->Process();});

    tcp_thread.detach();

    camera_provider.Process();

    std::cout << "End of app\r\n";
}
