#pragma once
#include <stdint.h>
#include <functional>
#include "libobsensor/ObSensor.h"
#include "libobsensor/ObSensor.hpp"

class CameraProvider
{
public:
    enum class StreamFormats
    {
        RGB,
        BGR,
        GRAY,
        IR,
        DEPTH
    };

    class IStreamCommander
    {
        public:
        class Command
        {
            public:
            enum class Type
            {
                None,
                SetStreamFormat,
                SetFrameWidth,
                SetFrameHeight,
                GetStreamFormat,
                GetFrameWidth,
                GetFrameHeight,
                StartStream,
            };
            Type CommandType = Type::None;
            uint64_t Parameter = 0;
        };

        virtual ~IStreamCommander(){};
        virtual Command GetNextCommand() = 0;
        virtual void SendResponce(uint64_t responce) = 0;
    };
    class IStreamPipeline
    {
        public:
        virtual ~IStreamPipeline(){};
        virtual void SendFrame(OBFormat format, uint16_t width, uint16_t height, void* data) = 0;
    };

    CameraProvider(IStreamCommander* sc, IStreamPipeline* sp);
    void Process();

private:
    bool                m_StreamIsRunning   = false;
    bool                m_ContinueStream    = false;
    int                 m_FrameWidth        = 640;
    int                 m_FrameHeigth       = 480;
    IStreamCommander*   m_Commander         = nullptr;
    IStreamPipeline*    m_StreamPipeline    = nullptr;
    OBSensorType        m_SensorType        = OBSensorType::OB_SENSOR_COLOR;
    OBFormat            m_Format            = OBFormat::OB_FORMAT_GRAY;
    ob::Pipeline        m_OBPipeline;
    std::shared_ptr<ob::VideoStreamProfile> vs_profile = nullptr;

    void CameraThread();
    void CameraLoopProcess();
    void PrintProfileSettings(std::shared_ptr<ob::VideoStreamProfile> profile);
    void ExecuteCommand(const CameraProvider::IStreamCommander::Command& command);
    void ExecuteStreamFormat(uint64_t param);
    void ExecuteFrameWidth(uint64_t param);
    void ExecuteFrameHeight(uint64_t param);
    void ExecuteStreamControl(uint64_t param);
};