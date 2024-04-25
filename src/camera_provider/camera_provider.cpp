#include <thread>
#include <chrono>
#include "camera_provider.hpp"

using namespace std::chrono_literals;


CameraProvider::CameraProvider(IStreamCommander *sc, IStreamPipeline *sp)
{
    m_Commander = sc;
    m_StreamPipeline = sp;
}

void CameraProvider::Process()
{
    while(1)
    {
        std::this_thread::sleep_for(100ms);

        ExecuteCommand(m_Commander->GetNextCommand());
    }

}

void CameraProvider::CameraThread()
{
    std::cout << "Starting camera thread\n";
    auto stream_profiles = m_OBPipeline.getStreamProfileList(m_SensorType);

    try 
    {
        vs_profile = stream_profiles->getVideoStreamProfile(m_FrameWidth, m_FrameHeigth, m_Format, 30);
    }
    catch(ob::Error &e) 
    {
        vs_profile = std::const_pointer_cast<ob::StreamProfile>(stream_profiles->getProfile(OB_PROFILE_DEFAULT))->as<ob::VideoStreamProfile>();
    }

    PrintProfileSettings(vs_profile);

    std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
    config->enableStream(vs_profile);
    m_OBPipeline.start(config);

    CameraLoopProcess();

    m_OBPipeline.stop();
    config->disableAllStream();

    std::cout << "Camera thread is finished\n";
}

void CameraProvider::CameraLoopProcess()
{
    uint64_t frame_counter = 0;
    m_StreamIsRunning = true;

    try
    {
        while(m_ContinueStream)
        {
            auto frameSet = m_OBPipeline.waitForFrames(100);
            if(frameSet == nullptr) 
            {
                continue;
            }

            if(++frame_counter % 2 != 0)
                continue;
            
            std::shared_ptr<ob::VideoFrame> frame = nullptr;
            switch(m_SensorType)
            {
                case OBSensorType::OB_SENSOR_COLOR:
                    frame = frameSet->colorFrame();
                    break;
                case OBSensorType::OB_SENSOR_DEPTH:
                    frame = frameSet->depthFrame();
                    break;
                case OBSensorType::OB_SENSOR_IR:
                    frame = frameSet->irFrame();
                    break;
                default:
                    std::cerr << "Unsupported sensor type.\n";
                    m_StreamIsRunning = false;
                    return;
            }

            m_StreamPipeline->SendFrame(frame->format(), 
                                        frame->width(), 
                                        frame->height(), 
                                        frame->data());

        }
    }
    catch(ob::Error &e) 
    {
        std::cerr << "function:" << e.getName() << "\nargs:" << e.getArgs() << "\nmessage:" << e.getMessage() << "\ntype:" << e.getExceptionType() << std::endl;
    }
    m_StreamIsRunning = false;
}

void CameraProvider::PrintProfileSettings(std::shared_ptr<ob::VideoStreamProfile> profile)
{
    char c_str[1024];
    int format  = static_cast<int>(profile->format());
    int type    = static_cast<int>(profile->type());
    int fps     = static_cast<int>(profile->fps());
    int height  = static_cast<int>(profile->height());
    int width   = static_cast<int>(profile->width());
    sprintf(c_str, "Profile settings:\n Format: %i\n Type: %i\n FPS: %i\n Height: %i\n Width: %i\n",format, type, fps, height, width);
    std::cout << c_str;
}

