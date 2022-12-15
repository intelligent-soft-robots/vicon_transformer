#include <vicon_transformer/ostream.hpp>


namespace vicon_transformer{

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
{
    std::copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(os, " "));
    return os;
}

std::ostream& operator<<(std::ostream& os, const VDSSDK::Output_GetVersion& x)
{
    os << x.Major << "." << x.Minor << "." << x.Point << "." << x.Revision;
    return os;
}

std::ostream& operator<<(std::ostream& os, const VDSSDK::Direction::Enum& dir)
{
    switch (dir)
    {
        case VDSSDK::Direction::Forward:
            os << "Forward";
            break;
        case VDSSDK::Direction::Backward:
            os << "Backward";
            break;
        case VDSSDK::Direction::Left:
            os << "Left";
            break;
        case VDSSDK::Direction::Right:
            os << "Right";
            break;
        case VDSSDK::Direction::Up:
            os << "Up";
            break;
        case VDSSDK::Direction::Down:
            os << "Down";
            break;
        default:
            os << "Unknown";
            break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const VDSSDK::Result::Enum& result)
{
    switch (result)
    {
        case VDSSDK::Result::Unknown:
            os << "Unknown";
            break;
        case VDSSDK::Result::NotImplemented:
            os << "NotImplemented";
            break;
        case VDSSDK::Result::Success:
            os << "Success";
            break;
        case VDSSDK::Result::InvalidHostName:
            os << "InvalidHostName";
            break;
        case VDSSDK::Result::InvalidMulticastIP:
            os << "InvalidMulticastIP";
            break;
        case VDSSDK::Result::ClientAlreadyConnected:
            os << "ClientAlreadyConnected";
            break;
        case VDSSDK::Result::ClientConnectionFailed:
            os << "ClientConnectionFailed";
            break;
        case VDSSDK::Result::ServerAlreadyTransmittingMulticast:
            os << "ServerAlreadyTransmittingMulticast";
            break;
        case VDSSDK::Result::ServerNotTransmittingMulticast:
            os << "ServerNotTransmittingMulticast";
            break;
        case VDSSDK::Result::NotConnected:
            os << "NotConnected";
            break;
        case VDSSDK::Result::NoFrame:
            os << "NoFrame";
            break;
        case VDSSDK::Result::InvalidIndex:
            os << "InvalidIndex";
            break;
        case VDSSDK::Result::InvalidCameraName:
            os << "InvalidCameraName";
            break;
        case VDSSDK::Result::InvalidSubjectName:
            os << "InvalidSubjectName";
            break;
        case VDSSDK::Result::InvalidSegmentName:
            os << "InvalidSegmentName";
            break;
        case VDSSDK::Result::InvalidMarkerName:
            os << "InvalidMarkerName";
            break;
        case VDSSDK::Result::InvalidDeviceName:
            os << "InvalidDeviceName";
            break;
        case VDSSDK::Result::InvalidDeviceOutputName:
            os << "InvalidDeviceOutputName";
            break;
        case VDSSDK::Result::InvalidLatencySampleName:
            os << "InvalidLatencySampleName";
            break;
        case VDSSDK::Result::CoLinearAxes:
            os << "CoLinearAxes";
            break;
        case VDSSDK::Result::LeftHandedAxes:
            os << "LeftHandedAxes";
            break;
        case VDSSDK::Result::HapticAlreadySet:
            os << "HapticAlreadySet";
            break;
        case VDSSDK::Result::EarlyDataRequested:
            os << "EarlyDataRequested";
            break;
        case VDSSDK::Result::LateDataRequested:
            os << "LateDataRequested";
            break;
        case VDSSDK::Result::InvalidOperation:
            os << "InvalidOperation";
            break;
        case VDSSDK::Result::NotSupported:
            os << "NotSupported";
            break;
        case VDSSDK::Result::ConfigurationFailed:
            os << "ConfigurationFailed";
            break;
        case VDSSDK::Result::NotPresent:
            os << "NotPresent";
            break;
    }
    return os;
}

}
