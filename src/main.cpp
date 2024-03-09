#include <iostream>

#include <errno.h>
#include <thread>
#include <chrono>
#include "libobsensor/ObSensor.hpp"
#include "libobsensor/hpp/Error.hpp"
#include "libobsensor/hpp/Pipeline.hpp"
#include "udpsender.hpp"

using namespace std::chrono_literals;

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

static UDPSender sender("192.168.0.13", 60000);

int CameraConfig();
int CameraProcess();
int SendFrame(VideoFrameHeader_t& header, void* data);

int main(int argc, char const *argv[])
{
    std::cout << "Camera app\r\n";

    if(int result = CameraConfig(); result < 0)
    {
        std::cout << "Camera config error: " << result << "\n";
        return result;
    }

    CameraProcess();

    std::cout << "End of app\r\n";
}

int CameraConfig()
{
    ob::Context context;
    auto deviceList = context.queryDeviceList();

    if(deviceList->deviceCount() < 1)
    {
        return -1;
    }

    auto device = deviceList->getDevice(0);
    
    try
    {
        device->setBoolProperty(OB_PROP_DEPTH_HOLEFILTER_BOOL, true);
    }
    catch(...)
    {
        return -2;
    }
    
    return 0;
}

int CameraProcess()
{
    try 
    {
        // Create a pipeline with default device
        ob::Pipeline pipe;

        // Get all stream profiles of the depth camera, including stream resolution, frame rate, and frame format
        auto irProfiles = pipe.getStreamProfileList(OB_SENSOR_COLOR);
        auto dptProfiles = pipe.getStreamProfileList(OB_SENSOR_DEPTH);

        std::shared_ptr<ob::VideoStreamProfile> irProfile = nullptr;
        std::shared_ptr<ob::VideoStreamProfile> dptProfile = nullptr;
        try 
        {
            // Find the corresponding profile according to the specified format, first look for the y16 format
            irProfile = irProfiles->getVideoStreamProfile(640, OB_HEIGHT_ANY, OB_FORMAT_Y16, 30);
            std::cout << "Applyed custom profile.\n";
        }
        catch(ob::Error &e) 
        {
            // If the specified format is not found, search for the default profile to open the stream
            irProfile = std::const_pointer_cast<ob::StreamProfile>(irProfiles->getProfile(OB_PROFILE_DEFAULT))->as<ob::VideoStreamProfile>();
            std::cout << "Applyed default profile.\n";
        }

        try 
        {
            // Find the corresponding profile according to the specified format, first look for the y16 format
            dptProfile = dptProfiles->getVideoStreamProfile(640, OB_HEIGHT_ANY, OB_FORMAT_Y16, 30);
        }
        catch(ob::Error &e) 
        {
            // If the specified format is not found, search for the default profile to open the stream
            dptProfile = std::const_pointer_cast<ob::StreamProfile>(dptProfiles->getProfile(OB_PROFILE_DEFAULT))->as<ob::VideoStreamProfile>();
        }

        // By creating config to configure which streams to enable or disable for the pipeline, here the depth stream will be enabled
        std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
        config->enableStream(irProfile);
        // config->enableStream(dptProfile);

        // Start the pipeline with config
        pipe.start(config);

        VideoFrameHeader_t header;
        header.depth = 2;

        uint32_t frame_counter = 0;

        while(1) 
        {
            auto frameSet = pipe.waitForFrames(100);
            if(frameSet == nullptr) 
            {
                continue;
            }

            if(++frame_counter % 2 != 0)
                continue;
            
            auto depthFrame = frameSet->colorFrame();
            
            header.width = static_cast<uint16_t>(depthFrame->width());
            header.height = static_cast<uint16_t>(depthFrame->height());
            
            int r = SendFrame(header, depthFrame->data());

            if(!(r > 0))
            {
                std::cerr << "Sending error: " << strerror(errno) << "\n";
            }
            
        }

        // Stop the pipeline
        pipe.stop();

        return 0;
    }
    catch(ob::Error &e) 
    {
        std::cerr << "function:" << e.getName() << "\nargs:" << e.getArgs() << "\nmessage:" << e.getMessage() << "\ntype:" << e.getExceptionType() << std::endl;
        return(EXIT_FAILURE);
    }
}

int SendFrame(VideoFrameHeader_t &header, void *data)
{
    size_t iBytes = static_cast<size_t>(header.depth) *
                    static_cast<size_t>(header.height) *
                    static_cast<size_t>(header.width);
    int iResult = 0;
    
    iResult = sender.Send(static_cast<void*>(&header), sizeof(VideoFrameHeader_t));
    return (iResult < 0) ? iResult : sender.Send(data, iBytes);
}