void CameraProvider::ExecuteCommand(const CameraProvider::IStreamCommander::Command& command)
{
    switch (command.CommandType)
    {
        case CameraProvider::IStreamCommander::Command::Type::SetStreamFormat:
            this->ExecuteStreamFormat(command.Parameter);
            std::cout << "SetStreamFormat: " << command.Parameter << "\n";
            break;

        case CameraProvider::IStreamCommander::Command::Type::SetFrameWidth:
            this->ExecuteFrameWidth(command.Parameter);
            std::cout << "SetFrameWidth: " << command.Parameter << "\n";
            break;

        case CameraProvider::IStreamCommander::Command::Type::SetFrameHeight:
            this->ExecuteFrameHeight(command.Parameter);
            std::cout << "SetFrameHeight: " << command.Parameter << "\n";
            break;

        case CameraProvider::IStreamCommander::Command::Type::GetStreamFormat:
            if(vs_profile != nullptr)
            {
                m_Commander->SendResponce(static_cast<uint64_t>(vs_profile->format()));
            }
            else
            {
                m_Commander->SendResponce(static_cast<uint64_t>(OB_FORMAT_UNKNOWN));
            }
            std::cout << "GetStreamFormat\n";
            break;

        case CameraProvider::IStreamCommander::Command::Type::GetFrameWidth:
            if(vs_profile != nullptr)
            {
                m_Commander->SendResponce(static_cast<uint64_t>(vs_profile->width()));
            }
            else
            {
                m_Commander->SendResponce(0);
            }
            std::cout << "GetFrameWidth\n";
            break;

        case CameraProvider::IStreamCommander::Command::Type::GetFrameHeight:
            if(vs_profile != nullptr)
            {
                m_Commander->SendResponce(static_cast<uint64_t>(vs_profile->height()));
            }
            else
            {
                m_Commander->SendResponce(0);
            }
            std::cout << "GetFrameHeight\n";
            break;

        case CameraProvider::IStreamCommander::Command::Type::StartStream:
            this->ExecuteStreamControl(command.Parameter);
            std::cout << "StartStream: " << command.Parameter << "\n";
            break;
        
        default: break;
    }
}

void CameraProvider::ExecuteStreamFormat(uint64_t param)
{
    StreamFormats format = static_cast<StreamFormats>(param);
    uint64_t responce = 0;

    switch (format)
    {
        case StreamFormats::RGB:
            m_Format = OBFormat::OB_FORMAT_RGB;
            m_SensorType = OBSensorType::OB_SENSOR_COLOR;
            break;
        case StreamFormats::BGR:
            m_Format = OBFormat::OB_FORMAT_BGR;
            m_SensorType = OBSensorType::OB_SENSOR_COLOR;
            break;
        case StreamFormats::GRAY:
            m_Format = OBFormat::OB_FORMAT_UYVY;
            m_SensorType = OBSensorType::OB_SENSOR_COLOR;
            break;
        case StreamFormats::IR:
            m_Format = OBFormat::OB_FORMAT_Y16;
            m_SensorType = OBSensorType::OB_SENSOR_IR;
            break;
        case StreamFormats::DEPTH:
            m_Format = OBFormat::OB_FORMAT_Y16;
            m_SensorType = OBSensorType::OB_SENSOR_DEPTH;
            break;
        default:
            responce = 1;
            break;
    }

    m_Commander->SendResponce(responce);
}

void CameraProvider::ExecuteFrameWidth(uint64_t param)
{
    m_FrameWidth = static_cast<int>(param);
    m_Commander->SendResponce(0);
}

void CameraProvider::ExecuteFrameHeight(uint64_t param)
{
    m_FrameHeigth = static_cast<int>(param);
    m_Commander->SendResponce(0);
}

void CameraProvider::ExecuteStreamControl(uint64_t param)
{
    bool bParam = param > 0;
    uint64_t responce = 0;

    if(bParam == m_StreamIsRunning)
    {
        m_Commander->SendResponce(1);
        return;
    }

    if(bParam)
    {
        m_ContinueStream = true;
        std::thread t([this]{this->CameraThread();});
        t.detach();
    }
    else
    {
        m_ContinueStream = false;
    }
    std::this_thread::sleep_for(200ms);

    responce = (bParam == m_StreamIsRunning) ? 0 : 2;

    m_Commander->SendResponce(responce);
}
